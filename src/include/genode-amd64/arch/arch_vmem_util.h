int genode_register_page_fault_handler(
	int (*handler)(void *address, int write, int ip, struct trap_state *ts));
