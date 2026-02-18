#include "utils.h"

#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>

TEST_FUNCT(foo)
{
	// ph_printf("test case 1\n");
	/* ������� ��� */
	CU_ASSERT_EQUAL(0, 0);
}
TEST_FUNCT(foo2)
{
	// ph_printf("test case 2\n");
	/* ������� ��� */
	CU_ASSERT_EQUAL(1, 1);
}

void runSuite(void)
{
	/* ��� ���-���� */

	// ph_printf("test suite\n");

	CU_pSuite suite = CUnitCreateSuite("Suite1");
	if (suite) {
		ADD_SUITE_TEST(suite, foo)
		ADD_SUITE_TEST(suite, foo2)
	}
}
