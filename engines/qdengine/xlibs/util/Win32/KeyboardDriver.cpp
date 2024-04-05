#include "StdAfx.h"
#include "keyboarddriver.h"
#include "WinVersion.h"

KeyboardDriver::KeyboardDriver(IEventHandler* handler)
: handler_(handler)
{
}

KeyboardDriver::~KeyboardDriver()
{

}

void KeyboardDriver::doKey(utf32_char vKey, utf32_char unicode, bool iDown, bool autoRepeat, CharacterType charType)
{
	if(!handler_)
		return;

	if(unicode)
		handler_->handle(WM_USER + WM_CHAR, unicode, (autoRepeat ? 1 : 0) | (iDown ? 0 : 1 << 31));

	if(iDown){
		if(!autoRepeat)
			handler_->handle(WM_USER + WM_KEYDOWN, vKey, 0);
	}
	else
		handler_->handle(WM_USER + WM_KEYUP, vKey, 0);
}

bool KeyboardDriver::handleKeyMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYUP:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			bool down = (message == WM_KEYDOWN) || (message == WM_SYSKEYDOWN);
			bool autoRep = ((lParam & 0x40000000) != 0);
			
			utf32_char vKey, unicode;
			CharacterType type;
			if(win32KeyToUKey((LONG)wParam, (LONG)lParam, vKey, unicode, type))
					doKey(vKey, unicode, down, autoRep, type);
		}
		return true;
	case WM_DEADCHAR:
	case WM_SYSDEADCHAR:
	case WM_CHAR:
	//case WM_UNICHAR:
	case WM_SYSCHAR:
		//if (wParam != UNICODE_NOCHAR)
		//{
			return true; // Pretend we handle it. May get beeps otherwise.
		//}
		return false;
	}

	return false;
}

bool KeyboardDriver::win32KeyToUKey(LONG vKey, LONG flags, utf32_char& extVKey, utf32_char& unicode, CharacterType& charType)
{
	charType = NORMAL_CHAR;
	unicode = 0;

	extVKey = vKey;

#define EXT_KEY(key) ((key & 0x01000000) != 0)

	switch (vKey)
	{
	case VK_MENU:
		extVKey = EXT_KEY(flags) ? VK_RMENU : VK_LMENU;
		return true;
	case VK_CONTROL:
		extVKey = EXT_KEY(flags) ? VK_RCONTROL : VK_LCONTROL;
		return true;
	case VK_SHIFT:
		extVKey = EXT_KEY(flags) ? VK_RSHIFT : VK_LSHIFT;
		return true;
	case VK_UP:
		//странность клавиш стрелок: если стоит флаг расширенной клавиши, - это блок курсорных клавиш
		//если не стоит, то калькулятор с выключенным num lock
		extVKey = EXT_KEY(flags) ? VK_UP : VK_NUMPAD8;
		return true;
	case VK_DOWN:
		extVKey = EXT_KEY(flags) ? VK_DOWN : VK_NUMPAD2;
		return true;
	case VK_LEFT:
		extVKey = EXT_KEY(flags) ? VK_LEFT : VK_NUMPAD4;
		return true;
	case VK_RIGHT:
		extVKey = EXT_KEY(flags) ? VK_RIGHT : VK_NUMPAD6;
		return true;
	case VK_CLEAR:
		extVKey = VK_NUMPAD5;
		return true;
	case VK_INSERT:
		extVKey = EXT_KEY(flags) ? VK_INSERT : VK_NUMPAD0;
		return true;
	case VK_DELETE:
		extVKey = EXT_KEY(flags) ? VK_DELETE : VK_DECIMAL;
		return true;
	case VK_PRIOR:
		extVKey = EXT_KEY(flags) ? VK_PRIOR : VK_NUMPAD9;
		return true;
	case VK_NEXT:
		extVKey = EXT_KEY(flags) ? VK_NEXT : VK_NUMPAD3;
		return true;
	case VK_HOME:
		extVKey = EXT_KEY(flags) ? VK_HOME : VK_NUMPAD7;
		return true;
	case VK_END:
		extVKey = EXT_KEY(flags) ? VK_END : VK_NUMPAD1;
		return true;
	case VK_RETURN:
		extVKey = EXT_KEY(flags) ? VK_RETURN : VK_RETURN;
		unicode = L'\n';
		return true;
	case VK_NUMPAD0:
		unicode = L'0';
		return true;
	case VK_NUMPAD1:
		unicode = L'1';
		return true;
	case VK_NUMPAD2:
		unicode = L'2';
		return true;
	case VK_NUMPAD3:
		unicode = L'3';
		return true;
	case VK_NUMPAD4:
		unicode = L'4';
		return true;
	case VK_NUMPAD5:
		unicode = L'5';
		return true;
	case VK_NUMPAD6:
		unicode = L'6';
		return true;
	case VK_NUMPAD7:
		unicode = L'7';
		return true;
	case VK_NUMPAD8:
		unicode = L'8';
		return true;
	case VK_NUMPAD9:
		unicode = L'9';
		return true;
	case VK_LMENU:
	case VK_RMENU:
	case VK_LCONTROL:
	case VK_RCONTROL:
	case VK_LSHIFT:
	case VK_RSHIFT:
	case VK_BACK:
	case VK_TAB:
	case VK_ESCAPE:
	case VK_F1:
	case VK_F2:
	case VK_F3:
	case VK_F4:
	case VK_F5:
	case VK_F6:
	case VK_F7:
	case VK_F8:
	case VK_F9:
	case VK_F10:
	case VK_F11:
	case VK_F12:
	case VK_NUMLOCK:
	case VK_PAUSE:
	case VK_APPS:
	case VK_CAPITAL:
	case VK_SNAPSHOT:
	case VK_SCROLL:
		return true;
	default:
		{
			WCHAR wCh[2];
			int ret;
			BYTE keystate[256];

			if(!GetKeyboardState(keystate))
				return false;
			
			if(isWinNT())
				ret = ToUnicode(vKey, flags, keystate, wCh, 2, 0);
			else {
				char outCh[2];
				ret = ToAscii(vKey, flags, keystate, (PWORD)&outCh, 0);
				if(ret != 0)
					MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, outCh, (ret > 0) ? ret : 0, wCh,  sizeof (wCh) / sizeof (WCHAR));
			}
			if(ret > 0) // Composed or normal char. (Composed shouldn't happen.)
				unicode = wCh[(ret == 1) ? 0 : 1];
			else if(ret < 0) {
				unicode = wCh[0];
				charType = DEAD_CHAR; // Dead char
			}

			return true;
		}
	}
#undef EXT_KEY

	return false;
}

