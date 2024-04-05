#ifndef __SYSTEM_UTIL_H__
#define __SYSTEM_UTIL_H__

/////////////////////////////////////////////////////////////////////////////////
//		Memory check
/////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define DBGCHECK
//void win32_check();
//#define DBGCHECK	win32_check();
#else
#define DBGCHECK
#endif

/////////////////////////////////////////////////////////////////////////////////
//		Key Press
/////////////////////////////////////////////////////////////////////////////////
#define VK_TILDE	0xC0
#define VK_LDBL		0x88
#define VK_RDBL		0x89
#define VK_WHEELUP	0xA7
#define VK_WHEELDN	0xA8

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST + 1)
#endif

bool applicationHasFocus();

inline bool isPressed(int key) { return applicationHasFocus() && (GetAsyncKeyState(key) & 0x8000); } 
inline bool isShiftPressed() { return isPressed(VK_SHIFT); }
inline bool isControlPressed() { return isPressed(VK_CONTROL); }
inline bool isAltPressed() { return isPressed(VK_MENU); }

const unsigned int KBD_CTRL = 1 << 8;
const unsigned int KBD_SHIFT = 1 << 9;
const unsigned int KBD_MENU = 1 << 10;

inline int addModifiersState(int keyCode){
	int code(keyCode);
	code |= isControlPressed() ? KBD_CTRL : 0; 
	code |= isShiftPressed() ? KBD_SHIFT : 0; 
	code |= isAltPressed() ? KBD_MENU : 0; 
	return code;
}

// --- LocData ------
const char* getLocDataPath();
string getLocDataPath(const char* dir);

// --- Formatting ------
string formatTimeWithHour(int timeMilis);
string formatTimeWithoutHour(int timeMilis);

//-------------------------------------------------
bool openFileDialog(string& filename, const char* initialDir, const char* extention, const char* title);
bool saveFileDialog(string& filename, const char* initialDir, const char* extention, const char* title);
const char* popupMenu(vector<const char*> items); // returns zero if cancel
int popupMenuIndex(vector<const char*> items); // returns -1 if cancel
const char* editText(const char* defaultValue);
const char* editTextMultiLine(const char* defaultValue, HWND hwnd);

//-------------------------------------------------
void setLogicFp();
bool checkLogicFp();

//-------------------------------------------------

extern string default_font_name;

#endif //__SYSTEM_UTIL_H__
