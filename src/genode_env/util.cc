#include "util.h"

#include <base/log.h>
#include <util/string.h>

#include <stddef.h>

extern "C"
{
#include <ph_io.h>
#include <ph_random.h>
#include <ph_setjmp.h>
#include <stdarg.h>

	// XXX : It is a buffered output. Buffer will be flush on new line or when full.
	//       Used mainly in printf.

	static const unsigned int buff_size = 256;
	static char log_buff[buff_size];
	static unsigned int log_buff_cnt = 0;

	int ph_putchar(char c)
	{
		// flushing if full or new line
		if (c == '\n' || log_buff_cnt == buff_size - 1) {
			log_buff[log_buff_cnt] = '\0';
			Genode::log(Genode::Cstring(log_buff));
			log_buff_cnt = 0;

			if (log_buff_cnt == buff_size - 1) {
				log_buff_cnt = 0;
				log_buff[0] = c;
			}
		} else {
			log_buff[log_buff_cnt] = c;
			log_buff_cnt++;
		}
		return 0;
	}

	void *ph_memcpy(void *dst0, const void *src0, size_t length)
	{
		return Genode::memcpy(dst0, src0, length);
	}

	void *ph_memmove(void *dst0, const void *src0, size_t length)
	{
		return Genode::memmove(dst0, src0, length);
	}

	int ph_memcmp(const void *s1v, const void *s2v, size_t size)
	{
		return Genode::memcmp(s1v, s2v, size);
	}

	void *ph_memset(void *dst0, int c0, size_t length)
	{
		return Genode::memset(dst0, c0, length);
	}

	void bzero(void *dst0, size_t length)
	{
		ph_memset(dst0, 0x0, length);
	}

	char *ph_strrchr(const char *p, int ch)
	{
		char *res = (char *)p;
		(void)ch;
#warning Unimplemented function: ph_strrchr
		Genode::warning("Unimplemented function ph_strrchr()!");
		return res;
	}

	int ph_atoi(const char *nptr)
	{
		int res = 0;

		Genode::ascii_to_signed<int>(nptr, res);

		return res;
	}

	long ph_atol(const char *nptr)
	{
		long res = 0;

		Genode::ascii_to_signed<long>(nptr, res);

		return res;
	}

	long ph_strtol(const char *nptr, char **endptr, int base)
	{
		if (base != 10) {
			Genode::warning("ph_strtol: conversion from base ",
							base,
							" are not supported");
			return -1;
		}

		if (endptr != 0) {
			Genode::warning("ph_strtol: endptr != 0. Ignoring it");
		}

		long res = 0;

		Genode::ascii_to(nptr, res);

		return res;
	}

	long long ph_strtoq(const char *nptr, char **endptr, int base)
	{
		if (base != 10) {
			Genode::warning("ph_strtol: conversion from base ",
							base,
							" are not supported");
			return -1;
		}

		if (endptr != 0) {
			Genode::warning("ph_strtol: endptr != 0. Ignoring it");
		}

		// TODO : come up with the wrokaround to use long long
		long res = 0;

		Genode::ascii_to(nptr, res);

		return res;
	}

	unsigned long long ph_strtouq(const char *nptr, char **endptr, int base)
	{
		if (base != 10) {
			Genode::warning("ph_strtol: conversion from base ",
							base,
							" are not supported");
			return -1;
		}

		if (endptr != 0) {
			Genode::warning("ph_strtol: endptr != 0. Ignoring it");
		}

		unsigned long long res = 0;

		Genode::ascii_to(nptr, res);

		return res;
	}

	// XXX : Inefficient implementation!
	double ph_pow(double a, double b)
	{
		double res = a;
		if (b > 0) {
			for (int i = 0; i < b; i++) {
				res = res * a;
			}
		} else if (b == 0) {
			res = 1;
		} else {
			res = 1;
			for (int i = 0; i > b; i--) {
				res = res / a;
			}
		}

		return res;
	}

	// TODO: actually use `level` parameter for something
	void ph_syslog(int level, const char *fmt, ...)
	{
		va_list ap;
		(void)level;

		va_start(ap, fmt);
		ph_printf("\033[33m");
		ph_vprintf(fmt, ap);
		ph_printf("\033[0m\n");
		va_end(ap);
	}

#if USE_LIBC_SETJMP == 0
	int ph_setjmp(jmp_buf b)
	{
		(void)b;
		Genode::error("Unimplemented ph_setjmp()!");
		return 0;
	}

	void ph_longjmp(jmp_buf b, int s)
	{
		(void)b;
		(void)s;
		Genode::error("Unimplemented ph_longjmp()!");
	}
#endif

	void ph_qsort(void *ptr,
				  size_t count,
				  size_t size,
				  int (*comp)(const void *, const void *))
	{
#warning Unimplemented function ph_qsort
		(void)ptr;
		(void)count;
		(void)size;
		(void)comp;
		Genode::error("Unimplemented ph_qsort()!");
	}

	// Values generated with python
	// [randint(0,256000) for i in range(0,256)]
	static long rand_values[] = {
		56291,  225407, 7855,   213070, 840,    216299, 223672, 23933,  24227,  211798,
		118698, 126563, 249437, 195334, 154306, 219309, 232985, 74851,  235957, 168503,
		62937,  79663,  189047, 108763, 168591, 201464, 89279,  48323,  171442, 179605,
		110086, 196320, 40991,  201642, 22851,  153146, 251985, 226315, 57260,  253273,
		40141,  100622, 227730, 188302, 222971, 133226, 140178, 25136,  162690, 176494,
		56205,  204805, 75173,  19178,  221358, 235607, 70752,  123004, 208562, 101076,
		243484, 59143,  248734, 61394,  142638, 92952,  195281, 69033,  178008, 49539,
		248526, 84649,  38138,  212542, 66847,  155979, 83555,  33585,  251317, 56702,
		12967,  54682,  163803, 31215,  92027,  179822, 168254, 133681, 15430,  200196,
		225904, 80254,  228339, 143973, 101511, 6377,   56388,  169887, 120172, 83144,
		25810,  69392,  18861,  145817, 114960, 6464,   148170, 183586, 11733,  221080,
		61711,  165298, 180330, 230406, 92150,  28587,  115041, 97519,  46128,  158018,
		131989, 238683, 206911, 187221, 165051, 189245, 129863, 212104, 228044, 46704,
		134959, 229954, 131244, 87291,  89771,  191525, 228776, 23104,  178208, 13560,
		43531,  16948,  98727,  46839,  184942, 93659,  186800, 64631,  29849,  1798,
		166193, 119221, 62333,  186229, 198178, 48817,  228316, 74426,  163339, 174786,
		239859, 234109, 202882, 33326,  50361,  45859,  111578, 216796, 107347, 142968,
		143716, 243837, 104782, 79125,  235334, 137632, 70050,  230618, 209911, 216159,
		177702, 51825,  82625,  221723, 97829,  105485, 205873, 58965,  18847,  4217,
		35355,  84597,  254055, 107582, 205363, 212165, 49338,  78702,  156363, 237736,
		134165, 90458,  218216, 135,    234781, 161008, 136165, 216681, 172099, 224297,
		153810, 242483, 229971, 13117,  119536, 2021,   210076, 42042,  219320, 53236,
		7753,   249945, 75217,  77196,  98760,  38927,  252170, 80528,  27595,  205039,
		8584,   84719,  200360, 129654, 4642,   121124, 150474, 199099, 198931, 55333,
		124107, 141975, 243034, 205190, 30234,  70040,  119058, 61010,  174383, 1798,
		82457,  121423, 172271, 75975,  75432,  72242};

	// XXX : Might be not thread safe
	long ph_random()
	{
		static int cnt = 0;
		cnt = (cnt + 1) % 256;
		return rand_values[cnt];
	}
}