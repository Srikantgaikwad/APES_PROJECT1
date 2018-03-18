#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "message.h"



/*Test log_init function which */
static void test_log_init(void** state)
{
	
	int status = log_init();
	assert_int_equal(status, SUCCESS);
}


static void test_loglightqueue(void** state)
{
	int status = log_lightqueue();
	assert_int_equal(status, FAIL);
}


static void test_logtempqueue(void** state)
{
		int status = log_tempqueue();
	assert_int_equal(status, FAIL);
}

static void test_logmainqueue(void** state)
{
	
	int status = log_mainqueue();
	assert_int_equal(status, FAIL);
}


int main(void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(test_log_init),
        cmocka_unit_test(test_loglightqueue),
        cmocka_unit_test(test_logtempqueue),
        cmocka_unit_test(test_logmainqueue),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
