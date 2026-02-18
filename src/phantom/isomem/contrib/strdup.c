/*
** Copyright 2004, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <ph_malloc.h>
#include <ph_string.h>
#include <phantom_types.h>

char *ph_strdup(const char *str)
{
	size_t len;
	char *copy;

	len = ph_strlen(str) + 1;
	copy = ph_malloc(len);
	if (copy == 0)
		return 0;
	ph_memcpy(copy, str, len);
	return copy;
}


char *ph_strndup(const char *str, size_t n)
{
	size_t len;
	char *copy;

	len = ph_strlen(str) + 1;

	n++;
	if (len > n)
		len = n;


	copy = ph_malloc(len);
	if (copy == 0)
		return 0;

	ph_memcpy(copy, str, len);
	copy[len - 1] = '\0';

	return copy;
}
