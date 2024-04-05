#ifndef __LOCSTRING_H__
#define __LOCSTRING_H__

class Archive;

class LocString
{
public:
	void serialize(Archive& ar);
	void update();
	
	const wchar_t* c_str() const { return text_.c_str(); }
	bool empty() const { return text_.empty(); }

	const char* key() const { return key_.c_str(); }

private:
	string key_;
	wstring text_;
};

#endif //__LOCSTRING_H__
