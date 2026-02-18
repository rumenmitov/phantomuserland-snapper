
#define DEBUG_MSG_PREFIX "test"
#include "debug_ext.h"
#define debug_level_flow  10
#define debug_level_error 10
#define debug_level_info  10

#include "test.h"

#include <errno.h>
#include <hal.h>
#include <kernel/config.h>
#include <kernel/timedcall.h>
#include <phantom_libc.h>
#include <thread_private.h>
#include <threads.h>
#include <vm/alloc.h>
#include <vm/object.h>
#include <vm/object_flags.h>

#define TEST_CHATTY  0
#define TEST_SOFTIRQ 1


// -----------------------------------------------------------------------
// Timed calls
// -----------------------------------------------------------------------


#define DUMPQ 0


static volatile int called = 0;


// static char *msg = "timed func 5000";
static void echo(void *_a)
{
	called++;
	ph_printf("Echo: '%s'\n", (char *)_a);
}


static timedcall_t t1 = {echo, "hello 5", 5, 0, 0, {0, 0}, 0};
static timedcall_t t2 = {echo, "hello 100", 100, 0, 0, {0, 0}, 0};
static timedcall_t t3 = {echo, "hello 2000", 2000, 0, 0, {0, 0}, 0};
static timedcall_t t4 = {echo, "hello 10 000", 10000, 0, 0, {0, 0}, 0};
static timedcall_t t5 = {echo, "hello  5 000", 5000, 0, 0, {0, 0}, 0};


int do_test_timed_call(const char *test_parm)
{
	(void)test_parm;


	ph_printf("Testing timed call undo, must be no echoes:\n");
	called = 0;

	phantom_request_timed_call(&t2, 0);
	phantom_undo_timed_call(&t2);

	hal_sleep_msec(200);  // Twice the time
	test_check_eq(called, 0);


#if DUMPQ
	// dump_timed_call_queue();
#endif


	called = 0;

	ph_printf("Testing timed calls, wait for echoes:\n");

	phantom_request_timed_call(&t1, 0);
	// test_check_false(called); // it is still possible for this test to fail with
	// correct code!
	phantom_request_timed_call(&t2, 0);
#if DUMPQ
	// dump_timed_call_queue();
#endif
	phantom_request_timed_call(&t3, 0);
	phantom_request_timed_call(&t4, 0);

	// TIMEDCALL_FLAG_AUTOFREE requires call to free from interrupt :(
	// phantom_request_timed_func( echo, msg, 5000, 0 );
	phantom_request_timed_call(&t5, 0);

#if DUMPQ
	dump_timed_call_queue();
#endif

	// We check for >= (ge) because hal_sleep... can cleep for more than asked, and timed
	// calls are usually quite on time

	hal_sleep_msec(6);
	test_check_ge(called, 1);

	hal_sleep_msec(200);  // Have lag of 106 msec, OK?
	test_check_ge(called, 2);

	hal_sleep_msec(2000 - 100);  // Still have lag of 106 msec
	test_check_ge(called, 3);

	hal_sleep_msec(5000 - 2000);  // Have lag of 206 msec
	test_check_ge(called, 4);

	hal_sleep_msec(2500 + 10000 - 5000 - 2000);  // Strange - need quite big lag here...
	test_check_ge(called, 5);


	ph_printf("Done testing timed calls\n");
	return 0;
}
