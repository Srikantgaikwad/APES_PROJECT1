#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "tmp102.h"

static void test_rwallregs(void** state)
{
	int fd = tmp102_init(2);
	int stat = rw_allregs_tmp102(fd);
    assert_int_equal(stat, SUCCESS);

}

static void test_enableint(void **state)
{
	int fd = tmp102_init(2);

	int stat;
	stat = write_pointerreg(fd, TEMPREG_ADDRESS);
	assert_int_equal(stas, SUCCESS);
	stat = write_pointerreg(fd, CONFREG_ADDRESS);
	assert_int_equal(stat, SUCCESS);
	stat = write_pointerreg(fd, POINTER_ADDRESS);
	assert_int_equal(stat, SUCCESS);
}

static void test_shutdownmode(void **state)
{

	int fd = tmp102_init(2);
	int stat;
	uint16_t *res;
	res = malloc(sizeof(uint16_t));
	stat = read_configreg(fd, res);
	assert_int_equal(stat, SUCCESS);
	shutdown_mode(fd, SHUTDOWN_MODE);
	printf("Changing to shutdown mode TMP102\n");
	stat = read_configreg(fd, res);
	assert_int_equal(stat, SUCCESS);
}


static void test_pritnttemperature(void **state)
{
	int fd = tmp102_init(2);
	int stat;
	stat = print_temperature(fd, CONFIG_DEFAULT);
	assert_int_not_equal(stat, FAIL);
}

int main(void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(test_rwallregs),
        cmocka_unit_test(test_writepointer),
        cmocka_unit_test(test_shutdownmode),
        cmocka_unit_test(test_pritnttemperature),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
