#include "test.h"

#include <video/rect.h>


int do_test_rectangles(const char *test_parm)
{
	(void)test_parm;

	rect_t out1, out2, oldw, neww;

	oldw.x = 1;
	oldw.y = 1;
	oldw.xsize = 10;
	oldw.ysize = 10;

	neww.x = 2;
	neww.y = 2;
	neww.xsize = 10;
	neww.ysize = 10;

	// int o2 =
	rect_sub(&out1, &out2, &oldw, &neww);

	// rect_dump( &out1 );
	// rect_dump( &out2 );

	test_check_eq(out1.x, 1);
	test_check_eq(out1.y, 1);
	test_check_eq(out1.xsize, 1);
	test_check_eq(out1.ysize, 10);

	test_check_eq(out2.x, 1);
	test_check_eq(out2.y, 1);
	test_check_eq(out2.xsize, 10);
	test_check_eq(out2.ysize, 1);

	rect_t a, b;

	a.x = 10;
	a.y = 10;
	a.xsize = 10;
	a.ysize = 10;

	b.x = 15;
	b.y = 15;
	b.xsize = 10;
	b.ysize = 10;

	test_check_false(rect_includes(&a, &b));
	test_check_true(rect_intersects(&a, &b));

	b.xsize = 2;
	b.ysize = 2;

	test_check_true(rect_includes(&a, &b));
	test_check_true(rect_intersects(&a, &b));

	b.x = 35;
	b.y = 35;

	test_check_false(rect_includes(&a, &b));
	test_check_false(rect_intersects(&a, &b));

	return 0;
}
