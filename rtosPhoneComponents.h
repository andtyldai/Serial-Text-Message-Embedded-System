/*----------------------------------------------------------------------------
	PHONECOMPONENTS HEADER FILE
		Contains the declarations of all external resources and threads.
 *---------------------------------------------------------------------------*/

#ifndef __rtosPhoneComponents
	#define __rtosPhoneComponents
	#define LCDWIDTH 320
	#define LCDHEIGHT 240
	#define CHARWIDTH 16
	#define CHARHEIGHT 24
	#include "stdint.h"
	#include "rtx_os.h"
	
	// Message Struct
	typedef struct message message;
	
	typedef struct message{
		
		// Message content
		uint8_t message[160];
		
		// Message length
		uint8_t length;
		
		// Time of message
		uint8_t hourMSD;
		uint8_t minuteMSD;
		uint8_t secondMSD;
		uint8_t hourLSD;
		uint8_t minuteLSD;
		uint8_t secondLSD;
		
		// Link to next and previous messages
		message* next;
		message* prev;
	} message;
	
	// Time
	extern uint8_t hour; 
	extern uint8_t minute; 
	extern uint8_t second;
	
	// Recieved message
	extern message* currentMessage;
	
	// Head of linked-list
	extern message* head;
	
	// Tail of linked-list
	extern message* tail;
	
	// Displayed message
	extern message* viewedMessage;
	
	// Queue for new chars
	extern osMessageQueueId_t charQueue;
	
	// Pool for new chars
	extern osMemoryPoolId_t newChars;
	
	// Pool for messages
	extern osMemoryPoolId_t messages;
	
	// Number of finished messages
	extern uint8_t numOfMesg;
	
	// mutexes
	extern osMutexId_t mutHour; 
	extern osMutexId_t mutMinute; 
	extern osMutexId_t mutSecond;
	extern osMutexId_t mutMessage;
	extern osMutexId_t mutScreen;
	
	// semaphores
	extern osSemaphoreId_t semDisplayTime;
	extern osSemaphoreId_t semTime;
	extern osSemaphoreId_t semSecond;
	extern osSemaphoreId_t semMinute;
	extern osSemaphoreId_t semHour;
	extern osSemaphoreId_t semWriteMesg;
	extern osSemaphoreId_t semChar;
	extern osSemaphoreId_t semNewMesg;
	
	// threads
	extern osThreadId_t tid_thdDisplayTime;
	int Init_thdDisplayTime(void);	
	extern osThreadId_t tid_thdSecond;
	int Init_thdSecond (void);	
	extern osThreadId_t tid_thdMinute;
	int Init_thdMinute(void);
	extern osThreadId_t tid_thdHour;
	int Init_thdHour(void);
	extern osThreadId_t tid_thdMesg;
	int Init_thdMesg (void);
	extern osThreadId_t tid_thdDisplayMesg;
	int Init_thdDisplayMesg (void);
	
	#endif
