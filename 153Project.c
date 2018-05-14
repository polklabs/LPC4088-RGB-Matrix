/*
===============================================================================
 Name        : 153Project.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#include "Game.h"
#include "board.h"
#include "Sound.h"
#include <stdio.h>
#include <stdbool.h>

/*****************************************************************************
 * LED Matrix variables
 ****************************************************************************/
#define GPIO_CLK_PIN     5
#define GPIO_LAT_PIN     8
#define GPIO_OE_PIN      9
#define GPIO_A_PIN       11
#define GPIO_B_PIN       3
#define GPIO_C_PIN       2
#define GPIO_R1_PIN      10
#define GPIO_G1_PIN      12
#define GPIO_B1_PIN      14
#define GPIO_R2_PIN      15
#define GPIO_G2_PIN      13
#define GPIO_B2_PIN      6
#define GPIO_CLK_PORT    2
#define GPIO_PORT        2

//Clock rate of display 60,000Hz ~~ 30fps
#define TICKRATE_HZ1 (35000)

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

//INPUT BUTTON STUFF
#define GPIO_Up_PIN 	    	25			  // GPIO pin number mapped to interrupt
#define GPIO_Down_PIN     		27			  // GPIO pin number mapped to interrupt
#define GPIO_Right_PIN     		26			  // GPIO pin number mapped to interrupt
#define GPIO_Left_PIN     		23			  // GPIO pin number mapped to interrupt
#define GPIO_Click_PIN     		22			  // GPIO pin number mapped to interrupt
#define GPIO_INTERRUPT_PORT    	GPIOINT_PORT2  // GPIO port number mapped to interrupt

#define GPIO_AUDIO_PIN			3
#define GPIO_AUDIO_PORT			5

//Timer for input debouncing
#define TIMER0_IRQ_HANDLER				TIMER0_IRQHandler  // TIMER0 interrupt IRQ function name
#define TIMER0_INTERRUPT_NVIC_NAME		TIMER0_IRQn        // TIMER0 interrupt NVIC interrupt name

//Timer for updating game state
#define TIMER1_IRQ_HANDLER				TIMER1_IRQHandler  // TIMER1 interrupt IRQ function name
#define TIMER1_INTERRUPT_NVIC_NAME		TIMER1_IRQn        // TIMER1 interrupt NVIC interrupt name

//Timer for audio notes
#define TIMER2_IRQ_HANDLER				TIMER2_IRQHandler  // TIMER1 interrupt IRQ function name
#define TIMER2_INTERRUPT_NVIC_NAME		TIMER2_IRQn        // TIMER1 interrupt NVIC interrupt name

//Timer for audio
#define TIMER3_IRQ_HANDLER				TIMER3_IRQHandler  // TIMER1 interrupt IRQ function name
#define TIMER3_INTERRUPT_NVIC_NAME		TIMER3_IRQn        // TIMER1 interrupt NVIC interrupt name

bool fDebouncing;  // Boolean variable for tracking debouncing behavior

//Pause screen matrix
int screen[32][16] = {{0}};
bool newGAME = true;

//For sounds
bool playing = false;
int lead_note_count = 0;
int curr_lead_note = 0;
float curr_lead_note_time_remaining = 0;
float lead_freq, note_value;
unsigned long duration;

//External variables from Game.c
extern int matrix[32][16];
extern bool pause;
extern bool lose;
extern bool drained;

//Used for outputing to display
bool clkEnbl = true;
int bit = 0;
int yBit = 0;
int xBit = 0;

/*****************************************************************************
 * Private functions
 ****************************************************************************/
