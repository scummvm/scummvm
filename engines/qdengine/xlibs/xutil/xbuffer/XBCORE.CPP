#include "xglobal.h"

XBuffer::XBuffer()
{ 
	makeFree_=0;
	buffer_=NULL;
	alloc(XB_DEFSIZE);
	radix_ = XB_DEFRADIX;
	digits_ = XB_DEFDIGITS;
	automaticRealloc_ = true;
}

XBuffer::XBuffer(unsigned int sz, bool automatic_realloc_)
{ 
	makeFree_=0;
	buffer_=NULL;
	alloc(sz);
	radix_ = XB_DEFRADIX;
	digits_ = XB_DEFDIGITS;
	automaticRealloc_ = automatic_realloc_;
}

void XBuffer::alloc(unsigned int sz)
{
	free();
	buffer_ = (char*)malloc(size_ = sz);
	offset_ = 0L;
	radix_ = XB_DEFRADIX;
	digits_ = XB_DEFDIGITS;
	makeFree_ = 1;
	if(size_) 
		*buffer_ = 0;
}

XBuffer::XBuffer(void* p,int sz)
{
	buffer_ = (char*)p;
	size_ = sz;
	offset_ = 0;
	radix_ = XB_DEFRADIX;
	digits_ = XB_DEFDIGITS;
	makeFree_ = 0;
	automaticRealloc_ = 0;
}

void XBuffer::free()
{
	if(makeFree_ && buffer_){
		::free(buffer_);
		buffer_ = NULL;
		}
}

void XBuffer::fill(char fc)
{ memset(buffer_,fc,size_); }

void XBuffer::set(int off,int mode)
{
	switch(mode){
		case XB_BEG:
			offset_ = off;
			break;
		case XB_CUR:
			offset_+=off;
			break;
		case XB_END:
			offset_ = size_ - off;
			break;
		}
}

void XBuffer::init()
{ 
	offset_ = 0; 
	if(size())
		*buffer_ = 0; 
}

unsigned int XBuffer::read(void* s, unsigned int len) 
{	
	memcpy(s, buffer_ + offset_, len); 
	offset_ += len; 
	return len; 
}

void XBuffer::handleOutOfSize()
{
	if(automaticRealloc_){
		buffer_ = (char*)realloc(buffer_, size_ *= 2);
		if(!buffer_){
			xassert(0 && "Out of XBuffer (low of system memory)");
			ErrH.Abort("Out of XBuffer (low of system memory)");
		}
	}
	else {
		xassert(0 && "Out of XBuffer");
		ErrH.Abort("Out of XBuffer");
	}
}

unsigned int XBuffer::write(const void* s, unsigned int len, bool bin_flag) 
{	
	if(bin_flag)
		while(offset_ + len > size_) 
			handleOutOfSize();
	else
		while(offset_ + len >= size_) 
			handleOutOfSize();

	memcpy(buffer_ + offset_, s, len);
	offset_ += len;

	if(!bin_flag)
		buffer_[offset_] = '\0';

	return len;
}

XBuffer& XBuffer::operator< (const char* v) 
{ 
	if(v) 
		write(v, strlen(v), 0); 
	return *this; 
}

XBuffer& XBuffer::operator> (char* v) 
{ 
	if(v) 
		read(v, strlen(buffer_ + offset_) + 1); 
	return *this; 
}

