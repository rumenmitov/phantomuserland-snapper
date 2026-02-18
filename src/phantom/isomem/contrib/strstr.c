/*
** Copyright 2001, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <ph_string.h>

char *ph_strstr(char const *s1, char const *s2)
{
	int l1, l2;

	l2 = ph_strlen(s2);
	if (!l2)
		return (char *)s1;
	l1 = ph_strlen(s1);
	while (l1 >= l2) {
		l1--;
		if (!ph_memcmp(s1, s2, l2))
			return (char *)s1;
		s1++;
	}
	return 0;
}
