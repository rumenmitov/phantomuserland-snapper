// Functions required by several system calls (strings and binaries)
// TODO : Built-in gcc

#include <ph_string.h>
#include <phantom_libc.h>
#include <phantom_types.h>
#include <stdlib.h>


long ph_atoln(const char *str, size_t n)
{
	char buf[80];

	if (n > (sizeof(buf) - 1))
		n = sizeof(buf) - 1;

	ph_strlcpy(buf, str, n);

	return (ph_strtol(buf, (char **)0, 10));
}

int ph_atoin(const char *str, size_t n)
{
	return (int)ph_atoln(str, n);
}


char *ph_strnstrn(char const *s1, int l1, char const *s2, int l2)
{
	// int l1, l2;

	// l2 = ph_strlen(s2);
	if (!l2)
		return (char *)s1;
	// l1 = ph_strlen(s1);
	while (l1 >= l2) {
		l1--;
		if (!ph_memcmp(s1, s2, l2))
			return (char *)s1;
		s1++;
	}
	return 0;
}
