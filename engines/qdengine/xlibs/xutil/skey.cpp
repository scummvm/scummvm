#include "xglobal.h"
#include "sKey.h"
#include "Serialization\Serialization.h"


const char* toHex(unsigned char byte)
{
	static char char_values[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	static char byteBuf[3];
	byteBuf[0] = char_values[byte >> 4];
	byteBuf[1] = char_values[byte & 0x0F];
	byteBuf[2] = '\0';
	return byteBuf;
}

const char* sKey::vk_table_[]  = {
	"", "LB", "RB", "Cancel", "MB", "", "", "", "Backspace", "Tab", "", "", "Clear", "Enter", "", "", //0x00 - 0x0F
		"Shift", "Control", "Alt", "Pause", "Caps", "", "", "", "", "", "", "Escape", "", "", "", "", //0x10 - 0x1F
		"Space", "PgUP", "PgDN", "End", "Home", "Left", "Up", "Right", "Down", "", "", "", "PrSrc", "Ins", "Del", "", //0x20 - 0x2F
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "", "", "", "", "", //0x30 - 0x3F
		"", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", //0x40 - 0x4F
		"P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "LWin", "RWin", "Apps", "", "Sleep", //0x50 - 0x5F
		"num0", "num1", "num2", "num3", "num4", "num5", "num6", "num7", "num8", "num9", "num*", "num+", "", "num-", "num.", "num/", //0x60 - 0x6F
		"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16", //0x70 - 0x7F
		"F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", "LDBL", "RDBL", "", "", "", "", "", "", //0x80 - 0x8F
		"NumLock", "ScrollLock", "LShift", "RShift", "LCtrl", "RCtrl", "LAlt", "RAlt", "", "", "", "", "", "", "", "", //0x90 - 0x9F
		"", "", "", "", "", "", "", "WheelUp", "WheelDn", "", "", "", "", "", "", "", //0xA0 - 0xAF
		"", "", "", "", "", "", "", "", "", "", ";", "+", ",", "-", ".", "/", //0xB0 - 0xBF
		"~", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", //0xC0 - 0xCF
		"", "", "", "", "", "", "", "", "", "", "", "[", "\\", "]", "\"", "", //0xD0 - 0xDF
		"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", //0xE0 - 0xEF
		"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "WakeUp" //0xF0 - 0xFF
};

sKey::sKey(int _fullkey, bool checkAsyncState)
{
	fullkey = _fullkey;
	if(key == VK_CONTROL) 
		ctrl = 1;
	if(key == VK_SHIFT)
		shift = 1;
	if(key == VK_MENU)
		menu = 1;
	if(checkAsyncState){
		ctrl = GetAsyncKeyState(VK_CONTROL) >> 15;
		shift = GetAsyncKeyState(VK_SHIFT) >> 15;
		menu = GetAsyncKeyState(VK_MENU) >> 15;
	}
}

bool sKey::serialize(Archive& ar, const char* name, const char* nameAlt)
{
	if(ar.isEdit()){
		if(ar.openStruct(*this, name, nameAlt)){
			ar.serialize(fullkey, "key", 0);
			ar.closeStruct(name);
		}
		return true;
	}
	else
		return ar.serialize(fullkey, name, nameAlt);
}

string sKey::toString(bool compact) const
{
	string keyName;
	const char* plus = compact ? "+" : " + ";
	
	if(ctrl && key != VK_CONTROL)
		keyName += sKey::nameCtrl();
	
	if(shift && key != VK_SHIFT){
		if(!keyName.empty())
			keyName += plus;
		keyName += sKey::nameShift();
	}
	
	if(menu && key != VK_MENU){
		if(!keyName.empty())
			keyName += plus;
		keyName += sKey::nameMenu(); // Alt
	}

	if(key){
		if(!keyName.empty())
			keyName += plus;

		const char* locName = sKey::name(key);
		if(locName && *locName)
			keyName += locName;
		else{
			keyName += "0x";
			keyName += toHex(key);
		}
	}
	return keyName;
}
