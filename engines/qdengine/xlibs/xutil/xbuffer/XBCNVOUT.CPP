#include "xglobal.h"
#include "math.h"

double strtod_(char* str, char** strOut)
{
    while(isspace(*str))
		++str;

	bool positive = true;
    if(*str == '-'){
		positive = false;
        ++str;
    }

    double val = 0;
    while(isdigit(*str)){
        val *= 10;
        val += (double)(*str++ - '0');
    }
    
	if(*str == '.'){
		++str;
        double dig = 1;
        while(isdigit(*str)){
			dig  /= 10;
            val += ((double)(*str++ - '0'))*dig;
        }
    }
    
	if(*str == 'e' || *str == 'E' || *str == 'd' || *str == 'D'){
		bool positivePower = true;
		if(*++str == '+')
			++str;
		else if(*str == '-'){
			positivePower = false;
			++str;
		}
		int power = strtol(str, &str, 10);
		if(!positivePower)
			power = -power;
		val *= pow(10., power);
	}

	*strOut = str;
    return positive ? val : -val;
}

XBuffer& XBuffer::operator>= (char& var)
{
	char* p = buffer_ + offset_;
	var = (char)strtol(p,&p,0);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (unsigned char& var)
{
	char* p = buffer_ + offset_;
	var = (unsigned char)strtoul(p,&p,0);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (short& var)
{
	char* p = buffer_ + offset_;
	var = (short)strtol(p,&p,0);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (unsigned short& var)
{
	char* p = buffer_ + offset_;
	var = (unsigned short)strtoul(p,&p,0);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (int& var)
{
	char* p = buffer_ + offset_;
	var = strtol(p,&p,0);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (unsigned& var)
{
	char* p = buffer_ + offset_;
	var = strtoul(p,&p,0);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (long& var)
{
	char* p = buffer_ + offset_;
	var = strtol(p,&p,0);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (unsigned long& var)
{
	char* p = buffer_ + offset_;
	var = strtoul(p,&p,0);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (double& var)
{
	char* p = buffer_ + offset_;
	var = strtod_(p,&p);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (long double& var)
{
	char* p = buffer_ + offset_;
	var = strtod_(p,&p);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (float& var)
{
	char* p = buffer_ + offset_;
	var = (float)strtod_(p,&p);
	offset_ += p - (buffer_ + offset_);
	return *this;
}



