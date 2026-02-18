// #include <kernel/net.h>
#include "genode_misc.h"

#include <stddef.h>
#include <sys/errno.h>

errno_t net_curl(const char *url, char *obuf, size_t obufsize, const char *headers)
{
	_stub_print();
	return 0;
}

const char *http_skip_header(const char *buf)
{
	_stub_print();
	return buf;
}