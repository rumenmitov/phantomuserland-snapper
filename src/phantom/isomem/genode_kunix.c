#include "genode_misc.h"

#include <kunix.h>

errno_t k_open(int *fd, const char *name, int flags, int mode)
{
	_stub_print();
	return 0;
}

errno_t k_read(int *nread, int fd, void *addr, int count)
{
	_stub_print();
	return 0;
}

errno_t k_write(int *nwritten, int fd, const void *addr, int count)
{
	_stub_print();
	return 0;
}

errno_t k_seek(int *pos, int fd, int offset, int whence)
{
	_stub_print();
	return 0;
}

errno_t k_stat(const char *path, struct stat *data, int statlink)
{
	_stub_print();
	return 0;
}

// just get file size
errno_t k_stat_size(const char *path, unsigned int *size)
{
	_stub_print();
	return 0;
}

errno_t k_close(int fd)
{
	_stub_print();
	return 0;
}
