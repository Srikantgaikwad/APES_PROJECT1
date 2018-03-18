/**********************************************************************
*@Filename:message.h
*
*@Description:This is a header for messages and queues in main.c
*@Author:Joyce Cho, Srikant Gaikwad
*@Date:3/10/2018
 *********************************************************************/
#ifndef MESSAGE_H_
#define MESSAGE_H_

FILE *file;
char file_name[50];

/*enum for log_level*/
typedef enum{
	STARTUP,
	INFO,
	ALERT,
	CRITICAL
}log_level_t;

/*enum for source*/
typedef enum{
	LOG_TASK,
	LIGHT_TASK,
	TEMP_TASK,
	MAIN_TASK
}source_t;

/*enum for log_type*/
typedef enum{
	INIT,
	FAILURE,
	DATA,
	REQUEST,
	RESPONSE,
	HEART_BEAT
}log_type_t;

enum{
	LUMEN,
	RW_ALL_REGS,
	PRINT_ID,
	ENABLE_INT
};

/*basic message structure*/
typedef struct {
	struct timeval time_stamp;
	log_level_t log_level;
	source_t src_id;
	source_t dest_id;
	log_type_t log_type;
	float data;
}message_t;


/* 7 different message queues
 * Names are self explanatory
 */
#define	TEMP_TO_LOG		"/temptolog"
#define	LIGHT_TO_LOG	"/lighttolog"
#define LOG_TO_LIGHT	"/logtolight"
#define	LOG_TO_TEMP		"/logtotemp"
#define	MAIN_TO_LOG		"/maintolog"
#define TEMP_TO_MAIN	"/temptomain"
#define LIGHT_TO_MAIN	"/lighttomain"


pthread_t loggerThread, temperatureThread, lightThread,socket_thread;

mqd_t temp_to_log, light_to_log, log_to_light, log_to_temp, main_to_log,\
		temp_to_main, light_to_main;

int status = 0;

char data[500];

#define LIGHT_TASK	0x01
#define TEMP_TASK	0x02

/*condition variables*/
int c = 0, k = 0, f = 0;
int ifTempAlive = 0, ifLightAlive = 0, log_alive = 0, socket_alive;
int gotSignal = 0;

int counter_temp = 1;
int counter_light = 1;
int temp_failure = 0;
int temp_degree = 0;
int light_failure = 0;
float prev_lumen = 0;
int day = 0, night = 0;
int temp_life = 0, light_life = 0;
int fd;



struct	mq_attr	attr;
pthread_mutex_t mutex;

int closeMsgQueue(void);
void openMsgQueue(void);
int msg_temperatureNoHB(void);
int msg_lightNoHB(void);
int log_tempqueue();
int log_lightqueue(void);
int log_mainqueue(void);
int log_init();

#endif /* MESSAGE_H_ */
