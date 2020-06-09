#include <MK64F12.h>
#include "utils.h"
#include<stdio.h>

/*----------------------------------------------------------------------------
  Function that configures PIT2 timer
 *----------------------------------------------------------------------------*/
void PIT2_Config(void) {
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK; 							// Enable clock to PIT module
	PIT->MCR = 0;											 							// Enable PIT timers
	PIT -> CHANNEL[2].LDVAL = 0x00203333; 					// Set load value of zeroth PIT (1/10 second)
	NVIC_EnableIRQ(PIT2_IRQn);
	PIT -> CHANNEL[2].TCTRL |= PIT_TCTRL_TIE_MASK; 	// Request interrupt when TIF set
	PIT -> CHANNEL[2].TCTRL |= PIT_TCTRL_TEN_MASK; 	// Enable timer channel
	NVIC_SetPriority(PIT2_IRQn, 3);
}

/*----------------------------------------------------------------------------
  Function that configures interrupts
 *----------------------------------------------------------------------------*/
void Interrupt_Config(void) {
	PORTE -> PCR[26] &= ~PORT_PCR_MUX_MASK; 											// Configure switch
  PORTE -> PCR[26] |= PORT_PCR_IRQC(1001) | PORT_PCR_MUX(1); 		// Connect switch as interrupt
	PTE -> PDDR &= ~GPIO_PDDR_PDD(1<<4); 													// Assign switch to input
	NVIC_EnableIRQ(PIT0_IRQn); 																		// Enable CMSIS-CORE API
}

/*----------------------------------------------------------------------------
  Function that configures ADC
 *----------------------------------------------------------------------------*/
void ADC_Config(void) {
	
	SIM_SCGC6 |= SIM_SCGC6_ADC0_MASK; /*Enable the ADC0 Clock*/
	ADC0_CFG1 |= 0xC; /*16bits ADC*/
	ADC0_SC1A |= 0x1F; /*Disable the module, ADCH = 1111 */
	
}

/*----------------------------------------------------------------------------
  Function that reads ADC
 *----------------------------------------------------------------------------*/
unsigned short ADC_read16b(void) {
	
	ADC0_SC1A = 26 & ADC_SC1_ADCH_MASK; //Write to SC1A to start conversion
	while(ADC0_SC2 & ADC_SC2_ADACT_MASK); //Conversion in progress
	while(!(ADC0_SC1A & ADC_SC1_COCO_MASK)); //Wait until conversion complete
	return ADC0_RA;
	
}

/*----------------------------------------------------------------------------
  Function that initializes LEDs
 *----------------------------------------------------------------------------*/
void LED_Initialize(void) {

  SIM->SCGC5    |= (1 <<  10) | (1 <<  13);  /* Enable Clock to Port B & E */ 
  PORTB->PCR[22] = (1 <<  8) ;               /* Pin PTB22 is GPIO */
  PORTB->PCR[21] = (1 <<  8);                /* Pin PTB21 is GPIO */
  PORTE->PCR[26] = (1 <<  8);                /* Pin PTE26 is GPIO */
  
  PTB->PDOR = (1 << 21 | 1 << 22 );          /* switch Red/Green LED off  */
  PTB->PDDR = (1 << 21 | 1 << 22 );          /* enable PTB18/19 as Output */

  PTE->PDOR = 1 << 26;            /* switch Blue LED off  */
  PTE->PDDR = 1 << 26;            /* enable PTE26 as Output */
}

/*----------------------------------------------------------------------------
  Function that initializes GPIO PTD3
 *----------------------------------------------------------------------------*/
void PTD3_Initialize(void) {

  SIM->SCGC5    |= (1 <<  12);  						 /* Enable Clock to Port D */ 
  PORTD->PCR[3]  = (1 <<  8) ;               /* Pin PTD3 is GPIO */
  
  PTD->PDOR = (0 << 3);          						 /* Switch PTD3 off  */
  PTD->PDDR = (1 << 3);          						 /* Enable PTD3 as Output */
}

/*----------------------------------------------------------------------------
  Function that initializes push button SW2
 *----------------------------------------------------------------------------*/
void SW2_Initialize(void) {

  SIM->SCGC5    |= (1 <<  11);  						 /* Enable Clock to Port C */ 
  PORTC->PCR[6]  = (1 <<  8) ;               /* Pin PTC6 (SW2) is GPIO */
	
}

/*----------------------------------------------------------------------------
  Function that initializes push button SW3
 *----------------------------------------------------------------------------*/
void SW3_Initialize(void) {

  SIM->SCGC5    |= (1 <<  9);  						 /* Enable Clock to Port A */ 
  PORTA->PCR[4]  = (1 <<  8) ;               /* Pin PTA4 (SW3) is GPIO */
	
}

