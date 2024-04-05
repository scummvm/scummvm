#ifndef __WBUFFER_H
#define __WBUFFER_H

#include <memory.h>
#include <stddef.h>

typedef size_t sizetype;
typedef ptrdiff_t difftype;
typedef unsigned char BYTE;

class WBuffer
{
public:
	enum {
		CB_LEN	= 63,
		CB_SIZE = CB_LEN + 1,
		DEFSIZE = 256,
		DEFRADIX = 10,
		DEFDIGITS = 8
	};

	enum {
		BEG = 0,
		CUR = 1,
		END = 2
	};

	WBuffer();
	WBuffer(sizetype sz, bool automaticRealloc = false);
	WBuffer(void* p, sizetype sz);
	~WBuffer(){ free(); }

	int setRadix(int r) { int old = radix_; radix_ = (r <= 0 || r > 16 ? DEFRADIX : r); return old; }
	int setDigits(int d) { int old = digits_; digits_ = (d <= 0 || d > CB_LEN ? DEFDIGITS : d); return old; }

	void alloc(sizetype sz);
	void free();
	bool makeFree() const { return makeFree_; }

	void fill(char fc = '\0');
	void fill(wchar_t fc = L'\0');
	void set(difftype off, int mode = BEG, bool binary = false);
	void init();
	bool end() const { return offset_ > size_; }

	sizetype tell() const { return offset_; }
	sizetype size() const { return size_; }
	BYTE* buffer() const { return buffer_; }

	sizetype read(void* s, sizetype len)
	{
		memcpy(s, buffer_ + offset_, len); 
		offset_ += len; 
		return len; 
	}

	sizetype write(const void* s, difftype len, bool binary = true)
	{	
		if(binary)
			while(offset_ + len > size_) 
				handleOutOfSize();
		else {
			len *= sizeof(wchar_t);
			while(offset_ + len + sizeof(wchar_t) > size_) 
				handleOutOfSize();
		}

		memcpy(buffer_ + offset_, s, len);
		offset_ += len;

		if(!binary)
			reinterpret_cast<wchar_t&>(buffer_[offset_]) = L'\0';

		return len;
	}

	WBuffer& operator< (const wchar_t* v);
	WBuffer& operator< (const char* v);

	template<class T> WBuffer& operator< (T v) { return write(v); }
	template<class T> WBuffer& operator> (T& v) { return read(v); }

	WBuffer& writeNumAnsiBuf(const char* ansi, sizetype maxSize = CB_LEN);

	WBuffer& operator<= (int v) {
		char cb[CB_SIZE];
		return writeNumAnsiBuf(_itoa(v, cb, radix_));
	}
	WBuffer& operator<= (unsigned int v) {
		char cb[CB_SIZE];
		return writeNumAnsiBuf(_itoa(v, cb, radix_));
	}
	WBuffer& operator<= (long v) {
		char cb[CB_SIZE];
		return writeNumAnsiBuf(_ltoa(v, cb, radix_));
	}
	WBuffer& operator<= (unsigned long v) {
		char cb[CB_SIZE];
		return writeNumAnsiBuf(_ultoa(v, cb, radix_));
	}
	WBuffer& operator<= (float v) {
		char cba[CB_SIZE];
		return writeNumAnsiBuf(_gcvt(v, digits_, cba));
	}
	WBuffer& operator<= (double v) {
		char cba[CB_SIZE];
		return writeNumAnsiBuf(_gcvt(v, digits_, cba));
	}

	WBuffer& operator>= (char&);
	WBuffer& operator>= (unsigned char&);
	WBuffer& operator>= (short&);
	WBuffer& operator>= (unsigned short&);
	WBuffer& operator>= (int&);
	WBuffer& operator>= (unsigned int&);
	WBuffer& operator>= (long&);
	WBuffer& operator>= (unsigned long&);
	WBuffer& operator>= (float&);
	WBuffer& operator>= (double&);

	const wchar_t* c_str() const { return reinterpret_cast<const wchar_t*>(buffer_); }
	operator const wchar_t* () const { return reinterpret_cast<const wchar_t*>(buffer_); } 

	WBuffer& operator++(){ offset_ += sizeof(wchar_t); return *this; }
	WBuffer& operator--(){ offset_ -= sizeof(wchar_t); return *this; }
	WBuffer& operator+= (difftype v){ offset_ += v * sizeof(wchar_t); return *this; }
	WBuffer& operator-= (difftype v){ offset_ -= v * sizeof(wchar_t); return *this; }

	const BYTE& operator[](sizetype ind) const { return buffer_[ind]; }
	BYTE& operator[](sizetype ind){ return buffer_[ind]; }
	
	const wchar_t& operator()() const { return reinterpret_cast<const wchar_t&>(buffer_[offset_]); }
	wchar_t& operator()(){ return reinterpret_cast<wchar_t&>(buffer_[offset_]); }

	template<class T> WBuffer& write(const T& v){ while(offset_ + sizeof(T) > size_) handleOutOfSize(); memcpy(&buffer_[offset_], &v, sizeof(T)); offset_ += sizeof(T); return *this; }
	template<class T> WBuffer& read(T& v){ memcpy(&v, &buffer_[offset_], sizeof(T)); offset_ += sizeof(T); return *this; }

private:
	BYTE* buffer_;
	sizetype size_;
	sizetype offset_;
	int radix_;
	int digits_;
	bool makeFree_;
	bool automaticRealloc_;

	long wstrtol(wchar_t*& str, bool unsig);
	double wstrtod(wchar_t*& str);
	void handleOutOfSize();
};

#endif //__WBUFFER_H