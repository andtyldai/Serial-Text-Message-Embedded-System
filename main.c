/*----------------------------------------------------------------------------
  MAIN
		Creates all global variables, handles external interrupts, creates mutexes,
		creates semaphores, runs setup functions for CPU and peripherals.
  ---------------------------------------------------------------------------*/

#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"
#include "Board_GLCD.h"
#include "Board_LED.h"
#include "Board_Buttons.h"
#include "GLCD_Config.h"
#include "stm32f2xx_hal.h"
#include "rtx_os.h"
#include "rtosPhoneComponents.h"
#include "I2C.h"
#include "JOY.h"
#include "Serial.h"

// character font
extern GLCD_FONT GLCD_Font_16x24;

// interrupt handler prototypes
void EXTI15_10_IRQHandler(void);
void EXTI0_IRQHandler(void);

// thread function prototype
void app_hw_init(void *argument);

// default time
uint8_t hour=21, minute=37, second=55;

// mutex id's
osMutexId_t mutScreen, mutMessage, mutSecond, mutHour, mutMinute;

// semaphore id's
osSemaphoreId_t semDisplayTime, semWriteMesg, semChar, semNewMesg, semSecond, semMinute, semHour;

// memory pool
osMemoryPoolId_t messages, newChars;
osMessageQueueId_t charQueue;
typedef struct message message;
uint8_t numOfMesg = 0;

// returns the current RTOS kernel tick count
uint32_t HAL_GetTick(void) { 
  return osKernelGetTickCount();
}

// initialization thread function
void app_hw_init (void *argument) {
	
	// LCD setup
	GLCD_Initialize();
	GLCD_SetBackgroundColor(0b0110001100010000); // Light Blue-Grey
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
	GLCD_ClearScreen(); 
	GLCD_SetFont(&GLCD_Font_16x24);
	
	// creates display and time threads
	Init_thdDisplayTime();
	osThreadSetPriority(tid_thdDisplayTime, osPriorityBelowNormal);
	Init_thdSecond();
	Init_thdMinute();
	Init_thdHour();
	Init_thdMesg();
	Init_thdDisplayMesg();
	
  // ends the thread
	osThreadExit();
	
}
 
