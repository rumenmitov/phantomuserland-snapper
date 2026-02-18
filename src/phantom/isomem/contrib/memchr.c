/*
** Copyright 2001, Manuel J. Petit. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <phantom_types.h>

void *ph_memchr(void const *ptr, int ch, size_t count)
{
	size_t i;
	unsigned char const *b = ptr;
	unsigned char x = ch & 0xff;

	for (i = 0; i < count; i++) {
		if (b[i] == x)
			return (void *)(b + i);
	}

	return 0;
}