void Matrix_init(){
	//CLK
	Chip_GPIO_WriteDirBit(LPC_GPIO, GPIO_CLK_PORT, GPIO_CLK_PIN, true);
	Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_CLK_PORT, GPIO_CLK_PIN, true);
	//LAT
	Chip_GPIO_WriteDirBit(LPC_GPIO, GPIO_PORT, GPIO_LAT_PIN, true);
	Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_PORT, GPIO_LAT_PIN, true);
	//OE
	Chip_GPIO_WriteDirBit(LPC_GPIO, GPIO_PORT, GPIO_OE_PIN, true);
	Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_PORT, GPIO_OE_PIN, true);

	//A
	Chip_GPIO_WriteDirBit(LPC_GPIO, GPIO_PORT, GPIO_A_PIN, true);
	Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_PORT, GPIO_A_PIN, true);
	//B
	Chip_GPIO_WriteDirBit(LPC_GPIO, GPIO_PORT, GPIO_B_PIN, true);
	Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_PORT, GPIO_B_PIN, true);
	//C
	Chip_GPIO_WriteDirBit(LPC_GPIO, GPIO_PORT, GPIO_C_PIN, true);
	Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_PORT, GPIO_C_PIN, true);

	//R1
	Chip_GPIO_WriteDirBit(LPC_GPIO, GPIO_PORT, GPIO_R1_PIN, true);
	Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_PORT, GPIO_R1_PIN, true);
	//G1
	Chip_GPIO_WriteDirBit(LPC_GPIO, GPIO_PORT, GPIO_G1_PIN, true);
	Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_PORT, GPIO_G1_PIN, true);
	//B1
	Chip_GPIO_WriteDirBit(LPC_GPIO, GPIO_PORT, GPIO_B1_PIN, true);
	Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_PORT, GPIO_B1_PIN, true);

	//R2
	Chip_GPIO_WriteDirBit(LPC_GPIO, GPIO_PORT, GPIO_R2_PIN, true);

	Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_PORT, GPIO_R2_PIN, true);
	//G2
	Chip_GPIO_WriteDirBit(LPC_GPIO, GPIO_PORT, GPIO_G2_PIN, true);
	Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_PORT, GPIO_G2_PIN, true);
	//B2
	Chip_GPIO_WriteDirBit(LPC_GPIO, GPIO_PORT, GPIO_B2_PIN, true);
	Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_PORT, GPIO_B2_PIN, true);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/
/**
 * @brief 	Handle interrupt from TIMER0 - Updates the game state
 * @return 	Nothing
 */
void TIMER1_IRQHandler(void)
{
	//Update the game
	update(); 				  // Update boolean variable

	//If the game is lost update faster
	if(lose == true){
		Chip_TIMER_SetMatch(LPC_TIMER1,0,70);
	}

	Chip_TIMER_Disable(LPC_TIMER1);		  // Stop TIMER0
	Chip_TIMER_Reset(LPC_TIMER1);		  // Reset TIMER0
	Chip_TIMER_ClearMatch(LPC_TIMER1,0);  // Clear TIMER0 interrupt

	//If the board is drained - newGame state
	if(drained == true){
		playing = false;
		drained = false;
		newGAME = true;
		lose = false;
		pause = true;
		Chip_TIMER_SetMatch(LPC_TIMER1,0,300);
	}
	else{
		Chip_TIMER_Enable(LPC_TIMER1);
	}
}

//Interupt for playing sound
void TIMER2_IRQHandler(void)
{
	Chip_TIMER_Disable(LPC_TIMER3);

	if(curr_lead_note > lead_note_count-1){
		curr_lead_note = 0;
		curr_lead_note_time_remaining = lead_times[curr_lead_note];
		Chip_TIMER_Disable(LPC_TIMER3);		  // Stop TIMER0
		Chip_TIMER_Reset(LPC_TIMER3);		  // Reset TIMER0
		Chip_TIMER_ClearMatch(LPC_TIMER3,0);  // Clear TIMER0 interrupt
	}

	lead_freq = lead_notes[curr_lead_note];

	note_value = curr_lead_note_time_remaining;
	duration = note_value * 1000000 * (60.0/BPM);
	duration/=2900;

	Chip_TIMER_SetMatch(LPC_TIMER2,0,duration);			   // Set match value

	if(lead_freq < 1){
		lead_freq = 2;
	}
	Chip_TIMER_SetMatch(LPC_TIMER3,0,100000/lead_freq);			   // Set match value
	Chip_TIMER_Reset(LPC_TIMER3);		  // Reset TIMER0
	Chip_TIMER_ClearMatch(LPC_TIMER3,0);  // Clear TIMER0 interrupt

	// Advance lead note
	curr_lead_note_time_remaining -= note_value;
	if (curr_lead_note_time_remaining < 0.001) {
		curr_lead_note++;
		curr_lead_note_time_remaining = lead_times[curr_lead_note];
	}

	Chip_TIMER_Reset(LPC_TIMER2);		  // Reset TIMER0
	Chip_TIMER_ClearMatch(LPC_TIMER2,0);  // Clear TIMER0 interrupt
	Chip_TIMER_Enable(LPC_TIMER3);
}

//Interupt for playing sound
void TIMER3_IRQHandler(void)
{
	if(playing == true)
		Chip_GPIO_SetPinState(LPC_GPIO, GPIO_AUDIO_PORT, GPIO_AUDIO_PIN, !Chip_GPIO_GetPinState(LPC_GPIO, GPIO_AUDIO_PORT, GPIO_AUDIO_PIN));

	Chip_TIMER_Reset(LPC_TIMER3);		  // Reset TIMER0
	Chip_TIMER_ClearMatch(LPC_TIMER3,0);  // Clear TIMER0 interrupt
}

/**
 * @brief 	Handle interrupt from TIMER0 - Debounces the input
 * @return 	Nothing
 */
