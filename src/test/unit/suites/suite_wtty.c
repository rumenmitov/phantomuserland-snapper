#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>

// can't test wtty out of kernel for we need sync primitives impl out of kernel too

#if 0
#include "utils.h"
#include "wtty.h"

#define BS 1024

TEST_FUNCT(foo) {
	char buf[BS];

    //ph_printf("test case 1\n");
	wtty_t	*w	= wtty_init( 128 );

	int rc;

	rc = wtty_read( w, buf, BS, 1);	// nowait
    CU_ASSERT_EQUAL(rc, 0);

	rc = wtty_putc( w, 'a' );
    CU_ASSERT_EQUAL(rc, 0);

	rc = wtty_read( w, buf, BS, 1);	// nowait
    CU_ASSERT_EQUAL(rc, 1);


	wtty_destroy( w );
}


TEST_FUNCT(foo2) {
    //ph_printf("test case 2\n");
    /* ������� ��� */
    CU_ASSERT_EQUAL(1, 1);
}
#endif

void runSuite(void)
{
	/* ��� ���-���� */

	// ph_printf("test suite\n");

	CU_pSuite suite = CUnitCreateSuite("Wtty");
	if (suite) {
		// ADD_SUITE_TEST(suite, foo)
		// ADD_SUITE_TEST(suite, foo2)
	}
}
