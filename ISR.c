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
#include <sys/time.h>
#include <wiringPi.h>
#include <unistd.h>
#include <signal.h>
#include "wiringPiI2C.h"
#include "PAJ7620U2.h"

// Which GPIO pin we're using. For this program we'll use physical pin numbers.
#define PIN 11
#define PIN13 13

#define SENSOR_ADDR 0x73
#define PAJ7620_REG_BANK_SEL 0xEF
#define INIT_REG_ARRAY_SIZE (sizeof(initRegisterArray)/sizeof(initRegisterArray[0]))



// How much time a change must be since the last in order to count as a change
// (in microseconds); allows to avoid generating interrupts on contact bouncing etc.
#define IGNORE_CHANGE_BELOW_USEC 10000

// Current state of the pin
static volatile int state;

// Time of last change
struct timeval last_change;

struct timeval now2;

// Handler for GPIO interrupt
void handle(void) {
	struct timeval now;
	unsigned long diff;

	gettimeofday(&now, NULL);

	// Time difference in microseconds
	diff = (now.tv_sec * 1000000 + now.tv_usec) - (last_change.tv_sec * 1000000 + last_change.tv_usec);

	// Filter any changes in intervals shorter than diff (like contact bouncing etc.):
	if (diff > IGNORE_CHANGE_BELOW_USEC) {

	// Check whether the last state was on or off:
		if (state) {
			// Print info to console:
			printf("Input goes off\n");
			// You can add some code to do when input goes off here...

			

		}
		else {
			// Print info to console:
			printf("Input goes on\n");
			// Add code to do when input goes on here...
		}

		// Change the "state" variable value:
		state = !state;
	}
	// Store the time for last state change:
	last_change = now;
}

// Handler for i2c interrupt
void handler(int signum) {


}






