
#include <stdio.h>
#include "fsl_common.h"
#include "3140_accel.h"
#include "3140_serial.h"
#include "3140_i2c.h"
#include "3140_concur.h"
#include "utils.h"
#include "realtime.h"

/*---------------------------------------------------------------------*/
/* This project makes the K64F into a model rocket launch controller   */
/* and flight computer. It starts with an accelerometer game to unlock */ 
/* the launch controller, requires the generation of a password to     */
/* retrieve flight data, launches the rocket with the use of an        */
/* external relay to close the ignition circuit, and records           */
/* acceleration data during flight. You can see all the functionality  */
/* without the rocket. Open the serial monitor or read the strings in  */
/* the calls to sprintf() for instructions and recorded flight data.   */
/* If you want to bypass the unlocking of the controller and go        */
/* straight to setting a password, press SW3 immediately at the start  */
/* of program execution. This is cheating, but to each their own.			 */
/*---------------------------------------------------------------------*/

/*--------------------------------------*/
/* User choices                         */
/*--------------------------------------*/

/* If 1 play the game, if 0 skip the game */
int game_on = 1;

/* Number of seconds in the launch countdown */
double t_minus = 5.0;

/* Number of data points to collect during flight */
int points = 400;

/*--------------------------------------*/
/* Global variables                     */
/*--------------------------------------*/
static char string[100]; 	// Array of bytes for storing string
static SRAWDATA accelDat; // Struct for accelerometer data
int16_t x; 								// Accelerometer x direction
int16_t y; 								// Accelerometer y direction
int16_t z; 								// Accelerometer x direction
int max_x = 0; 						// Maximum acceleration of rocket
int color;								// Color selected by LED_Rand()
int interrupt_count = 0;  // Number of interrupts triggered by PIT2
int t_plus_LED = 1;				// Show LED after launch?
double flight_time;				// Total flight time
int password[4];					// Password for accessing flight data

/*--------------------------------------*/
/* Stack space for processes            */
/*--------------------------------------*/
#define NRT_STACK 40
#define RT_STACK  40

/*--------------------------------------*/
/* Time structs for realtime processes  */
/*--------------------------------------*/
realtime_t t_0 = {0, 0};
realtime_t t_1 = {1, 0};
realtime_t t_2 = {2, 0};
realtime_t t_3 = {3, 0};
realtime_t t_4 = {4, 0};
realtime_t t_5 = {5, 0};
realtime_t t_6 = {6, 0};
realtime_t t_7 = {7, 0};
realtime_t t_8 = {8, 0};
realtime_t t_9 = {9, 0};
realtime_t t_10 = {10, 0};

/*--------------------------------------*/
/* Function to read accelerometer       */
/*--------------------------------------*/
void updateAccel(int16_t * x, int16_t * y, int16_t * z) {
		ACCEL_getAccelDat(&accelDat);
		* x = accelDat.x;
		* y = accelDat.y;
		* z = accelDat.z;
}

/*--------------------------------------*/
/*         Realtime processes           */
/*--------------------------------------*/

/* Process for Round 1 */
void pRT_Round1(void) {
		color = LED_Rand(rand());
		delay_n(10);
		updateAccel(&x, &y, &z);
		LED_Off();
	  if(!passedRound(x, y, color)) youFailed();
		delay_n(2);
}

/* Process for Round 2 */
void pRT_Round2(void) {
		color = LED_Rand(rand());
		delay_n(8);
		updateAccel(&x, &y, &z);
		LED_Off();
	  if(!passedRound(x, y, color)) youFailed();
		delay_n(2);
}

/* Process for Round 3 */
void pRT_Round3(void) {
		color = LED_Rand(rand());
		delay_n(6);
		updateAccel(&x, &y, &z);
		LED_Off();
	  if(!passedRound(x, y, color)) youFailed();
		delay_n(2);
}

/* Process 1 for generating rainbow */
void pRT_Rainbow1(void) {
	int k = 0;
	for(k=0; k<1000; k++) {
		LED_Off();
		wait((1000-k)/2);
		LEDRed_On();
		wait(k/2);
	}
}

/* Process 2 for generating rainbow */
void pRT_Rainbow2(void) {
	int k = 0;
	for(k=0; k<1000; k++) {
		LEDRed_On();
		wait(1000-k);		
		LEDYellow_On();
		wait(k);
	}
}

