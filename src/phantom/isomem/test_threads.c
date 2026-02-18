/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2010 Dmitry Zavalishin, dz@dz.ru
 *
 * Tests - threads, sync
 *
 *
 **/

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
#include <ph_random.h>
#include <phantom_libc.h>
#include <thread_private.h>
#include <threads.h>
#include <vm/alloc.h>
#include <vm/object.h>
#include <vm/object_flags.h>

#define TEST_CHATTY  0
#define TEST_SOFTIRQ 0


static volatile int thread_activity_counter = 0;
static volatile int thread_stop_request = 0;

static hal_cond_t c;
static hal_mutex_t m;
static hal_sem_t s;

// static void YIELD();
// #define YIELD() hal_sleep_msec(400)
#define YIELD() hal_sleep_msec(40)

// #define FINISH() do { hal_sleep_msec(1000); } while(1)
#define FINISH()


static void pressEnter(char *text)
{
	if (TEST_CHATTY)
		ph_printf("%s\n", text);
}

static int inmutex = 0;
static void checkEnterMutex()
{
	inmutex++;
	if (inmutex > 1) {
		SHOW_ERROR0(0, "mutex reentered");
		test_fail(-1);
	}
}

static void checkLeaveMutex()
{
	inmutex--;
}

// Need PHANTOM_OBJECT_STORAGE_FLAG_IS_INTERNAL so that ref_dec won't descent
pvm_object_storage_t p = {
	/*
		._ah.object_start_marker = PVM_OBJECT_START_MARKER,
		._ah.refCount = 1,
		._ah.alloc_flags =
	   PVM_OBJECT_AH_ALLOCATOR_FLAG_ALLOCATED|PHANTOM_OBJECT_STORAGE_FLAG_IS_INTERNAL,

		._ah.gc_flags = 0,
		._ah.exact_size = 0, // Ok for refcount test :)
	*/
	._ah =
		{
			.object_start_marker = PVM_OBJECT_START_MARKER,
			.refCount = 1,
			.alloc_flags = PVM_OBJECT_AH_ALLOCATOR_FLAG_ALLOCATED |
						   PHANTOM_OBJECT_STORAGE_FLAG_IS_INTERNAL,

			.gc_flags = 0,
			.exact_size = 0,  // Ok for refcount test :)
		},

	._class = 0,
	._satellites = 0,
	._flags = 0,
	._da_size = 0

};

static int n_t_empty = 0;

static void t_empty(void *a)
{
	(void)a;

	ref_inc_p(&p);

	hal_sleep_msec(ph_random() % 6000);
	// TODO do some random things, such as sleep, set timer, etc

	ref_inc_p(&p);
	ref_inc_p(&p);
	ref_inc_p(&p);

	ref_dec_p(&p);
	ref_dec_p(&p);
	ref_dec_p(&p);
	ref_dec_p(&p);

	n_t_empty--;
}


static void t_wait(void *a)
{
	t_current_set_priority(THREAD_PRIO_HIGH);

	char *name = a;
	while (!thread_stop_request) {
		thread_activity_counter++;

		if (TEST_CHATTY)
			ph_printf("--- thread %s will wait 4 cond ---\n", name);

		hal_mutex_lock(&m);
		checkEnterMutex();
		checkLeaveMutex();
		hal_cond_wait(&c, &m);
		checkEnterMutex();
		checkLeaveMutex();
		hal_mutex_unlock(&m);

		if (TEST_CHATTY)
			ph_printf("--- thread %s runs ---\n", name);
		// pressEnter("--- thread a runs ---\n");
		YIELD();
	}
	FINISH();
}


static int counter = 0;
static void thread1(void *a)
{
	char *name = a;
	while (!thread_stop_request) {
		thread_activity_counter++;

		if (TEST_CHATTY)
			ph_printf("--- thread %s runs ---\n", name);
		pressEnter("");

		if (TEST_CHATTY)
			ph_printf("Will lock mutex\n");
		hal_mutex_lock(&m);
		if (TEST_CHATTY)
			ph_printf("locked mutex\n");
		checkEnterMutex();
		YIELD();
		if (TEST_CHATTY)
			ph_printf("Will unlock mutex\n");
		checkLeaveMutex();
		hal_mutex_unlock(&m);
		if (TEST_CHATTY)
			ph_printf("unlocked mutex\n");

		if (ph_random() & 1)
			hal_sem_acquire(&s);

		counter++;
		if (counter > 7) {
			counter = 0;
			if (TEST_CHATTY)
				ph_printf("Will signal cond\n");
			hal_cond_signal(&c);
			if (TEST_CHATTY)
				ph_printf("Signalled cond\n");
		}
		YIELD();
	}
	FINISH();
}

