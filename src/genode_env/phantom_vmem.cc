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
#define debug_level_flow 0
#define debug_level_error 10
#define debug_level_info 10

#include <stddef.h>
#include <stdint.h>

#include "phantom_env.h"

using namespace Phantom;

extern "C"
{

#include <arch/arch_vmem_util.h>
#include <kernel/page.h>
#include <machdep.h>
#include <hal.h>

    /*
     *
     *  Paging
     *
     */

    static int paging_inited = 0;

    // /// Differentiate between page tables in paged and non-page areas
    // int ptep_is_paged_area(int npde)
    // {
    //     _stub_print();
    //     return 1;
    // }

    void phantom_paging_start(void)
    {
        // _stub_print();
    }

    void phantom_paging_init(void)
    {
        // _stub_print();
        phantom_paging_start();
        paging_inited = 1;
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
    // int32_t arch_switch_pdir(bool paged_mem_enable)
    // {
    //     _stub_print();
    //     return 0;
    // }

    // int32_t arch_get_pdir(bool paged_mem_enable)
    // {
    //     _stub_print();
    //     return 0;
    // }

    // int arch_is_object_land_access_enabled() //< check if current thread attempts to access object space having access disabled
    // {
    //     _stub_print();
    //     return 0;
    // }

#endif

    /*
     *
     * Virtual memory control
     *
     */

    
  vmem_ptr_t hal_get_objspace_start(void) 
  {
    return (void*) Phantom::main_obj->_vmem_adapter.OBJECT_SPACE_START;
  }

    // Used to map and unmap pages
    void hal_page_control_etc(
        physaddr_t p, void *page_start_addr,
        page_mapped_t mapped, page_access_t access,
        u_int32_t flags)
    {
        (void)flags;

        // Check if we are in object space
        // if (!hal_addr_is_in_object_vmem(page_start_addr))
        // {
        //     Genode::error("Trying to map outside obj.space! ", page_start_addr);
        //     return;
        // }

        bool writeable = false;
        if (access == page_readwrite or access == page_rw)
        {
            writeable = true;
        }

        // Phantom::main_obj->_vme
        if (mapped == page_map)
        {
            // Genode::log("Remapping addr, step 1 ", page_start_addr);
            Phantom::main_obj->_vmem_adapter.unmap_page((addr_t)page_start_addr);
            // Genode::log("Remapping addr, step 2 ", page_start_addr);
            Phantom::main_obj->_vmem_adapter.map_page(p, (addr_t)page_start_addr, writeable);
            // Genode::log("Remapped addr ", page_start_addr);
        }
        else if (mapped == page_unmap)
        {
            Phantom::main_obj->_vmem_adapter.unmap_page((addr_t)page_start_addr);
        }
        else
        {
            warning("IO mapping is not supported yet");
        }
    }

    /*
     *
     * HAL functions to allocate/free phys/virtual memory
     *
     */

    errno_t hal_alloc_vaddress(void **result, int n_pages) // alloc address of a page, but not memory
    {
        (void)result;
        (void)n_pages;
        Genode::error("Trying to allocate virtual addr without physical one! Returning error");
        return 1;
    }

    void hal_free_vaddress(void *addr, int num)
    {
        (void)addr;
        (void)num;
        Genode::error("Trying to free only a virtual addr without physical one!");
    }

    void hal_init_physmem_alloc(void)
    {
    }

    void hal_init_physmem_alloc_thread(void)
    {
        // #if USE_RESERVE
        //     hal_start_thread( replentishThread, 0, THREAD_FLAG_KERNEL );
        // #endif
        //     dbg_add_command(&cmd_mem_stat, "mem", "Physical memory stats");
    }

    errno_t hal_alloc_phys_pages(physaddr_t *result, int npages) // alloc and not map
    {

        // Genode::log("Phys alloc consumed ram : ", main_obj->_vmem_adapter._pseudo_phys_heap.consumed());

        try
        {
            if (npages <= 0)
            {
                return 1;
            }

            void *temp_res = nullptr;
            // temp_res = main_obj->_vmem_adapter._pseudo_phys_heap.alloc(ARCH_PAGE_SIZE * npages);
            temp_res = main_obj->_vmem_adapter.alloc_pseudo_phys(npages);

            *result = (physaddr_t)temp_res;

            // Genode::log("Phys alloc: numpages= : ", npages, " addr=", Hex((physaddr_t)temp_res));

            return 0;
        }
        catch (Out_of_caps)
        {
            Genode::log("Failed to allocate ", npages, " phys pages, Out_of_caps");
        }
        catch (Out_of_ram)
        {
            Genode::log("Failed to allocate ", npages, " phys pages, Out_of_ram");
        }
        catch (Service_denied)
        {
            Genode::log("Failed to allocate ", npages, " pages, Denied");
        }

        return 1; // XXX : Not sure if it is ok
    }

    void hal_free_phys_pages(physaddr_t paddr, int npages)
    {
        // main_obj->_vmem_adapter._pseudo_phys_heap.free((void *)paddr, npages);
        main_obj->_vmem_adapter.free_pseudo_phys((void *)paddr, npages);
    }

    errno_t hal_alloc_phys_page(physaddr_t *result)
    {
        return hal_alloc_phys_pages(result, 1);
    }

    void hal_free_phys_page(physaddr_t paddr) // alloc and not map - WILL PANIC if page is mapped!
    {
        hal_free_phys_pages(paddr, 1);
    }


  void
  hal_init_object_vmem(void *start_of_virtual_address_space)
  {
    // INFO This is only necessary for base-linux since, on Linux the
    // dataspaces are emulated and need to be explicitly mapped to the
    // physical RAM.
#ifdef PHANTOM_LINUX

    Genode::retry<Genode::Out_of_ram>([&]() {
      Genode::retry<Genode::Out_of_caps>([&]() {
        // INFO Mapping the range
        // [attribute.offset = 0, OBJECT_SPACE_SIZE] in _ram_ds to
        // [attribute.at = 0, OBJECT_SPACE_SIZE] in _obj_space.
        Region_map::Attr attribute {
          .size = main_obj->_vmem_adapter.OBJECT_SPACE_SIZE,
          .offset = 0,
          .use_at = true,
          .at = 0,
          .executable = false,
          .writeable = true};

        main_obj->_vmem_adapter._obj_space
          .attach(main_obj->_vmem_adapter._ram_ds, attribute)
          .with_result([&](const Genode::Region_map::Range&) {
            Genode::log("virtual address space inited");
          },
            [](const Region_map::Attach_error &e) {
              switch (e) {
              case Region_map::Attach_error::INVALID_DATASPACE:
                Genode::error("couldn't attach region map: invalid ds");
                break;
              case Region_map::Attach_error::REGION_CONFLICT:
                Genode::error("couldn't attach region map: region conflict");
                break;
              case Region_map::Attach_error::OUT_OF_CAPS:
                throw Genode::Out_of_caps();
              case Region_map::Attach_error::OUT_OF_RAM:
                throw Genode::Out_of_ram();
              }
              throw;
            });
      },
        [&]() { main_obj->_vmem_adapter._rm.upgrade_caps(10); }, 16U);
    },
      [&]() { main_obj->_vmem_adapter._rm.upgrade_ram(8 * 1024); });

#endif // PHANTOM_LINUX
  }


    // Required by page fault handler. It is always enabled, so return 1
    int arch_is_object_land_access_enabled()
    {
        return 1;
    }

    void hal_pv_alloc(physaddr_t *pa, void **va, int size_bytes)
    {
        int npages = ((size_bytes - 1) / ARCH_PAGE_SIZE) + 1;

        if (hal_alloc_phys_pages(pa, npages))
            panic("out of physmem");

        *va = (void*)main_obj->_vmem_adapter.map_somewhere((addr_t)*pa, true, npages);
    }

    void hal_pv_free(physaddr_t pa, void *va, int size_bytes)
    {
        if (hal_addr_is_in_object_vmem(va))
        {
            Genode::error("Object space addr in hal_pv_free! Will not free");
            return;
        }

        int npages = ((size_bytes - 1) / ARCH_PAGE_SIZE) + 1;

        main_obj->_vmem_adapter.unmap_page((addr_t)va);
        hal_free_phys_pages(pa, npages);
    }

    void
    memcpy_v2p(physaddr_t to, void *_from, size_t size)
    {
        void *addr;
        char *from = (char*) _from; // avoid void* pointer arithmetics
        // if (hal_alloc_vaddress(&addr, 1))
        //     panic("out of vaddresses");

#if 1 // panics, need test
        if (!PAGE_ALIGNED(to))
        {
            // Process partial page

            physaddr_t page = PREV_PAGE_ALIGN(to);
            int shift = to - page;
            size_t part = ARCH_PAGE_SIZE - shift;

            if (part > size)
                part = size;

            assert(shift < ARCH_PAGE_SIZE);
            assert(shift > 0);

            // hal_page_control(page, addr, page_map, page_rw);
            addr = (void*)main_obj->_vmem_adapter.map_somewhere(page, true, 1);

            ph_memcpy((char*)addr + shift, from, part);

            // hal_page_control(page, addr, page_unmap, page_noaccess);
            main_obj->_vmem_adapter.unmap_page((addr_t)addr);
            addr = nullptr;

            to += part;
            from += part;
            size -= part;
        }
#endif

        if (size > 0)
            assert(PAGE_ALIGNED(to));

        while (size > 0)
        {
            int stepSize = size > ARCH_PAGE_SIZE ? ARCH_PAGE_SIZE : size;
            size -= stepSize;

            // hal_page_control(to, addr, page_map, page_rw);
            addr = (void*)main_obj->_vmem_adapter.map_somewhere((addr_t)to, true, 1);

            ph_memcpy(addr, from, stepSize);
            // Genode::log("V2P: ", Hex((addr_t)addr), " ", Hex((addr_t)from), " ", stepSize);

            // hal_page_control(to, addr, page_unmap, page_noaccess);
            main_obj->_vmem_adapter.unmap_page((addr_t)addr);
            addr = nullptr;

            to += ARCH_PAGE_SIZE;
            from += ARCH_PAGE_SIZE;
        }

        assert(size == 0);

        // hal_free_vaddress(addr, 1);
    }

    void
    memcpy_p2v(void *_to, physaddr_t from, size_t size)
    {
        void *addr;
        char *to = (char*) _to; // avoid void* pointer arithmetics
        // if (hal_alloc_vaddress(&addr, 1))
        //     panic("out of vaddresses");

        assert(PAGE_ALIGNED(from));

        do
        {
            int stepSize = size > ARCH_PAGE_SIZE ? ARCH_PAGE_SIZE : size;
            size -= stepSize;

            // hal_page_control(from, addr, page_map, page_rw);
            addr = (void*)main_obj->_vmem_adapter.map_somewhere((addr_t)from, true, 1);

            ph_memcpy(to, addr, stepSize);

            // hal_page_control(from, addr, page_unmap, page_noaccess);
            main_obj->_vmem_adapter.unmap_page((addr_t)addr);
            addr = nullptr;

            to += ARCH_PAGE_SIZE;
            from += ARCH_PAGE_SIZE;

        } while (size > 0);

        assert(size == 0);

        // hal_free_vaddress(addr, 1);
    }

    void
    hal_copy_page_v2p(physaddr_t to, void *from)
    {
        void *addr;
        // if (hal_alloc_vaddress(&addr, 1))
        //     panic("out of vaddresses");
        // hal_page_control(to, addr, page_map, page_rw);
        addr = (void*)main_obj->_vmem_adapter.map_somewhere((addr_t)to, true, 1);

        ph_memcpy(addr, from, hal_mem_pagesize());

        // hal_page_control(to, addr, page_unmap, page_noaccess);
        main_obj->_vmem_adapter.unmap_page((addr_t)addr);
        addr = nullptr;
        // hal_free_vaddress(addr, 1);
    }

    int genode_register_page_fault_handler(int (*pf_handler)(void *address, int write, int ip, struct trap_state *ts))
    {
        Genode::log("Registering page fault handler (", Hex((addr_t)pf_handler), ")!");
        main_obj->_vmem_adapter.fault_handler.register_fault_handler(pf_handler);
        return 0;
    }
}
