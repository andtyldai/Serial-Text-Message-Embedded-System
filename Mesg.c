/*----------------------------------------------------------------------------
	MESSAGE THREAD
		Recieves characters and adds them to the message.
 *---------------------------------------------------------------------------*/

#include "cmsis_os2.h"
#include "rtx_os.h"
#include "rtosPhoneComponents.h" 

// thread id
osThreadId_t tid_thdMesg;

// thread function prototype
void thdMesg (void *argument);

uint8_t* msg;

// done = 1 when there is no current message
static int8_t done = 1;
osStatus_t status;
message* currentMessage;
message* head;
message* tail;
message* viewedMessage;

// thread initializer
int Init_thdMesg (void) {
 
  tid_thdMesg = osThreadNew (thdMesg, NULL, NULL);
  if (!tid_thdMesg) return(-1);  
  return(0);
}

// thread function
void thdMesg (void *argument) {
 
  while (1) {
		
		if (osSemaphoreAcquire(semChar, osWaitForever) == osOK) {	
						
			// if: no messages are stored or in progress
			if (numOfMesg == 0 && done == 1) {		//If there are no messages
				
				// tries to acquire the mutex for the message
				osMutexAcquire(mutMessage, osWaitForever);
				
				// Reset current message to be able to recieve new message
				currentMessage = osMemoryPoolAlloc(messages, NULL);
				head = currentMessage;
				currentMessage->next = tail;
				tail->prev = currentMessage;
				currentMessage->prev = head;
				head->next = currentMessage;
				currentMessage->length = 0;
				
				// Message in progress
				done = 0;
				
				// Gives up the message mutex
				osMutexRelease(mutMessage);
			}
			
			// if: there are less than 10, but none are in progress
			if (numOfMesg < 10 && done == 1) {
				
				// Tries to acquire the mutex message
				osMutexAcquire(mutMessage, osWaitForever);
				
				// Reset current message to be able to recieve new message
				currentMessage = osMemoryPoolAlloc(messages, 0);
				tail->next = currentMessage;
				head->prev = currentMessage;
				currentMessage->prev = tail;
				currentMessage->next = head;
				currentMessage->length = 0;
				
				// Message in progress
				done = 0;
				
				// Gives up the message mutex
				osMutexRelease(mutMessage);
				
			}
			
			// Gets the char from the queue
			status = osMessageQueueGet(charQueue, &msg, NULL, osWaitForever); // get the character from the QUEUE
			
			// if: there is a started message and there is a char in the queue
			if (done == 0 && status == osOK) {
				
				// Tries to acquire the message mutex
				osMutexAcquire(mutMessage, osWaitForever);

				
				// if: *msg = 0x20-0x80 and the message is not full
				if((*msg >= 0x20 && *msg <= 0x80) && currentMessage->length < 160){
					
					// adds *msg to the currentMessage and increments the length
					currentMessage->message[(currentMessage->length)++] = *msg;
					
				}
	
				// Force ends the message if it exceeds 160 characters
				if (currentMessage->length >= 160) {*msg = 0x0D;}
				
				if (*msg == 0x0D){
					
					// Tries to acquire the time mutexes
					osMutexAcquire(mutHour, osWaitForever);
					osMutexAcquire(mutMinute, osWaitForever);
					osMutexAcquire(mutSecond, osWaitForever);
				
					// Gets message time
					currentMessage->hourMSD = (hour > 9)?0x30+hour/10:0x20;
					currentMessage->hourLSD = 0x30 + hour % 10;	
					currentMessage->minuteMSD = 0x30 + minute/10;
					currentMessage->minuteLSD = 0x30 + minute%10;
					currentMessage->secondMSD = 0x30 + second/10;
					currentMessage->secondLSD = 0x30 + second%10;
					
					// gives up mutexes after making local copies
					osMutexRelease(mutHour);
					osMutexRelease(mutMinute);
					osMutexRelease(mutSecond);
			
					numOfMesg++;
					done = 1;
					
					// Makes the current message the tail
					tail = currentMessage;
					
										
					// Updates the viewedMessage
					osSemaphoreRelease(semNewMesg);
					
					// Updates the screen
					osSemaphoreRelease(semWriteMesg);
				}

				// Gives up the message mutex
				osMutexRelease(mutMessage);
			}
			
			// Frees the char's memory
			osMemoryPoolFree(newChars, msg);
		}	
		
		// Suspends the thread
		osThreadYield ();
	}
}