/*----------------------------------------------------------------------------
  Function that toggles GPIO PTD3
 *----------------------------------------------------------------------------*/

void PTD3_Toggle (void) {
	PTD->PTOR = 1 << 3; 	   /* PTD3 Toggle */
}

/*----------------------------------------------------------------------------
  Function that toggles the red LED
 *----------------------------------------------------------------------------*/

void LEDRed_Toggle (void) {
	PTB->PTOR = 1 << 22; 	   /* Red LED Toggle */
}

/*----------------------------------------------------------------------------
  Function that toggles the blue LED
 *----------------------------------------------------------------------------*/
void LEDBlue_Toggle (void) {
	PTB->PTOR = 1 << 21; 	   /* Blue LED Toggle */
}

/*----------------------------------------------------------------------------
  Function that toggles the green LED
 *----------------------------------------------------------------------------*/
void LEDGreen_Toggle (void) {
	PTE->PTOR = 1 << 26; 	   /* Green LED Toggle */
}

/*----------------------------------------------------------------------------
  Function that toggles white
 *----------------------------------------------------------------------------*/
void LEDWhite_Toggle (void) {
	PTB->PTOR = 1 << 21; 	   /* Blue LED Toggle */
	PTB->PTOR = 1 << 22; 	   /* Red LED Toggle */
	PTE->PTOR = 1 << 26; 	   /* Green LED Toggle */
}

/*----------------------------------------------------------------------------
  Function that toggles yellow
 *----------------------------------------------------------------------------*/
void LEDYellow_Toggle (void) {
	PTE->PTOR = 1 << 26; 	   /* Green LED Toggle */
	PTB->PTOR = 1 << 22; 	   /* Red LED Toggle */
}

/*----------------------------------------------------------------------------
  Function that turns on Red LED & all the others off
 *----------------------------------------------------------------------------*/
void LEDRed_On (void) {
	// Save and disable interrupts (for atomic LED change)
	uint32_t m;
	m = __get_PRIMASK();
	__disable_irq();
	
  PTB->PCOR   = 1 << 22;   /* Red LED On*/
  PTB->PSOR   = 1 << 21;   /* Blue LED Off*/
  PTE->PSOR   = 1 << 26;   /* Green LED Off*/
	
	// Restore interrupts
	__set_PRIMASK(m);
}

/*----------------------------------------------------------------------------
  Function that turns on Green LED & all the others off
 *----------------------------------------------------------------------------*/
void LEDGreen_On (void) {
	// Save and disable interrupts (for atomic LED change)
	uint32_t m;
	m = __get_PRIMASK();
	__disable_irq();
	
  PTB->PSOR   = 1 << 21;   /* Blue LED Off*/
  PTE->PCOR   = 1 << 26;   /* Green LED On*/
  PTB->PSOR   = 1 << 22;   /* Red LED Off*/
	
	// Restore interrupts
	__set_PRIMASK(m);
}

/*----------------------------------------------------------------------------
  Function that turns on Blue LED & all the others off
 *----------------------------------------------------------------------------*/
void LEDBlue_On (void) {
	// Save and disable interrupts (for atomic LED change)
	uint32_t m;
	m = __get_PRIMASK();
	__disable_irq();
	
  PTE->PSOR   = 1 << 26;   /* Green LED Off*/
  PTB->PSOR   = 1 << 22;   /* Red LED Off*/
  PTB->PCOR   = 1 << 21;   /* Blue LED On*/
	
	// Restore interrupts
	__set_PRIMASK(m);
}

void LEDWhite_On (void) {
	// Save and disable interrupts (for atomic LED change)
	uint32_t m;
	m = __get_PRIMASK();
	__disable_irq();
	
  PTE->PCOR   = 1 << 26;   /* Green LED On*/	
  PTB->PCOR   = 1 << 21;   /* Blue LED On*/
  PTB->PCOR   = 1 << 22;   /* Red LED On*/
	
	// Restore interrupts
	__set_PRIMASK(m);
}

/*----------------------------------------------------------------------------
  Function that turns on Blue LED & all the others off
 *----------------------------------------------------------------------------*/
void LEDYellow_On (void) {
	// Save and disable interrupts (for atomic LED change)
	uint32_t m;
	m = __get_PRIMASK();
	__disable_irq();
	
  PTB->PSOR   = 1 << 21;   /* Blue LED Off*/
  PTE->PCOR   = 1 << 26;   /* Green LED On*/
  PTB->PCOR   = 1 << 22;   /* Red LED On*/
	
	// Restore interrupts
	__set_PRIMASK(m);
}

