#include "xglobal.h"

char* XStream::getline(char* buf, unsigned len)
{
	int i = -1;
	do {
		i++;
		read(&buf[i],1);
	   } while(buf[i] != '\n' && i < (int)len && !eof());
	if(eof())
		buf[i] = '\0';
	else
		buf[i - 1] = '\0';
	return buf;
}

