#ifdef PHANTOM_PHYSALLOC_STUB

#include "genode_misc.h"

#include <kernel/physalloc.h>
#include <stddef.h>
#include <stdint.h>

void phantom_phys_alloc_init_static(physalloc_t *arena,
									u_int32_t n_alloc_units,
									void *mapbuf)
{
	_stub_print();
}

void phantom_phys_alloc_init(physalloc_t *arena, u_int32_t n_alloc_units)
{
	_stub_print();
}

errno_t phantom_phys_alloc_page(physalloc_t *arena, physalloc_item_t *ret)
{
	_stub_print();
}

void phantom_phys_free_page(physalloc_t *arena, physalloc_item_t free)
{
	_stub_print();
}

errno_t phantom_phys_alloc_region(physalloc_t *arena, physalloc_item_t *ret, size_t len)
{
	_stub_print();
}

void phantom_phys_free_region(physalloc_t *arena, physalloc_item_t start, size_t n_pages)
{
	_stub_print();
}

#endif