/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "xglobal.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

XConsole& XConsole::operator< (const char* v)
{
	UpdateBuffer(v);
	return *this;
}

XConsole& XConsole::operator< (char v)
{
	_ConvertBuffer[0]=v; _ConvertBuffer[1]=0;
	UpdateBuffer(&_ConvertBuffer[0]);
	return *this;
}

XConsole& XConsole::operator< (unsigned char v)
{
	_ConvertBuffer[0]=v; _ConvertBuffer[1]=0;
	UpdateBuffer(&_ConvertBuffer[0]);
	return *this;
}

XConsole& XConsole::operator<= (short var)
{
	char* s = itoa(var,_ConvertBuffer,radix);
	UpdateBuffer(s);
	return *this;
}

XConsole& XConsole::operator<= (unsigned short var)
{
	char* s = ltoa(var,_ConvertBuffer,radix);
	UpdateBuffer(s);
	return *this;
}

XConsole& XConsole::operator<= (int var)
{
	char* s = itoa(var,_ConvertBuffer,radix);
	UpdateBuffer(s);
	return *this;
}

XConsole& XConsole::operator<= (unsigned var)
{
	char* s = ltoa(var,_ConvertBuffer,radix);
	UpdateBuffer(s);
	return *this;
}

XConsole& XConsole::operator<= (long var)
{
	char* s = ltoa(var,_ConvertBuffer,radix);
	UpdateBuffer(s);
	return *this;
}

XConsole& XConsole::operator<= (unsigned long var)
{
	char* s = ltoa(var,_ConvertBuffer,radix);
	UpdateBuffer(s);
	return *this;
}

XConsole& XConsole::operator<= (float var)
{
	char* s = gcvt(var,digits,_ConvertBuffer);
	UpdateBuffer(s);
	return *this;
}

XConsole& XConsole::operator<= (double var)
{
	char* s = gcvt(var,digits,_ConvertBuffer);
	UpdateBuffer(s);
	return *this;
}

XConsole& XConsole::operator<= (long double var)
{
	char* s = gcvt(var,digits,_ConvertBuffer);
	UpdateBuffer(s);
	return *this;
}
