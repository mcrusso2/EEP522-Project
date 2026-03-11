/* Demonstration C GPIO interrupt handling routine for Raspberry Pi
	This is a modified code found at https://github.com/phil-lavin/raspberry-pi-gpio-interrupt
	The program displays a notice whenever you:
	-turn on the Raspberry Pi's pin 11 (apply 3.3V),
	-turn the pin off.

	This routine uses wiringPi library (follow the installation instructions at wiringpi.com),
	and should be compiled with a command:

	gcc source_filename -o executable_filename -lwiringPi

	You must have root privileges to run it - I don't know any workaround yet:
	sudo ./executable_filename

	Then the program displays a notice whenever you turn the GPIO pin 11 on and off.
	It runs as an infinite loop and you can cancel it by pressing ctrl-C.
*/

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
#include <ctype.h>
#include "PAJ7620U2.h"
#include "playerControls.h"

//Find the line of the name of the bluetooth speaker and extract the ID number labeled from audio Sink
int grabID() {
	char output[256];
	//speaker is named Astronaut
	FILE *fp = popen("wpctl status | grep Astronaut | grep vol", "r");
	if (!fp) {
		printf("Failed to run command\n");
		return -1;
	}

	int id = -1;

	while (fgets(output, sizeof(output), fp)) {

		// Extract the first integer in the line
		char *p = output;
		while (*p && !isdigit(*p)) p++;   // skip until digit

		if (*p) {
			id = strtol(p, NULL, 10);
		}
	}

	pclose(fp);

	printf("ID: %d\n", id);
	return id;
}

void pipelines(){
	int inpipe[2];
	int outpipe[2];
	
	pipe(inpipe);
	pipe(outpipe);
	
	pid_t pid = fork();
	if(pid<0){//error for fork
		perror("fork fail");
		exit(1);
	}
	
	else if(pid == 0){//child
		dup2(outpipe[0], STDIN_FILENO);
		dup2(inpipe[1], STDOUT_FILENO);

		close(outpipe[1]);
		close(outpipe[0]);
		close(inpipe[1]);
		close(inpipe[0]);

		execlp("mpg123", "mpg123", "-R", NULL);
		perror("execlp failed");
		exit(1);

	}
	else{//parent
		close(outpipe[0]);
		close(inpipe[1]);

	
		mpg_out = fdopen(inpipe[0], "r");  // read from mpg123
		mpg_in  = fdopen(outpipe[1], "w"); // write to mpg123

	}
}



int main(void) {

	char buf[1024];

	// Init  -- use the physical pin number on RPi P1 connector
	wiringPiSetupPhys();


	//Grab Bluetooth ID
	int btID = grabID();


	if(!PAJ7620U2_init()){
		printf("\nGesture Sensor Error\n");
		return 0;
	}

    printf("Gesture sensor OK.\n");

	I2C_writeByte(PAJ_BANK_SELECT, 0);//Select Bank 0
	for (int i = 0; i < Gesture_Array_SIZE; i++){
		I2C_writeByte(Init_Gesture_Array[i][0], Init_Gesture_Array[i][1]);//Gesture register initializes
	}

	pipelines();
	

    load_playlist();
	play_current_track();

	while(1){

		if(pause_flag){
			int sensor_data = I2C_readU16( PAJ_INT_FLAG1);
			if(sensor_data & PAJ_FORWARD){
				stop_player();
			}
		}
		else{
			// Check for messages from mpg123, wait for child process to send something
			if(fgets(buf, sizeof(buf), mpg_out) != NULL) {

				// fgets(buf, sizeof(buf), mpg_out);
				// Check for @P messages
				if (buf[0] == '@' && buf[1] == 'P') {
					int status = -1;
					sscanf(buf, "@P %d", &status);

					if (status == 0) {
						// Track ended ? load next
						next_track();
					}
				}
				
				int sensor_data = I2C_readU16( PAJ_INT_FLAG1);
				if(sensor_data){
					printf("Sensor data: %d\n", sensor_data);
					switch (sensor_data){
						case PAJ_UP:			    printf("Up\r\n");				break;
						case PAJ_DOWN:				printf("Down\r\n");				break;
						case PAJ_LEFT:				printf("Left\r\n");				break;
						case PAJ_RIGHT:				printf("Right\r\n"); 			break;
						case PAJ_FORWARD:			printf("Forward\r\n");			break;
						case PAJ_BACKWARD:			printf("Backward\r\n"); 		break;
						case PAJ_CLOCKWISE:			printf("Clockwise\r\n"); 		break;
						case PAJ_COUNT_CLOCKWISE:	printf("AntiClockwise\r\n"); 	break;
						case PAJ_WAVE:				printf("Wave\r\n"); 			break;
						default: break;
					}

					if(sensor_data & PAJ_FORWARD){//pause or play song
						stop_player();
					}

					else if(sensor_data & PAJ_BACKWARD){//next song
						next_track();
					}

					else if(sensor_data & PAJ_UP){//increase volume
						// wpctl set-volume 83 0.1+
						// system("wpctl set-volume 83 0.1+");
						//use btID instead of hardcoding 83
						char command[256];
						snprintf(command, sizeof(command), "wpctl set-volume %d 0.1+ -l 1.0", btID);
						system(command);

					}
					else if(sensor_data & PAJ_DOWN){//decrease volume
						// wpctl set-volume 83 0.05-
						// system("wpctl set-volume 83 0.05-");
						char command[256];
						snprintf(command, sizeof(command), "wpctl set-volume %d 0.05-", btID);
						system(command);
					}

					sensor_data=0;
					delay(100);
				}

			}
			else{
				//No messsage from mpg123
				// Waste time but not CPU
				delay(100);
			}
		}
	}
}
