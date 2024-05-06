/*----------------------------------------------------------------------------
	DISPLAYMESSAGE THREAD
		Recieves the previously created message objects, displays them on the
		screen with their times, scrolls through them, switchess messages, and
		deletes them with the joystick.
 *---------------------------------------------------------------------------*/

#include "cmsis_os2.h"
#include "rtx_os.h"
#include "rtosPhoneComponents.h"
#include "Board_GLCD.h" 
#include "JOY.h"
 
// thread id
osThreadId_t tid_thdDisplayMesg;

// thread function prototype
void thdDisplayMesg (void *argument);

static uint8_t offset = 0;
uint8_t x, y;
uint32_t joystick;
 
// thread initializer
int Init_thdDisplayMesg(void) {
 
  tid_thdDisplayMesg = osThreadNew (thdDisplayMesg, NULL, NULL);
  if (!tid_thdDisplayMesg) return(-1);  
  return(0);
}
 
// thread function
void thdDisplayMesg (void *argument) {
 
  while (1) {			
			
		// polls every 10 seconds
		osDelay(osKernelGetTickFreq()/10);
		
		// if: message is recieved
		if (osSemaphoreAcquire(semNewMesg, 0)==osOK){
			offset= 0;

			// Tries to acquire the message mutex
			osMutexAcquire(mutMessage, osWaitForever);
			
			// View the tail of the list
			viewedMessage = tail;
			
			// Gives up the message mutex
			osMutexRelease(mutMessage);
							
		}
					
		// if: message on screen needs to be updated
		if (osSemaphoreAcquire(semWriteMesg, 0)==osOK) {
			
			// Adjust k to current value of offset
			uint8_t k = offset*16;
			
			// Tries to acquire the screen mutex
			osMutexAcquire(mutScreen,osWaitForever);
				
			// Clears the current message
			for (y=0;y<4;y++) {
				for (x=0; x<16; x++) {
						GLCD_DrawChar((x+2)*CHARWIDTH,(y+2)*CHARHEIGHT,' ');
				}
			}
			
			// Gives up the message mutex
			osMutexRelease(mutScreen);
				
			// if: there is at least one message
			if (numOfMesg > 0){
				
				// Tries to acquire the message and screen mutexes
				osMutexAcquire(mutMessage,osWaitForever);
				osMutexAcquire(mutScreen,osWaitForever);
				
				// Writes the message on the screen
				x = 0;
				y = 0;
				while(k < viewedMessage->length && y < 3){
					GLCD_DrawChar((x+2)*CHARWIDTH, (y+3)*CHARHEIGHT, viewedMessage->message[k++]);
					x++;
					if(x >= 16){
						x = 0;
						y++;
					}
				}
		
				// Displays the time the message was sent
				GLCD_DrawChar( 6*CHARWIDTH, 6*CHARHEIGHT, viewedMessage->hourMSD);
				GLCD_DrawChar( 7*CHARWIDTH, 6*CHARHEIGHT, viewedMessage->hourLSD);
				GLCD_DrawChar( 8*CHARWIDTH, 6*CHARHEIGHT, ':'    );
				GLCD_DrawChar( 9*CHARWIDTH, 6*CHARHEIGHT, viewedMessage->minuteMSD);
				GLCD_DrawChar( 10*CHARWIDTH, 6*CHARHEIGHT, viewedMessage->minuteLSD);
				GLCD_DrawChar( 11*CHARWIDTH, 6*CHARHEIGHT, ':');		
				GLCD_DrawChar( 12*CHARWIDTH, 6*CHARHEIGHT, viewedMessage->secondMSD);
				GLCD_DrawChar( 13*CHARWIDTH, 6*CHARHEIGHT, viewedMessage->secondLSD);
				
				// Gives up the message and screen mutexes
				osMutexRelease(mutMessage);
				osMutexRelease(mutScreen);
				
			}
			
		}
		
		// Polls the joystick
		joystick = JOY_GetKeys();
		
		switch (joystick){
		// if: joystick is up (scroll up)
		case JOY_LEFT:
			
			// if: message is not scrolled all the way up
			if(offset>0){
					
				// Decrements offset (scrolls up)
				offset--;
				
				// Updates the message on the screen
				osSemaphoreRelease(semWriteMesg);
				
				// Waits until the joystick is released
				do joystick = JOY_GetKeys(); while (joystick == JOY_LEFT );
				
			}
			break;
		
		// if: joystick is down (scroll down)
		case JOY_RIGHT:
			
			// if: message is not scrolled all the way down
			if(offset < (viewedMessage->length / 16) - 2){
					
				// Increments offset (scrolls down)
				offset++;
				
				// Updates the message on the screen
				osSemaphoreRelease(semWriteMesg);
				
				// Waits until the joystick is released
				do joystick = JOY_GetKeys(); while (joystick == JOY_RIGHT);
					
			}
			break;
			
		// if: joystick is right (next message)
		case JOY_UP:
			
			// if: there is at least one message and this isn't the last one
			if (numOfMesg > 0 && viewedMessage != tail) {
				
				// Tries to acquire the message mutex
				osMutexAcquire(mutMessage,osWaitForever);
				
				// View the next message
				viewedMessage = viewedMessage->next;
				
				// Updates the message on the screen
				osSemaphoreRelease(semWriteMesg);
				
				// Resets the scrolling
				offset = 0;
				
				// Gives up the message mutex
				osMutexRelease(mutMessage);
				
				// Waits until the joystick is released
				do joystick = JOY_GetKeys(); while (joystick == JOY_UP );//Waits for joystick to stop being pressed
				
			}
			break;
			
		// if: joystick is left (previous message)
		case JOY_DOWN:
					
			// if: there is at least one message and this isn't the first one
			if (numOfMesg > 0 && viewedMessage != head) {
				
				// Tries to acquire the message mutex
				osMutexAcquire(mutMessage,osWaitForever);
				
				// Views the previous message
				viewedMessage = viewedMessage->prev;
				
				// Updates the message on the screen
				osSemaphoreRelease(semWriteMesg);
				
				// Resets the scrolling
				offset = 0;
				
				// Gives up the message mutex
				osMutexRelease(mutMessage);
				
				// Waits until the joystick is released
				do joystick = JOY_GetKeys(); while (joystick == JOY_DOWN);
				
			}	
			break;
				
		// if: joystick is pressed (delete message)
		case JOY_CENTER:
			
			// if: there is at least one message to delete
			if (numOfMesg > 0) {
				
				// Waits until the joystick is released
				do joystick = JOY_GetKeys(); while (joystick == JOY_CENTER );
				
				// Tries to acquire the screen mutex
				osMutexAcquire(mutScreen,osWaitForever);
				
				// Prints " Press again
				//           to delete  "
				GLCD_DrawChar( 4*CHARWIDTH, 8*CHARHEIGHT, 'P');
				GLCD_DrawChar( 5*CHARWIDTH, 8*CHARHEIGHT, 'r');
				GLCD_DrawChar( 6*CHARWIDTH, 8*CHARHEIGHT, 'e');
				GLCD_DrawChar( 7*CHARWIDTH, 8*CHARHEIGHT, 's');
				GLCD_DrawChar( 8*CHARWIDTH, 8*CHARHEIGHT, 's');
				GLCD_DrawChar( 10*CHARWIDTH, 8*CHARHEIGHT, 'a');
				GLCD_DrawChar( 11*CHARWIDTH, 8*CHARHEIGHT, 'g');
				GLCD_DrawChar( 12*CHARWIDTH, 8*CHARHEIGHT, 'a');
				GLCD_DrawChar( 13*CHARWIDTH, 8*CHARHEIGHT, 'i');
				GLCD_DrawChar( 14*CHARWIDTH, 8*CHARHEIGHT, 'n');
				GLCD_DrawChar( 5*CHARWIDTH, 9*CHARHEIGHT, 't');
				GLCD_DrawChar( 6*CHARWIDTH, 9*CHARHEIGHT, 'o');
				GLCD_DrawChar( 8*CHARWIDTH, 9*CHARHEIGHT, 'd');
				GLCD_DrawChar( 9*CHARWIDTH, 9*CHARHEIGHT, 'e');
				GLCD_DrawChar( 10*CHARWIDTH, 9*CHARHEIGHT, 'l');
				GLCD_DrawChar( 11*CHARWIDTH, 9*CHARHEIGHT, 'e');
				GLCD_DrawChar( 12*CHARWIDTH, 9*CHARHEIGHT, 't');
				GLCD_DrawChar( 13*CHARWIDTH, 9*CHARHEIGHT, 'e');
				
				// Gives up the screen mutex
				osMutexRelease(mutScreen);
		
			
			
				// del = will the message be deleted
				bool del = false;
			
				// If the user presses the joystick again in 5 s, del = TRUE
				for(uint32_t i = 0; i < 500; i++){
				
					// Wait 10 ms
					osDelay(osKernelGetTickFreq()/100);
					if(JOY_GetKeys() == JOY_CENTER){

						del = true;
						break;
				
					}
			
				}
			
				// if: only one message and the user pressed delete twice
				if (numOfMesg == 1 && del){
			
					// Decrements number of messages
					numOfMesg--;
					
					// Deletes the message
					head = NULL;
					tail = NULL;
					tail->prev = head;
					head->next = tail;
			
					// Tries to acquire the screen mutex
					osMutexAcquire(mutScreen,osWaitForever);
					
					// Clears the screen
					GLCD_ClearScreen();
					for (y=0;y<4;y++) {
						for (x=0; x<16; x++) {
							GLCD_DrawChar((x+2)*CHARWIDTH,(y+3)*CHARHEIGHT,' ');
						}
					}
				
					// Gives up the screen mutex
					osMutexRelease(mutScreen);
				
				}
			
				// if: more than one message and the user pressed delete twice
				if (numOfMesg > 1 && del) {
			    
					// Decrements the number of messages
					numOfMesg--;
				
					// If there is a previous message, set its next value
					if(viewedMessage->prev != NULL){
						(viewedMessage->prev)->next = viewedMessage->next;
					}
				
					// If there is a next message, set its previous value
					if(viewedMessage->next != NULL){
						(viewedMessage->next)->prev = viewedMessage->prev;
					}
				
					// View the previous message
					viewedMessage = viewedMessage->prev;
					
					// Resets the scroll
					offset = 0;
				
					// Update the printed message
					osSemaphoreRelease(semWriteMesg);
								
				}

				// Tries to acquire the screen mutex
				osMutexAcquire(mutScreen, osWaitForever);
				
				// Clears delete instructions
				for (y=0; y< 2; y++)	{
					for (x=0; x < 20; x++){
						GLCD_DrawChar( x*CHARWIDTH,(y+8)*CHARHEIGHT, ' ');
					}					
				}
			
				// Gives up screen mutex
				osMutexRelease(mutScreen);
				
			}

		}
		
		// Suspends thread
		osThreadYield ();
	}
	
}
