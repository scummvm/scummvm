#include "xglobal.h"

XStream& XStream::operator<= (char var)
{
	char* s = itoa(var,ConvertBuffer_,radix);
	write(s,strlen(s));
	return *this;
}

XStream& XStream::operator<= (unsigned char var)
{
	char* s = itoa(var,ConvertBuffer_,radix);
	write(s,strlen(s));
	return *this;
}

XStream& XStream::operator<= (short var)
{
	char* s = itoa(var,ConvertBuffer_,radix);
	write(s,strlen(s));
	return *this;
}

XStream& XStream::operator<= (unsigned short var)
{
	char* s = ltoa(var,ConvertBuffer_,radix);
	write(s,strlen(s));
	return *this;
}

XStream& XStream::operator<= (int var)
{
	char* s = itoa(var,ConvertBuffer_,radix);
	write(s,strlen(s));
	return *this;
}

XStream& XStream::operator<= (unsigned var)
{
	char* s = ultoa(var,ConvertBuffer_,radix);
	write(s,strlen(s));
	return *this;
}

XStream& XStream::operator<= (long var)
{
	char* s = ltoa(var,ConvertBuffer_,radix);
	write(s,strlen(s));
	return *this;
}

XStream& XStream::operator<= (unsigned long var)
{
	char* s = ultoa(var,ConvertBuffer_,radix);
	write(s,strlen(s));
	return *this;
}

XStream& XStream::operator<= (float var)
{
	char* s = gcvt(var,digits,ConvertBuffer_);
	write(s,strlen(s));
	return *this;
}

XStream& XStream::operator<= (double var)
{
	char* s = gcvt(var,digits,ConvertBuffer_);
	write(s,strlen(s));
	return *this;
}

XStream& XStream::operator<= (long double var)
{
	char* s = gcvt(var,digits,ConvertBuffer_);
	write(s,strlen(s));
	return *this;
}
