#ifndef _GENODE_MISC_H
#define _GENODE_MISC_H

// // #include <stdio.h>

// TODO : Stack

// inline void _stub_print();
void _stub_print_func(const char *func, const char *file, int line);

#define _stub_print() _stub_print_func(__FUNCTION__, __FILE__, __LINE__);

#endif
