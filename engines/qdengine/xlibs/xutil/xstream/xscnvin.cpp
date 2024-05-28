#include "qdengine/xlibs/xutil/xglobal.h"

XStream& XStream::operator<= (char var) {
	Common::String s = Common::String::format("%c", var);
	write(s.c_str(), s.size());
	return *this;
}

XStream& XStream::operator<= (unsigned char var) {
	Common::String s = Common::String::format("%c", var);
	write(s.c_str(), s.size());
	return *this;
}

XStream& XStream::operator<= (short var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size());
	return *this;
}

XStream& XStream::operator<= (unsigned short var) {
	Common::String s = Common::String::format("%u", var);
	write(s.c_str(), s.size());
	return *this;
}

XStream& XStream::operator<= (int var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size());
	return *this;
}

XStream& XStream::operator<= (unsigned var) {
	Common::String s = Common::String::format((const char *)var, ConvertBuffer_, radix);
	write(s.c_str(), s.size());
	return *this;
}

XStream& XStream::operator<= (long var) {
	Common::String s = Common::String::format("%ld", var);
	write(s.c_str(), s.size());
	return *this;
}

XStream& XStream::operator<= (unsigned long var) {
	Common::String s = Common::String::format("%lu", var);
	write(s.c_str(), s.size());
	return *this;
}

XStream& XStream::operator<= (float var) {
	char* s = gcvt(var, digits, ConvertBuffer_);
	write(s, strlen(s));
	return *this;
}

XStream& XStream::operator<= (double var) {
	char* s = gcvt(var, digits, ConvertBuffer_);
	write(s, strlen(s));
	return *this;
}

XStream& XStream::operator<= (long double var) {
	char* s = gcvt(var, digits, ConvertBuffer_);
	write(s, strlen(s));
	return *this;
}
