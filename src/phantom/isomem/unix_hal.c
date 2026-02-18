/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2009 Dmitry Zavalishin, dz@dz.ru
 *
 * HAL bindings for unix, compiled with Phantom headers.
 *
 **/

#define DEBUG_MSG_PREFIX "vm.unixhal"
#include <debug_ext.h>
// #define debug_level_flow 10
// #define debug_level_error 10
// #define debug_level_info 10

#include "event.h"

#include <stdarg.h>
#include <threads.h>

#ifndef PHANTOM_GENODE

#include "gcc_replacements.h"

#endif

#include "hal.h"
#include "vm/alloc.h"
#include "vm/internal_da.h"
// #include "main.h"
#include "vm/alloc.h"

// #include "unixhal.h"

// struct hardware_abstraction_level hal;

// void hal_init( vmem_ptr_t va, long vs )
// {
//     ph_printf("Unix HAL init @%p\n", va);

//     unix_hal_init();

//     hal.object_vspace = va;
//     hal.object_vsize = vs;

//     pvm_alloc_init( va, vs );

//     //hal_start_kernel_thread( (void*)&unixhal_debug_srv_thread );
//     hal_start_kernel_thread( (void*)&winhal_debug_srv_thread );
// }

// void hal_disable_preemption()
// {
//     unix_hal_disable_preemption();
// }

// void hal_enable_preemption()
// {
//     unix_hal_enable_preemption();
// }

// vmem_ptr_t hal_object_space_address() { return hal.object_vspace; }

// void    hal_halt()
// {
//     //fflush(stderr);
//     ph_printf("\n\nhal halt called, exiting.\n");
//     getchar();
//     exit(1);
// }

// void hal_sleep_msec(int miliseconds)
// {
//     //usleep(1000*miliseconds);
//     //sleep( ((miliseconds-1)/1000)+1 );
//     unix_hal_sleep_msec(miliseconds);
// }

// alloc wants it
// int phantom_virtual_machine_threads_stopped = 0;

// no snaps here

// volatile int phantom_virtual_machine_snap_request = 0;

#include <vm/stacks.h>

// static void *dm_mem, *dm_copy;
// static int dm_size = 0;
// void setDiffMem( void *mem, void *copy, int size )
// {
//     dm_mem = mem;
//     dm_copy = copy;
//     dm_size = size;
// }

// void checkDiffMem()
// {
// #if 0
//     char *mem = dm_mem;
//     char *copy = dm_copy;
//     char *start = dm_mem;
//     int prevdiff = 0;

//     int i = dm_size;
//     while( i-- )
//     {
//         if( *mem != *copy )
//         {
//             if( !prevdiff )
//             {
//                 ph_printf(", d@ 0x%04x", mem - start );
//             }
//             prevdiff = prevdiff ? 2 : 1;
//             *copy = *mem;
//         }
//         else
//         {
//             if( prevdiff == 2 )
//             {
//                 ph_printf( "-%04x", mem - start -1 );
//                 prevdiff = 0;
//             }
//         }
//         mem++;
//         copy++;
//     }

//     ph_printf(" Press Enter...");
//     getchar();
// #endif
// }

void event_q_put_global(ui_event_t *e) {}

void event_q_put_any(ui_event_t *e) {}

void event_q_put_win(int x, int y, int info, struct drv_video_window *focus) {}

int drv_video_window_get_event(drv_video_window_t *w, struct ui_event *e, int wait)
{
	ph_printf("\nGetEvent!?\n");
	w->events_count--;
	assert(!wait);
	return 0;
}

// int hal_save_cli() { return 1; }
// void hal_sti() {}
// void hal_cli() {}

struct wtty *get_thread_ctty(struct phantom_thread *t)
{
	return 0;
}
int GET_CPU_ID()
{
	return 0;
}

void hal_cpu_reset_real()
{

	ph_printf("ERROR : Supposed to restart!");
	hal_sleep_msec(1000000);
}

// errno_t phantom_connect_object( struct data_area_4_connection *da, struct
// data_area_4_thread *tc) { return ENOMEM; } errno_t phantom_disconnect_object( struct
// data_area_4_connection *da ) { return ENOMEM; } errno_t
// phantom_connect_object_internal(struct data_area_4_connection *da, int connect_type,
// pvm_object_t host_object, void *arg) { return 0; }

// -----------------------------------------------------------------------
// debug_ext.h support
// -----------------------------------------------------------------------

void console_set_error_color()
{
	ph_printf("\x1b[31m");
}
void console_set_normal_color()
{
	ph_printf("\x1b[37m");
}
void console_set_message_color(void)
{
	ph_printf("\x1b[34m");
}
void console_set_warning_color(void)
{
	ph_printf("\x1b[33m");
}

int debug_max_level_error = 255;
int debug_max_level_info = 255;
int debug_max_level_flow = 255;

void phantom_check_threads_pass_bytecode_instr_boundary(void)
{
	ph_printf("!phantom_check_threads_pass_bytecode_instr_boundary unimpl!\n");
}

void console_set_fg_color(struct rgba_t c) {}

// void vm_map_page_mark_unused( addr_t page_start)
// {
//     //ph_printf("asked to mark page unused\n");
// }

#warning stub
struct _key_event;
void phantom_dev_keyboard_get_key(struct _key_event *out)
{
	while (1)
		hal_sleep_msec(10000);
}

void phantom_set_console_getchar(int (*_getchar_impl)(void)) {}

// -----------------------------------------------------------
// output
// -----------------------------------------------------------

#if 0

void debug_console_putc(int c)
{
    if( kout_f ) fputc( c, kout_f );
    else ph_putchar(c);
}

#include <kernel/debug.h>


// Print to log file only
void lprintf(char const *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    if( klog_f )
        vfprintf( klog_f, fmt, ap);
    else
        ph_vprintf(fmt, ap);
    va_end(ap);
}

#endif

// -----------------------------------------------------------
// -----------------------------------------------------------

// known call: int set_net_timer( void ) //&e, 10000, stat_update_persistent_storage, 0, 0
// );

int set_net_timer(net_timer_event *e,
				  unsigned int delay_ms,
				  net_timer_callback callback,
				  void *args,
				  int flags)
{
	(void)e;
	(void)delay_ms;
	(void)callback;
	(void)args;
	(void)flags;

	// panic("set_net_timer");
	// lprintf("set_net_timer called, backtrace (\"gdb bt\") me\n");

	return -1;  // ERR_GENERAL - todo - errno_t
}

static int dummy_snap_catch;

volatile int *snap_catch_va = &dummy_snap_catch;

#include <exceptions.h>

// void check_global_lock_entry_count(void) {}

// void vm_lock_persistent_memory() {}
// void vm_unlock_persistent_memory() {}
