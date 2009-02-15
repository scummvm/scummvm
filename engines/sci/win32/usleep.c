#ifdef _WIN32

#include <windows.h>

extern void
usleep (long usec)
{
	LARGE_INTEGER lFrequency;
	LARGE_INTEGER lEndTime;
	LARGE_INTEGER lCurTime;

	QueryPerformanceFrequency (&lFrequency);
	if (lFrequency.QuadPart)
	{
		QueryPerformanceCounter (&lEndTime);
		lEndTime.QuadPart += (LONGLONG) usec * lFrequency.QuadPart / 1000000;
		do
		{
			QueryPerformanceCounter (&lCurTime);
			Sleep(0);
		} while (lCurTime.QuadPart < lEndTime.QuadPart);
	}
}

#endif
