/*----------------------------------------------------------------------------
	MINUTE THREAD:
		Handles the incrementing of the minute variable after acquiring the mutex
		for the minute variable. When the minute variable reaches 60, it resets to
		0 and schedules the hour thread.
  ---------------------------------------------------------------------------*/

#include "cmsis_os2.h"
#include "rtosPhoneComponents.h"
 
// thread id
osThreadId_t tid_thdMinute;
 
// thread function prototype
void thdMinute (void *argument);
 
// thread initializer
int Init_thdMinute (void) {
 
	tid_thdMinute = osThreadNew(thdMinute, NULL, NULL);
  
	if (tid_thdMinute == NULL) {
		return(-1);
  }
 
  return(0);
}
 
// thread function
void thdMinute (void *argument) {
 
  while (1) {
		
		// tries to acquire the minute semaphore
		osSemaphoreAcquire(semMinute, osWaitForever);
		
		// tries to acquire the minute mutex
		osMutexAcquire(mutMinute, osWaitForever);
		
		// increments the minute variable and resets it at 60
		minute = (minute + 1) % 60;
		
		// when the minute is reset, the hour thread is scheduled
		if (minute == 0) {
			
			osSemaphoreRelease(semHour);
			
		}
		
		// gives up the minute mutex
		osMutexRelease(mutMinute);
		
		// schedules the display thread
		osSemaphoreRelease(semDisplayTime);
		
    // suspends the thread
		osThreadYield();
  }
}
