/*----------------------------------------------------------------------------
  SECOND THREAD
		Handles the incrementing of the second variable after acquiring the mutex
		for the second variable. When the second variable reaches 60, it resets to
		0 and schedules the minute thread.
  ---------------------------------------------------------------------------*/

#include "cmsis_os2.h"
#include "rtosPhoneComponents.h"
 
// thread id
osThreadId_t tid_thdSecond;

// thread function prototype
void thdSecond (void *argument);

// thread initializer
int Init_thdSecond (void) {
 
  tid_thdSecond = osThreadNew(thdSecond, NULL, NULL);
	
  if (tid_thdSecond == NULL) {
    return(-1);
  }
 
  return(0);
}

// thread function
void thdSecond (void *argument) {
 
  while (1) {
		
		// tries to acquire the second semaphore
		osSemaphoreAcquire(semSecond, 0);
		
		// waits 1 second 
		osDelay(osKernelGetTickFreq());
		
		// tries to acquire the second mutex 
    osMutexAcquire(mutSecond, osWaitForever);

		// increments the second variable and resets it at 60
		second = (second + 1) % 60;
		
		// when the second is reset, the minute thread is scheduled
		if (second == 0) {
		
			osSemaphoreRelease(semMinute);
		
		}
		
		// gives up the second mutex
		osMutexRelease(mutSecond);
				
		// schedules the display thread
		osSemaphoreRelease(semDisplayTime);
		
    // suspends the thread
		osThreadYield();
  }
}
