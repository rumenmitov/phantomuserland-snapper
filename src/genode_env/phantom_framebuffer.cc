#include "phantom_env.h"

// Get pointer to framebuffer
extern "C" char *ph_framebuffer_get_screen()
{
	return (char *)main_obj->_framebuffer->pixels;
}

// Output contents of framebuffer to screen
extern "C" void ph_framebuffer_refresh()
{
	main_obj->_framebuffer->refresh_screen();
}

// Returns 0 if framebuffer driver adapter is started successfully,
// -1 if an error occurs
extern "C" int ph_framebuffer_start_adapter()
{
	if (main_obj->_framebuffer.constructed())
		return 0;

	try {
		main_obj->_framebuffer.construct(main_obj->_env);
	} catch (...) {
		return -1;
	}

	return 0;
}
