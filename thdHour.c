/*----------------------------------------------------------------------------
  HOUR THREAD
		Handles the incrementing of the hour variable after acquiring the mutex
		for the hour variable. When the hour variable reaches 24, it resets to 0.
  ---------------------------------------------------------------------------*/

#include "cmsis_os2.h"
#include "rtosPhoneComponents.h" 

// thread id 
osThreadId_t tid_thdHour;
 
// thread function prototype
void thdHour (void *argument);
 
// thread initializer
int Init_thdHour (void) {
 
  tid_thdHour = osThreadNew(thdHour, NULL, NULL);
  if (tid_thdHour == NULL) {
    return(-1);
  }
 
  return(0);
}
 
// thread function
void thdHour (void *argument) {
 
  while (1) {
			
		// tries to acquire the hour semaphore
		osSemaphoreAcquire(semHour, osWaitForever);
		
		// tries to acquire the hour mutex
		osMutexAcquire(mutHour, osWaitForever);
		
		// increments the hour variable and resets it at 24
		hour = (hour + 1) % 24;
		
		// only needed if the clock includes days
		/* 
		if (hour == 0) {
			
			osSemaphoreRelease(semDay, osWaitForever);
			
		}*/
		
		// gives up the hour mutex
		osMutexRelease(mutHour);
		
		// schedules the display thread
		osSemaphoreRelease(semDisplayTime);
		
		// suspends the thread
    osThreadYield();
  }
}
