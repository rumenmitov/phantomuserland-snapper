/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2010 Dmitry Zavalishin, dz@dz.ru
 *
 * Translate trap to signal. If no one handling - kill the thread.
 *
 *
 **/

#include "genode_misc.h"

// #include <kernel/trap.h>
// #include <kernel/interrupts.h>
// #include <phantom_assert.h>
// #include <phantom_libc.h>
// #include <threads.h>
// #include <thread_private.h>
// #include "misc.h"

// #if ARCH_ia32
// #  include <ia32/seg.h>
// #endif


//! This is what called from low-level asm trap code
void phantom_kernel_trap(struct trap_state *ts)
{
	_stub_print();
}


// returns nonzero if handled
int phantom_check_user_trap(struct trap_state *ts)
{
	// _stub_print();
	return 1;  // Return 1 to say that everything is ok
}

int trap_panic(struct trap_state *ts)
{
	_stub_print();
}

void dump_ss(struct trap_state *st)
{
	_stub_print();
}