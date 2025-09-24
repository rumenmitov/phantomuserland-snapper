#ifndef GENODE_PHANTOM_VMEM_H
#define GENODE_PHANTOM_VMEM_H

#include <base/entrypoint.h>
#include <base/heap.h>
#include <base/registry.h>
#include <dataspace/client.h>
#include <region_map/client.h>
#include <rm_session/connection.h>

// #include <libc/component.h>

#include <cpu/memory_barrier.h>

#include "local_attached_ram_dataspace.h"
#include "phantom_env.h"

#include <arch/arch-page.h>

#include <arch/arch_vmem_trap_state.h>

using namespace Genode;

namespace Phantom {
class Local_fault_handler;
class Vmem_adapter;
class Aligned_page_allocator;
}; // namespace Phantom

/**
 * Region-manager fault handler resolves faults by attaching new dataspaces
 */
class Phantom::Local_fault_handler : public Genode::Entrypoint {
private:
  Genode::Env &_env;
  Region_map &_region_map;
  Signal_handler<Local_fault_handler> _handler;
  size_t _page_size;

  const bool _debug = false;

  // Handler set by Phantom init routine
  int (*_pf_handler)(void *address, int write, int ip,
                     struct trap_state *ts) = nullptr;

  volatile unsigned _fault_cnt{0};

  int test_pf_handler(void *address, int write, int ip, struct trap_state *ts) {
    // TODO : Need to clean up after this thing

    warning("Test page fault handler! addr=", Hex((addr_t)address),
            " write=", write, " ip=", Hex(ip), " ts=", Hex((addr_t)ts));
    // log("PF: addr=%p, ip=%p, write=%d, ts.state=%d", address, write, ip,
    // ts->state);
    Ram_dataspace_capability ds = _env.ram().alloc(_page_size);
    // _region_map.attach_at(ds, (Genode::addr_t)address & ~(_page_size - 1));
    return 0;
  }

  void _handle_fault() {
    if (_debug)
      log("Reached fault handler");

    Region_map::Fault state = _region_map.fault();

    _fault_cnt++;

    if (_debug)
      log("region-map state is ",
          state.type == Region_map::Fault::Type::READ    ? "READ_FAULT"
          : state.type == Region_map::Fault::Type::WRITE ? "WRITE_FAULT"
          : state.type == Region_map::Fault::Type::EXEC  ? "EXEC_FAULT"
                                                         : "READY",
          ", pf_addr=", Hex(state.addr, Hex::PREFIX));

    struct trap_state ts_stub;
    ts_stub.state = 0;

    // Calling external handler
    // TODO : fix IP
    if (_pf_handler != nullptr) {
      // Libc::with_libc([&](){
      _pf_handler((void *)state.addr,
                  state.type == Region_map::Fault::Type::WRITE ? 1 : 0, -1,
                  &ts_stub);
      // });
    } else {
      test_pf_handler((void *)state.addr,
                      state.type == Region_map::Fault::Type::WRITE ? 1 : 0, -1,
                      &ts_stub);
    }

    if (_debug)
      log("returning from handle_fault");
  }

public:
  Local_fault_handler(Genode::Env &env, Region_map &region_map,
                      size_t page_size)
      : Entrypoint(env, sizeof(addr_t) * 2048, "local_fault_handler",
                   Affinity::Location()),
        _env(env), _region_map(region_map),
        _handler(*this, *this, &Local_fault_handler::_handle_fault),
        _page_size(page_size) {
    region_map.fault_handler(_handler);

    if (_debug)
      log("fault handler: waiting for fault signal");
  }

  // Have to intialize it dynamically since Phantom registers page fault handler
  // after intialization of object space
  void register_fault_handler(int (*pf_handler)(void *address, int write,
                                                int ip,
                                                struct trap_state *ts)) {
    _pf_handler = pf_handler;
  }

  void dissolve() { Entrypoint::dissolve(_handler); }

  unsigned fault_count() {
    asm volatile("" ::: "memory");
    return _fault_cnt;
  }
};

// Handle to put inside registry (used inside vmem adapter)
typedef Registered<Phantom::Local_attached_ram_dataspace> Attached_ds_handle;

struct Phys_region {
  Genode::Ram_dataspace_capability _ram_ds;
  addr_t _pseudo_addr;
  size_t _num_pages;

  // TODO : may be replaced with some singleton
  Allocator &_addr_allocator;
  Ram_allocator &_ram;

