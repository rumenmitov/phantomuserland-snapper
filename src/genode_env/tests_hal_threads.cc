
extern "C"
{

#include "tests_hal.h"

#include <hal.h>
#include <stdio.h>
#include <threads.h>

	static int counter = 0;
	static hal_mutex_t cnt_mutex;
	static hal_spinlock_t spinlock;

	static void thread_func()
	{
		// printf("Entered thread_func()\n");
		// hal_mutex_lock(&cnt_mutex);
		// printf("Entered thread_func() lock\n");
		// counter++;
		// hal_mutex_unlock(&cnt_mutex);
		// printf("Leaving thread_func() lock\n");
		// hal_spin_unlock(&spinlock);
		// printf("Unlocking thread_func() spin\n");

		hal_mutex_lock(&cnt_mutex);
		counter++;
		hal_mutex_unlock(&cnt_mutex);
		hal_spin_unlock(&spinlock);
	}

	static void thread_arg_func(void *args)
	{
		(void)args;
		// hal_mutex_lock(&cnt_mutex);
		// counter++;
		// hal_mutex_unlock(&cnt_mutex);
		// hal_spin_unlock(&spinlock);
		thread_func();
	}

	bool Phantom::test_hal_thread_creation()
	{
		// Initialization
		hal_mutex_init(&cnt_mutex, "t_cnt_mutex");
		hal_spin_unlock(&spinlock);

		// Creating threads
		hal_start_kernel_thread_arg(thread_arg_func, 0);
		hal_spin_lock(&spinlock);

		hal_start_kernel_thread_arg(thread_arg_func, 0);
		hal_spin_lock(&spinlock);

		hal_start_kernel_thread_arg(thread_arg_func, 0);
		hal_spin_lock(&spinlock);

		hal_start_kernel_thread_arg(thread_arg_func, 0);
		hal_spin_lock(&spinlock);

		hal_start_kernel_thread_arg(thread_arg_func, 0);
		hal_spin_lock(&spinlock);

		hal_start_kernel_thread_arg(thread_arg_func, 0);
		hal_spin_lock(&spinlock);

		hal_start_kernel_thread_arg(thread_arg_func, 0);
		hal_spin_lock(&spinlock);

		hal_start_kernel_thread_arg(thread_arg_func, 0);
		hal_spin_lock(&spinlock);

		hal_start_kernel_thread_arg(thread_arg_func, 0);
		hal_spin_lock(&spinlock);

		hal_start_kernel_thread_arg(thread_arg_func, 0);
		hal_spin_lock(&spinlock);

		hal_start_kernel_thread(thread_func);
		hal_spin_lock(&spinlock);

		hal_start_kernel_thread(thread_func);
		hal_spin_lock(&spinlock);

		// Cleaning up
		hal_mutex_destroy(&cnt_mutex);
		if (counter == 12) {
			return true;
		}

		return false;
	}
}