/*----------------------------------------------------------------------------
  Function that turns all LEDs off
 *----------------------------------------------------------------------------*/
void LED_Off (void) {	
	// Save and disable interrupts (for atomic LED change)
	uint32_t m;
	m = __get_PRIMASK();
	__disable_irq();
	
  PTB->PSOR   = 1 << 22;   /* Green LED Off*/
  PTB->PSOR   = 1 << 21;   /* Red LED Off*/
  PTE->PSOR   = 1 << 26;   /* Blue LED Off*/
	
	// Restore interrupts
	__set_PRIMASK(m);
}

/*----------------------------------------------------------------------------
  Function that turns on LED based on random number
 *----------------------------------------------------------------------------*/
int LED_Rand (int random) {	
	// Save and disable interrupts (for atomic LED change)
	uint32_t m;
	m = __get_PRIMASK();
	__disable_irq();
	
	random = random % 5;
	
	switch(random) {
		case 0:
			LEDRed_On();
			break;
		case 1:
			LEDGreen_On();
			break;
		case 2:
			LEDBlue_On();
			break;
		case 3:
			LEDWhite_On();
			break;
		default:
			LEDYellow_On();
	}
	
	// Restore interrupts
	__set_PRIMASK(m);
	
	return random;
	
}

void LEDRed_Fade (void) {
	delay_short();
	int k = 0;
	for(k=0; k<1000; k++) {
		LED_Off();
		wait((1000-k)*6);
		LEDRed_On();
		wait(k*6);
	}
	for(k=0; k<1000; k++) {
		LEDRed_On();
		wait((1000-k)*6);
		LED_Off();
		wait(k*6);
	}
}

void LEDGreen_Fade (void) {
	delay_short();
	int k = 0;
	for(k=0; k<1000; k++) {
		LED_Off();
		wait((1000-k)*6);
		LEDGreen_On();
		wait(k*6);
	}
	for(k=0; k<1000; k++) {
		LEDGreen_On();
		wait((1000-k)*6);
		LED_Off();
		wait(k*6);
	}
}

void LEDBlue_Fade (void) {
	delay_short();
	int k = 0;
	for(k=0; k<1000; k++) {
		LED_Off();
		wait((1000-k)*6);
		LEDBlue_On();
		wait(k*6);
	}
	for(k=0; k<1000; k++) {
		LEDBlue_On();
		wait((1000-k)*6);
		LED_Off();
		wait(k*6);
	}
}

void LEDWhite_Fade (void) {
	delay_short();
	int k = 0;
	for(k=0; k<1000; k++) {
		LED_Off();
		wait((1000-k)*6);
		LEDWhite_On();
		wait(k*6);
	}
	for(k=0; k<1000; k++) {
		LEDWhite_On();
		wait((1000-k)*6);
		LED_Off();
		wait(k*6);
	}
}

void LEDYellow_Fade (void) {
	delay_short();
	int k = 0;
	for(k=0; k<1000; k++) {
		LED_Off();
		wait((1000-k)*6);
		LEDYellow_On();
		wait(k*6);
	}
	for(k=0; k<1000; k++) {
		LEDYellow_On();
		wait((1000-k)*6);
		LED_Off();
		wait(k*6);
	}
}

void fadeColor (int color) {
		switch(color) {
		case 0:
			LEDRed_Fade();
			break;
		case 1:
			LEDGreen_Fade();
			break;
		case 2:
			LEDBlue_Fade();
			break;
		case 3:
			LEDWhite_Fade();
			break;
		default:
			LEDYellow_Fade();
			break;
	}
}

int whatColor (int x, int y) {
	int color;
	if(isLeft(x, y)) {
		color = 0; // Red
	} else if(isRight(x, y)) {
		color = 1; // Green
	} else if(isAway(x, y)) {
		color = 2; // Blue
	} else if(isFlat(x, y)) {
		color = 3; // White
	} else {
		color = 4; // Yellow
	}
	return color;	
}

/*----------------------------------------------------------------------------
  Returns 1 if the round is passed, 0 otherwise
 *----------------------------------------------------------------------------*/
int passedRound (int x, int y, int color) {
		switch(color) {
		case 0:
			if(isLeft(x, y)) return 1;
		  return 0;
		case 1:
			if(isRight(x, y)) return 1;
		  return 0;
		case 2:
			if(isAway(x, y)) return 1;
		  return 0;
		case 3:
			if(isFlat(x, y)) return 1;
		  return 0;
		default:
			if(isToward(x, y)) return 1;
		  return 0;
	}
}