int main(void) {


	// Init  -- use the physical pin number on RPi P1 connector
	wiringPiSetupPhys();

	// Set pin to input in case it's not
	pinMode(PIN, INPUT);
	pullUpDnControl(PIN, PUD_UP);

	// Bind to interrupt
	wiringPiISR(PIN, INT_EDGE_BOTH, &handle);

	// Get initial state of pin
	state = digitalRead(PIN);

	// Feedback for user that the program has started, depending on input state:
	if (state) {
		printf("Started! Initial state is on\n");
	}
	else {
		printf("Started! Initial state is off\n");
	}


	//setup timer interrupt
	pinMode(PIN13, OUTPUT);
	pullUpDnControl(PIN13, PUD_UP);
	digitalWrite(PIN13,LOW);

	//putting timer code here
	// signal(SIGALRM, handler);



	// int sensor = wiringPiI2CSetup(SENSOR_ADDR);
	// if(sensor == -1) {
	// 	printf("Failed to init I2C communication.\n");
	// 	return -1;
	// }
	// printf("I2C communication successfully setup.\n");


	if(!PAJ7620U2_init())
	{	printf("\nGesture Sensor Error\n");
		return 0;
	}

    printf("Gesture sensor OK.\n");
	I2C_writeByte(PAJ_BANK_SELECT, 0);//Select Bank 0
	for (int i = 0; i < Gesture_Array_SIZE; i++)
	{
		I2C_writeByte(Init_Gesture_Array[i][0], Init_Gesture_Array[i][1]);//Gesture register initializes
	}


	//get current directory
	// char cwd[1024];
	// if (getcwd(cwd, sizeof(cwd)) != NULL) {
	// 	printf("Current working dir: %s\n", cwd);
	// } else {
	// 	perror("getcwd() error");
	// }

	// //Play song using mpg123 command line tool
	// //cwd + Playlist folder
	// char song  = cwd + "/Playlist/Memories Of Tokyo-To - 07 - Jet Set Classic (Interlude) [OFFICIAL].mp3";

	// system("mpg123 -q ")


	// Waste time but not CPU
	while(1){
		
		int sensor_data = I2C_readU16( PAJ_INT_FLAG1);
		printf("Sensor data: %d\n", sensor_data);
		switch (sensor_data)
			{
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
			sensor_data=0;
			delay(50);

	}
}


// /*
//  * isr_debounce.c:
//  *	Wait for Interrupt test program  WiringPi >=3.16 - ISR2 method
//  *
//  *
//  */

// #include <stdio.h>
// #include <string.h>
// #include <errno.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <wiringPi.h>
// #include <time.h>

// #define BOUNCETIME 3000 // microseconds
// #define BOUNCETIME_WFI  300
// #define TIMEOUT    10000
// //*************************************
// // BCM pins
// // IRQpin : setup as input with internal pullup. Connected with push button to GND with 1K resistor in series.
// // OUTpin : connected to a LED with 470 Ohm resistor in series to GND. Toggles LED with every push button pressed.
// //*************************************
// #define IRQpin     16
// #define OUTpin     12

// int toggle = 0;
// //waitForInterrupt
// static void wfi(struct WPIWfiStatus wfiStatus, void* userdata) {
// //  struct timeval now;
//   long long int timenow, diff;
//   struct timespec curr;
//   char *edgeType;

//   if (clock_gettime(CLOCK_MONOTONIC, &curr) == -1) {
//         printf("clock_gettime error");
//         return;
//   }

//   timenow = curr.tv_sec * 1000000LL + curr.tv_nsec/1000L; // convert to microseconds
//   diff = timenow - wfiStatus.timeStamp_us;
//   if (wfiStatus.edge == INT_EDGE_RISING)
//       edgeType = "rising";
//   else if (wfiStatus.edge == INT_EDGE_FALLING)
//       edgeType = "falling";
//   else
//       edgeType = "none";
//   printf("gpio BCM = %d, IRQ edge = %s, timestamp = %lld microseconds, timenow = %lld, diff = %lld\n", wfiStatus.gpioPin, edgeType, wfiStatus.timeStamp_us, timenow, diff);
//   if (toggle == 0) {
//     digitalWrite (OUTpin, HIGH);
//     toggle = 1;
//   }
//   else {
//     digitalWrite (OUTpin, LOW);
//     toggle = 0;
//   }
// }


// int main (void)
// {
//   int major, minor;
//   wiringPiVersion(&major, &minor);
//   printf("\nISR debounce test (WiringPi %d.%d)\n\n", major, minor);

//   wiringPiSetupGpio();
//   pinMode(IRQpin, INPUT);
//   // pull up/down mode (PUD_OFF, PUD_UP, PUD_DOWN) => down
//   pullUpDnControl(IRQpin, PUD_UP);
//   pinMode(OUTpin, OUTPUT);
//   digitalWrite (OUTpin, LOW) ;

//   printf("Testing waitForInterrupt on both edges IRQ @ GPIO%d, timeout is %d\n", IRQpin, TIMEOUT);
//   struct WPIWfiStatus wfiStatus = waitForInterrupt2(IRQpin, INT_EDGE_BOTH, TIMEOUT, BOUNCETIME_WFI);
//   if (wfiStatus.status < 0) {
//     printf("waitForInterrupt returned error\n");
//     pinMode(OUTpin, INPUT);
//     return 0;
//   }
//   else if (wfiStatus.status == 0) {
//     printf("waitForInterrupt timed out\n\n");
//   }
//   else {
//     if (wfiStatus.edge == INT_EDGE_FALLING)
//         printf("waitForInterrupt: GPIO pin %d falling edge fired at %lld microseconds\n\n", wfiStatus.gpioPin, wfiStatus.timeStamp_us);
//     else
//         printf("waitForInterrupt: GPIO pin %d rising edge fired at %lld microseconds\n\n", wfiStatus.gpioPin, wfiStatus.timeStamp_us);
//   }

//   printf("Testing IRQ @ GPIO%d on both edges and bouncetime %d microseconds. Toggle LED @ GPIO%d on IRQ.\n\n", IRQpin, BOUNCETIME, OUTpin);
//   printf("To stop program hit return key\n\n");

//   wiringPiISR2(IRQpin, INT_EDGE_BOTH, &wfi, BOUNCETIME, NULL); 

//   getc(stdin);

//   wiringPiISRStop (IRQpin);
//   pinMode(OUTpin, INPUT);

//   return 0;
// }