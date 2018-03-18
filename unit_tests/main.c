#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<signal.h>
#include<time.h>
#include<mqueue.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include"../include/message.h"
#include"../include/usrled.h"
#include"../include/tmp102.h"
#include"../include/apds9301.h"

float temperature1=0;
float lumen1=0;

static void signal_handler(int signal){
  switch(signal)
  {
    case SIGINT:
      printf("\n Got SIGINT\n");
      
      ifLightAlive = 1;
	  ifTempAlive = 1;
	  log_alive = 1;
       socket_alive = 1;
      /* send out the message to indicate threads are no heartbeat */        
      msg_temperatureNoHB();
      msg_lightNoHB();
      sleep(5);

      /*activate global signalling variables*/
	  

      exitProgram();
	  break;
  }
}

int log_tempqueue(){

	message_t msgFromTemperature;

	/*checks for message in /temptolog queue*/
	if(mq_receive(temp_to_log, (char*)&msgFromTemperature, \
                        sizeof(msgFromTemperature), NULL)>0){

		/*If log type is dataa*/
    	if(msgFromTemperature.log_type == DATA){

    		/*check for global signalling variable f.
    		 * f is 1 when given temperature is in Fahrenheit*/
    		if(f == 1){
    			sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : Temperature task | TEMPERATURE : %fF \n",\
    					msgFromTemperature.time_stamp.tv_sec, msgFromTemperature.time_stamp.tv_usec, msgFromTemperature.data);
    			temperature1 = msgFromTemperature.data;
    			fwrite(data, sizeof(char), strlen(data), file);
    			f = 0;
    		}

    		/*check for global signalling variable k.
    		 * k is 1 when given temperature is in Kelvin*/
    		if(k == 1){
    			sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : Temperature task | TEMPERATURE : %fK \n",\
    					msgFromTemperature.time_stamp.tv_sec, msgFromTemperature.time_stamp.tv_usec, msgFromTemperature.data);
    			fwrite(data, sizeof(char), strlen(data), file);
    			k = 0;
    		}

    		/*check for global signalling variable C.
    		 * C is 1 when given temperature is in Centigrade*/
    		if(c == 1){
    			sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : Temperature task | TEMPERATURE : %fC \n",\
    					msgFromTemperature.time_stamp.tv_sec, msgFromTemperature.time_stamp.tv_usec, msgFromTemperature.data);
    			fwrite(data, sizeof(char), strlen(data), file);
    			c = 0;
    		}

    	}

    	/* checks for log_type INIT, indicates task just initialised*/
    	if(msgFromTemperature.log_type == INIT){
    		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : Temperature task | Task Initialized\n",\
    				msgFromTemperature.time_stamp.tv_sec, msgFromTemperature.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    	}

    	/*Checks if the message is of request type*/
    	if(msgFromTemperature.log_type == REQUEST){
    		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : Temperature task | Request Light\n",\
    				msgFromTemperature.time_stamp.tv_sec, msgFromTemperature.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
            if( mq_send(log_to_light, (const char*)&msgFromTemperature, sizeof(msgFromTemperature), 1) == -1)
            	printf("\nUnable to send");

    	}

    	/*checks if the recieved message is of response type*/
    	if(msgFromTemperature.log_type == RESPONSE){
    		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : Temperature task | Temperature : %f | Response to Light task \n",\
    				msgFromTemperature.time_stamp.tv_sec, msgFromTemperature.time_stamp.tv_usec, msgFromTemperature.data);
    		fwrite(data, sizeof(char), strlen(data), file);
    	}

    } else
    	return FAIL;

	return SUCCESS;
}


/*dumps messages in light_to_log queue into file*/
int log_lightqueue(void){

	message_t msgFromLight;

	/*checks for any message in the queue*/
    if( mq_receive(light_to_log, (char*)&msgFromLight, \
                        sizeof(msgFromLight), NULL) > 0){
    	/*checks if the log_type is DATA and log_level is INFO*/
    	if(msgFromLight.log_type == DATA && msgFromLight.log_level == INFO){
    		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : Light task 	 | Lumen : %f\n",\
    				msgFromLight.time_stamp.tv_sec, msgFromLight.time_stamp.tv_usec, msgFromLight.data);
    		fwrite(data, sizeof(char), strlen(data), file);
    	}

    	/*checks if the log_type is init*/
    	if(msgFromLight.log_type == INIT){
    		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : Light task       | Task Initialized\n",\
    				msgFromLight.time_stamp.tv_sec, msgFromLight.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    	}
    	/*checks if log_type is REQUEST*/
    	if(msgFromLight.log_type == REQUEST){
    		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : Light task       | Request Temperature\n",\
    				msgFromLight.time_stamp.tv_sec, msgFromLight.time_stamp.tv_usec);
    		/*forwards messgae to log_to_temp queue*/
    		fwrite(data, sizeof(char), strlen(data), file);
            if( mq_send(log_to_temp, (const char*)&msgFromLight, sizeof(msgFromLight), 1) == -1)
            	printf("\nUnable to send");
    	}

    	/*checks if the message is a response*/
    	if(msgFromLight.log_type == RESPONSE ){
    		if( msgFromLight.data == RW_ALL_REGS){
        		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : Light task       | Successful RWs all regs | Response to temp task \n",\
        				msgFromLight.time_stamp.tv_sec, msgFromLight.time_stamp.tv_usec);
        		fwrite(data, sizeof(char), strlen(data), file);
    		}

    		else if( msgFromLight.data == PRINT_ID){
        		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : Light task       | Part no: 5 revision: 0 | Response to temp task \n",\
        				msgFromLight.time_stamp.tv_sec, msgFromLight.time_stamp.tv_usec);
        		fwrite(data, sizeof(char), strlen(data), file);
    		}

    		else if( msgFromLight.data == ENABLE_INT){
        		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : Light task       | Interrupt Enabled | Response to temp task \n",\
        				msgFromLight.time_stamp.tv_sec, msgFromLight.time_stamp.tv_usec);
        		fwrite(data, sizeof(char), strlen(data), file);
    		}
    		else {
    		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : Light task       | Lumen : %f | Response to temp task \n",\
    				msgFromLight.time_stamp.tv_sec, msgFromLight.time_stamp.tv_usec, msgFromLight.data);
    		fwrite(data, sizeof(char), strlen(data), file);
    		}
    	}

    	/*checks for alert in log_level*/
    	if(msgFromLight.log_level == ALERT){
    		/*if its a alert and receieved data is 0, Day tansformed to night*/
    		if(msgFromLight.data == 0){
    				sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : Light task       | It's Night\n",\
    						msgFromLight.time_stamp.tv_sec, msgFromLight.time_stamp.tv_usec);
    				fwrite(data, sizeof(char), strlen(data), file);
    		}

    		/*If data is 1, night transformed to day*/
    		if(msgFromLight.data == 1){
    				sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : Light task       | It's Day\n",\
    						msgFromLight.time_stamp.tv_sec, msgFromLight.time_stamp.tv_usec);
    				fwrite(data, sizeof(char), strlen(data), file);
    		}
    	}
    } else
    	return FAIL;

    return SUCCESS;

}


/*dumps messgaes recieved from main task*/
int log_mainqueue(void){
	message_t rmsg_main;

	/*checks if there is any message in main_to_log queue*/
	if(mq_receive(main_to_log, (char*)&rmsg_main, \
	                            sizeof(rmsg_main), NULL)>0){

		/* checks of the log_level is alert and its from LIGHT_TASK*/
    	if(rmsg_main.data == LIGHT_TASK && rmsg_main.log_level == ALERT){
    		printf("Recievd Light failure\n");
    		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : MAIN task | APDS 9301 I2C Failed\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : MAIN task | Terminating Light task\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		/*Activates global signalling variable
    		 * If it becomes 1 lightThread closes
    		 */
    		ifLightAlive = 1;
    		sleep(1);
    		pthread_cancel(lightThread);
    		printf("Terminating Light thread\n");

    	}

		/* checks of the log_level is alert and its from TEMP_TASK*/
    	else if(rmsg_main.data == TEMP_TASK && rmsg_main.log_level == ALERT ){
    		printf("Recievd temp failure\n");
    		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : MAIN task | TMP 102 I2C Failed\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : MAIN task | Terminating Temperature task\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		ifTempAlive = 1;
    		/*Activates global signaling variable
    		 * If it becomes 1 temperatureThread closes
    		 */
    		sleep(1);
    		pthread_cancel(temperatureThread);
    		printf("Terminating TEMP thread\n");
    	}

		/* checks of the log_level is critical and its from LIGHT_TASK
		 * It means light_thead closed
		 * */
    	else if(rmsg_main.data == LIGHT_TASK && rmsg_main.log_level == CRITICAL ){
    		printf("Recievd Light no hb\n");
    		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : MAIN task | No LIGHT task Heart beat\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : MAIN task | Terminating Light task\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		pthread_cancel(lightThread);
    		printf("Terminating Light thread\n");
    	}
		/* checks of the log_level is critical and its from TEMP_TASK
		 * It means temp_thead closed
		 * */
    	else if(rmsg_main.data == TEMP_TASK && rmsg_main.log_level == CRITICAL ){
    		printf("Recievd temp no hb\n");
    		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : MAIN task | No TEMP Heart beat\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : MAIN task | Terminating Temperature task\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		pthread_cancel(temperatureThread);
    		printf("Terminating TEMP thread\n");
    	}
	} else
		return FAIL;

	return SUCCESS;
}


void exitProgram(void){
  pthread_cancel(loggerThread);
	pthread_cancel(temperatureThread);
	pthread_cancel(lightThread);
     pthread_cancel(socket_thread);
	mq_close(temp_to_log);
	mq_unlink(TEMP_TO_LOG);
	mq_close(light_to_log);
	mq_unlink(LIGHT_TO_LOG);
	mq_close(log_to_light);
	mq_unlink(LOG_TO_LIGHT);
	mq_close(log_to_temp);
	mq_unlink(LOG_TO_TEMP);
	mq_close(main_to_log);
	mq_unlink(MAIN_TO_LOG);
	mq_close(temp_to_main);
	mq_unlink(TEMP_TO_MAIN);
	mq_close(light_to_main);
	mq_unlink(LIGHT_TO_MAIN);
	led2off();
	led1off();
	gotSignal = 1;
	fclose(file);
}

void *socket_task(){   
    printf("In socket Task\n");
    int sock0;
    struct sockaddr_in addr;
    struct sockaddr_in client;
    socklen_t len;
    int sock_client;
    
    sock0 = socket(AF_INET, SOCK_STREAM, 0);
    
    /* set socket */
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock0, (struct sockaddr*)&addr, sizeof(addr));
    printf("\t[Info] binding...\n");
    
    /* listening the client */
    listen(sock0, 5);
    printf("\t[Info] listening...\n");
    
    /* wait for the connection */
    printf("\t[Info] wait for connection...\n");
    len = sizeof(client);
    sock_client = accept(sock0, (struct sockaddr *)&client, &len);
    printf("\t[Info] Testing...\n");
    char *paddr_str = inet_ntoa(client.sin_addr);
    printf("\t[Info] Receive connection from %s...\n", paddr_str);
    
    /* write data to client */
   // printf("\t[Info] Say hello back...\n");

    	printf("Temperature %f\n", temperature1);
    write(sock_client,"Temperature = %f", temperature1);
     write(sock_client,"lumen = %f", lumen1);

    
    printf("\t[Info] Close client connection...\n");
    close(sock_client);
    
    /* close listening socket */
    printf("\t[Info] Close self connection...\n");
    close(sock0);


}

int log_init(){

	struct timeval time_init;
	gettimeofday(&time_init, NULL);
	int status= 0;
	status = sprintf(data, "Timestamp : %ld secs, %ld usecs | Source : Log task | Log task initialised\n",\
			time_init.tv_sec, time_init.tv_usec);
	if(status < 0)
		return FAIL;
	fwrite(data, sizeof(char), strlen(data), file);

	return SUCCESS;
}

/*This is log thread as mentioned in description
 * It's also decision task
 */
void *log_task(){

	printf("In Log Task\n");
	/* access checks for file existence
	 * If the file existed it's deleted and created a new*/
	if( access (file_name, F_OK ) != -1){
		int ret = remove(file_name);
		   if(ret == 0) {
		      printf("File deleted successfully\n");
		   } else {
		      printf("Error: unable to delete the file\n");
		   }
	}

	file = fopen(file_name, "a+");

	status = log_init();

    while(log_alive == 0) {

    	/* checks for messages from main and dumps in file*/
    	log_mainqueue();

    	/* Lot of global signalling variables are used, hence mutex*/
    	pthread_mutex_lock(&mutex);
    	/* checks for messages from temperature task and dumps in file*/
    	log_tempqueue();
    	log_tempqueue();
    	/* checks for messages from light task and dumps in file*/
    	log_lightqueue();
		pthread_mutex_unlock(&mutex);

      usleep(1000000);
	}

}


/*This is log thread as mentioned in description
 */
void *temp_task(){
  printf("Enter Temperature task \n");
  static message_t send_msg, rmsg;

  /* Send start message of temperature with heartbeat initialization*/
  message_t initTemp;
  gettimeofday(&initTemp.time_stamp, NULL);
  initTemp.log_level = STARTUP;
  initTemp.src_id = TEMP_TASK;
  initTemp.dest_id = LOG_TASK;
  initTemp.log_type = INIT;
  initTemp.data = 0;
  if( mq_send(temp_to_log, (const char*)&initTemp, sizeof(initTemp), 1) == -1)
  {
    printf("\nUnable to send");
    return FAIL;
  }

  /* A sample request message to light. It is sent to log task and it takes decision*/
  gettimeofday(&send_msg.time_stamp, NULL);
  send_msg.log_level = INFO;
  send_msg.src_id = TEMP_TASK;
  send_msg.dest_id = LOG_TASK;
  send_msg.log_type = REQUEST;
  send_msg.data = 0;

	if( mq_send(temp_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
		printf("\nUnable to send");


	/* ifTempAlive is set when sigint is caught or if sensor fails responding*/
    while(ifTempAlive == 0) {
    	pthread_mutex_lock(&mutex);

    	/*inialiise sensor on i2c 2 bus*/
    	fd = tmp102_init(2);
    	if( fd == FAIL)
    		temp_failure = FAIL;

    	/* get temperature raw data*/
    	float temp = print_temperature(fd, CONFIG_DEFAULT);
    	if( temp == FAIL)
    		temp_failure = FAIL;

    	/*convert it into Human readable format*/
    	float temperature;

    	/* It changes from Celcisu to Fahrenhiet to kelvin in every while loop*/
    	if(temp_degree == 0){
    		temperature = callibrate_temp(temp, CELSIUS);
    		/*change global variables*/
    		c = 1;
    		f = 0;
    		k = 0;
    		temp_degree++;
    	} else if(temp_degree == 1){
    		temperature = callibrate_temp(temp, FAHRENHEIT);
    		f = 1;
    		c = 0;
    		k = 0;
    		temp_degree++;
    	} else if(temp_degree == 2){
    		temperature = callibrate_temp(temp, KELVIN);
    		c = 0;
    		f = 0;
    		k = 1;
    		temp_degree = 0;
    	}
    	if( temperature == FAIL)
    		temp_failure = FAIL;

    	printf("Temperature %f\n", temperature);

    	/*If check library is changed to 1 all tmp102 library functions are checked*/
    	if(CHECK_LIBRARY){
    		status = rw_allregs_tmp102(fd);
    		if(status != FAIL)
    			printf("Successful read and writes in TMP102\n");
    		print_temperature(fd, CONFIG_DEFAULT);
    		uint16_t *res;
    		res = malloc(sizeof(uint16_t));
    		read_configreg(fd, res);
    		shutdown_mode(fd, SHUTDOWN_MODE);
    		printf("Changing to shutdown mode TMP102\n");
    		read_configreg(fd, res);
    	}

    	/* close sensor*/
    	close_tmp102(fd);
    	pthread_mutex_unlock(&mutex);

    	//temp_failure = FAIL;

    	/* if sensor fails to respond send a message to main*/
    	if(temp_failure == FAIL){
        	gettimeofday(&send_msg.time_stamp, NULL);
        	send_msg.log_level = ALERT;
        	send_msg.src_id = TEMP_TASK;
        	send_msg.dest_id = MAIN_TASK;
        	send_msg.log_type = FAILURE;
        	send_msg.data = 0;


        	if( mq_send(temp_to_main, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
        			printf("\nUnable to send heart_beat : TEMP TASK");

    	} else {

    		/* for every 7 secs a request is sent to fetch light to log_task*/
        	if(counter_temp % 7 ==  0)
        	{
        		gettimeofday(&send_msg.time_stamp, NULL);
        		send_msg.log_level = INFO;
        		send_msg.src_id = TEMP_TASK;
        		send_msg.dest_id = LOG_TASK;
        		send_msg.log_type = REQUEST;
        		send_msg.data = LUMEN;

        		if( mq_send(temp_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
        			printf("\nUnable to send");
        		goto end;
        	}

        	/* For every 11 second make apds9301 library to read and write all regs*/
        	if(counter_temp % 11 ==  0)
        	{
        		gettimeofday(&send_msg.time_stamp, NULL);
        		send_msg.log_level = INFO;
        		send_msg.src_id = TEMP_TASK;
        		send_msg.dest_id = LOG_TASK;
        		send_msg.log_type = REQUEST;
        		send_msg.data = RW_ALL_REGS;

        		if( mq_send(temp_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
        			printf("\nUnable to send");
        		goto end;
        	}

        	/* for every 13 secs a request is sent to print apds9301 sensnor id to log_task*/
        	if(counter_temp % 13 ==  0)
        	{
        		gettimeofday(&send_msg.time_stamp, NULL);
        		send_msg.log_level = INFO;
        		send_msg.src_id = TEMP_TASK;
        		send_msg.dest_id = LOG_TASK;
        		send_msg.log_type = REQUEST;
        		send_msg.data = PRINT_ID;

        		if( mq_send(temp_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
        			printf("\nUnable to send");
        		goto end;
        	}

        	/*For every 17 seconds make a request to enable interrupt in light sensor*/
        	if(counter_temp % 17 ==  0)
        	{
        		gettimeofday(&send_msg.time_stamp, NULL);
        		send_msg.log_level = INFO;
        		send_msg.src_id = TEMP_TASK;
        		send_msg.dest_id = LOG_TASK;
        		send_msg.log_type = REQUEST;
        		send_msg.data = ENABLE_INT;

        		if( mq_send(temp_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
        			printf("\nUnable to send");
        		goto end;
        	}




        	/*If it recieves a message from log_task
        	 * It means light task asked for temperature
        	 * sends a message to log task including temperature
        	 */
            if( mq_receive(log_to_temp, (char*)&rmsg, sizeof(rmsg), NULL) != -1){
            	gettimeofday(&send_msg.time_stamp, NULL);
            	send_msg.log_level = INFO;
            	send_msg.src_id = TEMP_TASK;
            	send_msg.dest_id = LOG_TASK;
            	send_msg.log_type = RESPONSE;
            	send_msg.data = temperature;

            		if( mq_send(temp_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
            			printf("\nUnable to send");
            } else {

            	/*If no request is recied it sends a message with temperature to log ask*/
            	gettimeofday(&send_msg.time_stamp, NULL);
            	send_msg.log_level = INFO;
            	send_msg.src_id = TEMP_TASK;
            	send_msg.dest_id = LOG_TASK;
            	send_msg.log_type = DATA;
            	send_msg.data = temperature;

            	if( mq_send(temp_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
            		printf("\nUnable to send");
            }

            /*If there is no failure send a message to main task as heart beat*/
            end:
        	gettimeofday(&send_msg.time_stamp, NULL);
        	send_msg.log_level = INFO;
        	send_msg.src_id = TEMP_TASK;
        	send_msg.dest_id = MAIN_TASK;
        	send_msg.log_type = HEART_BEAT;
        	send_msg.data = 0;

        	if( mq_send(temp_to_main, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
        		printf("\nUnable to send heart_beat : TEMP TASK");

    	}
    	counter_temp++;

        usleep(1000000);
    }

}


void *light_task(){

	printf("In LIGHT task \n");
	static message_t send_msg, rmsg;

    message_t initLight;
    /* send message to log task that this thread is initilised*/
    gettimeofday(&initLight.time_stamp, NULL);
    initLight.log_level = STARTUP;
    initLight.src_id = LIGHT_TASK;
    initLight.dest_id = LOG_TASK;
    initLight.log_type = INIT;
    initLight.data = 0;

    if( mq_send(light_to_log, (const char*)&initLight, sizeof(initLight), 1) == -1){
        printf("\nUnable to send");
        return FAIL;
    }

	/* ifLightAlive is set when sigint is caught or if sensor fails responding*/
    while(ifLightAlive == 0) {


    	pthread_mutex_lock(&mutex);

    	/*inialiise sensor on i2c 2 bus*/
    	fd = sensor_init(2);
    	if( fd == FAIL)
    		light_failure = FAIL;

    	/*If check library is changed to 1 all APDS9301 library functions are checked*/
    	if(CHECK_LIBRARY){
    		int status = rw_allregs_apds(fd);
    		if(status != FAIL)
    			printf("Successful reads and writes to all registers in APDS9301\n");
    		write_interrupt_controlreg(fd, INT_ENABLE);
    		status = read_interrupt_controlreg(fd);
    		if(status != FAIL)
    			printf("Interrupt enabled in APDS9301\n");
    		print_id(fd);

    	}

    	/* gets calibrated lumen value from APDS9301*/
    	float lumen = get_luminosity(fd);
    	if( lumen == FAIL)
    		light_failure = FAIL;

    	/*compares previos state with present
    	 * If there is change it means there is transformation
    	 * if lumen is less than 1  and previous value is greater than 1
    	 * its a night
    	 *
    	 * If lumen is greater than 1 and previous value is less than 1
    	 * its a day
    	 */
    	if(lumen < 1 && prev_lumen > 1)
    		night = 1;
    	if( lumen > 1 && prev_lumen < 1)
    		day = 1;
    	printf("lumen %f, prev_lumen %f, day %d, night %d\n", lumen, prev_lumen, day, night);
        lumen1 = lumen;
    	prev_lumen = lumen;



    	/*checks for sensor failue and if there is, a message is sent to main task*/
    	//light_failure = FAIL;
    	if(light_failure == FAIL){
        	gettimeofday(&send_msg.time_stamp, NULL);
        	send_msg.log_level = ALERT;
        	send_msg.src_id = LIGHT_TASK;
        	send_msg.dest_id = MAIN_TASK;
        	send_msg.log_type = FAILURE;
        	send_msg.data = 0;

        	if( mq_send(light_to_main, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
        			printf("\nUnable to send heart_beat : LIGHT TASK");

    	} else {


        	/*If it recieves a message from log_task*/

            if( mq_receive(log_to_light, (char*)&rmsg, sizeof(rmsg), NULL) != -1){
            	/*If the received message asks for LUMEN*/
            	if(rmsg.log_type == REQUEST && rmsg.data == LUMEN){

            		gettimeofday(&send_msg.time_stamp, NULL);
            		send_msg.log_level = INFO;
            		send_msg.src_id = LIGHT_TASK;
            		send_msg.dest_id = LOG_TASK;
            		send_msg.log_type = RESPONSE;
            		send_msg.data = lumen;

            		day = 0;
            		night = 0;
                	if( mq_send(light_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
                		printf("\nUnable to send");
                	printf("Responding as Lumen \n");
            	}

            	/*If the message asks for coammand read write all regs*/
            	else if(rmsg.log_type == REQUEST && rmsg.data == RW_ALL_REGS){
            		printf("Response to temp\n");
            		gettimeofday(&send_msg.time_stamp, NULL);
            		send_msg.log_level = INFO;
            		send_msg.src_id = LIGHT_TASK;
            		send_msg.dest_id = LOG_TASK;
            		send_msg.log_type = RESPONSE;
            		send_msg.data = RW_ALL_REGS;

            		day = 0;
            		night = 0;
            		printf("Responding as RW all regs \n");
            		/*RW all regs*/
                	int status = rw_allregs_apds(fd);
                	/*On a successful read and write send a message*/
                	if(status == SUCCESS){
                		if( mq_send(light_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
                			printf("\nUnable to send");
                	}
            	}

            	/*If messgae asks for Enable interrupt*/
            	else if(rmsg.log_type == REQUEST && rmsg.data == ENABLE_INT){
            		printf("Response to temp\n");
            		gettimeofday(&send_msg.time_stamp, NULL);
            		send_msg.log_level = INFO;
            		send_msg.src_id = LIGHT_TASK;
            		send_msg.dest_id = LOG_TASK;
            		send_msg.log_type = RESPONSE;
            		send_msg.data = ENABLE_INT;

            		day = 0;
            		night = 0;
            		printf("Responding as write interrupt  \n");
                	int status = write_interrupt_controlreg(fd, INT_ENABLE);
                	/*If interrupt is enabled successfully send a message*/
                	if(status == SUCCESS){
                		if( mq_send(light_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
                			printf("\nUnable to send");
                	}
            	}

            	/*If the message asks for Sensor ID*/
            	else if(rmsg.log_type == REQUEST && rmsg.data == PRINT_ID){
            		printf("Response to temp\n");
            		gettimeofday(&send_msg.time_stamp, NULL);
            		send_msg.log_level = INFO;
            		send_msg.src_id = LIGHT_TASK;
            		send_msg.dest_id = LOG_TASK;
            		send_msg.log_type = RESPONSE;
            		send_msg.data = PRINT_ID;

            		day = 0;
            		night = 0;
            		printf("Responding as print ID  \n");
                	int status = print_id(fd);
                	if(status == SUCCESS){
                		if( mq_send(light_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
                			printf("\nUnable to send");
                	}
            	}



            } else {

            	/* for every 10 secs a request is sent to fetch light to log_task*/
            	if( counter_light % 10 == 0){
            		gettimeofday(&send_msg.time_stamp, NULL);
            		send_msg.log_level = INFO;
            		send_msg.src_id = LIGHT_TASK;
            		send_msg.dest_id = LOG_TASK;
            		send_msg.log_type = REQUEST;
            		send_msg.data = counter_light;

            		if( mq_send(light_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
            			printf("\nUnable to send");
            	}

            	/* If its a transformation to a day. send a message to log_task as alert*/
            	if( day == 1){
            		gettimeofday(&send_msg.time_stamp, NULL);
            		send_msg.log_level = ALERT;
            		send_msg.src_id = LIGHT_TASK;
            		send_msg.dest_id = LOG_TASK;
            		send_msg.log_type = DATA;
            		send_msg.data = 1;

            		if( mq_send(light_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
            			printf("\nUnable to send");
            		day = 0;
            	}

            	/* If its a transformation to a night. send a message to log_task as alert*/
            	else if( night == 1){

            		gettimeofday(&send_msg.time_stamp, NULL);
            		send_msg.log_level = ALERT;
            		send_msg.src_id = LIGHT_TASK;
            		send_msg.dest_id = LOG_TASK;
            		send_msg.log_type = DATA;
            		send_msg.data = 0;

            		if( mq_send(light_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
            			printf("\nUnable to send");
            		night = 0;
                	/* If its not a  transformation. send a message to log_task as info and lumen*/
            	} else if (day == 0 && night == 0){

            		gettimeofday(&send_msg.time_stamp, NULL);
        			send_msg.log_level = INFO;
        			send_msg.src_id = LIGHT_TASK;
        			send_msg.dest_id = LOG_TASK;
        			send_msg.log_type = DATA;
        			send_msg.data = lumen;

        			if( mq_send(light_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
        			printf("\nUnable to send");
            	}
            }



            /*If there is no failure send a message to main task as heart beat*/
    		gettimeofday(&send_msg.time_stamp, NULL);
    		send_msg.log_level = INFO;
    		send_msg.src_id = LIGHT_TASK;
    		send_msg.dest_id = MAIN_TASK;
    		send_msg.log_type = HEART_BEAT;
    		send_msg.data = 0;

    		if( mq_send(light_to_main, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
    				printf("\nUnable to send heart_beat : LIGHT TASK");

    	}
    	/*close file*/
    	close_apds9301(fd);

    	pthread_mutex_unlock(&mutex);
    	counter_light++;
        usleep(1000000);
    }

}

/* It closes all queues*/

int closeMsgQueue(void){

	int status = 0;
	status = mq_close(temp_to_log);
	if(status == FAIL)
		return FAIL;
	status = mq_unlink(TEMP_TO_LOG);
	if(status == FAIL)
		return FAIL;
	status = mq_close(light_to_log);
	if(status == FAIL)
		return FAIL;
	status = mq_unlink(LIGHT_TO_LOG);
	if(status == FAIL)
		return FAIL;
	status = mq_close(log_to_light);
	if(status == FAIL)
		return FAIL;
	status = mq_unlink(LOG_TO_LIGHT);
	if(status == FAIL)
		return FAIL;
	status = mq_close(log_to_temp);
	if(status == FAIL)
		return FAIL;
	status = mq_unlink(LOG_TO_TEMP);
	if(status == FAIL)
		return FAIL;
	status = mq_close(main_to_log);
	if(status == FAIL)
		return FAIL;
	status = mq_unlink(MAIN_TO_LOG);
	if(status == FAIL)
		return FAIL;
	status = mq_close(temp_to_main);
	if(status == FAIL)
		return FAIL;
	status = mq_unlink(TEMP_TO_MAIN);
	if(status == FAIL)
		return FAIL;
	status = mq_close(light_to_main);
	if(status == FAIL)
		return FAIL;
	status = mq_unlink(LIGHT_TO_MAIN);
	if(status == FAIL)
		return FAIL;

	return SUCCESS;
}


void openMsgQueue(void){

	temp_to_log = mq_open (TEMP_TO_LOG, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	light_to_log = mq_open (LIGHT_TO_LOG, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	log_to_light = mq_open (LOG_TO_LIGHT, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	log_to_temp = mq_open (LOG_TO_TEMP, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	main_to_log = mq_open (MAIN_TO_LOG, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	temp_to_main = mq_open (TEMP_TO_MAIN, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	light_to_main = mq_open (LIGHT_TO_MAIN, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);

}

int msg_temperatureNoHB(void){

	message_t send_msg;
	gettimeofday(&send_msg.time_stamp, NULL);
	send_msg.log_level = CRITICAL;
	send_msg.src_id = MAIN_TASK;
	send_msg.dest_id = LOG_TASK;
	send_msg.log_type = FAILURE;
	send_msg.data = 2;
	printf("Sending no heartbeat signal to log\n");
	if( mq_send(main_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1){
		printf("\nUnable to send errorlog : MAIN TASK");
		return FAIL;
	}
	return SUCCESS;

}

int msg_lightNoHB(void){
	message_t send_msg;
	printf("In send\n");
	gettimeofday(&send_msg.time_stamp, NULL);
	send_msg.log_level = CRITICAL;
	send_msg.src_id = MAIN_TASK;
	send_msg.dest_id = LOG_TASK;
	send_msg.log_type = FAILURE;
	send_msg.data = 1;

	printf("Sending no heartbeat signal to log\n");
	if( mq_send(main_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1){
		printf("\nUnable to send errorlog : MAIN TASK");
		return FAIL;
	}
	return SUCCESS;

}

int main(int argc, char *argv[]){
  strcpy(file_name, argv[1]);

  /* Init message with attributes*/
  message_t msg_from_light, msg_from_temp, send_msg;

  attr.mq_maxmsg = 10;
  attr.mq_msgsize = sizeof(msg_from_light);
  attr.mq_flags = 0;

  pthread_mutex_init(&mutex, NULL);

  /* Init signal handler */
  struct sigaction sig;
  sig.sa_flags = SA_SIGINFO;
  sigemptyset(&sig.sa_mask);
  sig.sa_handler = signal_handler;
  if(sigaction(SIGINT, &sig, NULL) == -1)
  {
    perror("sigaction");
  }

  /* close message queue for initialization */
  closeMsgQueue();
  /* start message queue*/
  openMsgQueue();

  /* create three threads*/
  if(pthread_create(&socket_thread, NULL, socket_task, NULL))
        {
          printf("Could not create socket thread\n");
        }
  pthread_create(&loggerThread, NULL, log_task, NULL);
  pthread_create(&temperatureThread, NULL, temp_task, NULL);
  pthread_create(&lightThread, NULL, light_task, NULL);

  usleep(1000000);

  /* check if the threads are live */
  while( gotSignal == 0 || (ifLightAlive | ifTempAlive) == 0)
  {
    if(ifTempAlive == 0)
    {
      if(mq_receive(temp_to_main, (char*)&msg_from_temp, sizeof(msg_from_temp), NULL) == -1)
      {
        printf("\nNo heartbeat from temp task\n");
        temp_life++;
      }else 
      {
        if(msg_from_temp.log_type == FAILURE)
        {
          msg_temperatureNoHB();
          led2on();
          printf("In temp check%d\n", ifTempAlive);
        }
      }
      if(temp_life == 10)
      {
        led2on();
        msg_lightNoHB();
      }
    }

    if(ifLightAlive == 0)
    {
	  /*checks for message from light_task*/
	  if(mq_receive(light_to_main, (char*)&msg_from_light, sizeof(msg_from_light), NULL) == -1)
      {
	    printf("\n Cannot Receive heartbeat from light task \n");
				light_life++;
	  }else
      {
	    if(msg_from_light.log_type == FAILURE)
        {
          gettimeofday(&send_msg.time_stamp, NULL);
		  send_msg.log_level = ALERT;
		  send_msg.src_id = MAIN_TASK;
		  send_msg.dest_id = LOG_TASK;
		  send_msg.log_type = FAILURE;
		  send_msg.data = 1;
        
          if( mq_send(main_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
          {  
            printf("\n Cannot Send Error message to LoggerThread ");
			led1on();
			printf("In light check\n");
          }  
		}
	  }
      /* wait for ten loops, if it is getting no heartbeat, kill light thread */  
	  if(light_life == 10)
      {
	    led1on();
        gettimeofday(&send_msg.time_stamp, NULL);
	    send_msg.log_level = CRITICAL;
	    send_msg.src_id = MAIN_TASK;
	    send_msg.dest_id = LOG_TASK;
	    send_msg.log_type = FAILURE;
	    send_msg.data = 1;

        if( mq_send(main_to_log, (const char*)&send_msg, sizeof(send_msg), 1) == -1)
        {
          printf("\nUnable to send errorlog : MAIN TASK");
		  led1on();
        }
      }
    }
    usleep(1000000);
  }
  sleep(2);

  /*IF there is no sigint but want to close everything*/
  if(gotSignal !=1)
    exitProgram();

  pthread_join(loggerThread, NULL);
  pthread_join(temperatureThread, NULL);
  pthread_join(lightThread, NULL);
  pthread_join(socket_thread, NULL);



}
