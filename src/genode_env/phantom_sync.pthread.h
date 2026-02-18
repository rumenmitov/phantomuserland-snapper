#include <base/blockade.h>
#include <base/log.h>
#include <base/mutex.h>
#include <base/semaphore.h>

#include <pthread.h>
#include <semaphore.h>

namespace Phantom
{
	class Phantom_mutex_wrapper;
}

class Phantom::Phantom_mutex_wrapper
{
private:
	pthread_mutex_t _mutex = NULL;

	// Required since Genode implementation does not allow
	// to look up the state of the locks or mutexes
	// XXX : Used mainly in assertions
	volatile bool _is_locked = false;
	pthread_mutex_t _state_mutex = NULL;

	void updateMutexState(bool new_is_locked)
	{
		pthread_mutex_lock(&_state_mutex);

		_is_locked = new_is_locked;

		pthread_mutex_unlock(&_state_mutex);
	}

public:
	Phantom_mutex_wrapper()
	{
		pthread_mutex_init(&_mutex, NULL);
		pthread_mutex_init(&_state_mutex, NULL);
	}

	Phantom_mutex_wrapper(const Phantom::Phantom_mutex_wrapper &) = delete;
	int operator=(const Phantom::Phantom_mutex_wrapper &) = delete;

	~Phantom_mutex_wrapper()
	{
		pthread_mutex_destroy(&_mutex);
		pthread_mutex_destroy(&_state_mutex);
	}

	void acquire()
	{

		pthread_mutex_lock(&_mutex);

		// XXX : Not a good place to preempt
		// Assuming that nothing bad will happen if we say that
		// it is locked after we are locking it

		updateMutexState(true);
	}

	void release()
	{
		pthread_mutex_unlock(&_mutex);

		// XXX : Not a good place to preempt
		// Assuming that nothing bad will happen if we say that
		// it is unlocked a bit later

		updateMutexState(false);
	}

	bool isLocked()
	{
		pthread_mutex_lock(&_state_mutex);
		bool res = _is_locked;
		pthread_mutex_unlock(&_state_mutex);
		return res;
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
		sem_t sem;
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