#ifndef __SKEY_H__
#define __SKEY_H__

#include <string>

class Archive;

struct sKey
{
	enum {
		CONTROL = 1 << 8,
		SHIFT = 1 << 9,
		MENU = 1 << 10
	};

	union {
		struct {	
			unsigned char key;
			unsigned char ctrl : 1;
			unsigned char shift : 1;
			unsigned char menu	: 1;
		};
		int fullkey;
	};

	explicit sKey(int fullkey = 0, bool checkAsyncState = false);

	operator int () const { return fullkey; }

	bool serialize(Archive& ar, const char* name, const char* nameAlt);

	static const char* name(int fullKey) { return vk_table_[fullKey & 0xFF]; }
	static const char* nameCtrl() { return "Ctrl"; }
	static const char* nameShift() { return "Shift"; }
	static const char* nameMenu() { return "Alt"; }

	std::string toString(bool compact = false) const;

protected:
	static const char* vk_table_[];
};

#endif //__SKEY_H__