void TIMER0_IRQHandler(void)
{
	fDebouncing = false; 				  // Update boolean variable
	Chip_TIMER_Disable(LPC_TIMER0);		  // Stop TIMER0
	Chip_TIMER_Reset(LPC_TIMER0);		  // Reset TIMER0
	Chip_TIMER_ClearMatch(LPC_TIMER0,0);  // Clear TIMER0 interrupt
}

//Sets the color bits for outputting to Matrix
void setColor(int x, int y){

	if(y >= 32){
		y = 31;
	}
	if(x >= 8){
		x = 7;
	}

	//off, white, blue, green, red
	bool color[][3] = {{false,false,false}, {false,false,true}, {false,true,false}, {true,false,false},{true, true, false},{true, false, true},{false, true, true},{true,true,true}};

	int choice = screen[y][x];
	if(pause == false || choice == 0)
		choice = matrix[y][x];
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_R1_PIN, color[choice][0]);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_G1_PIN, color[choice][1]);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_B1_PIN, color[choice][2]);

	choice = screen[y][x+8];
	if(pause == false || choice == 0)
		choice = matrix[y][x+8];
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_R2_PIN, color[choice][0]);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_G2_PIN, color[choice][1]);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_B2_PIN, color[choice][2]);
}

//Set the three bit position of the current column
void setPos(int x){
	bool pos[][3] = {{false,false,false},{false,false,true},{false,true,false},{false,true,true},{true,false,false},{true,false,true},{true,true,false},{true,true,true}};
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_C_PIN, pos[x][0]);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_B_PIN, pos[x][1]);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_A_PIN, pos[x][2]);
}

/**
 * @brief	Handle interrupt from SysTick timer - Updates the display and gets inputs
 * @return	Nothing
 */
void SysTick_Handler(void)
{
	if(fDebouncing) {}  // If not debouncing
	else {
		//Pause or new game
		if(Chip_GPIO_GetPinState(LPC_GPIO, GPIO_INTERRUPT_PORT, GPIO_Up_PIN) == false){
			if(newGAME == true){
				newGAME = false;
				//Create a new game
				newGame(Chip_TIMER_ReadCount(LPC_TIMER3));
				//Enable game updates
				Chip_TIMER_Enable(LPC_TIMER1);

				//Start playing audio
				curr_lead_note = 0;
				curr_lead_note_time_remaining = lead_times[curr_lead_note];
				playing = true;
			}
			Pause();
			if(pause == true)
				playing = false;
			else
				playing = true;
		}
		//Move down faster
		if(Chip_GPIO_GetPinState(LPC_GPIO, GPIO_INTERRUPT_PORT, GPIO_Down_PIN) == false)
			update();
		if(Chip_GPIO_GetPinState(LPC_GPIO, GPIO_INTERRUPT_PORT, GPIO_Right_PIN) == false)
			moveDotRight();
		if(Chip_GPIO_GetPinState(LPC_GPIO, GPIO_INTERRUPT_PORT, GPIO_Left_PIN) == false)
			moveDotLeft();

		// Start debounce delay
		fDebouncing = true;  // Update boolean variable
		// Start timer here
		Chip_TIMER_Enable(LPC_TIMER0);  // Start TIMER0
	}

	//Toggle CLOCK
	clkEnbl = !clkEnbl;
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_CLK_PORT, GPIO_CLK_PIN, clkEnbl);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_OE_PIN, clkEnbl);

	//4 bit color
	if(bit >= 2){
		yBit++;
		setColor(xBit, yBit);
		bit=0;
	}

	//32 pixels per column
	if(yBit >= 32){
		yBit = 0;
		Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_LAT_PIN, true);
		Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_OE_PIN, true);
		setPos(xBit);
		Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_LAT_PIN, false);
		Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_OE_PIN, false);
		setColor(xBit, yBit);
		xBit++;
	}

	//8 rows
	if(xBit >= 8){
		Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_LAT_PIN, true);
		Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_OE_PIN, true);
		setPos(xBit);
		Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_LAT_PIN, false);
		Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_OE_PIN, false);
		setColor(xBit, yBit);
		xBit=0;
	}
	bit++;
}

