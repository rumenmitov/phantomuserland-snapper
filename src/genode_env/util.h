/*
 * Some miscellaneous functions used across the project
 */
#ifndef GENODE_PHANTOM_UTIL_H
#define GENODE_PHANTOM_UTIL_H

extern "C"
{
#include <phantom_types.h>

	long ph_strtol(const char *nptr, char **endptr, int base);
	int ph_putchar(char c);
	void *ph_memcpy(void *dst0, const void *src0, size_t length);
	void *ph_memmove(void *dst0, const void *src0, size_t length);
	int ph_memcmp(const void *s1v, const void *s2v, size_t size);
	void *ph_memset(void *dst0, int c0, size_t length);
	long ph_strtol(const char *nptr, char **endptr, int base);
	long long ph_strtoq(const char *nptr, char **endptr, int base);
	unsigned long long ph_strtouq(const char *nptr, char **endptr, int base);

	// Math functions
	double ph_pow(double, double);
}

#endif  // GENODE_PHANTOM_UTIL_H
