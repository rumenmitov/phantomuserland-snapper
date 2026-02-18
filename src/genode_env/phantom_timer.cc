#include "phantom_env.h"

extern "C"
{
#include <hal.h>
#include <kernel/board.h>
#include <kernel/stats.h>
#include <kernel/timedcall.h>
#include <ph_time.h>
#include <threads.h>


	static bigtime_t msecDivider = 0;
	static bigtime_t secDivider = 0;
	static hal_mutex_t *time_mutex;

	// Supposed to be called from libc context
	// void board_init_kernel_timer(void)
	// {
	//     hal_mutex_init(time_mutex, "time mutex");
	//     main_obj->_timer_adapter.set_handler(hal_time_tick);
	// }


	// XXX : Important: it has to be executed from libc context
	//! Called from timer interrupt, tick_rate is in uSec
	void hal_time_tick(int tick_rate_us)
	{
		hal_mutex_lock(time_mutex);

		msecDivider += tick_rate_us;

		while (msecDivider > 1000) {
			msecDivider -= 1000;
			phantom_process_timed_calls();

			secDivider++;
			if (secDivider > 1000) {
				secDivider -= 1000;
				// Once a sec
				stat_update_second_stats();
			}
		}

		hal_mutex_unlock(time_mutex);
	}

	time_t fast_time(void)
	{
		// TODO : Implement using Genode's RTC
		return 1337;
	}

	bigtime_t hal_system_time(void)
	{
		return main_obj->_timer_adapter.curr_time_us();
	}


	// bigtime_t hal_system_time(void){
	//     hal_mutex_lock(time_mutex);
	//     bigtime_t res = msecDivider;
	//     hal_mutex_unlock(time_mutex);
	//     return res;
	// }

	// bigtime_t hal_system_time_lores(void)
	// {
	//     _stub_print();
	//     return 0;
	// }

	// bigtime_t hal_local_time(void)
	// {
	//     _stub_print();
	//     return 0;
	// }
}