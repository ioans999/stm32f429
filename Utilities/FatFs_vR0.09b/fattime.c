/* Martin Thomas 4/2009 */

#include "integer.h"
#include "fattime.h"
#include "stm32f4xx_rtc.h"

DWORD get_fattime (void)
{
	DWORD res;
	RTC_TimeTypeDef rtcTime;
  RTC_DateTypeDef rtcDate;
	  
	RTC_GetTime( RTC_Format_BIN, &rtcTime );
	RTC_GetDate( RTC_Format_BIN, &rtcDate );	
	
	res =  (((DWORD)rtcDate.RTC_Year - 1980) << 25)
			| ((DWORD)rtcDate.RTC_Month << 21)
			| ((DWORD)rtcDate.RTC_Date << 16)
			| (WORD)(rtcTime.RTC_Hours << 11)
			| (WORD)(rtcTime.RTC_Minutes << 5)
			| (WORD)(rtcTime.RTC_Seconds >> 1);

	return res;
}

