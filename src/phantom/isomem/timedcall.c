/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2010 Dmitry Zavalishin, dz@dz.ru
 *
 * Call function from timer interrupt.
 * Delay is in msec.
 *
 *
 **/

#include <kernel/timedcall.h>

#define DEBUG_MSG_PREFIX "timer"
#include "debug_ext.h"
#define debug_level_flow  0
#define debug_level_error 10
#define debug_level_info  10

#include "genode_misc.h"

void phantom_timed_call_init(void)
{
	_stub_print();
}

void phantom_process_timed_calls(void)
{
	_stub_print();
}

void phantom_request_timed_call(timedcall_t *entry, u_int32_t flags)
{
	(void)entry;
	(void)flags;

	_stub_print();
}

void phantom_undo_timed_call(timedcall_t *entry)
{
	(void)entry;

	_stub_print();
}
