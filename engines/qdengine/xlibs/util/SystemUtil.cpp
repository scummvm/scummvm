#include "StdAfx.h"
#include "SystemUtil.h"
#include "RenderObjects.h"
#include "GameOptions.h"
#include <crtdbg.h>
#include <commdlg.h>
#include "resource.h"
#include "float.h"
#include "Game\IniFile.h"
#include "Render\inc\IRenderDevice.h"

string default_font_name="Scripts\\Resource\\fonts\\default.ttf";

/////////////////////////////////////////////////////////////////////////////////
//		Memory check
/////////////////////////////////////////////////////////////////////////////////
void win32_check()
{
	_ASSERTE(_CrtCheckMemory()) ;
}

/////////////////////////////////////////////////////////////////////////////////
string formatTimeWithHour(int timeMilis) {
	string res;
	if (timeMilis >= 0) {
		int sec = timeMilis / 1000.0f;
		int min = sec / 60.0f;
		sec -= min * 60;
		int hour = min / 60.0f; 
		min -= hour * 60; 
		char str[11];
		sprintf(str, "%d", hour);
		res = (hour < 10) ? "0" : "";
		res += string(str) + ":";
		sprintf(str, "%d", min);
		res += (min < 10) ? "0" : "";
		res += string(str) + ":";
		sprintf(str, "%d", sec);
		res += (sec < 10) ? "0" : "";
		res += string(str);
	}
	return res;
}

string formatTimeWithoutHour(int timeMilis) {
	string res;
	if (timeMilis >= 0) {
		int sec = timeMilis / 1000.0f;
		int min = sec / 60.0f;
		sec -= min * 60;
		char str[11];
		sprintf(str, "%d", min);
		res = (min < 10) ? "0" : "";
		res += string(str) + ":";
		sprintf(str, "%d", sec);
		res += (sec < 10) ? "0" : "";
		res += string(str);
	}
	return res;
}

//-------------------------------------------------
bool openFileDialog(string& filename, const char* initialDir, const char* extention, const char* title)
{
	XBuffer filter;
	filter < title < '\0' < "*." < extention < '\0' < '\0';

	OPENFILENAME ofn;
	memset(&ofn,0,sizeof(ofn));
	char fname[2048];
	strcpy(fname,filename.c_str());
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = gb_RenderDevice->GetWindowHandle();
	string fullTitle = string("Open: ") + title;
	ofn.lpstrTitle = fullTitle.c_str();
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = fname;
	ofn.nMaxFile = sizeof(fname)-1;
	ofn.lpstrInitialDir = initialDir;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_EXPLORER|OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = extention;
	if(!GetOpenFileName(&ofn))
		return false;
	filename = fname;
	return true;
}

bool saveFileDialog(string& filename, const char* initialDir, const char* extention, const char* title)
{
	XBuffer filter;
	filter < title < '\0' < "*." < extention < '\0' < '\0';

	OPENFILENAME ofn;
	memset(&ofn,0,sizeof(ofn));
	char fname[2048];
	strcpy(fname,filename.c_str());
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = gb_RenderDevice->GetWindowHandle();
	string fullTitle = string("Save: ") + title;
	ofn.lpstrTitle = fullTitle.c_str();
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = fname;
	ofn.nMaxFile = sizeof(fname)-1;
	ofn.lpstrInitialDir = initialDir;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_EXPLORER|OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = extention;
	if(!GetSaveFileName(&ofn))
		return false;
	filename = fname;
	return true;
}

const char* popupMenu(vector<const char*> items) // returns zero if cancel
{
	if(items.empty())
		return 0;

	HMENU hMenu = CreatePopupMenu();
	
	vector<const char*>::iterator i;
	FOR_EACH(items, i)
		AppendMenu(hMenu, MF_STRING, 1 + i - items.begin(), *i);
	
	POINT point; 
	GetCursorPos(&point);
	int index = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, gb_RenderDevice->GetWindowHandle(), 0);
	
	DestroyMenu(hMenu);
	
	if(index > 0 && index <= items.size())
		return items[index - 1];
	else
		return 0;
}

int popupMenuIndex(vector<const char*> items) // returns -1 if cancel
{
	if(items.empty())
		return -1;

	HMENU hMenu = CreatePopupMenu();
	
	vector<const char*>::iterator i;
	FOR_EACH(items, i)
		AppendMenu(hMenu, MF_STRING, 1 + i - items.begin(), *i);
	
	POINT point; 
	GetCursorPos(&point);
	int index = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, gb_RenderDevice->GetWindowHandle(), 0);
	
	DestroyMenu(hMenu);
	
	if(index > 0 && index <= items.size())
		return index - 1;
	else
		return -1;
}



//-----------------------------------------
static string editTextString;
static BOOL CALLBACK DialogProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
	case WM_SYSCOMMAND:
		if(wParam==SC_CLOSE)
		{
			EndDialog(hwnd,IDCANCEL);
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			char tmpstr[256];
			HWND h;

			h=GetDlgItem(hwnd,IDC_INPUT_TEXT);
			GetWindowText(h,tmpstr,256);
			editTextString = tmpstr;

			EndDialog(hwnd,IDOK);
			break;
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hwnd,IDCANCEL);
			break;
		}
		break;
	case WM_INITDIALOG:
		{
			HWND h;
			h=GetDlgItem(hwnd,IDC_INPUT_TEXT);
			SetWindowText(h, editTextString.c_str());
		}
		return TRUE;
	}
	return FALSE;
}

const char* editText(const char* defaultValue)
{
	editTextString = defaultValue;
	int ret = DialogBox(GetModuleHandle(0),MAKEINTRESOURCE(IDD_DIALOG_INPUT_TEXT),gb_RenderDevice->GetWindowHandle(),DialogProc);
//	if(ret!=IDOK)
//		return 0;
	return editTextString.c_str();
}

const char* editTextMultiLine(const char* defaultValue, HWND hwnd)
{
	editTextString = defaultValue;
	int ret = DialogBox(GetModuleHandle(0),MAKEINTRESOURCE(IDD_DIALOG_INPUT_TEXT_MULTILINE),hwnd,DialogProc);
//	if(ret!=IDOK)
//		return 0;
	return editTextString.c_str();
}

/////////////////////////////////////////
#define GAME_LOGIC_FLOAT_POINT_PRECISION _PC_24
void setLogicFp()
{
	_controlfp(GAME_LOGIC_FLOAT_POINT_PRECISION, _MCW_PC); //_PC_24

#ifndef _FINAL_VERSION_
	if(iniFile.ControlFpEnable){
		_clearfp();
		_controlfp( _controlfp(0,0) & ~(EM_OVERFLOW | EM_ZERODIVIDE | EM_DENORMAL |  EM_INVALID),  MCW_EM ); 
	}
#endif
}
bool checkLogicFp()
{
	return (_controlfp(0,0)&_MCW_PC) == GAME_LOGIC_FLOAT_POINT_PRECISION;
}

//////////////////////////////////////////

const char* getLocDataPath() 
{
	return GameOptions::instance().getLocDataPath();
}

string getLocDataPath(const char* dir) 
{
	string path = getLocDataPath();
	path += dir;
	return path;
}


