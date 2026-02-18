#ifndef PHANTOM_ENV_FRAMEBUFFER_H
#define PHANTOM_ENV_FRAMEBUFFER_H

#include <base/attached_rom_dataspace.h>

#include <gui_session/connection.h>
#include <gui_session/gui_session.h>
#include <input/keycodes.h>
#include <timer_session/connection.h>

// Phantom includes
extern "C"
{
#include <event.h>
}

namespace Phantom
{
	class FramebufferAdapter;
};

extern "C" void ph_framebuffer_report_mouse_event(int x, int y, unsigned int state);

// Genode keycodes to phantom's flags
static unsigned int keycode_to_mouse_flags(Input::Keycode key)
{
	switch (key) {
		case Input::Keycode::BTN_LEFT:
			return UI_MOUSE_BTN_LEFT;
		case Input::Keycode::BTN_RIGHT:
			return UI_MOUSE_BTN_RIGHT;
		case Input::Keycode::BTN_MIDDLE:
			return UI_MOUSE_BTN_MIDDLE;
		default:
			return 0;
	}
}

/**
 * Adapter for Genode's framebuffer driver
 *
 * TODO: support changing screen mode
 * TODO: add keyboard input adapter
 */
class Phantom::FramebufferAdapter : private Genode::Noncopyable
{
protected:
	FramebufferAdapter(const FramebufferAdapter &) = delete;
	FramebufferAdapter &operator=(const FramebufferAdapter &) = delete;

	Genode::Env &_env;

	Genode::Entrypoint _ep {
		_env, 1 << 18, "framebuffer_ep", Genode::Affinity::Location()};

	Gui::Connection _gui {_env};

	/* SCREEN DISPLAY */

	Gui::Session::View_attr _view_attributes {.title = "View",
											  .rect = {.at = {0, 0}, .area = {1024, 768}},
											  .front = true};

	Gui::Top_level_view _view {_gui, _view_attributes.rect};

	Genode::Dataspace_capability _init_fb_ds(void)
	{
		_gui.buffer({_view_attributes.rect.area, false});
		return _gui.framebuffer.dataspace();
	}

	Genode::Attached_dataspace _fb_ds {_env.rm(), _init_fb_ds()};

	void _handle_refresh_screen(Genode::Duration duration)
	{
		(void)duration;
		_gui.framebuffer.panning(_view_attributes.rect.p1());
	}

	/* INFO
	   Screen refresh handler runs 1 / rate.
	   Want 60 fps := 60 / 1s = 1 / (1s / 60) => rate := 1s / 60
	 */
	const unsigned FPS = 60;

	Timer::Connection _screen_refresh_timer {_env, _ep};
	Timer::Periodic_timeout<Phantom::FramebufferAdapter> _screen_refresh_timeout {
		_screen_refresh_timer,
		*this,
		&Phantom::FramebufferAdapter::_handle_refresh_screen,
		Genode::Microseconds(1000000UL / FPS)};

	/* INPUT */

	Genode::Signal_handler<FramebufferAdapter> _input_signal_handler {
		_ep, *this, &FramebufferAdapter::_handle_input};

	int _mouse_x = 0;
	int _mouse_y = 0;
	unsigned int _mouse_flags = 0;

	void _report_mouse_events()
	{
		ph_framebuffer_report_mouse_event(_mouse_x, _mouse_y, _mouse_flags);
	}

	void _handle_input()
	{
		if (_gui.input.pending()) {
			_gui.input.for_each_event([&](Input::Event const &ev) {
				bool have_mouse_event = false;

				ev.handle_absolute_motion([&](int x, int y) {
					_mouse_x = x;
					_mouse_y = y;
					have_mouse_event = true;
				});

				/*
				 * Handling relative motion
				 */
				ev.handle_relative_motion([&](int x, int y) {
					Genode::warning("relative mouse motion is not implemented yet!");
				});

				ev.handle_press([&](Input::Keycode key, Input::Codepoint codepoint) {
					(void)codepoint;
					unsigned int flags = keycode_to_mouse_flags(key);
					if (!flags)
						return;

					_mouse_flags |= flags;
					have_mouse_event = true;

					Genode::log("button pressed");
				});

				ev.handle_release([&](Input::Keycode key) {
					unsigned int flags = keycode_to_mouse_flags(key);
					if (!flags)
						return;

					_mouse_flags &= ~flags;
					have_mouse_event = true;

					Genode::log("button released");
				});

				if (have_mouse_event) {
					ev_q_put_mouse(_mouse_x, _mouse_y, _mouse_flags);
					_report_mouse_events();
					//          refresh_screen();
				}
			});
		}

		// XXX: middle mouse press / release seem to always appear in the same
		// event buffer,
		//      so phantom never receives middle button clicks
	}

public:
	using PT = Genode::Pixel_rgb888;
	PT *pixels = nullptr;

	void refresh_screen(void)
	{
		_gui.framebuffer.refresh(_view_attributes.rect);
	}

	FramebufferAdapter(Genode::Env &env) : _env(env)
	{

		pixels = (PT *)_fb_ds.local_addr<PT>();

		_gui.input.sigh(_input_signal_handler);

		Genode::log("Framebuffer and input initialized");
	}
};

#endif  // PHANTOM_ENV_FRAMEBUFFER_H
