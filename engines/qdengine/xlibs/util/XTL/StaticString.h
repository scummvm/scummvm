#pragma once

class StaticString
{
public:
	StaticString(const char* str = "") : str_(str) {}
	operator const char*()const{ return str_; }
	const char* c_str()const{ return str_; }
	bool operator==(const char* rhs) const{
		return str_ == rhs || strcmp(str_, rhs) == 0;
	}
	bool operator!=(const char* rhs) const{
		return str_ != rhs && strcmp(str_, rhs) != 0;
	}
	bool operator<(const char* rhs) const{
		return strcmp(str_, rhs) < 0;
	}
	bool operator>(const char* rhs) const{
		return strcmp(str_, rhs) > 0;
	}
private:
	const char* str_;
};

