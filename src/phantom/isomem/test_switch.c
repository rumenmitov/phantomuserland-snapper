/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2010 Dmitry Zavalishin, dz@dz.ru
 *
 * Test suit test selector
 *
 * XXX: setjmp and longjmp are commented out for a while
 *
 *
 **/


#define DEBUG_MSG_PREFIX "test"
#include "debug_ext.h"
#define debug_level_flow  6
#define debug_level_error 10
#define debug_level_info  10


#include <errno.h>
#include <kernel/config.h>
#include <phantom_libc.h>
// #include <setjmp.h>
#include "misc.h"
#include "svn_version.h"
#include "test.h"

#include <hal.h>
#include <kernel/init.h>
#include <ph_string.h>
#include <threads.h>

// Prototypes of functions implementing tests
int do_test_ps2_mouse();
int do_test_vm_map();


static void (*fhandler_f)(void *arg);
static void *fhandler_arg;

void on_fail_call(void (*f)(void *arg), void *arg)  // Call f on failure
{
	fhandler_f = f;
	fhandler_arg = arg;
}

// static jmp_buf jb;
static int nFailed = 0;

void test_fail(errno_t rc)
{
	ph_printf("Test fail!\n");
	// longjmp( jb, rc );
}

void test_fail_msg(errno_t rc, const char *msg)
{
	ph_printf("Test fail: %s\n", msg);
	// longjmp( jb, rc );
}


/**
 *
 * Each test must report into the stdout one of:
 *
 * FAILED
 * PASSED
 * SKIPPED - this usually follows by PASSED, though.
 *
 **/


#ifdef PHANTOM_GENODE


#define TEST(name)                                         \
	({                                                     \
		fhandler_f = 0;                                    \
		{                                                  \
			if (all || (0 == ph_strcmp(test_name, #name))) \
				report(do_test_##name(test_parm), #name);  \
		}                                                  \
	})

#else

#define TEST(name)                                         \
	({                                                     \
		fhandler_f = 0;                                    \
		int rc;                                            \
		if ((rc = setjmp(jb))) {                           \
			report(rc, #name);                             \
		} else {                                           \
			if (all || (0 == ph_strcmp(test_name, #name))) \
				report(do_test_##name(test_parm), #name);  \
		}                                                  \
	})

#endif

void report(int rc, const char *test_name)
{
	if (!rc) {
		// CI: this message is being watched by CI scirpts (ci-runtest.sh)
		ph_printf("KERNEL TEST PASSED: %s\n", test_name);
		return;
	}

	char rcs[128];
	// XXX : Need to reimplement somewhere
	// ph_strerror_r(rc, rcs, sizeof(rcs));

	nFailed++;
	// CI: this message is being watched by CI scirpts (ci-runtest.sh)
	ph_printf("!!! KERNEL TEST FAILED: %s -> %d (%s)\n", test_name, rc, rcs);

	if (fhandler_f) {
		ph_printf("Post mortem:\n");
		fhandler_f(fhandler_arg);
	}
}


void run_test(const char *test_name, const char *test_parm)
{
	int all = 0 == ph_strcmp(test_name, "all");
	// int i;

#ifndef ARCH_ia32
	ph_printf("sleeping 2 sec\n");
	hal_sleep_msec(2000);
#endif

	ph_printf("Phantom ver %s svn %s test suite\n-----\n",
			  PHANTOM_VERSION_STR,
			  svn_version());

	TEST(hdir);

	TEST(misc);

	// TEST(crypt);

	// moved here to test first - rewritten
	// TEST(ports);


	// TODO mem leak! out of mem for 200!
	// for( i = 200; i; i-- )
	/* //   TODO : Uncomment
	{
		TEST(sem);
	}
	*/

	TEST(sem);

	// TEST(wtty);

	// TEST(pool);

	// check if starting many threads eats memory
	// for( i = 200; i; i-- )
	/* //   TODO : Uncomment
	{
		TEST(many_threads);
	}
	*/

#ifdef ARCH_mips
	//    TEST(sem);
	TEST(01_threads);
#endif

	TEST(physmem);
	// TEST(physalloc_gen);
	TEST(malloc);
	TEST(amap);
	/* //   TODO : Uncomment
	TEST(cbuf);
	*/
	// TEST(udp_send);
	// TEST(udp_syslog);
	// TEST(resolver);

	// TEST(tftp);


	// TEST(tcp_connect);


	// These are long
	// TEST(dpc);

	// XXX: timed calls are disabled for now
	//      they are mostly used in sync primitives,
	//      but we have ones from Genode
	// TEST(timed_call);


	// must test after timed calls for it depends on them
	// XXX : Seem to be using deprecated API
	// TEST(ports);


	// These are very long, do 'em last
	// TEST(threads);

	/* //   TODO : Uncomment
	TEST(absname);
	*/

#ifndef ARCH_ia32
//    TEST(sem);
// TEST(01_threads);
#endif

	TEST(rectangles);
	// TODO : Fix. Failing with seg fault
	// TEST(video);


	// TEST(video);

	// XXX : Uses ports, and not much things using them. Therefore disabled
	// TEST(userland);


	TEST(ps2_mouse);

	TEST(vm_map);

	ph_printf("\n-----\n");
	if (nFailed)
		ph_printf("some tests FAILED\n");
	else
		ph_printf("all tests PASSED\n");

	// CI: this message is being watched by CI scripts (ci-runtest.sh)
	ph_printf("-----\nPhantom test suite FINISHED\n-----\n");
}
