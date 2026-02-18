#define DEBUG_MSG_PREFIX "GenodeDiskIO"
#include <debug_ext.h>
#define debug_level_flow  10
#define debug_level_error 10
#define debug_level_info  10

// Required to access vmem functions
#include <hal.h>

// // #include <stdio.h>
#include "genode_disk.h"
#include "genode_misc.h"

#include <arch/arch-page.h>
#include <ph_io.h>
#include <ph_malloc.h>
#include <ph_string.h>
#include <stddef.h>
#include <stdlib.h>

static int seq_number = 0;
static genode_disk_dev_t vdev;
static char vdev_name[32] = "GenodeDisk0";

phantom_device_t *driver_genode_disk_probe()
{

	vdev.name = vdev_name;
	vdev.block_size = 512;
	vdev.block_count = 2095071;

	phantom_device_t *dev = (phantom_device_t *)ph_malloc(sizeof(phantom_device_t));
	dev->name = "Genode disk";
	dev->seq_number = seq_number++;
	dev->drv_private = &vdev;

	return dev;
}

static errno_t genode_disk_dequeue(struct phantom_disk_partition *p, pager_io_request *rq)
{
	_stub_print();
	return 1;  // always return failure
}
static errno_t genode_disk_raise(struct phantom_disk_partition *p, pager_io_request *rq)
{
	// _stub_print();
	return 0;
}
static errno_t genode_disk_trim(struct phantom_disk_partition *p, pager_io_request *rq)
{
	_stub_print();
	return 0;
}

static phantom_disk_partition_t *phantom_create_genode_partition_struct(
	long size, genode_disk_dev_t *vd)
{
	phantom_disk_partition_t *ret = phantom_create_partition_struct(0, 0, size);

	ret->asyncIo = driver_genode_disk_asyncIO;
	ret->flags |= PART_FLAG_IS_WHOLE_DISK;
	ret->specific = vd;

	// important since it is used to break request in the parts
	// XXX: Seems that Phantom has 512 bytes size hardcoded for the partition
	//      (though, it is a default settings)
	if (vd->block_size != 512) {
		SHOW_ERROR(0,
				   "Block size of the partition is not 512 bytes! Got size=%d",
				   vd->block_size);
		return 0;
	}

	ret->block_size = vd->block_size;

	ret->dequeue = genode_disk_dequeue;
	ret->raise = genode_disk_raise;
	ret->fence = driver_genode_disk_fence;
	ret->trim = genode_disk_trim;

	// ph_strlcpy( ret->name, "virtio", PARTITION_NAME_LEN );
	ph_strlcpy(ret->name, vd->name, PARTITION_NAME_LEN);

	return ret;
}

// Initialization and registration of a disk inside the system
void driver_genode_disk_init()
{

	// Creating and initializing partition

	phantom_disk_partition_t *part =
		phantom_create_genode_partition_struct(vdev.block_count, &vdev);

	if (part == 0) {
		SHOW_ERROR0(0, "Failed to create whole disk partition");
		return;
	}

	errno_t err = phantom_register_disk_drive(part);

	if (err) {
		SHOW_ERROR0(0, "Failed to register Genode disk!");
		return;
	}
}

// Main function for performing I/O
// int driver_genode_disk_asyncIO(struct phantom_disk_partition *part, pager_io_request
// *rq)
// {

//     // assert(part->specific != 0); // TODO : Causes a compilation error!!!

//     // Temp! Rewrite!
//     // assert(p->base == 0 );

//     int block_size = vdev.block_size;
//     const int sector_size = 512;

//     genode_disk_dev_t *vd = (genode_disk_dev_t *)part->specific;

//     // Getting data from rq
//     int blockNo = rq->blockNo;
//     int nSect = rq->nSect;
//     physaddr_t pa = rq->phys_page;

//     // Calculating size
//     int length_in_bytes = nSect * sector_size;
//     int length_in_pages = length_in_bytes / PAGE_SIZE + ((length_in_bytes % PAGE_SIZE)
//     ? 1 : 0);

//     // XXX : Seems to be not used
//     rq->parts = nSect;

//     // TODO : Rework to be more efficient
//     // TODO : IT SHOULD NOT HAPPEN IN OBJECT SPACE! Use another allocator
//     // TODO : Also, might result in some memory leaks
//     // Need to write to pseudo phys page. It means that we need to map it first
//     void *temp_mapping = NULL;

//     SHOW_FLOW(3, "genode_asyncIO: [%c] pa=0x%x blockNo=0x%x nSect=%d size=0x%x len=%d
//     len_in_pages=%d",
//         rq->flag_pagein ? 'r' : 'w',
//         pa,
//         rq->blockNo,
//         rq->nSect,
//         block_size,
//         length_in_bytes,
//         length_in_pages);

//     if (length_in_bytes % block_size != 0){
//         SHOW_ERROR(0, "nSect is not block aligned! (blockNo=%d nSect=%d)", blockNo,
//         nSect);
//     }

//     // Getting number of blocks to read by sectors
//     // const int io_buf_len = nSect * sector_size + (block_size - ((nSect *
//     sector_size) % block_size));

//     hal_alloc_vaddress(&temp_mapping, length_in_pages );
//     hal_pages_control(pa, temp_mapping, length_in_pages, page_map, rq->flag_pagein ?
//     page_rw : page_ro);

//     // Now we can perform IO
//     if (rq->flag_pageout)
//         driver_genode_disk_write(vd, temp_mapping, rq->blockNo, length_in_bytes);
//     else
//         driver_genode_disk_read(vd, temp_mapping, rq->blockNo, length_in_bytes);

//     // Now we can remove temporary mapping and dealloc address
//     hal_pages_control(0, temp_mapping, length_in_pages, page_unmap, page_rw);
//     hal_free_vaddress(temp_mapping, length_in_pages);

//     // XXX : pager_io_request requires some finishing operations
//     //       But not sure what is required. One of the ideas is to use
//     //       pager_io_request_done( rq );
//     //       But it is almost not used anywhere
//     // XXX : It is very controversial thing though. What will happen if we would like
//     to check what operation it was? rq->flag_pageout = 0; rq->flag_pagein  = 0;

//     // TODO : error handling

//     return 0;
// }