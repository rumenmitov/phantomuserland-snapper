#include "phantom_env.h"

#include <base/log.h>

extern "C"
{

#include <hal.h>
#include <ph_malloc.h>
#include <unistd.h>
#include <vm/bulk.h>

	void hal_sleep_msec(int milliseconds)
	{
		if (milliseconds < 0)
			milliseconds = 0;
		main_obj->_timer_adapter.sleep_microseconds(milliseconds * 1000);
	}

	void hal_disable_preemption()
	{
		// Genode::log("STUB: disabled preemption");
	}

	void hal_enable_preemption()
	{
		// Genode::log("STUB: enabled preemption");
	}

	// Bulk code related functions

	// XXX : char* to ensure byte long operations
	static char *bulk_code = 0;
	static unsigned int bulk_size = 0;
	static char *bulk_read_pos = 0;

	int bulk_seek_f(int pos)
	{
		bulk_read_pos = bulk_code + pos;
		return bulk_read_pos >= bulk_code + bulk_size;
	}

	int bulk_read_f(int count, void *data)
	{
		if (count < 0)
			return -1;

		int left = (bulk_code + bulk_size) - bulk_read_pos;

		if (count > left)
			count = left;

		ph_memcpy(data, bulk_read_pos, count);

		bulk_read_pos += count;

		return count;
	}

	void load_classes_module()
	{
		// The bulk classes code should be loaded as boot module and be accessible as ROM.
		// Main object should already setup everything by this point

		bulk_code = (char *)main_obj->_bulk_code_ptr;
		bulk_size = main_obj->_bulk_code_size;

		pvm_bulk_init(bulk_seek_f, bulk_read_f);
	}
}