int main (void) {
 
 	// setup
	SystemCoreClockUpdate();
	osKernelInitialize();
	HAL_Init();   			
	Buttons_Initialize();
	JOY_Init();
	SER_Init();
	LED_Initialize();
	
	// Turn on bus clock for the system configuration to turn on edge detector
	RCC->APB2ENR |= 1<<14;
	
	// USER button:
		// Enable GPIOG clock as USER is connected to port G pin 15	
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
		// Set the EXTI control register to look at Port G's pin 15
		SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI15_PG;	
		// Set the mode of GPIOG pin15 to "INPUT" using GPIOx_MODER register
		GPIOG->MODER &= ~0xc0000000; 
		// Enable EXTI15 - Edge interrupt for all pin 15's
		EXTI->IMR |= 1<<15;
		// configure for falling edge	
		EXTI->FTSR |= 1<<15;  			
		// enables EXTI15
		NVIC_EnableIRQ(EXTI15_10_IRQn);
		
	// WAKEUP button:
		// Enable GPIOC clock as WAKEUP is connected to port A pin 0
		RCC->AHB1ENR |= 1<<0;
			// Set the EXTI control register to look at Port A's pin 0
		SYSCFG->EXTICR[0] |= 0; 
		// Set the mode of GPIOA pin0 to "INPUT" using GPIOx_MODER register
		GPIOG->MODER &= ~0x00000003;
		// Enable EXTI0 - Edge interrupt for all pin 0's
		EXTI->IMR |= 1<<0;				
		// configure for falling edge
		EXTI->FTSR |= 1<<0;			
		// enables EXTI0
		NVIC_EnableIRQ(EXTI0_IRQn);
		
	// TAMPER button:
		// Enable GPIOC clock
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
		// Set the EXTI control register to look at Port C's pin 13
		SYSCFG->EXTICR[3] = (SYSCFG->EXTICR[3] & ~0x000000F0)	|	SYSCFG_EXTICR4_EXTI13_PC;
		// Set the mode of GPIOC pin 13 to INPUT
		GPIOC->MODER &= ~0x0c000000;
		// Enable EXTI10 - Edge interrupt for all pin 13s
		EXTI->IMR |= 1<<13;	
		// Configure for falling edge
		EXTI->FTSR |= 1<<13;
		// EXTI10 already enabled in USER button setup
		/*NVIC_EnableIRQ(EXTI15_10_IRQn);*/
		
	// USART:
		//Enable interrupts over USART
		USART3->CR1 |= USART_CR1_RXNEIE;			
		//Enables interrupts on pin 0
		NVIC_EnableIRQ (USART3_IRQn); //Enables interrupts on USART3
		
	// creates mutexes for global variables
	mutScreen = osMutexNew(NULL);
	if (mutScreen==NULL) while(1){}
	mutMessage = osMutexNew(NULL);
	if (mutMessage==NULL) while(1){}
	mutHour = osMutexNew(NULL);
	if (mutHour==NULL) while(1){}
	mutMinute = osMutexNew(NULL);
	if (mutHour==NULL) while(1){}
	mutSecond = osMutexNew(NULL);
	if (mutHour==NULL) while(1){}
		
	// creates semaphores for thread scheduling
	semDisplayTime= osSemaphoreNew(1/*only one key*/,0 /*initial avail. keys*/,NULL /*no wait time*/);
	if (semDisplayTime == NULL) while(1){}
	semSecond = osSemaphoreNew(1,1,NULL);
	if (semSecond == NULL) while (1){}
	semMinute = osSemaphoreNew(1,0,NULL); 
	if (semMinute == NULL) while (1){}
	semHour = osSemaphoreNew(1,0,NULL);
	if (semHour == NULL) while (1){}
	semWriteMesg=osSemaphoreNew(100, 0, NULL);
	if (semWriteMesg==NULL) while(1){}
	semChar=osSemaphoreNew(100, 0, NULL);
	if (semChar==NULL) while(1){}
	semNewMesg = osSemaphoreNew(100,0,NULL);
	if (semNewMesg==NULL) while(1){}

	// Message memory pool
	messages = osMemoryPoolNew(10, sizeof(message), NULL);
	
	// Character memory pool	
	newChars = osMemoryPoolNew(10,sizeof(uint8_t), NULL);//Memory pool to store received chars
	
	//Message queue to send messages from ISR to thread
	charQueue = osMessageQueueNew (10, sizeof(uint32_t), NULL);
	
	// no memory pool
	if (messages==NULL) while(1){}	//No Memory pool exists.
	
	// memory pool didn't create
	if (newChars==NULL) while(1){}	//Memory pool didnt create.
		
	// creates main thread
  osThreadNew(app_hw_init, NULL, NULL);
  
	// start thread execution	
	osKernelStart();
  
	// infinite loop
	for (;;) {}
}

// USER and TAMPER ISR
void EXTI15_10_IRQHandler(void){
	
	static int32_t i;

	// if: flag bit for EXTI15 is set (USER)
	if (EXTI->PR & 1<<15){
		
		// clears the interrupt flag for EXTI15
		EXTI->PR = 1<<15;
		
		// schedules the second thread
		osSemaphoreRelease(semSecond);
		
		// ends ISR
		return;	
		
	}		
	// if: flag bit for EXTI13 is set (TAMPER)
	else if (EXTI->PR & 1<<13) {
		
		// clears the interrupt flag for EXTI13
		EXTI->PR = 1<<13;
		
		// schedules the minute thread
		osSemaphoreRelease(semMinute);
		
		// ends ISR
		return;			
		
	}
	
	// EXTI10, EXTI11, EXTI12, or EXTI14 were triggered, infinite loop
	else {while(1) {i++;}}

}

// WAKEUP ISR
void EXTI0_IRQHandler(void){
	
	static int32_t i;
	
	// if: flag bit for EXTI0 is set (WAKEUP)
	if (EXTI->PR & 1<<0){
		
		// clears the interrupt flag for EXTI0
		EXTI->PR = 1<<0;
		
		// schedules the hour thread
		osSemaphoreRelease(semHour);
		
		// ends ISR
		return;	
		
	}		
	
	// infinite loop
	else {while(1) {i++;}}

}

void USART3_IRQHandler(void) {	//ISR for USART3
	
	// new character recieved from serial
	uint8_t newChar;
	newChar = SER_GetChar();
	
	// echo character back to terminal
	SER_PutChar(newChar);
	
	// allocates space for the recieved character in the memory pool
	uint32_t* message;
	message = osMemoryPoolAlloc(newChars, 0);
	
	// stores the character in its allocated space
	*message = newChar;
	
	osMessageQueuePut(charQueue, &message, NULL, NULL);	//Place char in queue
	
	// schedules 
	osSemaphoreRelease(semChar);	//Increment semaphore to acknowledge new char received
}