/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "xglobal.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */

extern void* hXConOutput;
extern void* hXConInput;

/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

#define _DEF_RADIX	10
#define _DEF_TAB	8
#define _DEF_DIGITS	8

//char _ConvertBuffer[_CONV_BUFFER_LEN + 1];

char* XConTitleString = "XConsole";

void XConsole::initialize(int mode)
{
	AllocConsole();
	SetConsoleTitle(XConTitleString);
	hXConOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	hXConInput = GetStdHandle(STD_INPUT_HANDLE);
}

void XConsole::UpdateBuffer(const char* p)
{
	DWORD cCharsWritten;
	WriteConsole(hXConOutput,p,strlen(p),&cCharsWritten,NULL);
}

void XConsole::clear()
{
	int numChars;
	CONSOLE_SCREEN_BUFFER_INFO conInfo;
	COORD coordScreen = {0, 0};
	DWORD cCharsWritten;

	GetConsoleScreenBufferInfo(hXConOutput,&conInfo);
	numChars = (conInfo.dwSize.X * conInfo.dwSize.Y) - conInfo.dwSize.X;

	FillConsoleOutputCharacter(hXConOutput,0x20,numChars,coordScreen,&cCharsWritten);

	setpos(0,0);
}

void XConsole::getpos(int &x, int &y)
{
	CONSOLE_SCREEN_BUFFER_INFO conInfo;
	GetConsoleScreenBufferInfo(hXConOutput,&conInfo);

	x = conInfo.dwCursorPosition.X;
	y = conInfo.dwCursorPosition.Y;
}

void XConsole::setpos(int x, int y)
{
	COORD pos;
	pos.X = x;
	pos.Y = y;

	SetConsoleCursorPosition(hXConOutput,pos);
}

XConsole::XConsole(void)
{
	radix	= _DEF_RADIX;
	digits	= _DEF_DIGITS;
	tab	= _DEF_TAB;
}

XConsole::~XConsole(void)
{
}

