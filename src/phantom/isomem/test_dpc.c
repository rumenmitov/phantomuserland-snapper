
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
#include <ph_string.h>
#include <phantom_libc.h>
#include <thread_private.h>
#include <threads.h>
#include <vm/alloc.h>
#include <vm/object.h>
#include <vm/object_flags.h>

#define TEST_CHATTY  0
#define TEST_SOFTIRQ 1


// TODO need more sleeping DPCs to test that DPC engine runs additional threads as needed

// -----------------------------------------------------------------------
// DPC
// -----------------------------------------------------------------------


#include <kernel/dpc.h>


static dpc_request dpc1;
static dpc_request dpc2;

#define DPC_ARG1 "xyz_1"
#define DPC_ARG2 "2_xyz"

static int dpc1_triggered = 0;
static int dpc2_triggered = 0;

static void dpc_serve1(void *arg)
{
	if (ph_strcmp(arg, DPC_ARG1)) {
		SHOW_ERROR0(0, "DPC 1 arg is wrong");
		test_fail(-1);
	}

	dpc1_triggered = 1;
	hal_sleep_msec(2000);
}

static void dpc_serve2(void *arg)
{
	if (ph_strcmp(arg, DPC_ARG2)) {
		SHOW_ERROR0(0, "DPC 2 arg is wrong");
		test_fail(-1);
	}
	dpc2_triggered = 1;
}

int do_test_dpc(const char *test_parm)
{
	(void)test_parm;

	int rc = 0;

	dpc_request_init(&dpc1, dpc_serve1);
	dpc_request_init(&dpc2, dpc_serve2);

	// DPC engine runs next thread only after a second
	hal_sleep_msec(2000);


	dpc_request_trigger(&dpc1, DPC_ARG1);
	hal_sleep_msec(200);
	dpc_request_trigger(&dpc2, DPC_ARG2);
	hal_sleep_msec(200);

	if (!dpc1_triggered) {
		rc = -1;
		SHOW_ERROR0(0, "DPC 1 lost");
	}

	if (!dpc2_triggered) {
		rc = -1;
		SHOW_ERROR0(0, "DPC 2 lost");
	}

	dpc1_triggered = 0;
	dpc2_triggered = 0;

	hal_sleep_msec(200);

	if (dpc1_triggered) {
		rc = -1;
		SHOW_ERROR0(0, "DPC 1 sporadic");
	}

	if (dpc2_triggered) {
		rc = -1;
		SHOW_ERROR0(0, "DPC 2 sporadic");
	}

	dpc_request_trigger(&dpc2, DPC_ARG2);

	hal_sleep_msec(200);

	if (dpc1_triggered) {
		rc = -1;
		SHOW_ERROR0(0, "DPC 1 sporadic after DPC 2 trigger");
	}

	if (!dpc2_triggered) {
		rc = -1;
		SHOW_ERROR0(0, "DPC 2 lost 2");
	}

	return rc;
}
