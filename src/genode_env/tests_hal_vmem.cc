#include <base/log.h>
#include <base/registry.h>
#include "phantom_env.h"

using namespace Phantom;

extern "C"
{

#include <hal.h>
#include "tests_hal.h"

    bool test_hal_vmem_alloc()
    {

        // Idea: - alloc bunch of addresses of different sizes
        //       - check their addresses
        //       - dealloc them

        const int addr_cnt = 20;
        void *addrs[addr_cnt];

        size_t initialy_available = main_obj->_vmem_adapter._obj_space_allocator.avail();
        log("Initially: ", initialy_available);

        for (int i = 0; i < addr_cnt; i++)
        {
            // errno_t err = 0;

            log("Allocating addr i=", i + 1);

            if (hal_alloc_vaddress(&addrs[i], i + 1))
            {
                return false;
            }

            log("allocated at ", addrs[i]);
            // log("is in obj space: ", hal_addr_is_in_object_vmem(addrs[i]));

            // if (!hal_addr_is_in_object_vmem(addrs[i]))
            // {
            //     return false;
            // }
        }

        for (int i = 0; i < addr_cnt; i++)
        {
            log("Freeing addr", addrs[i]);
            hal_free_vaddress(addrs[i], i + 1);
        }

        // Checking if we deallocated all pages

        log("Currently: ", main_obj->_vmem_adapter._obj_space_allocator.avail());

        if (initialy_available != main_obj->_vmem_adapter._obj_space_allocator.avail())
        {
            return false;
        }

        // Let's try to allocate 10 pages and free them in 3 steps

        void *temp_addr = nullptr;

        log("Allocating addr");

        if (hal_alloc_vaddress(&temp_addr, 10))
        {
            return false;
        }

        log("free 5");
        hal_free_vaddress((char *)temp_addr, 5);
        log("free 2");
        hal_free_vaddress((char *)temp_addr + (5 * ARCH_PAGE_SIZE), 2);
        log("free 3");
        hal_free_vaddress((char *)temp_addr + (7 * ARCH_PAGE_SIZE), 3);

        log("Currrntly: ", main_obj->_vmem_adapter._obj_space_allocator.avail());

        if (initialy_available != main_obj->_vmem_adapter._obj_space_allocator.avail())
        {
            return false;
        }

        return true;
    }

    static bool check_pseudo_phys_registry(int size)
    {
        int cnt = 0;

        main_obj->_vmem_adapter._pseudo_phys_pages_registry.for_each(
            [&](Phys_region_handle &rh)
            {
                (void)rh;
                cnt++;
            });

        if (cnt != size)
        {

            Genode::warning("check_pseudo_phys_registry() failed : cnt=", cnt, " expected=", size);
            return false;
        }

        return true;
    }

    bool test_hal_phys_alloc()
    {

        const int addr_cnt = 20;
        physaddr_t addrs[addr_cnt];

        // size_t initialy_consumed = main_obj->_vmem_adapter._pseudo_phys_ds_registry.;
        // log("Initially consumed: ", initialy_consumed);

        if (!check_pseudo_phys_registry(0))
        {
            return false;
        }

        for (int i = 0; i < addr_cnt; i++)
        {
            // errno_t err = 0;

            log("Allocating addr i=", i + 1);

            if (hal_alloc_phys_pages(&addrs[i], i + 1))
            {
                return false;
            }

            log("allocated at ", Hex(addrs[i]));
            // log("is in obj space: ", hal_addr_is_in_object_vmem(addrs[i]));

            // if (!hal_addr_is_in_object_vmem(addrs[i]))
            // {
            //     return false;
            // }
        }

        if (!check_pseudo_phys_registry(addr_cnt))
        {
            return false;
        }

        for (int i = 0; i < addr_cnt; i++)
        {
            log("Freeing addr", addrs[i]);
            hal_free_phys_pages(addrs[i], i + 1);
        }

        if (!check_pseudo_phys_registry(0))
        {
            return false;
        }

        log("Allocating addr");

        physaddr_t temp_addr = 0;

        if (hal_alloc_phys_pages(&temp_addr, 10))
        {
            return false;
        }

        if (!check_pseudo_phys_registry(1))
        {
            return false;
        }

        log("free 5");
        hal_free_phys_pages(temp_addr, 5); // should not work

        if (!check_pseudo_phys_registry(1))
        {
            return false;
        }

        hal_free_phys_pages(temp_addr, 10); // should work

        if (!check_pseudo_phys_registry(0))
        {
            return false;
        }

        return true;
    }

    bool test_hal_vmem_mapping()
    {
        void *va = nullptr;
        physaddr_t pa = 0x10000000;

        // mapping the page

        hal_pv_alloc(&pa, &va, ARCH_PAGE_SIZE);
        // hal_alloc_vaddress(&va, 1);
        // hal_alloc_phys_page(&pa);

        // hal_page_control_etc(pa, va, page_map, page_readwrite, 0);

        // Checking the access

        // Checking addresses
        char *addr_to_write = (char *)va;

        for (int i = 0; i < ARCH_PAGE_SIZE; i++)
        {
            char val1 = 'A';
            char val2 = 'B';

            // reading and checking the value so that there is no place to random

            if (*(addr_to_write + i) != val1)
            {
                *(addr_to_write + i) = val1;

                if (*addr_to_write != val1)
                {
                    error("Couldn't write the value on addr_to_write[", i, "]!");
                    return false;
                }
            }
            else
            {
                *(addr_to_write + i) = val2;

                if (*addr_to_write != val2)
                {
                    error("Couldn't write the value on addr_to_write[", i, "]!");
                    return false;
                }
            }
        }

        // Unmapping the page

        // hal_page_control_etc(pa, va, page_unmap, page_readwrite, 0);

        // hal_free_vaddress(va, 1);
        // hal_free_phys_page(pa);

        hal_pv_free(pa, va, ARCH_PAGE_SIZE);

        return true;
    }

    bool test_hal_vmem_remapping()
    {
        // Scenario: map write and unmap several phys pages on a single virtual one, then check

        const int phys_count = 10;

        void *va = nullptr;
        physaddr_t pa_orig = 0x0;

        // Allocating pv to get a free adress and the unmapping
        hal_pv_alloc(&pa_orig, &va, ARCH_PAGE_SIZE);
        hal_page_control(pa_orig, va, page_unmap, page_readwrite);

        physaddr_t pas[phys_count];

        // allocating one virtual page
        // hal_alloc_vaddress(&va, 1);

        // And several physical
        for (int i = 0; i < phys_count; i++)
        {
            hal_alloc_phys_page(&pas[i]);
        }

        // writing phys pages
        char *buf[ARCH_PAGE_SIZE];

        log("Writing phys pages using 1 virtual");

        for (int i = 0; i < phys_count; i++)
        {

            hal_page_control_etc(pas[i], va, page_map, page_readwrite, 0);

            memset(buf, i, ARCH_PAGE_SIZE);

            log("Starting memcpy");

            // *(int *)va = i;

            memcpy(va, buf, ARCH_PAGE_SIZE);

            hal_page_control_etc(pas[i], va, page_unmap, page_readwrite, 0);
            // hal_sleep_msec(1000);
        }

        // verifying their values on separate pages

        for (int i = 0; i < phys_count; i++)
        {

            hal_page_control_etc(pas[i], va, page_map, page_readwrite, 0);

            memset(buf, i, ARCH_PAGE_SIZE);
            if (memcmp(va, buf, ARCH_PAGE_SIZE))
            {
                error("Failed remapping test on ", i, "th case. memcmp()=",
                      memcmp(va, buf, ARCH_PAGE_SIZE), " va[0]=", Hex((u_int8_t)(*(char *)va)));

                return false;
            }

            hal_page_control_etc(pas[i], va, page_unmap, page_readwrite, 0);
        }

        // hal_free_vaddress(va, 1);

        for (int i = 0; i < phys_count; i++)
        {
            hal_free_phys_page(pas[i]);
        }

        hal_page_control(pa_orig, va, page_map, page_rw);
        hal_pv_free(pa_orig, va, ARCH_PAGE_SIZE);

        return true;
    }
};
