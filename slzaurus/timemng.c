#include	"compiler.h"
#include	"timemng.h"
#include	<time.h>


BOOL timemng_gettime(_SYSTIME *systime) {

	time_t	long_time;
struct tm	*now_time;

	time(&long_time);
	now_time = localtime(&long_time);
	if (now_time != NULL) {
		STOREINTELWORD(systime->year, now_time->tm_year + 1900);
		STOREINTELWORD(systime->month, now_time->tm_mon + 1);
		STOREINTELWORD(systime->week, now_time->tm_wday);
		STOREINTELWORD(systime->day, now_time->tm_mday);
		STOREINTELWORD(systime->hour, now_time->tm_hour);
		STOREINTELWORD(systime->minute, now_time->tm_min);
		STOREINTELWORD(systime->second, now_time->tm_sec);
		STOREINTELWORD(systime->milli, 0);
		return(SUCCESS);
	}
	else {
		return(FAILURE);
	}
}

