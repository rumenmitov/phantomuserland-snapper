/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2011 Dmitry Zavalishin, dz@dz.ru
 *
 * (new and unused) disk IO stack, incl. partitioning and fs search.
 *
**/

#define DEBUG_MSG_PREFIX "DiskIO"
#include <debug_ext.h>
#define debug_level_flow 0
#define debug_level_error 10
#define debug_level_info 10

#include <disk.h>
#include <pc/disk_partition.h>
#include <phantom_disk.h>
#include <phantom_assert.h>
#include <ph_malloc.h>
#include <ph_string.h>
#include <phantom_libc.h>
#include <ph_string.h>
#include <kernel/vm.h>
#include <kernel/page.h>
#include <kernel/stats.h>

// really need private
#include <thread_private.h>
#include <threads.h>

#include "fs_map.h"

// ------------------------------------------------------------
// Basic partition processing proxies
// ------------------------------------------------------------

/*
static errno_t checkRange( struct phantom_disk_partition *p, long blockNo, int nBlocks )
{
    if( blockNo < 0 || blockNo > p->size )
        return EINVAL;

    if( blockNo+nBlocks < 0 || blockNo+nBlocks > p->size )
        return EINVAL;

    return 0;
}
*/

//extern errno_t partAsyncIo( struct phantom_disk_partition *p, pager_io_request *rq ); // disk_pool.c
//extern errno_t partTrim( struct phantom_disk_partition *p, pager_io_request *rq ); // disk_pool.c



static errno_t partDequeue( struct phantom_disk_partition *p, pager_io_request *rq )
{
    assert(p->base);

    if( 0 == p->base->dequeue )
        return ENODEV;

    SHOW_FLOW( 11, "part dequeue rq %p", rq );

    return p->base->dequeue( p->base, rq );
}

static errno_t partRaise( struct phantom_disk_partition *p, pager_io_request *rq )
{
    assert(p->base);

    if( 0 == p->base->raise )
        return ENODEV;

    SHOW_FLOW( 11, "part raise rq %p", rq );

    return p->base->raise( p->base, rq );
}

static errno_t partFence( struct phantom_disk_partition *p )
{
    if( (0 == p->base) || (0 == p->base->fence) )
        return ENODEV;

    SHOW_FLOW( 11, "part fence p %p", p );

    return p->base->fence( p->base );
}




// ------------------------------------------------------------
// Upper level disk io functions
// ------------------------------------------------------------


// TODO: this function is only called once in genode port. why and do we need it?
static errno_t startSync( phantom_disk_partition_t *p, void *to, long blockNo, int nBlocks, int isWrite )
{
    assert( p->block_size < ARCH_PAGE_SIZE );
    SHOW_FLOW( 3, "blk %d [%d] (%d)", blockNo, nBlocks, isWrite );

    pager_io_request rq;

    pager_io_request_init( &rq );

    // rq.phys_page = (physaddr_t)phystokv(to); // why? redundant?
    rq.disk_page = blockNo;

    rq.blockNo = blockNo;
    rq.nSect   = nBlocks;

    rq.rc = 0;

    if(isWrite) rq.flag_pageout = 1;
    else rq.flag_pagein = 1;

    STAT_INC_CNT(STAT_CNT_BLOCK_SYNC_IO);
    STAT_INC_CNT( STAT_CNT_DISK_Q_SIZE ); // Will decrement on io done

    void *va;
    hal_pv_alloc( &rq.phys_page, &va, nBlocks * p->block_size );

    errno_t ret = EINVAL;

    if(isWrite) ph_memcpy( va, to, nBlocks * p->block_size );

    int ei = hal_save_cli();
    hal_spin_lock(&(rq.lock));
    rq.flag_sleep = 1; // Don't return until done
    rq.sleep_tid = GET_CURRENT_THREAD()->tid;

    SHOW_FLOW0( 3, "start io" );
    // hal_printf("!!! blockNo : %d\n", rq.blockNo);
    if( (ret = p->asyncIo( p, &rq )) )
    {
        SHOW_FLOW0( 3, "failed asyncIo" );
        rq.flag_sleep = 0;
        hal_spin_unlock(&(rq.lock));
        if( ei ) hal_sti();
        //return ret;
        goto ret;
    }
    SHOW_FLOW0( 3, "About to block" );
    thread_block( THREAD_SLEEP_IO, &(rq.lock) );
    SHOW_FLOW0( 3, "unblock" );
    if( ei ) hal_sti();

    if(!isWrite) ph_memcpy( to, va, nBlocks * p->block_size );
    ret = rq.rc;

    //return partAsyncIo( p, &rq );
    //return p->asyncIo( p, rq );


ret:
    hal_pv_free( rq.phys_page, va, nBlocks * p->block_size );
    return ret;
}

