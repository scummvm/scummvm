#ifndef __KEYBOARD_DRIVER_
#define __KEYBOARD_DRIVER_

#include <Windows.h>

typedef unsigned int utf32_char;

struct IEventHandler
{
	IEventHandler() {}
	virtual ~IEventHandler() {};
	virtual void handle(UINT message, WPARAM wParam, LPARAM lParam) = 0;
};

class KeyboardDriver
{
public:
	enum CharacterType
	{
		NORMAL_CHAR,
		DEAD_CHAR
	};

public:

	KeyboardDriver(IEventHandler* handler);
	~KeyboardDriver();

	bool handleKeyMessage(UINT message, WPARAM wParam, LPARAM lParam);

protected:
	void doKey(utf32_char codeRaw, utf32_char codeCooked, bool iDown, bool autoRepeat = false, CharacterType charType = NORMAL_CHAR);
	bool win32KeyToUKey(LONG vKey, LONG keyFlags, utf32_char& rawCode, utf32_char& cookedCode, CharacterType& charType);

private:
	IEventHandler* handler_;
};

#endif //__KEYBOARD_DRIVER_