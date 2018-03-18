#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "usrled.h"



static void test_led2on(void** state)
{
	int status = led2on();
	assert_int_equal(status, 1);
}

static void test_led2off(void** state)
{
	int status = led2off();
	assert_int_equal(status, 1);
}


static void test_led1on(void** state)
{
	int status = led1on();
	assert_int_equal(status, 1);
}


static void test_led1off(void** state)
{
	int status = led1off();
	assert_int_equal(status, 1);
}


int main(void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(test_led1on),
        cmocka_unit_test(test_led2on),
        cmocka_unit_test(test_led1off),
        cmocka_unit_test(test_led2off),

    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
