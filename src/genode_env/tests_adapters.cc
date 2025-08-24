#include "tests_adapters.h"
#include "phantom_env.h"

extern "C"
{
#include <disk.h>
#include <init_routines.h>
#include <genode_disk.h>
#include <ph_string.h>
}

using namespace Phantom;

bool Phantom::test_obj_space()
{

    // Reading from mem

    log("Reading from obj.space");

    addr_t read_addr = main_obj->_vmem_adapter.OBJECT_SPACE_START + 10;

    log("  read     mem                         ",
        (sizeof(void *) == 8) ? "                " : "",
        Hex_range<addr_t>(main_obj->_vmem_adapter.OBJECT_SPACE_START, main_obj->_vmem_adapter.OBJECT_SPACE_SIZE), " value=",
        Hex(*(unsigned *)(read_addr)));

    // Writing to mem

    log("Writing to obj.space");
    *((unsigned *)read_addr) = 256;

    log("    wrote    mem   ",
        Hex_range<addr_t>(main_obj->_vmem_adapter.OBJECT_SPACE_START, main_obj->_vmem_adapter.OBJECT_SPACE_SIZE), " with value=",
        Hex(*(unsigned *)read_addr));

    // Reading again

    log("Reading from obj.space");
    log("  read     mem                         ",
        (sizeof(void *) == 8) ? "                " : "",
        Hex_range<addr_t>(main_obj->_vmem_adapter.OBJECT_SPACE_START, main_obj->_vmem_adapter.OBJECT_SPACE_SIZE), " value=",
        Hex(*(unsigned *)(read_addr)));

    return true;
}

bool Phantom::test_block_device_adapter(Phantom::Disk_backend &disk)
{
    char test_word[] = "Hello, World!";
    bool success = false;

    physaddr_t temp_phys_page = 0;
    void *virt_addr = nullptr;

    phantom_disk_partition stub_part = {0};
    stub_part.block_size = 512;

    // Should allocate a whole page even if given only sector size
    hal_pv_alloc(&temp_phys_page, &virt_addr, 512);

    // Write then read

    memset(virt_addr, 0x0, 512);
    memcpy(virt_addr, test_word, strlen(test_word));

    pager_io_request write_req = {0};
    write_req.phys_page = temp_phys_page;
    write_req.blockNo = 10;
    write_req.nSect = 1;
    write_req.flag_pagein = 0;
    write_req.flag_pageout = 1;
    hal_spin_unlock(&write_req.lock);
    hal_spin_lock(&write_req.lock);

    log("Writing to the disk");
    driver_genode_disk_asyncIO(&stub_part, &write_req);

    log("Waiting on spinlock till finished");
    hal_spin_lock(&write_req.lock);

    log("Competed write (", !write_req.flag_pagein & !write_req.flag_pageout, ")");

    // Allocating one more page to read
    physaddr_t temp_phys_page_read = 0;
    void *virt_addr_read = nullptr;

    hal_pv_alloc(&temp_phys_page_read, &virt_addr_read, 512);
    memset(virt_addr_read, 0x0, 512);

    // Checking if they are not equal

    if (memcmp(virt_addr, virt_addr_read, 512) == 0)
    {
        error("Error: same content of both phys pages!");
        hal_pv_free(temp_phys_page, virt_addr, 512);
        hal_pv_free(temp_phys_page_read, virt_addr_read, 512);
        return false;
    }

    // Reading the same address

    pager_io_request read_req = {0};
    read_req.phys_page = temp_phys_page_read;
    read_req.blockNo = 10;
    read_req.nSect = 1;
    read_req.flag_pagein = 1;
    read_req.flag_pageout = 0;
    hal_spin_unlock(&read_req.lock);
    hal_spin_lock(&read_req.lock);

    log("Reading from the disk");
    driver_genode_disk_asyncIO(&stub_part, &read_req);

    log("Waiting on spinlock till finished");
    hal_spin_lock(&read_req.lock);

    log("Competed read (", !read_req.flag_pagein & !read_req.flag_pageout, ")");

    log("Comparing results");
    bool ok = ph_strcmp((char *)virt_addr, (char *)virt_addr_read) == 0;

    hal_pv_free(temp_phys_page, virt_addr, 512);
    hal_pv_free(temp_phys_page_read, virt_addr_read, 512);

    if (ok)
    {
        log("Single write-read test was successfully passed!");
        return true;
    }
    else
    {
        error("Single write-read test was failed!");
        return false;
    }

    return false;
}

bool Phantom::test_block_alignment(Phantom::Disk_backend &disk)
{
    phantom_init_stat_counters();

    const size_t block_size = 512;
    phantom_disk_partition_t stub_partition = {0};
    // required by startSync
    stub_partition.asyncIo = driver_genode_disk_asyncIO;
    stub_partition.block_size = block_size;

    // for (int i = 1; i <= 1; i++)
    // {
    char data[block_size];
    ph_memset(data, 1, block_size);
    phantom_sync_write_sector(&stub_partition, data, 1, 1);
    // }
}

bool Phantom::test_remapping()
{
    // allocate 1 virtual and 2 physical pages

    void *v = main_obj->_vmem_adapter._obj_space_allocator.alloc(ARCH_PAGE_SIZE);
    addr_t p_off = (addr_t)main_obj->_vmem_adapter.alloc_pseudo_phys(1);
    (void)p_off;
    addr_t p1 = (addr_t)main_obj->_vmem_adapter.alloc_pseudo_phys(1);
    addr_t p2 = (addr_t)main_obj->_vmem_adapter.alloc_pseudo_phys(1);

    main_obj->_vmem_adapter.map_page(p1, (addr_t)v, true);
    *(int *)v = 11;
    main_obj->_vmem_adapter.unmap_page((addr_t)v);

    main_obj->_vmem_adapter.map_page(p2, (addr_t)v, true);
    *(int *)v = 12;
    // *(int *)((char *)v + PAGE_SIZE) = 12;
    main_obj->_vmem_adapter.unmap_page((addr_t)v);

    main_obj->_vmem_adapter.map_page(p1, (addr_t)v, true);
    if (*(int *)v != 11)
    {
        error("Was not able to write 11 to phys page!");
        return false;
    }
    main_obj->_vmem_adapter.unmap_page((addr_t)v);

    main_obj->_vmem_adapter.map_page(p2, (addr_t)v, true);
    if (*(int *)v != 12)
    {
        error("Was not able to write 12 to phys page!");
        return false;
    }
    main_obj->_vmem_adapter.unmap_page((addr_t)v);

    return true;
}

bool Phantom::test_bulk()
{
    Genode::log("bulk classes ptr=", main_obj->_bulk_code_ptr, " size=", main_obj->_bulk_code_size);

    if (main_obj->_bulk_code_ptr == nullptr || main_obj->_bulk_code_size == 0){
        return false;
    }

    char first_str[16];
    memset(first_str, 0, 16);
    memcpy(first_str, main_obj->_bulk_code_ptr, 7);

    Genode::log("bulk classes rom data (0-7): '", Genode::Cstring(first_str), "'");

    if (strcmp(first_str, "phantom") == 0){
        return true;
    } else {
        return false;
    }
}
