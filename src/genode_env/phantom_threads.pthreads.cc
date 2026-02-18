// #define DEBUG_MSG_PREFIX "vm.unixhal"
// #include <debug_ext.h>
// #define debug_level_flow 10
// #define debug_level_error 10
// #define debug_level_info 10

// #include <stdarg.h>
// #include <pthread.h>
// #include "genode_misc.h"

#include "phantom_env.h"

#include <base/child.h>

#include <cpu/memory_barrier.h>
#include <libc/component.h>

// Important to keep this typedef before threads.h
typedef int errno_t;

using namespace Phantom;

extern "C"
{

#include <threads.h>
// XXX: Needed only for phantom_create_thread()
#include <pthread.h>
#include <stdlib.h>
#include <thread_private.h>

	/*
	 *
	 * Thread creation/destruction
	 * returns TID
	 *
	 */

	// static void* _thread_func_wrapper_args(void (*thread)(void *arg)){

	// }

	/*
	 *
	 * Thread entry points wrappers.
	 *
	 * XXX : Phantom's HAL expects that functions will not return anyhting,
	 * whereas pthread does. If thread doesn't have arguments it is just
	 * a wrapper that returns 0 after the function ends, but if it
	 * has arguments it is a little trickier.
	 * If function has arguments, we have to pass both function pointer
	 * and arguments pointer. To do this, there is a special struct.
	 * However, if we allocate it dynamically, we have to keep it somewhere and
	 * deallocate when the thread is killed. The workaround is to have spinlock
	 * in this structure. On initialization, set it to locked state and unlock after
	 * pointers were copied into the context of the wrapper.
	 *
	 */

	struct phantom_thread_args
	{
		void (*thread)(void *);
		void *args;
		void (*death_handler)(phantom_thread_t *);
		hal_spinlock_t *spin;
	};

	// struct phantom_thread_proxy_args
	// {
	//     void *(*thread)(void *);
	//     void *args;
	// };

	// void *
	// phantom_thread_proxy_func(void *raw_args)
	// {

	//     Genode::log("Thread proxy! tid=", pthread_self());

	//     phantom_thread_proxy_args *args = (phantom_thread_proxy_args *)raw_args;

	//     args->thread(args->args);

	//     return 0;
	// }

	void phantom_argless_thread_func_wrapper(void *args)
	{
		void (*thread_to_run)(void) = (void (*)(void))args;

		Genode::log("Starting thread without args! tid=",
					pthread_self(),
					" thread=",
					thread_to_run);

		thread_to_run();

		Genode::log("Finished thread without args! tid=",
					pthread_self(),
					" thread=",
					thread_to_run);
	}

	void *phantom_thread_func_wrapper(void *raw_args)
	{
		// Genode::log("Entered EP with args!");
		phantom_thread_args *wrapper = (phantom_thread_args *)raw_args;

		// Copying values

		void (*thread)(void *) = wrapper->thread;
		void *thread_args = wrapper->args;
		void (*death_handler)(phantom_thread_t *) = wrapper->death_handler;

		Genode::log("Starting thread! tid=",
					pthread_self(),
					" thread=",
					thread,
					" args=",
					thread_args,
					" death_handler=",
					death_handler);

		// Unlocking parent thread

		hal_spin_unlock(wrapper->spin);

		Genode::log("Starting thread (Unlocked spin)! tid=",
					pthread_self(),
					" thread=",
					thread,
					" args=",
					thread_args,
					" death_handler=",
					death_handler);

		// Executing thread

		// Libc::with_libc([&]()
		//                 { thread(thread_args); });

		thread(thread_args);

		Genode::log("Finishing thread (Starting DH)! tid=",
					pthread_self(),
					" thread=",
					thread,
					" args=",
					thread_args,
					" death_handler=",
					death_handler);

		// Executing death handler
		// XXX : Note that it will not be executed if thread was killed by someone

		if (death_handler != 0) {
			phantom_thread_t thread_struct;
			thread_struct.tid = (long)pthread_self();
			thread_struct.owner = 0;
			thread_struct.prio = 0;

			death_handler(&thread_struct);
		}

		Genode::log("Finishing thread (Finished DH)! tid=",
					pthread_self(),
					" thread=",
					thread,
					" args=",
					thread_args,
					" death_handler=",
					death_handler);

		return 0;
	}

	int hal_start_kernel_thread_arg_dh(void (*thread)(void *arg),
									   void *arg,
									   void (*death_handler)(phantom_thread_t *))
	{
		Genode::memory_barrier();
		// Genode::log("Starting new phantom kernel thread with args! thread=", thread, "
		// arg=", arg); Actually, no distinction between regular and kernel thread since
		// in both cases we will be in userspace

		// Setting up wrapper struct

		phantom_thread_args *thread_args =
			(phantom_thread_args *)ph_malloc(sizeof(phantom_thread_args));
		thread_args->spin = (hal_spinlock_t *)ph_malloc(sizeof(hal_spinlock_t));
		thread_args->thread = thread;
		thread_args->args = arg;
		thread_args->death_handler = death_handler;
		hal_spin_init(thread_args->spin);
		hal_spin_lock(thread_args->spin);

		// Starting thread

		int tid;
		// const pthread_attr_t *attr = 0;

		if (pthread_create(
				(pthread_t *)&tid, 0, phantom_thread_func_wrapper, thread_args)) {
			Genode::error("Failed to create kernel thread with args!");
		}

		// Waiting till thread started

		// Genode::memory_barrier();

		// Genode::error("!!! debug: spin=", Hex((addr_t)thread_args->spin));

		hal_spin_lock(thread_args->spin);

		log("Unlocked thread!");

		ph_free(thread_args);

		return tid;
	}

	int hal_start_kernel_thread_arg(void (*thread)(void *arg), void *arg)
	{
		return hal_start_kernel_thread_arg_dh(thread, arg, 0);
	}

	void hal_start_kernel_thread(void (*thread)(void))
	{

		hal_start_kernel_thread_arg_dh(phantom_argless_thread_func_wrapper,
									   (void *)thread,
									   0);
	}

	void hal_exit_kernel_thread()
	{
		// XXX : Seems that the legit way is to return from Thread::entry
		pthread_exit(0);

		while (true) {
			// Genode::error("Error: Phantom kernel thread %s was not terminated!",
			// Thread::myself()->name().string());
			Genode::error("Error: Phantom kernel thread %s was not terminated!",
						  pthread_self());
		}
	}

	tid_t hal_start_thread(void (*thread)(void *arg), void *arg, int flags)
	{
		// Genode::log("Starting new phantom thread! thread=", thread, " arg=", arg);
		(void)flags;
		hal_start_kernel_thread_arg(thread, arg);
		return 0;
	}

	// XXX : Don't use this function! Use hal_start_thread() instead
	phantom_thread_t *phantom_create_thread(void (*func)(void *), void *arg, int flags)
	{

		Genode::warning(
			"Called phantom_create_thread(), using hal_start_thread() instead!");
		hal_start_thread(func, arg, flags);

		return nullptr;
	}

	// XXX : If thread is killed this way, death handler will not be executed
	errno_t t_kill_thread(tid_t tid)
	{
		Genode::warning(
			"Thread killed using t_kill_thread. Death handler will not work. tid=", tid);
		pthread_cancel((pthread_t)((long)tid));
		return 0;
	}

	errno_t t_current_set_death_handler(void (*handler)(phantom_thread_t *tp))
	{
		Genode::warning("Attempted to set death handler! (",
						handler,
						")  tid=",
						(long)pthread_self());
		return 0;
	}

	/*
	 *
	 * Thread info
	 *
	 */

	phantom_thread_t *get_current_thread()
	{
		// XXX: Probably, better to avoid this allocation. It will inevitably lead to
		// dangling pointers PhantomGenericThread *ph_thread = (PhantomGenericThread
		// *)Thread::myself(); phantom_thread_t *alloced_thread_struct = new
		// (main_obj->_heap) phantom_thread_t(ph_thread->getPhantomStruct()); return
		// alloced_thread_struct;

		phantom_thread_t *allocated_thread_struct =
			(phantom_thread_t *)ph_malloc(sizeof(phantom_thread_t));

		allocated_thread_struct->tid = (long)pthread_self();
		allocated_thread_struct->owner = 0;
		allocated_thread_struct->prio = 0;

		return allocated_thread_struct;
	}

	tid_t get_current_tid(void)
	{
		return (long)pthread_self();
	}

	void *get_thread_owner(phantom_thread_t *t)
	{
		return t->owner;
	}

	errno_t t_get_owner(int tid, void **owner)
	{
		(void)owner;
		Genode::warning("Attempted to get owner of the thread! self=",
						pthread_self(),
						" tid=",
						(long)tid);
		return 0;
	}

	errno_t t_current_get_priority(int *prio)
	{
		(void)prio;
		Genode::warning("Attempted to get priority of the thread! tid=",
						(long)pthread_self());
		return 0;
	}

	errno_t t_get_ctty(tid_t tid, struct wtty **ct)
	{
		(void)tid;
		(void)ct;
		Genode::warning("Attempted to get ctty of the thread! tid=",
						(long)pthread_self());
		return 1;
	}

	/*
	 *
	 * Thread configuration
	 *
	 */

	void hal_set_current_thread_name(const char *name)
	{
		// XXX : Not used, actually
		//       Moreover, Genode doesn't allow to set name
		t_current_set_name(name);
	}

	errno_t hal_set_current_thread_priority(int p)
	{
		return t_current_set_priority(p);
	}

	errno_t t_current_set_name(const char *name)
	{
		// XXX : Not used, actually
		//       Moreover, Genode doesn't allow to set name
		Genode::warning(
			"Attempted to set name of the thread! tid=", pthread_self(), " name=", name);
		return 0;
	}
	errno_t t_current_set_priority(int p)
	{
		Genode::warning(
			"Attempted to set priority of the thread! tid=", pthread_self(), " prio=", p);
		return 0;
	}
	errno_t t_set_owner(tid_t tid, void *owner)
	{
		Genode::warning("Attempted to set owner of the thread! self=",
						pthread_self(),
						" tid=",
						tid,
						" owner=",
						owner);
		return 0;
	}

	/*
	 *
	 * Multithreading
	 *
	 */

	void t_smp_enable(int yn)
	{
		(void)yn;
		// _stub_print();
	}

	//< Enable or disable access to paged memory - calls arch pagemap func.
	void t_set_paged_mem(bool enable)
	{
		(void)enable;
		// _stub_print();
	}

	// TODO : Rework sleeping threads to use blockades or timers!
	void wake_sleeping_thread(void *arg)
	{
		(void)arg;
		// arg is tid
		// _stub_print();
	}

	/**
	 *
	 * Adds flag to sleep flags of thread, removes thread from
	 * runnable set, unlocks the lock given, switches off to some other thread.
	 */
	void thread_block(int sleep_flag, hal_spinlock_t *lock_to_be_unlocked)
	{
		(void)sleep_flag;

		// XXX : May be not that efficient. Used mainly while waiting disk IO
		hal_spin_lock(lock_to_be_unlocked);

		// _stub_print();
	}

	/*
	 *
	 * Snapshot machinery
	 *
	 */

	void t_migrate_to_boot_CPU(void)
	{
		// _stub_print();
	}

	// mark myself as snapper thread
	errno_t t_set_snapper_flag(void)
	{
		// _stub_print();
		return 0;
	}

	// TODO : Remove if not needed

	// void phantom_thread_wait_4_snap()
	// {
	//     // Just return
	// }

	// void phantom_snapper_wait_4_threads()
	// {
	//     // Do nothing in non-kernel version
	//     // Must be implemented if no-kernel multithread will be done
	// }

	// void phantom_snapper_reenable_threads()
	// {
	//     //
	// }

	// void phantom_activate_thread()
	// {
	//     // Threads do not work in this mode
	// }
}