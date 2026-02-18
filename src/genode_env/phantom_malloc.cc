#include "phantom_env.h"
#include <base/log.h>
#include <base/stdint.h>

extern "C" {
#include <ph_malloc.h>
#include <ph_string.h>
}

extern "C" void *ph_malloc(size_t size) {
  // Storing the size of the allocation in the beginning
  // since _heap.free() requires the size as well

  size_t total_size = sizeof(size_t) + size;

  auto alloc_res = main_obj->_heap.try_alloc(total_size);
  
  if (!alloc_res.ok()) {
    alloc_res.with_error([](Alloc_error err) { error(err); });
    return 0;
  }

  void *original_addr = nullptr;

  alloc_res.with_result(
    [&](Genode::Allocator::Allocation &alloc) {
        original_addr = alloc.ptr;
        alloc.deallocate = false;
      },
      [&](Alloc_error err) { error(err); });

  // just to ensure it is safe
  if (original_addr == nullptr) {
    error("ph_malloc: addr = nullptr!!!");
    return 0;
  }

  // Writing the size
  *((size_t *)original_addr) = size;

  size_t *adjusted_addr = ((size_t *)original_addr) + 1;
  // Genode::log("ph_malloc: addr=", original_addr, "(", adjusted_addr, "),
  // size=", size, " total_size=", total_size);

  return (void *)adjusted_addr;
}

extern "C" void ph_free(void *addr) {
  if (addr == NULL)
    return;
  // addr should be not the beginning of the allocation, but allocation +
  // sizeof(size_t)

  size_t *adjusted_addr = (size_t *)addr;
  void *original_addr = (void *)(adjusted_addr - 1);

  size_t size = *((size_t *)original_addr);

  main_obj->_heap.free(original_addr, size + sizeof(size_t));
}

// ph_calloc: Phantom's calloc. Allocates memory of n_elem * elem_size bytes and
// fills it with zeros
// TODO: fix inability to allocate more memory than size_t
extern "C" void *ph_calloc(size_t n_elem, size_t elem_size) {
  u_int64_t total_size = n_elem * elem_size;
  size_t alloc_size = (size_t)total_size;

  if (alloc_size != total_size) {
    error("ph_calloc: Too large allocation");
    return NULL;
  }

  // let malloc deal with 0 size allocations :)
  void *ptr = ph_malloc(alloc_size);
  if (ptr) {
    unsigned char *cursor = (unsigned char *)ptr;
    if (n_elem > elem_size) { // reduce # of memset calls
      size_t tmp = n_elem;
      n_elem = elem_size;
      elem_size = tmp;
    }

    for (size_t i = 0; i < n_elem; i++) {
      ph_memset(cursor, 0, elem_size);
      cursor += elem_size;
    }
  }

  return ptr;
}

// XXX : Allocates new area each time!
extern "C" void *ph_realloc(void *ptr, size_t size) {
  size_t old_size = *(((size_t *)ptr) - 1);
  size_t min_size = old_size < size ? old_size : size;

  void *new_area = ph_malloc(size);
  ph_memcpy(new_area, ptr, min_size);
  ph_free(ptr);

  return new_area;
}

// Returns the number of bytes currently allocated on heap
// Heap is primarily allocated via ph_malloc()
extern "C" int64_t phantom_heap_bytes_consumed() {
  return main_obj->_heap.consumed();
}
