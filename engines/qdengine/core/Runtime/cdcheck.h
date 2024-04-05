#ifndef __CDCHECK_H__
#define __CDCHECK_H__

#ifndef _DEBUG
#define _CDCHECK_
#endif

extern "C" int WINAPI OpenCD(BYTE drive_letter);
extern "C" int WINAPI FindCD();
extern "C" int WINAPI SetSpeed(BYTE speed);
extern "C" int WINAPI ReadCD();
extern "C" int WINAPI ReadCD2();
extern "C" int WINAPI ReadCheck(int mode);
extern "C" int WINAPI RunCheck(char *key);
extern "C" int WINAPI CloseCD();

#endif // __CDCHECK_H__
