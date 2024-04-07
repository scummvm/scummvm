#include "xglobal.h"

XStream& XStream::operator< (const char* v)
{
	write(v,(unsigned)strlen(v));
	return *this;
}

XStream& XStream::operator< (char v)
{
	write(&v,(unsigned)sizeof(char));
	return *this;
}

XStream& XStream::operator< (unsigned char v)
{
	write(&v,(unsigned)sizeof(char));
	return *this;
}

XStream& XStream::operator< (short v)
{
	write(&v,(unsigned)sizeof(short));
	return *this;
}

XStream& XStream::operator< (unsigned short v)
{
	write(&v,(unsigned)sizeof(unsigned short));
	return *this;
}

XStream& XStream::operator< (int v)
{
	write(&v,(unsigned)sizeof(int));
	return *this;
}

XStream& XStream::operator< (unsigned v)
{
	write(&v,(unsigned)sizeof(unsigned));
	return *this;
}

XStream& XStream::operator< (long v)
{
	write(&v,(unsigned)sizeof(long));
	return *this;
}

XStream& XStream::operator< (unsigned long v)
{
	write(&v,(unsigned)sizeof(unsigned long));
	return *this;
}

XStream& XStream::operator< (double v)
{
	write(&v,(unsigned)sizeof(double));
	return *this;
}

XStream& XStream::operator< (float v)
{
	write(&v,(unsigned)sizeof(float));
	return *this;
}

XStream& XStream::operator< (long double v)
{
	write(&v,(unsigned)sizeof(long double));
	return *this;
}




