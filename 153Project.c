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
#include <stdio.h>
#include <stdbool.h>

/*****************************************************************************
 * LED Matrix variables
 ****************************************************************************/
#define GPIO_CLK_PIN     5
#define GPIO_LAT_PIN     8
#define GPIO_OE_PIN      0
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

#define TICKRATE_HZ1 (50000)

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

//Timer for input debouncing
#define TIMER0_IRQ_HANDLER				TIMER0_IRQHandler  // TIMER0 interrupt IRQ function name
#define TIMER0_INTERRUPT_NVIC_NAME		TIMER0_IRQn        // TIMER0 interrupt NVIC interrupt name

//Timer for updating game state
#define TIMER1_IRQ_HANDLER				TIMER1_IRQHandler  // TIMER1 interrupt IRQ function name
#define TIMER1_INTERRUPT_NVIC_NAME		TIMER1_IRQn        // TIMER1 interrupt NVIC interrupt name

bool fDebouncing;  // Boolean variable for tracking debouncing behavior

//Pause screen matrix
int screen[32][16] = {{0}};
bool newGAME = true;

//External variables from Game.c
extern int matrix[32][16];
extern bool pause;

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

/*void moveDot(int dir){
	//matrix[yPos][xPos] = 0;
	if(dir == 1){
		if(xPos<15)
			xPos++;
	}
	else if(dir == 2){
		if(xPos>0)
			xPos--;
	}
	else if(dir == 3){
		if(yPos<31)
			yPos++;
	}
	else if(dir == 4){
		if(yPos>0)
			yPos--;
	}
	matrix[yPos][xPos] = dotColor;
}*/

/**
 * @brief 	Handle interrupt from TIMER0
 * @return 	Nothing
 */
void TIMER1_IRQHandler(void)
{
	update(); 				  // Update boolean variable
	Chip_TIMER_Disable(LPC_TIMER1);		  // Stop TIMER0
	Chip_TIMER_Reset(LPC_TIMER1);		  // Reset TIMER0
	Chip_TIMER_ClearMatch(LPC_TIMER1,0);  // Clear TIMER0 interrupt
	Chip_TIMER_Enable(LPC_TIMER1);
}

/**
 * @brief 	Handle interrupt from TIMER0
 * @return 	Nothing
 */
void TIMER0_IRQHandler(void)
{
	fDebouncing = false; 				  // Update boolean variable
	Chip_TIMER_Disable(LPC_TIMER0);		  // Stop TIMER0
	Chip_TIMER_Reset(LPC_TIMER0);		  // Reset TIMER0
	Chip_TIMER_ClearMatch(LPC_TIMER0,0);  // Clear TIMER0 interrupt
}

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

void setPos(int x){
	bool pos[][3] = {{false,false,false},{false,false,true},{false,true,false},{false,true,true},{true,false,false},{true,false,true},{true,true,false},{true,true,true}};
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_C_PIN, pos[x][0]);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_B_PIN, pos[x][1]);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_A_PIN, pos[x][2]);
}

/**
 * @brief	Handle interrupt from SysTick timer
 * @return	Nothing
 */
void SysTick_Handler(void)
{
	if(fDebouncing) {}  // If not debouncing
	else {
		if(Chip_GPIO_GetPinState(LPC_GPIO, GPIO_INTERRUPT_PORT, GPIO_Up_PIN) == false){
			if(newGAME == true){
				Chip_TIMER_Disable(LPC_TIMER2);
				newGAME = false;
				//Create a new game
				newGame(Chip_TIMER_ReadCount(LPC_TIMER2));
				//Enable game updates
				Chip_TIMER_Enable(LPC_TIMER1);
			}
			Pause();
		}
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

	clkEnbl = !clkEnbl;
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_CLK_PORT, GPIO_CLK_PIN, clkEnbl);

	if(clkEnbl == true || clkEnbl == false){
		if(bit >= 2){
			yBit++;
			setColor(xBit, yBit);
			bit=0;
		}

		if(yBit >= 32){
			yBit = 0;
			Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_LAT_PIN, true);
			Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_OE_PIN, false);
			setPos(xBit);
			Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_LAT_PIN, false);
			Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, GPIO_OE_PIN, false);
			setColor(xBit, yBit);
			xBit++;
		}

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
}

int main(void) {
	/* Generic Initialization */
	SystemCoreClockUpdate();
	Board_Init();

	// Set timer prescale value and set initial boolean variable state
	int PrescaleValue = 120000;  // Clock cycle / 1000 (set to ms increments)
	fDebouncing = false;	     // Set to false, not debouncing

	// Initialize TIMER0
	Chip_TIMER_Init(LPC_TIMER0);					   // Initialize TIMER0
	Chip_TIMER_PrescaleSet(LPC_TIMER0,PrescaleValue);  // Set prescale value
	Chip_TIMER_SetMatch(LPC_TIMER0,0,75);			   // Set match value
	Chip_TIMER_MatchEnableInt(LPC_TIMER0, 0);		   // Configure to trigger interrupt on match

	// Initialize TIMER1
	Chip_TIMER_Init(LPC_TIMER1);					   // Initialize TIMER1
	Chip_TIMER_PrescaleSet(LPC_TIMER1,PrescaleValue);  // Set prescale value
	Chip_TIMER_SetMatch(LPC_TIMER1,0,300);			   // Set match value
	Chip_TIMER_MatchEnableInt(LPC_TIMER1, 0);		   // Configure to trigger interrupt on match

	// Initialize TIMER2
	Chip_TIMER_Init(LPC_TIMER2);					   // Initialize TIMER2
	Chip_TIMER_PrescaleSet(LPC_TIMER2,PrescaleValue);  // Set prescale value


	// Configure GPIO interrupt pin as input
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_INTERRUPT_PORT, GPIO_Up_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_INTERRUPT_PORT, GPIO_Down_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_INTERRUPT_PORT, GPIO_Left_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_INTERRUPT_PORT, GPIO_Right_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_INTERRUPT_PORT, GPIO_Click_PIN);

	//Enable timer interupt
	NVIC_ClearPendingIRQ(TIMER0_INTERRUPT_NVIC_NAME);
	NVIC_EnableIRQ(TIMER0_INTERRUPT_NVIC_NAME);

	NVIC_ClearPendingIRQ(TIMER1_INTERRUPT_NVIC_NAME);
	NVIC_EnableIRQ(TIMER1_INTERRUPT_NVIC_NAME);

	Matrix_init();

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

/*	//Create a new game
	newGame();
	//Enable game updates
	Chip_TIMER_Enable(LPC_TIMER1);
*/
	pause = true;
	Chip_TIMER_Enable(LPC_TIMER2);
	/* Enable and setup SysTick Timer to take input and draw screen */
	SysTick_Config(SystemCoreClock / TICKRATE_HZ1);

	/* Let the game run */
	while (1) {
		__WFI();
	}

	return 0;
}