/*
static void t_sem_signal(void *a)
{
	(void) a;
	while( !thread_stop_request )
	{
		hal_sleep_msec( 350 );
		hal_sem_release( &s );
	}
}
*/

static errno_t threads_test()
{

	hal_cond_init(&c, "threadTest");
	hal_mutex_init(&m, "threadTest");
	hal_sem_init(&s, "threadTest");

	int i = 40;
	n_t_empty = i;
	while (i-- > 0)
		phantom_create_thread(t_empty, "Empty", 0);

	pressEnter("will create thread");
	phantom_create_thread(thread1, "__T1__", 0);
	phantom_create_thread(thread1, "__T2__", 0);
	// phantom_create_thread( thread1, "__T3__" );

	// phantom_create_thread( t_wait, "__TW__" );
	int tid = hal_start_kernel_thread_arg(t_wait, "__TW__");

	i = 40;
	while (i-- > 0) {
		if (TEST_CHATTY)
			pressEnter("will yield");
		YIELD();

		if (TEST_CHATTY)
			ph_printf("!! back in main\n");
	}

	t_kill_thread(tid);
	hal_sleep_msec(30);

	thread_stop_request = 1;
	hal_sleep_msec(10);

	thread_activity_counter = 0;
	hal_sleep_msec(1000);
	if (thread_activity_counter) {
		SHOW_ERROR0(0, "Can't stop thread");
		return -1;
	}

	while (n_t_empty > 0) {
		SHOW_FLOW(0, "wait for %d threads", n_t_empty);
		hal_sleep_msec(500);
	}

	if (p._ah.refCount != 1) {
		SHOW_ERROR(0, "p._ah.refCount = %d", p._ah.refCount);
		test_fail_msg(-1, "refcount");
	} else
		SHOW_ERROR(0, "p._ah.refCount = %d, SUCCESS", p._ah.refCount);

	return 0;
}

// TODO check that we passed after cond wait on permission
// TODO timeouts, incl manual awake of timed cond and sema

int do_test_threads(const char *test_parm)
{
	(void)test_parm;

	return threads_test();
}


// -----------------------------------------------------------------------
// Semaphores
// -----------------------------------------------------------------------


static hal_sem_t test_sem_0;
static volatile int stop_sem_test = 0;
static volatile int sem_released = 0;
static int softirq = -1;
static int rc = -1;

static void sem_rel(void *a)
{
	(void)a;
	// ph_printf("sema pre-release 1 (direct)\n");
	hal_sleep_msec(300);
	// ph_printf("sema release 1 (direct)\n");
	sem_released = 1;
	hal_sem_release(&test_sem_0);

#if TEST_SOFTIRQ
	hal_sleep_msec(300);
	ph_printf("sema release 2 (softirq %d)\n", softirq);
	sem_released = 1;
	hal_request_softirq(softirq);
#endif
	/*
	while(!stop_sem_test)
	{
		hal_sleep_msec( 500 );
	}

	*/
}


// static void sem_etc(void *a)
// {
//     (void) a;
//     rc = hal_sem_acquire_etc( &test_sem_0, 1, SEM_FLAG_TIMEOUT, 1000L*200L );
//     sem_released = 1;
// }


static void sem_softirq(void *a)
{
	(void)a;
	// can't print from irq
	// ph_printf("sema softirq\n");
	// hal_sleep_msec( 10 );
	sem_released = 1;
	hal_sem_release(&test_sem_0);
}


// Cleanup after failed test
static void sem_test_fail(void *arg)
{
	(void)arg;
	hal_sem_destroy(&test_sem_0);
}

int do_test_sem(const char *test_parm)
{
	(void)test_parm;
	ph_printf("Testing semaphores\n");

	hal_sem_init(&test_sem_0, "semTest");
	on_fail_call(sem_test_fail, 0);

	// if( softirq < 0 )
	// {
	//     // Do it once
	//     softirq = hal_alloc_softirq();
	//     if( softirq < 0 )
	//         test_fail_msg( 1, "Unable to get softirq" );
	//     else
	//         hal_set_softirq_handler( softirq, sem_softirq, 0 );
	// }

	// int tid =
	hal_start_kernel_thread_arg(sem_rel, 0);

	ph_printf("sema wait 1\n");

	// Direct

	// XXX : Strange behavior. It will not be unlocked and
	//       sem_rel thread will be stuck if to remove this sleep
	// TODO : Fix
	hal_sleep_msec(500);

	hal_sem_acquire(&test_sem_0);
	test_check_eq(sem_released, 1);

	hal_sleep_msec(100);

	ph_printf("sema wait 2\n");

#if TEST_SOFTIRQ
	// Softirq
	sem_released = 0;
	hal_sem_acquire(&test_sem_0);
	test_check_eq(sem_released, 1);
#endif

	hal_sleep_msec(100);


	// ph_printf("sema timeout\n");
	// sem_released = 0;
	// hal_start_kernel_thread_arg( sem_etc, 0 );
	// hal_sleep_msec( 100 );
	// test_check_eq(sem_released,0);
	// hal_sleep_msec( 120 );
	// if( !sem_released ) // give extra time
	//     hal_sleep_msec( 150 );
	// test_check_eq(sem_released,1);
	// test_check_eq( rc, ETIMEDOUT);


	stop_sem_test = 1;

	ph_printf("Done testing semaphores\n");

	hal_sem_destroy(&test_sem_0);
	return 0;
}


