#include "tests_hal.h"

#include <base/log.h>

extern "C"
{
#include <hal.h>

	// Single threaded test to check if lock state retrieval works
	bool test_hal_mutex_state()
	{
		hal_mutex_t mutex;


		hal_mutex_init(&mutex, "test_state");

		// Checking if initial state is OK
		if (hal_mutex_is_locked(&mutex) != 0) {
			return false;
		}
		Genode::log("Acquired mutex");

		hal_mutex_lock(&mutex);

		// Checking state after acquiring mutex
		if (hal_mutex_is_locked(&mutex) != 1) {
			return false;
		}

		Genode::log("Unlocking mutex");

		hal_mutex_unlock(&mutex);

		// Checking if it changed after releasing it
		if (hal_mutex_is_locked(&mutex) != 0) {
			return false;
		}

		hal_mutex_destroy(&mutex);

		return true;
	}
}