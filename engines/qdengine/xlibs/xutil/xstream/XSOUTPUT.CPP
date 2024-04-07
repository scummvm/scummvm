#include "xglobal.h"

XStream& XStream::operator> (char& v)
{
	read(&v,(unsigned)sizeof(char));
	return *this;
}

XStream& XStream::operator> (unsigned char& v)
{
	read(&v,(unsigned)sizeof(char));
	return *this;
}

XStream& XStream::operator> (short& v)
{
	read(&v,(unsigned)sizeof(short));
	return *this;
}

XStream& XStream::operator> (unsigned short& v)
{
	read(&v,(unsigned)sizeof(unsigned short));
	return *this;
}

XStream& XStream::operator> (int& v)
{
	read(&v,(unsigned)sizeof(int));
	return *this;
}

XStream& XStream::operator> (unsigned& v)
{
	read(&v,(unsigned)sizeof(unsigned));
	return *this;
}

XStream& XStream::operator> (long& v)
{
	read(&v,(unsigned)sizeof(long));
	return *this;
}

XStream& XStream::operator> (unsigned long& v)
{
	read(&v,(unsigned)sizeof(unsigned long));
	return *this;
}

XStream& XStream::operator> (double& v)
{
	read(&v,(unsigned)sizeof(double));
	return *this;
}

XStream& XStream::operator> (float& v)
{
	read(&v,(unsigned)sizeof(float));
	return *this;
}

XStream& XStream::operator> (long double& v)
{
	read(&v,(unsigned)sizeof(long double));
	return *this;
}