// -----------------------------------------------------------------------
// Thread state intact
// -----------------------------------------------------------------------

static volatile int zo_run = 2;
static volatile int zo_fail = 0;

#define zo_check_eq(a, b) \
	if ((a) != (b))       \
	zo_fail = 1


static void thread_ones(void *a)
{
	(void)a;

	register int a01 = 1;
	register int a02 = 1;
	register int a03 = 1;
	register int a04 = 1;
	register int a05 = 1;
	register int a06 = 1;
	register int a07 = 1;
	register int a08 = 1;
	register int a09 = 1;
	register int a10 = 1;


	int c = 2000;
	while (c-- > 0) {
		hal_sleep_msec(1);

		zo_check_eq(a01, 1);
		zo_check_eq(a02, 1);
		zo_check_eq(a03, 1);
		zo_check_eq(a04, 1);
		zo_check_eq(a05, 1);
		zo_check_eq(a06, 1);
		zo_check_eq(a07, 1);
		zo_check_eq(a08, 1);
		zo_check_eq(a09, 1);
		zo_check_eq(a10, 1);
	}

	zo_run--;

	hal_sleep_msec(10000);
}


static void thread_zeroes(void *a)
{
	(void)a;

	register int a01 = 0;
	register int a02 = 0;
	register int a03 = 0;
	register int a04 = 0;
	register int a05 = 0;
	register int a06 = 0;
	register int a07 = 0;
	register int a08 = 0;
	register int a09 = 0;
	register int a10 = 0;


	int c = 2000;
	while (c-- > 0) {
		hal_sleep_msec(1);

		zo_check_eq(a01, 0);
		zo_check_eq(a02, 0);
		zo_check_eq(a03, 0);
		zo_check_eq(a04, 0);
		zo_check_eq(a05, 0);
		zo_check_eq(a06, 0);
		zo_check_eq(a07, 0);
		zo_check_eq(a08, 0);
		zo_check_eq(a09, 0);
		zo_check_eq(a10, 0);
	}

	zo_run--;
	hal_sleep_msec(10000);
}


int do_test_01_threads(const char *test_parm)
{
	(void)test_parm;
	ph_printf("Testing thread state integrity\n");

	hal_start_kernel_thread_arg(thread_ones, 0);
	hal_start_kernel_thread_arg(thread_zeroes, 0);

	ph_printf("Wait for 01 threads to finish\n");

	while (zo_run)
		hal_sleep_msec(100);


	if (zo_fail)
		test_fail_msg(-1, "data corruption");

	return 0;
}


// -----------------------------------------------------------------------
// Running a lot of threads
// -----------------------------------------------------------------------


static int still_have_threads = 0;

// TODO crashes strangely on 1000
// #define TMANY_HOW_MUCH 1000
// #define TMANY_HOW_MUCH 200
#define TMANY_HOW_MUCH 100

static void simple_thread(void *a)
{
	(void)a;

	static int tc = 0;
	tc++;
	char tn[100];
	ph_snprintf(tn, sizeof(tn), "test t %d", tc);
	t_current_set_name(tn);

	hal_sleep_msec((ph_random() % 100) + 2000);
	still_have_threads--;
}

int do_test_many_threads(const char *test_parm)
{
	(void)test_parm;

	int tostart = TMANY_HOW_MUCH;

	while (tostart-- > 0) {
		still_have_threads++;
		hal_start_kernel_thread_arg(simple_thread, 0);
		ph_printf("Have %d threads run\n", still_have_threads);
	}

	ph_printf("Wait for threads to finish\n");

	while (still_have_threads > 0) {
		hal_sleep_msec(1000);
		ph_printf("Still have %d threads run\n", still_have_threads);
	}

	return 0;
}
