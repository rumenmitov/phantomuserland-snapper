#include "phantom_env.h"

namespace Phantom
{
	bool test_remapping();
	bool test_obj_space();
	bool test_block_device_adapter(Phantom::Disk_backend &disk);
	bool test_block_alignment(Phantom::Disk_backend &disk);
	bool test_malloc();
	bool test_bulk();
};  // namespace Phantom