/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2017 Dmitry Zavalishin, dz@dz.ru
 *
 * Tests - misc stuff, which has no personal test case:
 *
 *  - sprintf (varargs)
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
#include <kernel/config.h>
#include <ph_string.h>
#include <phantom_assert.h>
#include <phantom_libc.h>

// includes needed for stuff we test come after


int do_test_misc(const char *test_parm)
{
	(void)test_parm;

	const char *fmt = "test print %s and %d and %c and %ld";
	const char *out = "test print s and 1 and a and 50000000";

	{
		SHOW_FLOW0(1, "printf/vararg");

		char buf[1024];

		ph_snprintf(buf, sizeof(buf), fmt, "s", 1, 'a', 50000000l);

		test_check_false(ph_strcmp(out, buf));
	}


	return 0;
}