void youFailed (void) {
	while(1) {
		int k = 0;
		for(k=0; k<1000; k++) {
			LED_Off();
			wait(k/2);
			LEDRed_On();
			wait((1000-k)/2);
		}
		for(k=0; k<1000; k++) {
			LEDRed_On();
			wait(k/2);
			LED_Off();
			wait((1000-k)/2);
		}
	}
}

/*----------------------------------------------------------------------------
  Flash red, yellow, green, blue quickly twice
 *----------------------------------------------------------------------------*/
void LED_Rainbow (void) {
	LEDRed_On();
	delay_short();
	LEDYellow_On();
	delay_short();
	LEDGreen_On();
	delay_short();
	LEDBlue_On();
	delay_short();
	LED_Off();
}

/*------------FLAT-------------------------------------------------------------
	Indicates whether the board is flat
 *----------------------------------------------------------------------------*/
 int isFlat(int x, int y) {
	 int xlim = 300; // Tolerance on x
	 int ylim = 300; // Tolerance on y
	 int xoff = 0;   // Offset on x
	 int yoff = 0;   // Offset on y
	 if((xoff-xlim) < x && x < (xoff+xlim) && (yoff-ylim) < y && y < (yoff+ylim)) return 1;
	 return 0;
 }

 /*------------TOWARD---------------------------------------------------------
	Indicates whether the board is facing toward the user
 *----------------------------------------------------------------------------*/
 int isToward(int x, int y) {
	 int xlim = 300; 		// Tolerance on x
	 int ylim = 200; 		// Tolerance on y
	 int xoff = 0;   		// Offset on x
	 int yoff = -2050;  // Offset on y
	 if((xoff-xlim) < x && x < (xoff+xlim) && y < (yoff+ylim)) return 1;
	 return 0;
 }
 
 /*------------AWAY-----------------------------------------------------------
	Indicates whether the board is facing away from the user
 *----------------------------------------------------------------------------*/
 int isAway(int x, int y) {
	 int xlim = 300; 		// Tolerance on x
	 int ylim = 200; 		// Tolerance on y
	 int xoff = 0;   		// Offset on x
	 int yoff = 2050;  // Offset on y
	 if((xoff-xlim) < x && x < (xoff+xlim) && y > (yoff-ylim)) return 1;
	 return 0;
 }
 
 /*------------LEFT------------------------------------------------------------
	Indicates whether the board is tilted to the left
 *----------------------------------------------------------------------------*/
 int isLeft(int x, int y) {
	 int xlim = 200; // Tolerance on x
	 int ylim = 300; // Tolerance on y
	 int xoff = -2050;   // Offset on x
	 int yoff = 0;   // Offset on y
	 if(x < (xoff+xlim) && (yoff-ylim) < y && y < (yoff+ylim)) return 1;
	 return 0;
 }
 
 /*------------RIGHT-----------------------------------------------------------
	Indicates whether the board is tilted to the right
 *----------------------------------------------------------------------------*/
 int isRight(int x, int y) {
	 int xlim = 200; // Tolerance on x
	 int ylim = 300; // Tolerance on y
	 int xoff = 2000;   // Offset on x
	 int yoff = 0;   // Offset on y
	 if(x > (xoff-xlim) && (yoff-ylim) < y && y < (yoff+ylim)) return 1;
	 return 0;
 }
 
 /*------------DISPLAY---------------------------------------------------------
	Displays LEDs based on board orientation
  White = flat
  Yellow = toward
  Blue = away
  Red = left
  Green = right
 *----------------------------------------------------------------------------*/
 void displayOrientation(int x, int y) {
	 	if(isFlat(x, y)) {
			LEDWhite_On();
		} else if(isToward(x, y)){
			LEDYellow_On();
		} else if(isAway(x, y)) {
			LEDBlue_On();
		} else if(isLeft(x, y)) {
			LEDRed_On();
		} else if(isRight(x, y)) {
			LEDGreen_On();
		}  else {
			LED_Off();
		}
 }
 
 /*------------Z ACCEL---------------------------------------------------------
	Displays LEDs based on z-axis acceleration
  Green = accelerating up
  Red = accelerating down
 *----------------------------------------------------------------------------*/
 void zAccel(int z) {
	  if(z < 1000) {
			LEDRed_On();
		} else if(1000 < z && z < 3000) {
			LED_Off();
		} else {
			LEDGreen_On();
		}
 }

void delay(void){
	int j;
	for(j=0; j<1000000; j++);
}

void delay_n(int n) {
	int i;
	for(i = 0; i <n; i++) {
		delay();
	}
}

void delay_short(void){
	int j;
	for(j=0; j<500000; j++);
}

void wait(int k) {
	int j;
	for(j=0; j<k; j++);	
}
