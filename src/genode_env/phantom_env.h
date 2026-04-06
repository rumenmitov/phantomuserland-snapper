#ifndef PHANTOM_ENV_H
#define PHANTOM_ENV_H

#include "disk_backend.h"
#include "phantom_framebuffer.h"
#include "phantom_threads.h"
#include "phantom_timer.h"
#include "phantom_vmem.h"

#include <base/component.h>
#include <base/heap.h>
#include <rom_session/connection.h>

#include <snapper_session/connection.h>

#ifdef __cplusplus

namespace Phantom
{
	using namespace Genode;

	struct Main
	{
		Env &_env;
		Phantom::Vmem_adapter _vmem_adapter {_env};
		Heap _heap {_env.ram(), _env.rm()};
		Phantom::Timer_adapter _timer_adapter {_env};
		Phantom::PhantomThreadsRepo _threads_repo {_env, _heap};
		Phantom::Disk_backend _disk {_env, _heap};
		Snapper::Connection _snapper {_env};
    Genode::Attached_rom_dataspace _rom { _env, "config"};
    Genode::Root_directory _fs_root { _env, _heap, _rom.xml ().sub_node ("vfs") };
    

		// Constructible so that we can run Phantom without graphics if we want
		Constructible<Phantom::FramebufferAdapter> _framebuffer;
		// XXX : docs says that it "allows only one timeout at a time"
		// Timer::Connection _sleep_timer{_env, "phantom_sleep_timer"};

		// ROM with bulk classes
		Rom_connection _bulk_class_rom {_env, "phantom_classes"};
		void *_bulk_code_ptr {};
		const size_t _bulk_code_size {
			Dataspace_client(_bulk_class_rom.dataspace()).size()};

		Main(Env &env) : _env(env), _framebuffer()
		{
			Region_map::Attr attributes {.size = _bulk_code_size,
										 .offset = 0,
										 .use_at = false,
										 .executable = false,
										 .writeable = true};

			_env.rm()
				.attach(_bulk_class_rom.dataspace(), attributes)
				.with_result(
					[this](Genode::Env::Local_rm::Attachment &attachment) {
						_bulk_code_ptr = attachment.ptr;
						attachment.deallocate = false;
					},
					[](auto) { throw; });
		}
	};

	extern Main *main_obj;

	void test_obj_space(addr_t const addr_obj_space);
	void test_block_device(Disk_backend &disk);
}  // namespace Phantom

#endif  // __cplusplus

#endif  // PHANTOM_ENV_H
