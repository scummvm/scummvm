#ifndef __STRING_WRAPPERS_H__
#define __STRING_WRAPPERS_H__

// Адаптация строк для передачи по сети

class StringInWrapper
{
	string& value_;
public:
	explicit StringInWrapper(string& value) : value_(value) {}
	string& operator()() { return value_; }
};

class StringOutWrapper
{
	const string& value_;
public:
	explicit StringOutWrapper(const string& value) : value_(value) {}
	const string& operator()() const { return value_; }
};

inline XBuffer& operator > (XBuffer& in, StringInWrapper& str) { str() = in(in.tell()); in += str().size() + 1; return in; }
inline XBuffer& operator < (XBuffer& out, const StringOutWrapper& str) { out < str().c_str() < '\0'; return out; }


#endif //__STRING_WRAPPERS_H__