errno_t phantom_sync_read_block( phantom_disk_partition_t *p, void *to, long blockNo, int nBlocks )
{
    int m = ARCH_PAGE_SIZE/p->block_size;
    return startSync( p, to, blockNo*m, nBlocks*m, 0 );
}

errno_t phantom_sync_write_block( phantom_disk_partition_t *p, const void *to, long blockNo, int nBlocks )
{
    int m = ARCH_PAGE_SIZE/p->block_size;
    return startSync( p, (void *)to, blockNo*m, nBlocks*m, 1 );
}

errno_t phantom_sync_read_sector( phantom_disk_partition_t *p, void *to, long sectorNo, int nSectors )
{
    return startSync( p, to, sectorNo, nSectors, 0 );
}

errno_t phantom_sync_write_sector( phantom_disk_partition_t *p, const void *to, long sectorNo, int nSectors )
{
    return startSync( p, (void *)to, sectorNo, nSectors, 1 );
}






//! Convert usual pager request to partition code style request and start it
void disk_enqueue( phantom_disk_partition_t *p, pager_io_request *rq )
{
    int m = ARCH_PAGE_SIZE/p->block_size;
    rq->blockNo = rq->disk_page*m;
    rq->nSect = m;

    //assert( rq->flag_ioerror == 0 );
    assert( rq->rc == 0 );
    assert( rq->flag_pagein != rq->flag_pageout );

    STAT_INC_CNT(STAT_CNT_BLOCK_IO);

    STAT_INC_CNT( STAT_CNT_DISK_Q_SIZE ); // Will decrement on io done

    p->asyncIo( p, rq );
}

errno_t disk_dequeue( struct phantom_disk_partition *p, pager_io_request *rq )
{
    assert(p);
    assert(rq);

    //dump_partition(p);

    if( 0 == p->dequeue )
        return ENODEV;
    errno_t rc =  p->dequeue( p, rq );

    if( 0 == rc )
        STAT_INC_CNT_N( STAT_CNT_DISK_Q_SIZE, -1 );

    return rc;
}


errno_t disk_raise_priority( struct phantom_disk_partition *p, pager_io_request *rq )
{
    assert(p);
    assert(rq);

    if( 0 == p->raise )
        return ENODEV;
    return p->raise( p, rq );
}

errno_t disk_fence( struct phantom_disk_partition *p )
{
    assert(p);

    if( 0 == p->fence )
    {
        SHOW_ERROR( 0, "no fence on part %s", p->name );
        return ENODEV;
    }

    errno_t rc = p->fence( p );

    if( rc )
    {
        SHOW_ERROR( 0, "fence failed on part %s, rc %d", p->name, rc );
        hal_sleep_msec( 10000 ); // At least - some chance...
    }


    return rc;
}


// ------------------------------------------------------------
// Partitions list management
// ------------------------------------------------------------

static phantom_disk_partition_t partitions[MAX_DISK_PARTITIONS];
static int nPartitions = 0;

static void register_partition(phantom_disk_partition_t *p)
{
    if( nPartitions >= MAX_DISK_PARTITIONS )
    {
        ph_printf("Too many partitions, skipping one (%.64s)\n", p->name );
        return;
    }

    ph_printf("DEBUG: REGISTRING A PARTITION!!!\n");

    partitions[nPartitions] = *p;
    //dump_partition(p);
    print_partition(p);
}


