#include "xglobal.h"
#include "WBuffer.h"
#include <string>
#include "UnicodeConverter.h"
#include <math.h>

WBuffer::WBuffer()
{ 
	makeFree_ = false;
	buffer_ = 0;
	alloc(DEFSIZE);
	radix_ = DEFRADIX;
	digits_ = DEFDIGITS;
	automaticRealloc_ = true;
}

WBuffer::WBuffer(sizetype sz, bool automaticRealloc)
{ 
	makeFree_ = false;
	buffer_ = 0;
	alloc(sz);
	radix_ = DEFRADIX;
	digits_ = DEFDIGITS;
	automaticRealloc_ = automaticRealloc;
}

WBuffer::WBuffer(void* p, sizetype sz)
{
	buffer_ = reinterpret_cast<BYTE*>(p);
	size_ = sz;
	offset_ = 0;
	radix_ = DEFRADIX;
	digits_ = DEFDIGITS;
	makeFree_ = false;
	automaticRealloc_ = false;
}

void WBuffer::free()
{
	if(makeFree_ && buffer_){
		::free(buffer_);
		buffer_ = 0;
	}
}

void WBuffer::alloc(sizetype sz)
{
	free();
	sz += sz % 2;
	buffer_ = reinterpret_cast<BYTE*>(malloc(size_ = sz));
	offset_ = 0;
	radix_ = DEFRADIX;
	digits_ = DEFDIGITS;
	makeFree_ = true;
	if(size_) 
		*reinterpret_cast<wchar_t*>(buffer_) = L'\0';
}

void WBuffer::fill(char fc)
{
	memset(buffer_, fc, size_);
}

void WBuffer::fill(wchar_t fc)
{
	memset(buffer_, fc, size_ / 2);
	if(size_ % 2)
		buffer_[size_ - 1] = 0;
}

void WBuffer::set(difftype off, int mode, bool binary)
{
	if(!binary)
		off *= sizeof(wchar_t);
	switch(mode){
		case BEG:
			offset_ = off;
			break;
		case CUR:
			offset_ += off;
			break;
		case END:
			offset_ = size_ - off;
			break;
	}
}

void WBuffer::init()
{ 
	offset_ = 0; 
	if(size())
		*reinterpret_cast<wchar_t*>(buffer_) = L'\0';
}

WBuffer& WBuffer::writeNumAnsiBuf(const char* ansi, sizetype maxSize)
{
	wchar_t cbw[CB_SIZE];
	for(wchar_t* pbuf = cbw; *ansi && maxSize; ++ansi, ++pbuf, --maxSize){
		xassert(*ansi < 0x7F);
		*pbuf = static_cast<wchar_t>(*ansi);
	}
	write(cbw, pbuf - cbw, false);
	return *this;
}

WBuffer& WBuffer::operator< (const wchar_t* var)
{
	if(var)
		write(var, wcslen(var), false);
	return *this;
}

WBuffer& WBuffer::operator< (const char* var)
{
	std::wstring buf;
	a2w(buf, var);
	write(buf.c_str(), buf.size(), false);
	return *this;
}

WBuffer& WBuffer::operator>= (char& var)
{
	wchar_t* p = reinterpret_cast<wchar_t*>(buffer_ + offset_);
	var = static_cast<char>(wstrtol(p, false));
	offset_ += reinterpret_cast<BYTE*>(p) - (buffer_ + offset_);
	return *this;
}

WBuffer& WBuffer::operator>= (unsigned char& var)
{
	wchar_t* p = reinterpret_cast<wchar_t*>(buffer_ + offset_);
	var = static_cast<unsigned char>(wstrtol(p, true));
	offset_ += reinterpret_cast<BYTE*>(p) - (buffer_ + offset_);
	return *this;
}

WBuffer& WBuffer::operator>= (short& var)
{
	wchar_t* p = reinterpret_cast<wchar_t*>(buffer_ + offset_);
	var = static_cast<short>(wstrtol(p, false));
	offset_ += reinterpret_cast<BYTE*>(p) - (buffer_ + offset_);
	return *this;
}