/* Process 3 for generating rainbow */
void pRT_Rainbow3(void) {
	int k = 0;
	for(k=0; k<1000; k++) {
		LEDYellow_On();
		wait(1000-k);
		LEDGreen_On();
		wait(k);
	}
}

/* Process 4 for generating rainbow */
void pRT_Rainbow4(void) {
	int k = 0;
	for(k=0; k<1000; k++) {
		LEDGreen_On();
		wait(1000-k);
		LEDBlue_On();
		wait(k);
	}
}

/* Process 5 for generating rainbow */
void pRT_Rainbow5(void) {
	int k = 0;
	for(k=0; k<1000; k++) {
		LEDBlue_On();
		wait((1000-k)/2);
		LEDRed_On();
		wait(k/2);
	}
}

/* Process 6 for generating rainbow */
void pRT_Rainbow6(void) {
	int k = 0;
	for(k=0; k<1000; k++) {
		LEDBlue_On();
		wait((1000-k)/2);
		LED_Off();
		wait(k/2);
	}
}

/* Non-maskable interrupt handler (SW3) */
void NMI_Handler(void) {
	game_on = 0;
}

/* PIT2 Interrupt Handler */
void PIT2_IRQHandler(void) {
	interrupt_count++;
	interrupt_count = interrupt_count % 10;
	PIT -> CHANNEL[2].TFLG = 0x1; // Reset timer interrupt flag (TIF)
	t_minus -= 0.1;
	if(t_plus_LED) {
		if(interrupt_count == 0 && t_minus >= 11) {
			LEDWhite_On();
		} else if(interrupt_count == 0 && t_minus < 11 && t_minus > 1) {
			LEDRed_On();
		} else if(interrupt_count == 0) {
			LEDGreen_On();
		} else {
			LED_Off();
		}
	}
}

