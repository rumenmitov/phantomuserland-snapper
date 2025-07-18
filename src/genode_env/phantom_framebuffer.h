#ifndef PHANTOM_ENV_FRAMEBUFFER_H
#define PHANTOM_ENV_FRAMEBUFFER_H

#include <base/attached_rom_dataspace.h>
#include <gui_session/connection.h>
#include <gui_session/gui_session.h>
#include <input/keycodes.h>

// Phantom includes
extern "C" {
#include <event.h>
}

namespace Phantom {
class FramebufferAdapter;
};

extern "C" void ph_framebuffer_report_mouse_event(int x, int y,
                                                  unsigned int state);

// Genode keycodes to phantom's flags
static unsigned int keycode_to_mouse_flags(Input::Keycode key) {
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
class Phantom::FramebufferAdapter : private Genode::Noncopyable {
protected:
  // for some reason inheriting from noncopyable does not get rid of warnings
  // so defining these 2 here explicitly
  FramebufferAdapter(const FramebufferAdapter &) = delete;
  FramebufferAdapter &operator=(const FramebufferAdapter &) = delete;

  Genode::Env &_env;
  Genode::Entrypoint _ep{_env, 2048, "framebuffer_ep",
                         Genode::Affinity::Location()};

  Gui::Connection _gui{_env};
  Gui::Session::View_attr _view_attributes{
      .title = "View",
      .rect = {.at = {0, 0}, .area = {1024, 768}},
      .front = true};

  Gui::Top_level_view _view{_gui, _view_attributes.rect};

  Framebuffer::Session *_framebuffer_session = nullptr;
  Input::Session *_input_session = nullptr;

  Genode::Signal_handler<FramebufferAdapter> _input_signal_handler{
      _ep, *this, &FramebufferAdapter::_handle_input};

  int _mouse_x = 0;
  int _mouse_y = 0;
  unsigned int _mouse_flags = 0;

  void _report_mouse_events() {
    ph_framebuffer_report_mouse_event(_mouse_x, _mouse_y, _mouse_flags);
  }

  void _handle_input() {
    if (!_input_session->pending())
      return;

    int n_events = _input_session->flush();
    Input::Event *event_buffer = nullptr;

    _env.rm()
        .attach(_input_session->dataspace(),
                Genode::Region_map::Attr{.size = 0,
                                         .offset = 0,
                                         .use_at = false,
                                         .at = 0,
                                         .executable = false,
                                         .writeable = true})
        .with_result(
            [&event_buffer](Genode::Env::Local_rm::Attachment &attachment) {
              event_buffer = reinterpret_cast<Input::Event *>(attachment.ptr);
              attachment.deallocate = false;
            },
            [](auto) {
              Genode::error(
                  "couldn't arrach region map for input event buffer!");
              throw;
            });

    bool have_mouse_event = false;

    // XXX: middle mouse press / release seem to always appear in the same
    // event buffer,
    //      so phantom never receives middle button clicks
    for (int i = 0; i < n_events; i++) {
      event_buffer[i].handle_press(
          [this, &have_mouse_event](Input::Keycode key,
                                    Input::Codepoint codepoint) {
            (void)codepoint;
            unsigned int flags = keycode_to_mouse_flags(key);
            if (!flags)
              return;

            _mouse_flags |= flags;
            have_mouse_event = true;
          });
      event_buffer[i].handle_release(
          [this, &have_mouse_event](Input::Keycode key) {
            unsigned int flags = keycode_to_mouse_flags(key);
            if (!flags)
              return;

            _mouse_flags &= ~flags;
            have_mouse_event = true;
          });
      event_buffer[i].handle_absolute_motion(
          [this, &have_mouse_event](int x, int y) {
            _mouse_x = x;
            _mouse_y = y;
            have_mouse_event = true;
          });
    }

    if (have_mouse_event)
      _report_mouse_events();
  }

public:
  unsigned char *pixels = nullptr;

  void refresh_screen() {
    _framebuffer_session->refresh(_view_attributes.rect);

    Gui::Rect geometry(_view_attributes.rect);
    _gui.enqueue<Gui::Session::Command::Geometry>(_view.id(), geometry);
    _gui.execute();
  }

  FramebufferAdapter(Genode::Env &env) : _env(env) {
    _gui.view(Gui::View_id{0}, _view_attributes);

    _gui.enqueue<Gui::Session::Command::Front>(_view.id());

    _gui.execute();

    _gui.buffer({_view_attributes.rect.area, false});
    *_framebuffer_session = _gui.framebuffer;
    *_input_session = _gui.input;

    Genode::Region_map::Attr attributes{
        .size =
            Genode::Dataspace_client(_framebuffer_session->dataspace()).size(),
        .offset = 0,
        .use_at = false,
        .executable = false,
        .writeable = true};

    _env.rm()
        .attach(_framebuffer_session->dataspace(), attributes)
        .with_result(
            [this](Genode::Env::Local_rm::Attachment &attachment) {
              pixels = (unsigned char *)attachment.ptr;
              attachment.deallocate = false;
            },
            [](auto) { throw; });

    _input_session->sigh(_input_signal_handler);

    Genode::log("Framebuffer and input initialized");
  }
};

#endif // PHANTOM_ENV_FRAMEBUFFER_H
