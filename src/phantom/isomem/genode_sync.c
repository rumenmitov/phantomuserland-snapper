#ifdef PHANTOM_SYNC_STUB

#include "genode_misc.h"

#include <hal.h>

/*
 *
 * Mutexes
 *
 */

int hal_mutex_init(hal_mutex_t *m, const char *name)
{
	_stub_print();
}

int hal_mutex_lock(hal_mutex_t *m)
{
	_stub_print();
}

int hal_mutex_unlock(hal_mutex_t *m)
{
	_stub_print();
}

int hal_mutex_is_locked(hal_mutex_t *m)
{
	_stub_print();
}

errno_t hal_mutex_destroy(hal_mutex_t *m)
{
	_stub_print();
}

// static hal_mutex_t *heap_lock = 0; // won't be used until assigned

// Called after initing threads (and mutexes)
void heap_init_mutex(void)
{
	_stub_print();
}

/*
 *
 * Semaphores
 *
 */

// A: Probably can be kept as is. Only windows and drivers require semaphores

int hal_sem_acquire(hal_sem_t *s)
{
	_stub_print();
}

void hal_sem_release(hal_sem_t *s)
{
	_stub_print();
	(void)s;
}

int hal_sem_init(hal_sem_t *s, const char *name)
{
	_stub_print();
	(void)s;
	(void)name;
	return 0;
}

/*
 *
 * Spinlocks
 *
 */

#define _spin_unlock(p)                              \
	({                                               \
		register int _u__;                           \
		__asm__ volatile("xorl %0, %0; \n\
			  xchgl %0, %1"           \
						 : "=&r"(_u__), "=m"(*(p))); \
		0;                                           \
	})

// ret 1 on success
#define _spin_try_lock(p)                            \
	(!({                                             \
		register int _r__;                           \
		__asm__ volatile("movl $1, %0; \n\
			  xchgl %0, %1"           \
						 : "=&r"(_r__), "=m"(*(p))); \
		_r__;                                        \
	}))

void hal_spin_init(hal_spinlock_t *sl)
{
	sl->lock = 0;
}

void hal_spin_lock(hal_spinlock_t *sl)
{
	while (!_spin_try_lock(&(sl->lock)))
		while (sl->lock)
			;
}

void hal_spin_unlock(hal_spinlock_t *sl)
{
	_spin_unlock(&(sl->lock));
}

// Turns off interrupts too
void hal_wired_spin_lock(hal_spinlock_t *l)
{
	_stub_print();
}

void hal_wired_spin_unlock(hal_spinlock_t *l)
{
	_stub_print();
}

/*
 *
 * Conditional variables
 *
 */

struct phantom_cond_impl
{
	// CONDITION_VARIABLE cv;
	const char *name;
};

int hal_cond_init(hal_cond_t *c, const char *name)
{
	c->impl = ph_calloc(
		1,
		sizeof(struct phantom_cond_impl) +
			16);  // to prevent corruption if kernel hal mutex func will be called
	// InitializeConditionVariable( &(c->impl.cv) );
	c->impl->name = name;
	return 0;
}

errno_t hal_cond_wait(hal_cond_t *c, hal_mutex_t *m)
{
	assert(c->impl);
	hal_mutex_unlock(m);
	hal_sleep_msec(100);
	// SleepConditionVariableCS( &(c->impl.cv), &(m->impl->cs), 0 );
	hal_mutex_lock(m);
	return 0;
}

errno_t hal_cond_timedwait(hal_cond_t *c, hal_mutex_t *m, long msecTimeout)
{
	assert(c->impl);
	hal_sleep_msec(msecTimeout);
	// SleepConditionVariableCS( &(c->impl.cv), &(m->impl->cs), msecTimeout );
	return 0;
}

errno_t hal_cond_signal(hal_cond_t *c)
{
	assert(c->impl);
	// WakeConditionVariable( &(c->impl.cv) );
	return 0;
}

errno_t hal_cond_broadcast(hal_cond_t *c)
{
	assert(c->impl);
	// WakeAllConditionVariable( &(c->impl->cv) );
	return 0;
}

errno_t hal_cond_destroy(hal_cond_t *c)
{

	// if(m->impl.owner != 0)        panic("locked mutex killed");
	ph_free(c->impl);
	c->impl = 0;

	return 0;
}

#endif