/*--------------------------------------*/
/*         Main                         */
/*--------------------------------------*/
int main (void) {

	// Initialize LEDs
	LED_Initialize();
	
	// Initialize push button SW2
	SW2_Initialize();
	
	// Force restart the bus, this is done via GPIO commands and not via the onboard I2C module
	I2C_ReleaseBus(); 
	
	// Initialize I2C bus and UART serial communications
	I2C_Init();				
	uart_init();

	// Query and configure accelerometer
	uint8_t a = ACCEL_ReadWhoAmI();
	uint8_t b = ACCEL_getDefaultConfig(); // 4G mode
	
	// Get seed for rand() from accelerometer
	updateAccel(&x, &y, &z);
	srand(x+y+z);
	
	sprintf(string, "Turn the board in response to the colors. \n\r");
	uart_putString(string);
	sprintf(string, "If you see the flashing red light, you failed. \n\r");
	uart_putString(string);
	sprintf(string, "    White = flat. \n\r");
	uart_putString(string);
	sprintf(string, "    Yellow = right side up (FRDM at the top). \n\r");
	uart_putString(string);
	sprintf(string, "    Blue = upside down. \n\r");
	uart_putString(string);
	sprintf(string, "    Red = turned to the left. \n\r");
	uart_putString(string);
	sprintf(string, "    Green = turned to the right. \n\r");
	uart_putString(string);
	
	/*--------------------------------------*/
	/*         Rainbow                      */
	/*--------------------------------------*/
	if (process_rt_create(pRT_Rainbow1, RT_STACK, &t_0, &t_0) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow2, RT_STACK, &t_0, &t_1) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow3, RT_STACK, &t_0, &t_2) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow4, RT_STACK, &t_0, &t_3) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow5, RT_STACK, &t_0, &t_4) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow2, RT_STACK, &t_0, &t_6) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow3, RT_STACK, &t_0, &t_7) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow4, RT_STACK, &t_0, &t_8) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow6, RT_STACK, &t_0, &t_9) < 0) { return -1; }
	process_start();
	delay_n(5);
	
	// Board tilting game (optional: turn off by setting game_on to 0)
	if(game_on) {
		/*--------------------------------------*/
		/*         ROUND 1                      */
		/*--------------------------------------*/
		
		/* Start round 1 */ 
		int i;
		for(i=0;i<5;i++) {
			if (process_rt_create(pRT_Round1, RT_STACK, &t_0, &t_0) < 0) { return -1; }
		}
		process_start();
		
		/*--------------------------------------*/
		/*         ROUND 2                      */
		/*--------------------------------------*/
		
		sprintf(string, "You passed Round 1 of 3. \n\r");
		uart_putString(string);
		
		// Indicate start of round
		if (process_rt_create(pRT_Rainbow1, RT_STACK, &t_0, &t_0) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow2, RT_STACK, &t_0, &t_1) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow3, RT_STACK, &t_0, &t_2) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow4, RT_STACK, &t_0, &t_3) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow5, RT_STACK, &t_0, &t_4) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow2, RT_STACK, &t_0, &t_6) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow3, RT_STACK, &t_0, &t_7) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow4, RT_STACK, &t_0, &t_8) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow6, RT_STACK, &t_0, &t_9) < 0) { return -1; }
		process_start();
		delay_n(5);
		
		/* Start round 2 */ 
		for(i=0;i<5;i++) {
			if (process_rt_create(pRT_Round2, RT_STACK, &t_0, &t_0) < 0) { return -1; }
		}
		process_start();
		
		/*--------------------------------------*/
		/*         ROUND 3                      */
		/*--------------------------------------*/

		sprintf(string, "You passed Round 2 of 3. \n\r");
		uart_putString(string);
		
		// Indicate start of round
		if (process_rt_create(pRT_Rainbow1, RT_STACK, &t_0, &t_0) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow2, RT_STACK, &t_0, &t_1) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow3, RT_STACK, &t_0, &t_2) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow4, RT_STACK, &t_0, &t_3) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow5, RT_STACK, &t_0, &t_4) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow2, RT_STACK, &t_0, &t_6) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow3, RT_STACK, &t_0, &t_7) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow4, RT_STACK, &t_0, &t_8) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow6, RT_STACK, &t_0, &t_9) < 0) { return -1; }
		process_start();
		delay_n(5);
		
		/* Start round 3 */ 
		for(i=0;i<5;i++) {
			if (process_rt_create(pRT_Round3, RT_STACK, &t_0, &t_0) < 0) { return -1; }
		}
		process_start();
		
		sprintf(string, "You passed the game. Controller is unlocked. \n\r");
		uart_putString(string);
		
		// Indicate that you won the game
		if (process_rt_create(pRT_Rainbow1, RT_STACK, &t_0, &t_0) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow2, RT_STACK, &t_0, &t_1) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow3, RT_STACK, &t_0, &t_2) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow4, RT_STACK, &t_0, &t_3) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow5, RT_STACK, &t_0, &t_4) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow2, RT_STACK, &t_0, &t_6) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow3, RT_STACK, &t_0, &t_7) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow4, RT_STACK, &t_0, &t_8) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow6, RT_STACK, &t_0, &t_9) < 0) { return -1; }
		process_start();
		delay_n(5);
	}
	
	/*------------------------------------------------*/
	/*  Set a password                                */
	/*------------------------------------------------*/
	
	// Flash white three times to indicate password setting time
	int n;
	for(n=0; n<3; n++) {
		LEDWhite_On();
		delay_short();
		LED_Off();
		delay_short();
	}
	
	sprintf(string, "Set a 4-color password to retrieve data after flight. \n\r");
	uart_putString(string);
	sprintf(string, "Move the board to a color and press SW2 to set a color. \n\r");
	uart_putString(string);
	
	// Update the board position
	updateAccel(&x, &y, &z);
	
	/*--------------------------------------*/
	/*         PASSWORD[0]                  */
	/*--------------------------------------*/
	
	// Wait for the user to move the board to a new position and show feedback
	while(! ((isLeft(x, y) || isRight(x, y) || isToward(x, y) || isAway(x, y) || isFlat(x, y)) && !PTC->PDIR)) {
		updateAccel(&x, &y, &z);
		displayOrientation(x, y);		
	}
	
	password[0] = whatColor(x, y);
	fadeColor(whatColor(x, y));
	
	/*--------------------------------------*/
	/*         PASSWORD[1]                  */
	/*--------------------------------------*/
	
	// Wait for the user to move the board to a new position and show feedback
	while(! ((isLeft(x, y) || isRight(x, y) || isToward(x, y) || isAway(x, y) || isFlat(x, y)) && !PTC->PDIR)) {
		updateAccel(&x, &y, &z);
		displayOrientation(x, y);		
	}
	
	password[1] = whatColor(x, y);
	fadeColor(whatColor(x, y));
	
	/*--------------------------------------*/
	/*         PASSWORD[2]                  */
	/*--------------------------------------*/
	
	// Wait for the user to move the board to a new position and show feedback
	while(! ((isLeft(x, y) || isRight(x, y) || isToward(x, y) || isAway(x, y) || isFlat(x, y)) && !PTC->PDIR)) {
		updateAccel(&x, &y, &z);
		displayOrientation(x, y);		
	}
	
	password[2] = whatColor(x, y);
	fadeColor(whatColor(x, y));
	
	/*--------------------------------------*/
	/*         PASSWORD[3]                  */
	/*--------------------------------------*/
	
	// Wait for the user to move the board to a new position and show feedback
	while(! ((isLeft(x, y) || isRight(x, y) || isToward(x, y) || isAway(x, y) || isFlat(x, y)) && !PTC->PDIR)) {
		updateAccel(&x, &y, &z);
		displayOrientation(x, y);		
	}
	
	password[3] = whatColor(x, y);
	fadeColor(whatColor(x, y));
	
	delay_n(2);
	
	sprintf(string, "Password is saved. Controller is armed. \n\r");
	uart_putString(string);
	
	// Print password to serial monitor
	sprintf(string, "    PASSWORD: [%d %d %d %d] \n\r", password[0], password[1], password[2], password[3]);
	uart_putString(string);
	sprintf(string, "    (0 = Red, 1 = Green, 2 = Blue, 3 = White, 4 = Yellow) \n\r");
	uart_putString(string);
	
	// Indicate that you have successfully set a password
	if (process_rt_create(pRT_Rainbow1, RT_STACK, &t_0, &t_0) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow2, RT_STACK, &t_0, &t_1) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow3, RT_STACK, &t_0, &t_2) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow4, RT_STACK, &t_0, &t_3) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow5, RT_STACK, &t_0, &t_4) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow2, RT_STACK, &t_0, &t_6) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow3, RT_STACK, &t_0, &t_7) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow4, RT_STACK, &t_0, &t_8) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow6, RT_STACK, &t_0, &t_9) < 0) { return -1; }
	process_start();
	
	delay_n(2);
	
	/*------------------------------------------------*/
	/*  Controller armed: configure rocket launch     */
	/*------------------------------------------------*/
	
	// Switch accelerometer to 8g mode (only use for real rocket launch--not for demo)
	/* if (ACCEL_getAlternateConfig() != 1) {
		while(1) LEDRed_On();
	} */
	
	sprintf(string, "Hold down SW2 to leave idle and start the launch countdown. \n\r");
	uart_putString(string);
	
	// Initialize SW2 and start launch countdown when pressed
	while(PTC->PDIR) {
		LEDBlue_Fade();
	}
	
	sprintf(string, "Countdown is started. Green lights indicate flight has started. \n\r");
	uart_putString(string);
	
	// Configure and start PIT2 timer
	PIT2_Config();
	
	// Configure GPIO PTD3
	PTD3_Initialize();	
	
	// Wait for the launch countdown to expire
	while(t_minus > 0.0);
	
	// Write high to PTD3 (close relay to launch rocket)
	PTD3_Toggle();
	
	sprintf(string, "Rocket is launched. White flash means all data is collected. \n\r");
	uart_putString(string);
	
	// Collect data during rocket flight
	int accel[points];
	int m;
	for(m=0; m<points; m++) {
		updateAccel(&x, &y, &z);
		sprintf(string, "X, Y, Z: ( %d, %d, %d )\n\r", x, y, z);
		uart_putString(string);
		if(x > max_x) max_x = x;
		accel[m] = x;
	}
	
	// Flash white once to indicate data collection is complete
	LEDWhite_On();
	delay_n(4);
	LED_Off();
	
	sprintf(string, "Press SW2 to retrieve controller. \n\r");
	uart_putString(string);
	
	// Wait for retrieval and SW2 press
	while(PTC->PDIR);
	t_plus_LED = 0;
	flight_time = t_minus;
	
	/*------------------------------------------------*/
	/*  Unlock controller to access flight data       */
	/*------------------------------------------------*/
	
	sprintf(string, "Enter password to view flight data. \n\r");
	uart_putString(string);
	sprintf(string, "Turn board then press SW2 to enter a color. \n\r");
	uart_putString(string);
	
	// Flash white three times to indicate password entry time
	for(n=0; n<3; n++) {
		LEDWhite_On();
		delay_short();
		LED_Off();
		delay_short();
	}
	
	// Password[0]
	while(PTC->PDIR) {
		updateAccel(&x, &y, &z);
		displayOrientation(x, y);
	}
	if(whatColor(x, y) != password[0]) youFailed();
	fadeColor(whatColor(x, y));
	
	// Password[1]
	while(PTC->PDIR) {
		updateAccel(&x, &y, &z);
		displayOrientation(x, y);
	}
	color = whatColor(x, y);
	if(color != password[1]) youFailed();
	fadeColor(whatColor(x, y));
	
	// Password[2]
	while(PTC->PDIR) {
		updateAccel(&x, &y, &z);
		displayOrientation(x, y);
	}
	color = whatColor(x, y);
	if(color != password[2]) youFailed();
	fadeColor(whatColor(x, y));
	
	// Password[3]
	while(PTC->PDIR) {
		updateAccel(&x, &y, &z);
		displayOrientation(x, y);
	}
	color = whatColor(x, y);
	if(color != password[3]) youFailed();
	fadeColor(whatColor(x, y));
	
	delay_n(3);
	
	sprintf(string, "You successfully entered the password. \n\r");
	uart_putString(string);
	
	// Indicate that you have successfully entered the password
	if (process_rt_create(pRT_Rainbow1, RT_STACK, &t_0, &t_0) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow2, RT_STACK, &t_0, &t_1) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow3, RT_STACK, &t_0, &t_2) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow4, RT_STACK, &t_0, &t_3) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow5, RT_STACK, &t_0, &t_4) < 0) { return -1; }
	process_start();
	if (process_rt_create(pRT_Rainbow2, RT_STACK, &t_0, &t_6) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow3, RT_STACK, &t_0, &t_7) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow4, RT_STACK, &t_0, &t_8) < 0) { return -1; }
	if (process_rt_create(pRT_Rainbow6, RT_STACK, &t_0, &t_9) < 0) { return -1; }
	process_start();
	
	delay_n(2);
	
	sprintf(string, "Now printing vertical acceleration data. \n\r");
	uart_putString(string);
	
	// Print x acceleration data to serial monitor
	for(m=0; m<points; m++) {
		//accel[m] = accel[m]*0.000448;
		sprintf(string, "X: ( %d ) \n\r", accel[m]);
		uart_putString(string);
	}
	
	// Print max x acceleration to serial monitor
	sprintf(string, "MAX X: ( %d ) \n\r", max_x);
	uart_putString(string);
	
	sprintf(string, "1 bit = 0.488E-3 G. \n\r");
	uart_putString(string);
	
	// Print total flight time to serial monitor
	sprintf(string, "FLIGHT TIME:  %d seconds \n\r", (int) -flight_time);
	uart_putString(string);
	sprintf(string, "Have a nice day. \n\r");
	uart_putString(string);
	
	// Repeat rainbow indefinitely
	while(1) {
		if (process_rt_create(pRT_Rainbow1, RT_STACK, &t_0, &t_0) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow2, RT_STACK, &t_0, &t_1) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow3, RT_STACK, &t_0, &t_2) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow4, RT_STACK, &t_0, &t_3) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow5, RT_STACK, &t_0, &t_4) < 0) { return -1; }
		process_start();
		if (process_rt_create(pRT_Rainbow2, RT_STACK, &t_0, &t_6) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow3, RT_STACK, &t_0, &t_7) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow4, RT_STACK, &t_0, &t_8) < 0) { return -1; }
		if (process_rt_create(pRT_Rainbow6, RT_STACK, &t_0, &t_9) < 0) { return -1; }
		process_start();
	}
	
	return 0;
	
}
