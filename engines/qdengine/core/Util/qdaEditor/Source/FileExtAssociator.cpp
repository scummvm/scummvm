// FileExtAssociator.cpp: implementation of the FileExtAssociator class.
//
//////////////////////////////////////////////////////////////////////

#include "qd_precomp.h"

#include <stdio.h>
#include <shlobj.h>

#include "FileExtAssociator.h"

const char* GetOpenCommandLine()
{
	static char res[MAX_PATH * 2];

	char buf[MAX_PATH];
	GetModuleFileName(GetModuleHandle(0), buf, MAX_PATH);
	sprintf(res, "%s \"%%1\"", buf);
	
	return res;
}

const char* GetDefaultIconString()
{
	static char res[MAX_PATH * 2];

	char buf[MAX_PATH];
	GetModuleFileName(GetModuleHandle(0), buf, MAX_PATH);
	sprintf(res, "%s,1", buf);
	return res;
}

FileExtAssociator::FileExtAssociator()
{
	
}

FileExtAssociator::~FileExtAssociator()
{
	
}

/*
	IDS_APPS_SUB_KEY - имя exe-файла приложения 
	IDS_PROG_ID_REG_KEY	- а это загадочная строка. для своего приложения QuestEditor.exe
	я определил её как "qdaEditor.1"
	IDS_PROG_ID_KEY_VALUE - это значения для предыдущего ключа. Здесь находится описание
							того, что мы будем открывать
*/

static char* progKeyID = "qda_editor.1";
static char* progKeyValue = "QDA Animation";
static char* appsSubKey = "qda_editor.exe";

bool FileExtAssociator::associate(const char* ext_string)
{
#ifndef _DEBUG
	HKEY key_ext;
	if(RegCreateKeyEx(HKEY_CLASSES_ROOT, ext_string, 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &key_ext, 0) != ERROR_SUCCESS)
		return false;
	if(RegSetValueEx(key_ext, 0, 0, REG_SZ, (const BYTE*)progKeyID, strlen(progKeyID) + 1) != ERROR_SUCCESS)
		return false;
	if(RegFlushKey(key_ext) != ERROR_SUCCESS)
		return false;
	RegCloseKey(key_ext);

	HKEY prog_id;
	if(RegCreateKeyEx(HKEY_CLASSES_ROOT, progKeyID, 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &prog_id, 0) != ERROR_SUCCESS)
		return false;
	if(RegSetValueEx(prog_id, 0, 0, REG_SZ, (const BYTE*)progKeyValue, strlen(progKeyValue) + 1) != ERROR_SUCCESS)
		return false;
/*
	DWORD dwFlags = 0x00000010 | 0x00000020;
	if(RegSetValueEx(prog_id, "EditFlags", 0, REG_BINARY, (BYTE*)&dwFlags, sizeof(DWORD)) != ERROR_SUCCESS)
		return false;
*/
	if(!setShellSubKeys(prog_id))
		return false;

	if(RegFlushKey(prog_id) != ERROR_SUCCESS)
		return false;
	RegCloseKey(prog_id);

	HKEY key_apps;
	if(RegCreateKeyEx(HKEY_CLASSES_ROOT, "Applications", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &key_apps, 0) != ERROR_SUCCESS)
		return false;

	HKEY key_app;
	if(RegCreateKeyEx(key_apps, appsSubKey, 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &key_app, 0) != ERROR_SUCCESS)
		return false;
	if(RegSetValueEx(key_app, "NoOpenWith", 0, REG_SZ, (BYTE*)"", 1) != ERROR_SUCCESS)
		return false;

	HKEY key_icon;
	if(RegCreateKeyEx(key_app, "DefaultIcon", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &key_icon, 0) != ERROR_SUCCESS)
		return false;

	const char* str = GetDefaultIconString();
	if(RegSetValueEx(key_icon, 0, 0, REG_SZ, (const BYTE*)str, strlen(str) + 1) != ERROR_SUCCESS)
		return false;
//	if(RegFlushKey(key_icon) != ERROR_SUCCESS)
//		return false;
//	RegCloseKey(key_icon);

	if(!setShellSubKeys(key_app))
		return false;

	if(RegFlushKey(key_app) != ERROR_SUCCESS)
		return false;
	RegCloseKey(key_app);

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
#endif
	return true;
}

bool FileExtAssociator::setShellSubKeys(HKEY key)
{
	HKEY key_shell;
	if(RegCreateKeyEx(key, "shell", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &key_shell, 0) != ERROR_SUCCESS)
		return false;
	HKEY key_open;
	if(RegCreateKeyEx(key_shell, "open", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &key_open, 0) != ERROR_SUCCESS)
		return false;

	HKEY key_command;
	if(RegCreateKeyEx(key_open, "command", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &key_command, 0) != ERROR_SUCCESS)
		return false;
	const char* str = GetOpenCommandLine();
	if(RegSetValueEx(key_command, 0, 0, REG_SZ, (const BYTE*)str, strlen(str) + 1) != ERROR_SUCCESS)
		return false;
/*
	if(RegFlushKey(key_command) != ERROR_SUCCESS)
		return false;
	RegCloseKey(key_command);

	if(RegFlushKey(key_open) != ERROR_SUCCESS)
		return false;
	RegCloseKey(key_open);

	if(RegFlushKey(key_shell) != ERROR_SUCCESS)
		return false;
	RegCloseKey(key_shell);*/

	return true;
}
