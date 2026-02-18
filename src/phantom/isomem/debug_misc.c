#include <kernel/debug.h>
#include <kernel/debug_graphical.h>
#include <kernel/physalloc.h>
#include <video/window.h>

static rgba_t calc_pixel_color(physalloc_t *m, int elem, int units_per_pixel);

/**
 *
 * \brief Generic painter for any allocator using us.
 *
 * Used in debug window.
 *
 * \param[in] w Window to draw to
 * \param[in] r Rectangle to paint inside
 * \param[in] m Memory map (allocator state) to paint state of
 *
 **/
void paint_allocator_memory_map(window_handle_t w, rect_t *r, physalloc_t *m)
{
	int pixels = r->xsize * r->ysize;
	int units_per_pixel = 1 + ((m->total_size - 1) / pixels);

	int bits_per_elem = sizeof(map_elem_t) * 8;

	// make pixel to correspond to integer number of map_elem_t
	units_per_pixel = (((units_per_pixel - 1) / bits_per_elem) + 1) * bits_per_elem;

	int x, y;
	for (y = 0; y < r->ysize; y++) {
		for (x = 0; x < r->xsize; x++)
			w_draw_pixel(w,
						 x + r->x,
						 y + r->y,
						 calc_pixel_color(m, x + y * r->xsize, units_per_pixel));
	}
}

static rgba_t calc_pixel_color(physalloc_t *m, int elem, int units_per_pixel)
{
	map_elem_t *ep = m->map + elem;

	int state = 0;  // 0 = empty, 1 = partial, 2 = used
	int bits = 0;

	int i;
	for (i = 0; i < units_per_pixel; i++, ep++) {
		if (0 == *ep)
			continue;  // empty, no change
		if (0 == ~*ep)
			state = 2;  // full

		// partial, if was empty - make partial
		if (state == 0)
			state = 1;

		bits += __builtin_popcount(*ep);
	}

	switch (state) {
		case 0:
			return COLOR_BLUE;

		case 1: {
			// return COLOR_LIGHTGREEN;
			rgba_t c = COLOR_LIGHTGREEN;
			// lighter = less used
			c.g = 0xFF - (bits * 0xFF / (units_per_pixel * sizeof(map_elem_t) * 8));
			return c;
		}

		case 2:
			return COLOR_LIGHTRED;

		default:
			return COLOR_YELLOW;
	}
}
