#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "tmp102.h"
#include "apds9301.h"



static void test_rwallregs(void** state)
{
	int fd = sensor_init(2);
	int status;
	assert_int_not_equal(fd, FAIL);
	status = rw_allregs_apds(fd);
    assert_int_equal(status, SUCCESS);

}

static void test_writeint(void **state)
{

	int fd = sensor_init(2);
	int status;
	assert_int_not_equal(fd, FAIL);
	/*writing different values to interrupt registers*/
	status = write_interrupt_controlreg(fd, INT_ENABLE);
	assert_int_not_equal(status, FAIL);
	status = read_interrupt_controlreg(fd);
	assert_int_not_equal(status, FAIL);
}

static void test_printid(void **state)
{
	int fd = sensor_init(2);
	int status;
	assert_int_not_equal(fd, FAIL);
	status = print_id(fd);
	assert_int_equal(status, SUCCESS);
}

static void test_getluminosity(void **state)
{
	int fd = sensor_init(2);
	int status;
	assert_int_not_equal(fd, FAIL);
	status = get_luminosity(fd);
	assert_int_not_equal(status, FAIL);
}

int main(void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(test_rwallregs),
        cmocka_unit_test(test_writeint),
        cmocka_unit_test(test_printid),
        cmocka_unit_test(test_getluminosity),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
