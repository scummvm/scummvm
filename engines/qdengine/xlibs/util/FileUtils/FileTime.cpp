#include "StdAfx.h"
#include "XZip.h"
#include "FileTime.h"
#include "Serialization\Serialization.h"

FileTime::FileTime(const char* fileName)
{
	low_ = high_ = 0;
	if(fileName){
		XZipStream ff(0);
		if(ff.open(fileName, XS_IN)){
			unsigned int date,time;
			ff.gettime(date,time);
			FILETIME filetime = {0,0};
			DosDateTimeToFileTime(date,time,&filetime);
			low_ = filetime.dwLowDateTime;
			high_ = filetime.dwHighDateTime;
		}
	}
}

void FileTime::serialize(Archive& ar)
{
	ar.serialize(low_, "low", "low");
	ar.serialize(high_, "high", "high");
}

void FileTime::setCurrentTime()
{
	FILETIME filetime = {0,0};
	GetSystemTimeAsFileTime(&filetime);
	low_ = filetime.dwLowDateTime;
	high_ = filetime.dwHighDateTime;
}

int FileTime::year() const
{
	FILETIME fileTime = {low_, high_};
	SYSTEMTIME systemTime, localTime;
	FileTimeToSystemTime(&fileTime, &systemTime);
	SystemTimeToTzSpecificLocalTime(0, &systemTime, &localTime);
	return localTime.wYear;
}

int FileTime::month() const
{
	FILETIME fileTime = {low_, high_};
	SYSTEMTIME systemTime, localTime;
	FileTimeToSystemTime(&fileTime, &systemTime);
	SystemTimeToTzSpecificLocalTime(0, &systemTime, &localTime);
	return localTime.wMonth;
}

int FileTime::day() const
{
	FILETIME fileTime = {low_, high_};
	SYSTEMTIME systemTime, localTime;
	FileTimeToSystemTime(&fileTime, &systemTime);
	SystemTimeToTzSpecificLocalTime(0, &systemTime, &localTime);
	return localTime.wDay;
}

int FileTime::hour() const
{
	FILETIME fileTime = {low_, high_};
	SYSTEMTIME systemTime, localTime;
	FileTimeToSystemTime(&fileTime, &systemTime);
	SystemTimeToTzSpecificLocalTime(0, &systemTime, &localTime);
	return localTime.wHour;
}

int FileTime::minute() const
{
	FILETIME fileTime = {low_, high_};
	SYSTEMTIME systemTime, localTime;
	FileTimeToSystemTime(&fileTime, &systemTime);
	SystemTimeToTzSpecificLocalTime(0, &systemTime, &localTime);
	return localTime.wMinute;
}

int FileTime::second() const
{
	FILETIME fileTime = {low_, high_};
	SYSTEMTIME systemTime, localTime;
	FileTimeToSystemTime(&fileTime, &systemTime);
	SystemTimeToTzSpecificLocalTime(0, &systemTime, &localTime);
	return localTime.wSecond;
}

int FileTime::milliseconds() const
{
	FILETIME fileTime = {low_, high_};
	SYSTEMTIME systemTime, localTime;
	FileTimeToSystemTime(&fileTime, &systemTime);
	SystemTimeToTzSpecificLocalTime(0, &systemTime, &localTime);
	return localTime.wMilliseconds;
}

