
#include "tests_adapters.h"

#include <base/log.h>


extern "C"
{

#include <hal.h>
#include <ph_malloc.h>
#include <ph_string.h>

	bool test_malloc()
	{
		char test_string[] = "hello there!";
		char *a = (char *)ph_malloc(ph_strlen(test_string) + 1);
		if (ph_memcpy(a, test_string, ph_strlen(test_string) + 1) != a) {
			Genode::error("memcpy returned not dest address!!!");
			return false;
		}
		if (ph_memcmp(a, test_string, ph_strlen(test_string) + 1)) {
			ph_free(a);
			Genode::error("memcpy returned not dest address!!!");
			return false;
		}
		ph_free(a);
		return true;
	}
}