  Phys_region(Ram_allocator &ram, Allocator &addr_allocator, size_t num_pages)
      : _ram_ds(ram.alloc(ARCH_PAGE_SIZE * num_pages)),
        _pseudo_addr((addr_t)addr_allocator.alloc(ARCH_PAGE_SIZE * num_pages)),
        _num_pages(num_pages), _addr_allocator(addr_allocator), _ram(ram) {}

  virtual ~Phys_region() {
    _ram.free(_ram_ds);
    _addr_allocator.free((void *)_pseudo_addr, ARCH_PAGE_SIZE);
  }
};

typedef Registered<Phys_region> Phys_region_handle;

struct Phantom::Vmem_adapter {
  const bool _debug = false;

  addr_t OBJECT_SPACE_START = 0x80000000;
  addr_t OBJECT_SPACE_SIZE = 0x40000000;
  // TODO : defined as a macro that is required for other Phantom, need to
  // fix!!! const size_t ARCH_PAGE_SIZE = 4096;

  const unsigned int _phys_space_limit = 4096 * 1024 * 16;

  Genode::Env &_env;
  Genode::Rm_connection _rm{_env};

  Genode::Heap _metadata_heap{_env.ram(), _env.rm()};

  // Attached to env's rm
  Genode::Region_map_client _obj_space{_rm.create(OBJECT_SPACE_SIZE)};
  // fault handler (will register handler in rm as well)
  Phantom::Local_fault_handler fault_handler{_env, _obj_space, ARCH_PAGE_SIZE};
  // Object space virtual space allocator
  Genode::Allocator_avl _obj_space_allocator{&_metadata_heap};

  // Handles allocation of pseudo physical addresses
  Genode::Allocator_avl _pseudo_phys_addr_allocator{&_metadata_heap};
  // Registry to keep pages' dataspaces and their pseudo addresses
  Genode::Registry<Phys_region_handle> _pseudo_phys_pages_registry{};
  // Statistics
  unsigned long long total_allocated = 0;

  // Allocated phys region
  Ram_dataspace_capability _ram_ds{_env.ram().alloc(4096 * 1024 * 16)};

  Vmem_adapter(Env &env) : _env(env) {

    // Initializing obj space allocator
    _obj_space_allocator.add_range(OBJECT_SPACE_START + ARCH_PAGE_SIZE * 0x10,
                                   OBJECT_SPACE_SIZE).with_error([](auto error) {throw;});

    // Initializing pseudo phys allocator
    _pseudo_phys_addr_allocator.add_range(ARCH_PAGE_SIZE * 0x10, _phys_space_limit).with_error([](auto error) {throw;});

    addr_t ptr_obj = 0;

#ifndef PHANTOM_LINUX
    // ATTENTION! _obj_space is attached to the env's rm!
    // void *ptr_obj = env.rm().attach(_obj_space.dataspace(), 0, 0, true,
    // OBJECT_SPACE_START, false, true);
    Region_map::Attr attributes{.size = OBJECT_SPACE_SIZE,
                                .offset = 0,
                                .use_at = true,
                                .at = OBJECT_SPACE_START,
                                .executable = false,
                                .writeable = true};

    env.rm()
      .attach(_obj_space.dataspace(), attributes)
      .with_result(
        [&ptr_obj](Genode::Env::Local_rm::Attachment &attachment) {
          ptr_obj = (Genode::addr_t)attachment.ptr;
          attachment.deallocate = false;
        },
        [](auto) {
          error("couldn't attach dataspace!");
          throw;
        });
#else

    Region_map::Attr attributes{.size = OBJECT_SPACE_SIZE,
                                .offset = 0,
                                .use_at = false,
                                .at = OBJECT_SPACE_START,
                                .executable = false,
                                .writeable = true};

    env.rm().attach(_ram_ds, attributes).with_result([&](Genode::Env::Local_rm::Attachment &attachment) {
      ptr_obj = (Genode::addr_t)attachment.ptr;
      attachment.deallocate = false;
      OBJECT_SPACE_START = ptr_obj;
    }, [](auto) {throw;});
#endif // PHANTOM_LINUX

    Dataspace_client rm_obj_client(_obj_space.dataspace());
    log(_obj_space.dataspace());
    // log(_obj_space.state().type);
    addr_t const addr_obj = reinterpret_cast<addr_t>(ptr_obj);
    // XXX : Commented out since Genode on Linux doesn't give a real capability
    // to dataspace log(" region obj.space        ",
    //     Hex_range<addr_t>(addr_obj, rm_obj_client.size()));
  }

