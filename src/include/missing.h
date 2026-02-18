#ifndef MISSING_GENODE_H
#define MISSING_GENODE_H

#ifdef GENODE_PHANTOM

typedef struct signal_handling
{
	u_int32_t signal_pending;
	u_int32_t signal_mask;  // 0 for ignored

	// unimpl
	u_int32_t signal_stop;  // 1 = stop process
	u_int32_t signal_cont;  // 1 = resume process
	// u_int32_t           signal_kill; // 1 = kill process

	void *signal_handler[NSIGNAL];  // 0 = kill

} signal_handling_t;

#endif

#endif