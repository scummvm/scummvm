#ifdef _MSC_VER
// inline keyword only supported in C++
#	undef inline /* just to be sure it is not defined */
#	define inline __inline
#	define strcasecmp _stricmp
#	define strncasecmp _strnicmp
#	define snprintf _snprintf

#	if _MSC_VER < 1500
#		define vsnprintf _vsnprintf
#	endif
#endif

#ifdef _WIN32

#	ifdef sleep
#		undef sleep
#	endif

#	define sleep(x) \
	do { \
		if (x == 0) { \
			Sleep(0); \
		} else { \
			if (timeBeginPeriod(1) != TIMERR_NOERROR) \
				fprintf(stderr, "timeBeginPeriod(1) failed\n"); \
			Sleep(x); \
			if (timeEndPeriod(1) != TIMERR_NOERROR) \
				fprintf(stderr, "timeEndPeriod(1) failed\n"); \
		} \
	} while (0);

#	define RECT_T_TO_RECT(rect_t) \
	{ (rect_t).x, (rect_t).y, (rect_t).x + (rect_t).xl, (rect_t).y + (rect_t).yl }

#endif

