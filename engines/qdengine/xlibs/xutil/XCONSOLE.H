/*
		   XConsole (Windows 32 API version)
	 By K-D Lab::KranK, Dr.Tronick, K-D Lab::Steeler (C) 1993-97 V3.0nt
*/

#ifndef __XCONSOLE_H
#define __XCONSOLE_H

#undef	NULL
#ifndef NULL
#define NULL	0L
#endif

struct XConsole
{
	int radix;
	int digits;

	int tab;

	void UpdateBuffer(const char *);

	XConsole();
	~XConsole();

	XConsole& operator< (const char*);
	XConsole& operator< (char);
	XConsole& operator< (unsigned char);

	XConsole& operator<= (short);
	XConsole& operator<= (unsigned short);
	XConsole& operator<= (int);
	XConsole& operator<= (unsigned int);
	XConsole& operator<= (long);
	XConsole& operator<= (unsigned long);
	XConsole& operator<= (float);
	XConsole& operator<= (double);
	XConsole& operator<= (long double);

	void SetRadix(int _radix){ radix = _radix; }
	void SetDigits(int _digits){ digits = _digits; }
	void SetTab(int size){ tab = size; }
	void clear();

	void setpos(int _x,int _y);
	void getpos(int &x,int &y);
	void initialize(int mode);
};

extern XConsole XCon;

#endif /* __XCONSOLE_H */

