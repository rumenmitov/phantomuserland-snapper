#ifndef KERNEL_LIBKERN
#define KERNEL_LIBKERN
// Not that good header
// #include <sys/libkern.h>

// Used in pool.c (don't know why though)
static __inline unsigned int umin(unsigned int a, unsigned int b)
{
	return (a < b ? a : b);
}

// Used in libwin in rect_cmp.c
static __inline int imax(int a, int b)
{
	return (a > b ? a : b);
}
static __inline int imin(int a, int b)
{
	return (a < b ? a : b);
}

// Used in libwin in rgba_scroll and plenty of places
static __inline int abs(int a)
{
	return (a < 0 ? -a : a);
}
// static __inline long labs(long a) { return (a < 0 ? -a : a); }
// static __inline quad_t qabs(quad_t a) { return (a < 0 ? -a : a); }


#endif