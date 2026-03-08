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


const uint8_t initRegisterArray[][2] = {
    // BANK 0
    {0xEF,0x00}, {0x37,0x07}, {0x38,0x17}, {0x39,0x06}, {0x42,0x01}, 
    {0x46,0x2D}, {0x47,0x0F}, {0x48,0x3C}, {0x49,0x00}, {0x4A,0x1E}, 
    {0x4C,0x20}, {0x51,0x10}, {0x5E,0x10}, {0x60,0x27}, {0x80,0x42}, 
    {0x81,0x44}, {0x82,0x04}, {0x8B,0x01}, {0x90,0x06}, {0x95,0x0A}, 
    {0x96,0x0C}, {0x97,0x05}, {0x9A,0x14}, {0x9C,0x3F}, {0xA5,0x19}, 
    {0xCC,0x19}, {0xCD,0x0B}, {0xCE,0x13}, {0xCF,0x64}, {0xD0,0x21}, 
    // BANK 1
    {0xEF,0x01}, {0x02,0x0F}, {0x03,0x10}, {0x04,0x02}, {0x25,0x01},
    {0x27,0x39}, {0x28,0x7F}, {0x29,0x08}, {0x3E,0xFF}, {0x5E,0x3D}, 
    {0x65,0x96}, {0x67,0x97}, {0x69,0xCD}, {0x6A,0x01}, {0x6D,0x2C}, 
    {0x6E,0x01}, {0x72,0x01}, {0x73,0x35}, {0x77,0x01}, {0xEF,0x00},
};

int paj7620_readReg(int fd, uint8_t reg) {
    return wiringPiI2CReadReg8(fd, reg);
}

int paj7620_writeReg(int fd, uint8_t reg, uint8_t val) {
    return wiringPiI2CWriteReg8(fd, reg, val);
}


int paj7620_init(int fd) {
    delay(10);
    paj7620_writeReg(fd, 0xFF, 0x00);
    delay(50);

    // Check ID
    if (paj7620_readReg(fd, 0x01) != 0x76 ||
        paj7620_readReg(fd, 0x00) != 0x20) {
        printf("PAJ7620 ID mismatch\n");
        return -1;
    }

    // Load initialization table
    for (int i = 0; i < INIT_REG_ARRAY_SIZE; i++) {
        paj7620_writeReg(fd, initRegisterArray[i][0],
                             initRegisterArray[i][1]);
        delay(1);
    }

    // Set gesture mode (NEAR_240FPS)
    paj7620_setReportMode(fd, 2);

    return 0;
}

int paj7620_setReportMode(int fd, uint8_t reportMode) {
    uint8_t regIdleTime = 0;

    // Switch to Bank 1
    paj7620_writeReg(fd, PAJ7620_REG_BANK_SEL, 1);

    switch (reportMode) {
        case 0: regIdleTime = 53;  break;   // FAR_240FPS
        case 1: regIdleTime = 183; break;   // FAR_120FPS
        case 2: regIdleTime = 18;  break;   // NEAR_240FPS
        case 3: regIdleTime = 148; break;   // NEAR_120FPS
        default: return -1;
    }

    paj7620_writeReg(fd, 0x65, regIdleTime);

    // Back to Bank 0
    paj7620_writeReg(fd, PAJ7620_REG_BANK_SEL, 0);

    return 0;
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



	int sensor = wiringPiI2CSetup(SENSOR_ADDR);
	if(sensor == -1) {
		printf("Failed to init I2C communication.\n");
		return -1;
	}
	printf("I2C communication successfully setup.\n");


	if (paj7620_init(sensor) < 0) {
        printf("PAJ7620 init failed.\n");
        return -1;
    }

    printf("Gesture sensor initialized.\n");


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
		
		int sensor_data = wiringPiI2CReadReg8(sensor, 0x43);
		printf("Sensor data: %d\n", sensor_data);
		sleep(1);

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