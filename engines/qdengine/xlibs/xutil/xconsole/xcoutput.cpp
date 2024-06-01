/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/xlibs/xutil/xglobal.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

XConsole& XConsole::operator< (const char* v) {
	UpdateBuffer(v);
	return *this;
}

XConsole& XConsole::operator< (char v) {
	_ConvertBuffer[0] = v;
	_ConvertBuffer[1] = 0;
	UpdateBuffer(&_ConvertBuffer[0]);
	return *this;
}

XConsole& XConsole::operator< (unsigned char v) {
	_ConvertBuffer[0] = v;
	_ConvertBuffer[1] = 0;
	UpdateBuffer(&_ConvertBuffer[0]);
	return *this;
}

XConsole& XConsole::operator<= (short var) {
	Common::String s = Common::String::format("%hd", var);
	UpdateBuffer(s.c_str());
	return *this;
}

XConsole& XConsole::operator<= (unsigned short var) {
	Common::String s = Common::String::format("%u", var);
	UpdateBuffer(s.c_str());
	return *this;
}

XConsole& XConsole::operator<= (int var) {
	Common::String s = Common::String::format("%d", var);
	UpdateBuffer(s.c_str());
	return *this;
}

XConsole& XConsole::operator<= (unsigned var) {
	Common::String s = Common::String::format("%u", var);
	UpdateBuffer(s.c_str());
	return *this;
}

XConsole& XConsole::operator<= (long var) {
	Common::String s = Common::String::format("%ld", var);
	UpdateBuffer(s.c_str());
	return *this;
}

XConsole& XConsole::operator<= (unsigned long var) {
	Common::String s = Common::String::format("%lu", var);
	UpdateBuffer(s.c_str());
	return *this;
}

XConsole& XConsole::operator<= (float var) {
	char* s = gcvt(var, digits, _ConvertBuffer);
	UpdateBuffer(s);
	return *this;
}

XConsole& XConsole::operator<= (double var) {
	char* s = gcvt(var, digits, _ConvertBuffer);
	UpdateBuffer(s);
	return *this;
}

XConsole& XConsole::operator<= (long double var) {
	char* s = gcvt(var, digits, _ConvertBuffer);
	UpdateBuffer(s);
	return *this;
}
