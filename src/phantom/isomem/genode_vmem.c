#ifdef PHANTOM_VMEM_STUB

/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2010 Dmitry Zavalishin, dz@dz.ru
 *
 * Intel ia32 page table support.
 *
 * CONF_DUAL_PAGEMAP:
 *
 * We keep two page directories. One covers all memory, other excludes
 * persistent (paged) memory range. Two pagemaps are switched to enable/
 * disable persistent memory access for current thread.
 *
 * TODO rename all funcs to arch_ prefix
 *
 **/

#define DEBUG_MSG_PREFIX "paging"
#include <debug_ext.h>
#define debug_level_flow  0
#define debug_level_error 10
#define debug_level_info  10

#include "genode_misc.h"

#include <hal.h>
#include <ia32/phantom_pmap.h>
#include <ia32/proc_reg.h>
#include <kernel/config.h>
#include <kernel/ia32/cpu.h>
#include <kernel/page.h>
#include <machdep.h>
#include <ph_malloc.h>
#include <phantom_libc.h>

static int paging_inited = 0;

/// Differentiate between page tables in paged and non-page areas
int ptep_is_paged_area(int npde)
{
	_stub_print();
	return 1;
}

void phantom_paging_init(void)
{
	_stub_print();
	phantom_paging_start();
}

void phantom_paging_start(void)
{
	_stub_print();
}

#if CONF_DUAL_PAGEMAP
/**
 * \brief Enable or disable paged mem access. Must be called just
 * from t_set_paged_mem() in threads lib, as it saves cr3 state for
 * thread switch.
 *
 * \return cr3 value for threads lib to save.
 *
 **/
int32_t arch_switch_pdir(bool paged_mem_enable)
{
	_stub_print();
	return 0;
}

int32_t arch_get_pdir(bool paged_mem_enable)
{
	_stub_print();
	return 0;
}

int arch_is_object_land_access_enabled()  //< check if current thread attempts to access
										  //object space having access disabled
{
	_stub_print();
	return 0;
}

#endif

/*
 *
 * Virtual memory control
 *
 */

// Used to map and unmap pages
void hal_page_control_etc(physaddr_t p,
						  void *page_start_addr,
						  page_mapped_t mapped,
						  page_access_t access,
						  u_int32_t flags)
{
	_stub_print();
}

// TODO : Delete if unused

// void phantom_map_page(linaddr_t la, pt_entry_t mapping )
// {
//     _stub_print();
// }

// void phantom_unmap_page(linaddr_t la )
// {
//     _stub_print();
// }

// int phantom_is_page_accessed(linaddr_t la )
// {
//     _stub_print();
//     return 0;
// }

// int phantom_is_page_modified(linaddr_t la )
// {
//     _stub_print();
//     return 0;
// }

// void
// phantom_dump_pdir()
// {
//     _stub_print();
// }

// void arch_cpu_invalidate_TLB_range(addr_t start, addr_t end)
// {
//     _stub_print();
// }

// void arch_cpu_invalidate_TLB_list(addr_t pages[], int num_pages)
// {
//     _stub_print();
// }

#endif