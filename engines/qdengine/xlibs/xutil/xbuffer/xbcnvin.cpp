#include "qdengine/xlibs/xutil/xglobal.h"


XBuffer& XBuffer::operator<= (const char var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size(), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (unsigned char var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size(), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (short var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size(), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (unsigned short var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size(), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (int var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size(), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (unsigned var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size(), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (long var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size(), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (unsigned long var) {
	Common::String s = Common::String::format("%d", var);
	write(s.c_str(), s.size(), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (float var) {
	char* s = gcvt(var, digits_, ConvertBuffer_);
	write(s, strlen(s), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (double var) {
	char* s = gcvt(var, digits_, ConvertBuffer_);
	write(s, strlen(s), 0);
	return *this;
}

XBuffer& XBuffer::operator<= (long double var) {
	char* s = gcvt(var, digits_, ConvertBuffer_);
	write(s, strlen(s), 0);
	return *this;
}
