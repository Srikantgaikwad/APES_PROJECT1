
#include<stdio.h>
#include<unistd.h>

/**********************************************************************
*@Author:Srikant Gaikwad, Joyce
*@Date:3/10/2018
*@compiler:arm-linux-gnueabihf-gcc
 **********************************************************************/

int led2on(){

	printf("LED2 on\n");
	FILE *LED1 = NULL;
	char *LED2 = "/sys/class/leds/beaglebone:green:usr2/brightness";

	LED1 = fopen(LED2, "r+");
	fwrite("1", sizeof(char), 1, LED1);
	fclose(LED1);
	return 1;
}

int led2off(){

	printf("LED2 off\n");
	FILE *LED1 = NULL;
	char *LED2 = "/sys/class/leds/beaglebone:green:usr2/brightness";

	LED1 = fopen(LED2, "r+");
	fwrite("0", sizeof(char), 1, LED1);
	fclose(LED1);
	return 1;
}

int led1on(){

	printf("LED1 on\n");
	FILE *LED1 = NULL;
	char *LED2 = "/sys/class/leds/beaglebone:green:usr1/brightness";

	LED1 = fopen(LED2, "r+");
	fwrite("1", sizeof(char), 1, LED1);
	fclose(LED1);
	return 1;
}

int led1off(){

	printf("LED1 off\n");
	FILE *LED1 = NULL;
	char *LED2 = "/sys/class/leds/beaglebone:green:usr1/brightness";

	LED1 = fopen(LED2, "r+");
	fwrite("0", sizeof(char), 1, LED1);
	fclose(LED1);
	return 1;
}

