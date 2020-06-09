#ifndef __UTILS_H__
#define __UTILS_H__

void PIT2_Config(void);
void Interrupt_Config(void);
void ADC_Config(void);
unsigned short ADC_read16b(void);
void LED_Initialize(void);
void PTD3_Initialize(void);
void SW2_Initialize(void);
void SW3_Initialize(void);
void PTD3_Toggle (void);
void LEDRed_Toggle (void);
void LEDBlue_Toggle (void);
void LEDGreen_Toggle (void);
void LEDWhite_Toggle (void);
void LEDYellow_Toggle (void);
void LEDRed_On (void);
void LEDGreen_On (void);
void LEDBlue_On (void);
void LEDWhite_On (void);
void LEDYellow_On (void);
void LED_Off (void);
int LED_Rand (int rand);
void LEDRed_Fade (void);
void LEDGreen_Fade (void);
void LEDBlue_Fade (void);
void LEDWhite_Fade (void);
void LEDYellow_Fade (void);
void fadeColor (int color);
int whatColor (int x, int y);
int passedRound (int x, int y, int color);
void youFailed (void);
void LED_Rainbow (void);
int isFlat (int x, int y);
int isToward (int x, int y);
int isAway (int x, int y);
int isLeft (int x, int y);
int isRight (int x, int y);
void displayOrientation(int x, int y);
void zAccel(int z);
void delay (void);
void delay_n (int n);
void delay_short (void);
void wait (int k);

#endif

