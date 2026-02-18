#include <base/blockade.h>
#include <base/log.h>
#include <base/mutex.h>
#include <base/semaphore.h>

namespace Phantom
{
	class Phantom_mutex_wrapper;
}

class Phantom::Phantom_mutex_wrapper
{
private:
	Genode::Mutex _mutex {};

	// Required since Genode implementation does not allow
	// to look up the state of the locks or mutexes
	// XXX : Used mainly in assertions
	volatile bool _is_locked = false;
	Genode::Mutex _state_mutex {};

	void updateMutexState(bool new_is_locked)
	{
		Genode::Mutex::Guard guard(_state_mutex);
		_is_locked = new_is_locked;
	}

public:
	Phantom_mutex_wrapper() {}

	~Phantom_mutex_wrapper() {}

	void acquire()
	{
		_mutex.acquire();

		// XXX : Not a good place to preempt
		// Assuming that nothing bad will happen if we say that
		// it is locked after we locked it

		updateMutexState(true);
	}

	void release()
	{
		_mutex.release();

		// XXX : Not a good place to preempt
		// Assuming that nothing bad will happen if we say that
		// it is unlocked a bit later

		updateMutexState(false);
	}

	bool isLocked()
	{
		Genode::Mutex::Guard guard(_state_mutex);
		return _is_locked;
	}
};

extern "C"
{

	struct phantom_mutex_impl
	{
		Phantom::Phantom_mutex_wrapper *lock;
		const char *name;
	};

	struct phantom_sem_impl
	{
		Genode::Semaphore *sem;
		const char *name;
	};

	struct phantom_cond_impl
	{
		// Counter of threads waiting on blockade
		volatile int counter;
		// Mutex to ensure atomic access to counter
		Genode::Mutex *mutex;

		Genode::Blockade *blockade;
		const char *name;
	};
}