  ~Vmem_adapter() {}

  void map_page(addr_t phys_addr, addr_t virt_addr, bool writeable) {
    if (_debug)
      log("Mapping phys ", Hex(phys_addr), " to virt ", Hex(virt_addr));

    // Getting the region
    // Phys_region_handle *region = get_pseudo_phys_region(phys_addr);

    // if (region == nullptr)
    // {
    //     Genode::error("Mapping incorrect phys address [",
    //                   Hex(phys_addr), "->", Hex(virt_addr),
    //                   "]! Will not map");
    //     return;
    // }

    // addr_t offset = phys_addr - region->_pseudo_addr;

    addr_t offset = phys_addr;

    if (offset % ARCH_PAGE_SIZE > 0) {
      Genode::warning("Phys addr (offset) not page alligned");
    }

    Genode::memory_barrier();

    // If in object space, map to objects space rm, otherwise, map to env rm
    if (virt_addr >= OBJECT_SPACE_START &&
        virt_addr < OBJECT_SPACE_START + OBJECT_SPACE_SIZE) {

      // TODO : Error handling
      //        Handle 0 return address

      // Region_map::Local_addr laddr = _obj_space.attach(
      //     region->_ram_ds,
      //     ARCH_PAGE_SIZE,
      //     offset,
      //     true,
      //     virt_addr - OBJECT_SPACE_START,
      //     false,
      //     writeable);

      // _obj_spacea
      addr_t laddr = (intptr_t)0x0;
      Genode::retry<Genode::Out_of_ram>(
          [&]() {
            Genode::retry<Genode::Out_of_caps>(
                [&]() {
                  // Genode::log("attach attempt");
                  Region_map::Attr attribute{.size = ARCH_PAGE_SIZE,
                                             .offset = offset,
                                             .use_at = true,
                                             .at =
                                                 virt_addr - OBJECT_SPACE_START,
                                             .executable = false,
                                             .writeable = writeable};

                  _obj_space.attach(_ram_ds, attribute)
                      .with_result(
                          [&laddr](const Genode::Region_map::Range &range) {
                            laddr = range.start;
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
                              break;
                            }
                            throw;
                          });
                },
                [&]() { _rm.upgrade_caps(10); }, 16U);
          },
          [&]() { _rm.upgrade_ram(8 * 1024); });

      if (laddr != virt_addr - OBJECT_SPACE_START) {
        error("Mapped addr does not correspond to virtual one! Got ",
              Hex(laddr), " expected ", Hex(virt_addr - OBJECT_SPACE_START));
      }

      if (_debug)
        log("Map returned laddr=", Hex(laddr));
    } else {
      // TODO : Error handling
      //        Handle 0 return address

      // Region_map::Local_addr laddr = _env.rm().attach(
      //     region->_ram_ds,
      //     PAGE_SIZE,
      //     offset,
      //     true,
      //     virt_addr,
      //     false,
      //     writeable);

      addr_t laddr = 0;
      Region_map::Attr attribute{.size = ARCH_PAGE_SIZE,
                                 .offset = offset,
                                 .use_at = true,
                                 .at = virt_addr,
                                 .executable = false,
                                 .writeable = writeable};

      _env.rm()
          .attach(_ram_ds, attribute)
          .with_result(
              [&laddr](Genode::Env::Local_rm::Attachment &attachment) {
                laddr = (Genode::addr_t)attachment.ptr;
                attachment.deallocate = false;
              },
              [](auto) {
                Genode::error("couldn't attach region map!");
                throw;
              });

      if (laddr != virt_addr) {
        error("Mapped addr does not correspond to virtual one! Got ",
              Hex(laddr), " expected ", Hex(virt_addr));
      }

      if (_debug)
        log("Map returned laddr=", Hex(laddr));
    }
  }

  addr_t map_somewhere(addr_t phys_addr, bool writeable, size_t n_pages) {
    if (_debug)
      log("Mapping phys ", Hex(phys_addr), " to some virt ");

    // Getting the region
    // Phys_region_handle *region = get_pseudo_phys_region(phys_addr);

    // if (region == nullptr)
    // {
    //     Genode::error("Mapping incorrect phys address [",
    //                   Hex(phys_addr), "-> somewehere",
    //                   "]! Will not map");
    //     return nullptr;
    // }

    // addr_t offset = phys_addr - region->_pseudo_addr;
    addr_t offset = phys_addr;

    Genode::memory_barrier();

    // TODO : Error handling
    //        Handle 0 return address

    // Region_map::Local_addr laddr = _env.rm().attach(
    //     region->_ram_ds,
    //     n_pages * ARCH_PAGE_SIZE,
    //     offset,
    //     false,
    //     nullptr,
    //     false,
    //     writeable);

    addr_t laddr = 0;
    Region_map::Attr attribute{.size = n_pages * ARCH_PAGE_SIZE,
                               .offset = offset,
                               .use_at = false,
                               .at = 0,
                               .executable = false,
                               .writeable = writeable};

    _env.rm()
        .attach(_ram_ds, attribute)
        .with_result(
            [&laddr](Genode::Env::Local_rm::Attachment &attachment) {
              laddr = (Genode::addr_t)attachment.ptr;
              attachment.deallocate = false;
            },
            [](auto) {
              Genode::error("couldn't attach region map!");
              throw;
            });

    if (_debug)
      log("Map returned laddr=", Hex(laddr));

    return laddr;
  }

  void unmap_page(addr_t virt_addr) {
    Genode::memory_barrier();

    if (virt_addr >= OBJECT_SPACE_START &&
        virt_addr < OBJECT_SPACE_START + OBJECT_SPACE_SIZE) {
      if (_debug)
        log("unmapping virt ", Hex(virt_addr),
            " obj_space_addr=", virt_addr - OBJECT_SPACE_START);

      // TODO : Error handling
      _obj_space.detach(virt_addr - OBJECT_SPACE_START);
    } else {
      if (_debug)
        log("unmapping virt ", Hex(virt_addr));
      _env.rm().detach(virt_addr);
    }
  }

  void *alloc_pseudo_phys(unsigned int num_pages) {
    // // XXX : this implementation is not performant and should be replaced in
    // the future
    // // TODO : Add error handling

    // // Phys_page is RAII, so all allocations will be handled by it
    // Phys_region *page = new (&_metadata_heap) Phys_region_handle(
    //     _pseudo_phys_pages_registry,
    //     _env.ram(),
    //     _pseudo_phys_addr_allocator,
    //     num_pages);

    // total_allocated++;

    // Genode::log("Pseudo-phys total=", total_allocated);

    // return (void *)page->_pseudo_addr;

    return _pseudo_phys_addr_allocator.alloc(ARCH_PAGE_SIZE * num_pages);
  }

  void free_pseudo_phys(void *addr, int npages) {
    // // XXX: It is naive and non efficient implementation. Should be improved
    // later

    // bool success = false;

    // _pseudo_phys_pages_registry.for_each(
    //     [&](Phys_region_handle &region)
    //     {
    //         if (region._pseudo_addr != (addr_t)addr)
    //         {
    //             return;
    //         }

    //         if (region._num_pages != (size_t)(npages))
    //         {
    //             Genode::warning(
    //                 "free pseudo phys with incorrect number of pages: addr=",
    //                 Hex((long)addr),
    //                 " expected=",
    //                 Hex(region._num_pages * ARCH_PAGE_SIZE),
    //                 " received=",
    //                 Hex(npages * ARCH_PAGE_SIZE));

    //             return;
    //         }

    //         destroy(_metadata_heap, &region);
    //         success = true;
    //     });

    // if (!success)
    // {
    //     Genode::warning("Failed to free pseudo phys: addr=", Hex((long)addr),
    //     " npages=", npages);
    // } else {
    //     total_allocated--;
    // }

    _pseudo_phys_addr_allocator.free(addr, ARCH_PAGE_SIZE * npages);
  }

  Phys_region_handle *get_pseudo_phys_region(addr_t pseudo_phys_addr) {

    // TODO : Check if addr is page aligned

    Phys_region_handle *res = nullptr;
    bool success = false;

    _pseudo_phys_pages_registry.for_each([&](Phys_region_handle &region) {
      if (success) {
        return;
      }

      // Check if within the region
      if (region._pseudo_addr <= pseudo_phys_addr &&
          pseudo_phys_addr <
              region._pseudo_addr + region._num_pages * ARCH_PAGE_SIZE) {
        res = &region;
        success = true;
      }
    });

    if (!success) {
      Genode::warning("Phys addr not found: addr=",
                      Hex((long)pseudo_phys_addr));
    }

    return res;
  }
};

#endif