int main(void) {
	/* Generic Initialization */
	SystemCoreClockUpdate();
	Board_Init();

	// Set timer prescale value and set initial boolean variable state
	int PrescaleValue = 120000;  // Clock cycle / 1000 (set to ms increments)
	fDebouncing = false;	     // Set to false, not debouncing

	// Initialize TIMER0 - Debouncing
	Chip_TIMER_Init(LPC_TIMER0);					   // Initialize TIMER0
	Chip_TIMER_PrescaleSet(LPC_TIMER0,PrescaleValue);  // Set prescale value
	Chip_TIMER_SetMatch(LPC_TIMER0,0,75);			   // Set match value
	Chip_TIMER_MatchEnableInt(LPC_TIMER0, 0);		   // Configure to trigger interrupt on match

	// Initialize TIMER1 - Game Update
	Chip_TIMER_Init(LPC_TIMER1);					   // Initialize TIMER1
	Chip_TIMER_PrescaleSet(LPC_TIMER1,PrescaleValue);  // Set prescale value
	Chip_TIMER_SetMatch(LPC_TIMER1,0,300);			   // Set match value
	Chip_TIMER_MatchEnableInt(LPC_TIMER1, 0);		   // Configure to trigger interrupt on match

	// Initialize TIMER2 - Audio note length
	Chip_TIMER_Init(LPC_TIMER2);					   // Initialize TIMER2
	Chip_TIMER_PrescaleSet(LPC_TIMER2,PrescaleValue);  // Set prescale value
	Chip_TIMER_SetMatch(LPC_TIMER2,0,10);			   // Set match value
	Chip_TIMER_MatchEnableInt(LPC_TIMER2, 0);		   // Configure to trigger interrupt on match

	// Initialize TIMER3 - Audio Tone - Random seed
	Chip_TIMER_Init(LPC_TIMER3);					   // Initialize TIMER3
	Chip_TIMER_PrescaleSet(LPC_TIMER3,PrescaleValue/100);  // Set prescale value
	Chip_TIMER_SetMatch(LPC_TIMER3,0,100000/440);			   // Set match value
	Chip_TIMER_MatchEnableInt(LPC_TIMER3, 0);		   // Configure to trigger interrupt on match

	// Configure GPIO interrupt pin as input - Input Joystick pins
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_INTERRUPT_PORT, GPIO_Up_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_INTERRUPT_PORT, GPIO_Down_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_INTERRUPT_PORT, GPIO_Left_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_INTERRUPT_PORT, GPIO_Right_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_INTERRUPT_PORT, GPIO_Click_PIN);

	Chip_GPIO_WriteDirBit(LPC_GPIO, GPIO_AUDIO_PORT, GPIO_AUDIO_PIN, true);
	Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_AUDIO_PORT, GPIO_AUDIO_PIN, true);

	//Enable timer interrupt Debounce
	NVIC_ClearPendingIRQ(TIMER0_INTERRUPT_NVIC_NAME);
	NVIC_EnableIRQ(TIMER0_INTERRUPT_NVIC_NAME);

	//Enable timer interrupt game state
	NVIC_ClearPendingIRQ(TIMER1_INTERRUPT_NVIC_NAME);
	NVIC_EnableIRQ(TIMER1_INTERRUPT_NVIC_NAME);

	//Enable timer interrupt sound note
	NVIC_ClearPendingIRQ(TIMER2_INTERRUPT_NVIC_NAME);
	NVIC_EnableIRQ(TIMER2_INTERRUPT_NVIC_NAME);

	//Enable timer interrupt sound frequency
	NVIC_ClearPendingIRQ(TIMER3_INTERRUPT_NVIC_NAME);
	NVIC_EnableIRQ(TIMER3_INTERRUPT_NVIC_NAME);

	//Initialize matrix pin outputs
	Matrix_init();

	//Set initial state of matrix pins
	bool On = true;
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_CLK_PORT, GPIO_CLK_PIN, !On);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_LAT_PIN, !On);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_OE_PIN, On);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_A_PIN, !On);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_B_PIN, !On);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_C_PIN, !On);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_R1_PIN, !On);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_G1_PIN, !On);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_B1_PIN, !On);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_R2_PIN, !On);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_G2_PIN, !On);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_B2_PIN, !On);

	//Set color for first pixel
	setColor(0,0);

	//Create pause screen
	int a,b;
	for(a=1;a<31;a++)
		for(b=1;b<15;b++){
			if(a == 1 || a == 2 || a == 29 || a == 30)
				screen[a][b]=3;
			else if(b == 1 || b == 2 || b == 13 || b == 14)
				screen[a][b]=3;
			else
				screen[a][b]=1;
		}
	for(a=14;a<20;a++){
		screen[a][5]=5;
		screen[a][6]=5;
		screen[a][9]=5;
		screen[a][10]=5;
	}

	//Get sound ready to play
	lead_note_count = sizeof(lead_notes) / sizeof(float);

	curr_lead_note = 0;
	curr_lead_note_time_remaining = lead_times[curr_lead_note];

	//Get ready to start game
	pause = true;
	Chip_TIMER_Enable(LPC_TIMER2);
	Chip_TIMER_Enable(LPC_TIMER3);

	/* Enable and setup SysTick Timer to take input and draw screen */
	SysTick_Config(SystemCoreClock / TICKRATE_HZ1);

	/* Let the game run */
	while (1) {
		__WFI();
	}

	return 0;
}