WBuffer& WBuffer::operator>= (unsigned short& var)
{
	wchar_t* p = reinterpret_cast<wchar_t*>(buffer_ + offset_);
	var = static_cast<unsigned short>(wstrtol(p, true));
	offset_ += reinterpret_cast<BYTE*>(p) - (buffer_ + offset_);
	return *this;
}

WBuffer& WBuffer::operator>= (int& var)
{
	wchar_t* p = reinterpret_cast<wchar_t*>(buffer_ + offset_);
	var = static_cast<int>(wstrtol(p, false));
	offset_ += reinterpret_cast<BYTE*>(p) - (buffer_ + offset_);
	return *this;
}

WBuffer& WBuffer::operator>= (unsigned int& var)
{
	wchar_t* p = reinterpret_cast<wchar_t*>(buffer_ + offset_);
	var = static_cast<unsigned int>(wstrtol(p, true));
	offset_ += reinterpret_cast<BYTE*>(p) - (buffer_ + offset_);
	return *this;
}

WBuffer& WBuffer::operator>= (long& var)
{
	wchar_t* p = reinterpret_cast<wchar_t*>(buffer_ + offset_);
	var = wstrtol(p, false);
	offset_ += reinterpret_cast<BYTE*>(p) - (buffer_ + offset_);
	return *this;
}

WBuffer& WBuffer::operator>= (unsigned long& var)
{
	wchar_t* p = reinterpret_cast<wchar_t*>(buffer_ + offset_);
	var = wstrtol(p, true);
	offset_ += reinterpret_cast<BYTE*>(p) - (buffer_ + offset_);
	return *this;
}

WBuffer& WBuffer::operator>= (float& var)
{
	wchar_t* p = reinterpret_cast<wchar_t*>(buffer_ + offset_);
	var = static_cast<float>(wstrtod(p));
	offset_ += reinterpret_cast<BYTE*>(p) - (buffer_ + offset_);
	return *this;
}

WBuffer& WBuffer::operator>= (double& var)
{
	wchar_t* p = reinterpret_cast<wchar_t*>(buffer_ + offset_);
	var = wstrtod(p);
	offset_ += reinterpret_cast<BYTE*>(p) - (buffer_ + offset_);
	return *this;
}

long WBuffer::wstrtol(wchar_t*& str, bool unsig)
{
	while(iswspace(*str))
		++str;

	bool positive = true;
	if(*str == L'-'){
		if(unsig)
			return 0;
		positive = false;
		++str;
	}
	else if(*str == L'+')
		++str;

	long val = 0;
	while(iswdigit(*str)){
		val *= radix_;
		val += *str++ - L'0';
	}

	return positive ? val : -val;
}

double WBuffer::wstrtod(wchar_t*& str)
{
	while(iswspace(*str))
		++str;

	bool positive = true;
	if(*str == L'-'){
		positive = false;
		++str;
	}
	else if(*str == L'+')
		++str;

	double basement = static_cast<double>(radix_);
	double val = 0.;
	while(iswdigit(*str)){
		val *= basement;
		val += (double)(*str++ - L'0');
	}

	if(*str == L'.'){
		++str;
		double dig = 1;
		while(iswdigit(*str)){
			dig /= basement;
			val += ((double)(*str++ - L'0'))*dig;
		}
	}

	if(*str == L'e' || *str == L'E'){
		bool positivePower = true;
		if(*++str == L'+')
			++str;
		else if(*str == L'-'){
			positivePower = false;
			++str;
		}
		int power = wcstol(str, &str, radix_);
		if(!positivePower)
			power = -power;
		
		val *= pow(basement, power);
	}

	return positive ? val : -val;
}

void WBuffer::handleOutOfSize()
{
	if(automaticRealloc_){
		buffer_ = reinterpret_cast<BYTE*>(realloc(buffer_, size_ *= 2));
		if(!buffer_){
			xassert(0 && "Out of WBuffer (low of system memory)");
			ErrH.Abort("Out of WBuffer (low of system memory)");
		}
	}
	else {
		xassert(0 && "Out of WBuffer");
		ErrH.Abort("Out of WBuffer");
	}
}