phantom_disk_partition_t *phantom_create_partition_struct(phantom_disk_partition_t *base, long shift, long size)
{
    phantom_disk_partition_t *ret = ph_calloc( 1, sizeof(phantom_disk_partition_t) );

    ret->block_size = 512;
    ret->shift = shift;
    ret->size = size;
    ret->flags = 0;

    ret->specific = 0;

    ret->asyncIo = partAsyncIo;
    ret->dequeue = partDequeue;
    ret->raise   = partRaise;
    ret->fence   = partFence;
    ret->trim    = partTrim;

    ret->base = base;
    ret->baseh.h = -1;

    return ret;
}

// ------------------------------------------------------------
// New disk registration entry point
// ------------------------------------------------------------

errno_t phantom_register_disk_drive(phantom_disk_partition_t *p)
{
    assert( p->specific != 0 );
    assert( p->shift == 0 );
    assert( p->size != 0 );
    assert( p->base == 0 );

    // Temp! Rewrite!
    assert( p->block_size == 512 );

    //p->flags |= PART_FLAG_IS_WHOLE_DISK;
    assert( p->flags & PART_FLAG_IS_WHOLE_DISK );

    register_partition( p );
    // todo check sanity - no partitions overlap on each level

    return 0;
}


// ------------------------------------------------------------
// Partitions parsing
// ------------------------------------------------------------


//! Get full name for partition (incl subparts)
static errno_t doPartGetName( phantom_disk_partition_t *p, char *buf, size_t bufsz )
{
    if( p->base )
        partGetName( p->base, buf, bufsz );
    if( ph_strlcat( buf, "/", bufsz ) >= bufsz ) return ENOMEM;
    if( ph_strlcat( buf, p->name, bufsz ) >= bufsz ) return ENOMEM;
    return 0;
}

//! Get full name for partition (incl subparts)
errno_t partGetName( phantom_disk_partition_t *p, char *buf, size_t bufsz )
{
	*buf = 0;
    if( p == 0 )
        if( ph_strlcat( buf, "/", bufsz ) >= bufsz ) return ENOMEM;
    return doPartGetName( p, buf, bufsz );
}

//#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"

void dump_partition(phantom_disk_partition_t *p)
{
    if( p == 0 )
    {
        ph_printf("attempt to dump null Disk Partition\n");
        return;
    }

    char pname[128];
    if( partGetName( p, pname, sizeof(pname) ) )
    {
        ph_printf("Disk Partition ?? (%s)\n", p->label );
        return;
    }

    ph_printf("Disk Partition %s (%s)\n", pname, p->label );

    ph_printf(" - type %d%s\n", p->type, (p->type == PHANTOM_PARTITION_TYPE_ID) ? " (phantom)" : "" );
    ph_printf(" - flags %b\n", p->flags, "\020\1PhantomPartType\2PhantomFS\5Bootable\6Divided\7IsDisk" );
    ph_printf(" - blksz %d, start %ld, size %ld\n", p->block_size, p->shift, p->size );
    ph_printf(" - %s base, %s specific\n", p->base ? "has" : "no", p->specific ? "has" : "no" );

}

void print_partition(phantom_disk_partition_t *p)
{
    if( p == 0 )
    {
        ph_printf("attempt to print null Disk Partition\n");
        return;
    }

    char pname[128];
    if( partGetName( p, pname, sizeof(pname) ) )
    {
        ph_printf("Disk Partition ?? (%s)\n", p->label );
        return;
    }

    long sz = p->block_size; sz *= p->size; sz /= 1024;
    char *un = "Kb";

    if( sz > 1024*10 ) { un = "Mb"; sz /= 1024; }
    if( sz > 1024*10 ) { un = "Gb"; sz /= 1024; }

    ph_printf("Disk Partition %s (%s), %ld %s, flags %b\n", pname, p->label, sz, un, p->flags, "\020\1PhantomPartType\2PhantomFS\5Bootable\6Divided\7IsDisk" );
}


//#pragma GCC diagnostic pop

