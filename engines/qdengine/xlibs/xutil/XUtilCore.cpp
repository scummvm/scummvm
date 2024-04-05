#include "xglobal.h"
//#include <ostream.h>

void xtSysFinit(){}
void xtClearMessageQueue(){}

void win32_break(char* error,char* msg)
{
//	cerr << "--------------------------------\n";
//	cerr << error << "\n";
//	cerr << msg << "\n";
//	cerr  << "--------------------------------" << endl;
//	_ASSERT(FALSE) ;
}

const char* check_command_line(const char* switch_str)
{
	for(int i = 1; i < __argc; i ++){
		const char* s = strstr(__argv[i], switch_str);
		if(s){
			for(const char* p = __argv[i]; p < s; p++)
				if(*p != '-' && *p != '/')
					goto cont;
			s += strlen(switch_str);
			if(!*s && i + 1 < __argc)
				return __argv[i + 1];
			else
				return s;
			}
		cont:;
		}
	return 0;
}

#include <strsafe.h>

inline void swap(unsigned char& c1, unsigned char& c2)
{
	unsigned char tmp = c2;
	c2 = c1;
	c1 = tmp;
}

/// кодирование/раскодирование по алгоритму RC4
void rc4code(char* data, const char* key, int count = -1)
{
	size_t keylen;
	if(FAILED(StringCchLength(key, 32, &keylen)))
		return;

	if(keylen == 0 || count == 0)
		return;

	const int boxsize = 256; //2 ^ 8;
	unsigned char box[boxsize];

	unsigned int i = 0;

	for(i = 0; i < boxsize; ++i)
		box[i] = i;

	unsigned char j = 0;
	for(i = 0; i < boxsize; ++i){
		j = (j + box[i] + key[i % keylen]) % boxsize;
		swap(box[i], box[j]);
	}

	j = 0;
	i = 0;
	for(char* ptr = data; count > 0 || count < 0 && *ptr; ++ptr, --count){
		i = (i + 1) % boxsize;
		j = (j + box[i]) % boxsize;
		swap(box[i], box[j]);
		*ptr = *ptr ^ box[(box[i] + box[j]) % boxsize];
	}
}

// вывод Unicode текста в консоль
void dcprintfW(wchar_t *format, ...)
{
	static bool inited = false;
	if(!inited){
		inited = true;
		COORD consSize = {256, 2048};
		SMALL_RECT rect = {0, 0, 80, 80};
		AllocConsole();
		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), consSize);
		SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &rect);
	}

	wchar_t buffer[512];

	va_list args;
	va_start(args, format);

	int ret = StringCchVPrintfW(buffer, sizeof(buffer) / sizeof(buffer[0]) - 1, format, args);

	va_end(args);

	if(SUCCEEDED(ret) || ret == STRSAFE_E_INSUFFICIENT_BUFFER){
		DWORD written;
		WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), buffer, min((DWORD)wcslen(buffer), 511), &written, 0);
	}
}

// вывод Unicode текста в Debug Output
void dprintfW(wchar_t *format, ...)
{
	wchar_t buffer[512];

	va_list args;
	va_start(args, format);

	int ret = StringCchVPrintfW(buffer, sizeof(buffer) / sizeof(buffer[0]) - 1, format, args);

	va_end(args);

	if(SUCCEEDED(ret) || ret == STRSAFE_E_INSUFFICIENT_BUFFER)
		OutputDebugStringW(buffer);
}