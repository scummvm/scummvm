#include "xglobal.h"

XStream& XStream::operator>= (char& var)
{
	int ret = read(ConvertBuffer_,4);
	if(!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = (char)strtol(p,&p,0);
	seek(p - ConvertBuffer_ + 1 - ret,XS_CUR);
	return *this;
}

XStream& XStream::operator>= (unsigned char& var)
{
	int ret = read(ConvertBuffer_,4);
	if(!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = (unsigned char)strtoul(p,&p,0);
	seek(p - ConvertBuffer_ + 1 - ret,XS_CUR);
	return *this;
}

XStream& XStream::operator>= (short& var)
{
	int ret = read(ConvertBuffer_,6);
	if(!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = (short)strtol(p,&p,0);
	seek(p - ConvertBuffer_ + 1 - ret,XS_CUR);
	return *this;
}

XStream& XStream::operator>= (unsigned short& var)
{
	int ret = read(ConvertBuffer_,6);
	if(!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = (unsigned short)strtoul(p,&p,0);
	seek(p - ConvertBuffer_ + 1 - ret,XS_CUR);
	return *this;
}

XStream& XStream::operator>= (int& var)
{
	int ret = read(ConvertBuffer_,16);
	if(!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = strtol(p,&p,0);
	seek(p - ConvertBuffer_ + 1 - ret,XS_CUR);
	return *this;
}

XStream& XStream::operator>= (unsigned& var)
{
	int ret = read(ConvertBuffer_,16);
	if(!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = strtoul(p,&p,0);
	seek(p - ConvertBuffer_ + 1 - ret,XS_CUR);
	return *this;
}

XStream& XStream::operator>= (long& var)
{
	int ret = read(ConvertBuffer_,16);
	if(!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = strtol(p,&p,0);
	seek(p - ConvertBuffer_ + 1 - ret,XS_CUR);
	return *this;
}

XStream& XStream::operator>= (unsigned long& var)
{
	int ret = read(ConvertBuffer_,16);
	if(!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = strtoul(p,&p,0);
	seek(p - ConvertBuffer_ + 1 - ret,XS_CUR);
	return *this;
}

XStream& XStream::operator>= (double& var)
{
	int ret = read(ConvertBuffer_,CONV_BUFFER_LEN);
	if(!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = strtod(p,&p);
	seek(p - ConvertBuffer_ + 1 - ret,XS_CUR);
	return *this;
}

XStream& XStream::operator>= (float& var)
{
	int ret = read(ConvertBuffer_,CONV_BUFFER_LEN);
	if(!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = (float)strtod(p,&p);
	seek(p - ConvertBuffer_ + 1 - ret,XS_CUR);
	return *this;
}

XStream& XStream::operator>= (long double& var)
{
	int ret = read(ConvertBuffer_,CONV_BUFFER_LEN);
	if(!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = strtod(p,&p);
	seek(p - ConvertBuffer_ + 1 - ret,XS_CUR);
	return *this;
}
