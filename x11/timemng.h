

// Win32 SYSTEMTIME ストラクチャ

#if defined(__GNUC__)
typedef struct {
	BYTE	year[2];
	BYTE	month[2];
	BYTE	week[2];
	BYTE	day[2];
	BYTE	hour[2];
	BYTE	minute[2];
	BYTE	second[2];
	BYTE	milli[2];
} __attribute__ ((packed)) _SYSTIME;
#else
#pragma pack(push, 1)
typedef struct {
	BYTE	year[2];
	BYTE	month[2];
	BYTE	week[2];
	BYTE	day[2];
	BYTE	hour[2];
	BYTE	minute[2];
	BYTE	second[2];
	BYTE	milli[2];
} _SYSTIME;
#pragma pack(pop)
#endif


#ifdef __cplusplus
extern "C" {
#endif

BOOL timemng_gettime(_SYSTIME *systime);

#ifdef __cplusplus
}
#endif

