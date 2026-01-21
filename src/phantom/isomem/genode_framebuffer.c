#include <kernel/init.h>
#include <ph_io.h>
#include <ph_string.h>
#include <video/internal.h>
#include <video/screen.h>

#define DEBUG_MSG_PREFIX "genode.fb"
#define debug_level_flow 10
#define debug_level_error 10
#define debug_level_info 10
#include <debug_ext.h>

// functions linked from C++ code
extern char *ph_framebuffer_get_screen();
extern void ph_framebuffer_refresh();
extern int ph_framebuffer_start_adapter();

// Forward declaration
static char *framebuffer_screen(void);
static int framebuffer_probe(void);
static int framebuffer_start(void);
static int framebuffer_stub(void);
static void framebuffer_update(void);
static void framebuffer_bitblt(const rgba_t *, int, int, int, int, zbuf_t,
                               u_int32_t);
static void framebuffer_readblt(rgba_t *, int, int, int, int);
static void framebuffer_clear(int, int, int, int);
static void framebuffer_bitblt_part(const rgba_t *, int, int, int, int, int,
                                    int, int, int, zbuf_t, u_int32_t);

struct drv_video_screen_t drv_video_framebuffer = {
    "Framebuffer", // driver name
    1024, 768,
    32, // screen size & bit depth
    0, 0,
    0,                  // mouse x y flags
    framebuffer_screen, // screen

    framebuffer_probe, framebuffer_start,
    (void *)0,        // accel
    framebuffer_stub, // stop

    framebuffer_update, framebuffer_bitblt, framebuffer_readblt,

    vid_null, // mouse callback (apparently not used)

    vid_mouse_draw_deflt,       // mouse redraw cursor
    vid_mouse_set_cursor_deflt, // mouse set cursor
    vid_mouse_off_deflt,        // mouse disable
    vid_mouse_on_deflt,         // mouse enable

    (void *)vid_null, // copy (vid_null means cpu implementation will be used (i
                      // think))
    framebuffer_clear, framebuffer_bitblt_part};

static char *framebuffer_screen(void) { return ph_framebuffer_get_screen(); }

static int framebuffer_probe(void) {
  ph_printf("FRAMEBUFFER :::> probe\n");
  return 1;
}

static int framebuffer_start(void) {
  if (ph_framebuffer_start_adapter())
    return -1;
  return 0;
}

static int framebuffer_stub(void) {
  ph_printf("FRAMEBUFFER :::> stub (?)\n");
  return 0;
}

static void framebuffer_update(void) { ph_framebuffer_refresh(); }

static void framebuffer_bitblt(const rgba_t *from, int xpos, int ypos,
                               int xsize, int ysize, zbuf_t zpos,
                               u_int32_t flags) {
  // ph_printf("FRAMEBUFFER :::> blit\n" );
  vid_bitblt_rev(from, xpos, ypos, xsize, ysize, zpos, flags);
}

static void framebuffer_readblt(rgba_t *to, int xpos, int ypos, int xsize,
                                int ysize) {
  // ph_printf("FRAMEBUFFER :::> read\n" );
  vid_readblt_rev(to, xpos, ypos, xsize, ysize);
}

static void framebuffer_clear(int xpos, int ypos, int xsize, int ysize) {
  ph_printf("FRAMEBUFFER :::> clear\n");
}

static void framebuffer_bitblt_part(const rgba_t *from, int src_xsize,
                                    int src_ysize, int src_xpos, int src_ypos,
                                    int dst_xpos, int dst_ypos, int xsize,
                                    int ysize, zbuf_t zpos, u_int32_t flags) {
  // ph_printf("FRAMEBUFFER :::> part\n" );
  vid_bitblt_part_rev(from, src_xsize, src_ysize, src_xpos, src_ypos, dst_xpos,
                      dst_ypos, xsize, ysize, zpos, flags);
}

void phantom_start_video_driver(void) {
  ph_printf("Starting genode framebuffer 'driver'\n");

  video_drv = &drv_video_framebuffer;
  if (video_drv->start())
    SHOW_ERROR0(0, "Failed to start framebuffer adapter\n");

  scr_zbuf_init();
  scr_zbuf_turn_upside(0);
  switch_screen_bitblt_to_32bpp(1);

  scr_mouse_set_cursor(drv_video_get_default_mouse_bmp());

  drv_video_init_windows();
  init_main_event_q();
}

// linked to C++ adapater code
void ph_framebuffer_report_mouse_event(int x, int y, unsigned int state) {
  // BUG For some the y-axis is inverted is for the mouse, hence the
  // hack below.
  int new_y = video_drv->ysize - y;

  struct ui_event e;
  ph_memset(&e, 0, sizeof(struct ui_event));
  e.type = UI_EVENT_TYPE_MOUSE;
  e.focus = 0;

  // keep old flags
  e.m.buttons = state;
  e.m.clicked = state & (~video_drv->mouse_flags);
  e.m.released = (~state) & video_drv->mouse_flags;
  e.abs_x = x;
  e.abs_y = new_y;

  // if (e.m.clicked) ph_printf("Clicked: %d %d %d\n",
  //     e.m.clicked & UI_MOUSE_BTN_LEFT, e.m.clicked & UI_MOUSE_BTN_RIGHT,
  //     e.m.clicked & UI_MOUSE_BTN_MIDDLE);
  // if (e.m.released) ph_printf("Released: %d %d %d\n",
  //     e.m.released & UI_MOUSE_BTN_LEFT, e.m.released & UI_MOUSE_BTN_RIGHT,
  //     e.m.released & UI_MOUSE_BTN_MIDDLE);

  video_drv->mouse_x = x;
  video_drv->mouse_y = new_y;
  video_drv->mouse_flags = state;
  video_drv->mouse();
  video_drv->mouse_redraw_cursor();

  ev_q_put_any(&e);
}
