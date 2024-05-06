/*----------------------------------------------------------------------------
	DISPLAYTIME THREAD
		Makes copies of global time variables and displays them everytime the
		time is updated.
 *---------------------------------------------------------------------------*/

#include "cmsis_os2.h"
#include "rtx_os.h"
#include "rtosPhoneComponents.h"
#include "Board_GLCD.h" 
 
// thread id
osThreadId_t tid_thdDisplayTime;
 
// thread function prototype
void thdDisplayTime (void *argument);

// thread initializer
int Init_thdDisplayTime (void) {
 
  tid_thdDisplayTime = osThreadNew (thdDisplayTime, NULL, NULL);
  if (!tid_thdDisplayTime) return(-1);
  return(0);
	
}
 
void thdDisplayTime (void *argument) {
	
	// local copies of global variables
	uint32_t lhour, lminute, lsecond;
	
	// calculated digits for display
	uint32_t hourMSD, hourLSD, minuteMSD, minuteLSD, secondMSD, secondLSD;
	
  while (1) {
		
		// if: time is updated
		if (osSemaphoreAcquire(semDisplayTime, osWaitForever) == osOK) {
		
			// tries to acquire the mutexes for the time variables
			osMutexAcquire(mutHour, osWaitForever);
			osMutexAcquire(mutMinute, osWaitForever);
			osMutexAcquire(mutSecond, osWaitForever);
			
			// make local copies of global variables
			lhour = hour;
			lminute = minute;
			lsecond = second;
			
			// gives up mutexes after making local copies
			osMutexRelease(mutHour);
			osMutexRelease(mutMinute);
			osMutexRelease(mutSecond);
			
			// convert values to ASCII characters
			hourMSD = (lhour > 9)?0x30+hour/10:0x20;
			hourLSD = 0x30 + lhour % 10;	
			minuteMSD = 0x30 + lminute/10;
			minuteLSD = 0x30 + lminute%10;
			secondMSD = 0x30 + lsecond/10;
			secondLSD = 0x30 + lsecond%10;
			
			// tries to acquire the mutex to access the screen
			osMutexAcquire(mutScreen,osWaitForever);
			
			// displays time using local copies
			GLCD_DrawChar( 0*CHARWIDTH,0, hourMSD);
			GLCD_DrawChar( 1*CHARWIDTH,0, hourLSD);
			GLCD_DrawChar( 2*CHARWIDTH,0, ':');
			GLCD_DrawChar( 3*CHARWIDTH,0, minuteMSD);
			GLCD_DrawChar( 4*CHARWIDTH,0, minuteLSD);
			GLCD_DrawChar( 5*CHARWIDTH,0, ':');		
			GLCD_DrawChar( 6*CHARWIDTH,0, secondMSD);
			GLCD_DrawChar( 7*CHARWIDTH,0, secondLSD);

			// gives up the mutex for the screen
			osMutexRelease(mutScreen);

		}	
  }
}
