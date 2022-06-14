/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

//#include <vld.h>
// Use this to check for memory leaks!

#if 0 // def WIN32
#pragma comment(lib, "angelscript.lib")
#define UNICODE
#include <shlobj.h>
#include <windows.h>
#endif

#if 0 // ndef WIN32
// Include FLTK
#include "FL/fl_ask.H"
#endif

#define _UNICODE

#include <fstream>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <time.h>

#include "hpl1/engine/impl/LowLevelSystemSDL.h"
#include "hpl1/engine/impl/SqScript.h"

//#include "SDL/SDL.h"

#include "hpl1/engine/impl/scriptstring.h"
#include "hpl1/engine/impl/stdstring.h"

#include "hpl1/engine/system/String.h"

#include <clocale>
//#include <ObjectArray.h>

extern int hplMain(const hpl::tString &asCommandLine);

#if 0 // def WIN32
#include <windows.h>
int WINAPI WinMain(	HINSTANCE hInstance,  HINSTANCE hPrevInstance,LPSTR	lpCmdLine, int nCmdShow)
{
	return hplMain(lpCmdLine);
}
#else
int main(int argc, char *argv[]) {
	if (!std::setlocale(LC_CTYPE, "")) {
		fprintf(stderr, "Can't set the specified locale! Check LANG, LC_CTYPE, LC_ALL.\n");
		return 1;
	}

	hpl::tString cmdline = "";
	for (int i = 1; i < argc; i++) {
		if (cmdline.length() > 0) {
			cmdline.append(" ").append(argv[i]);
		} else {
			cmdline.append(argv[i]);
		}
	}
	return hplMain(cmdline);
}
#endif

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// LOG WRITER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

static cLogWriter gLogWriter(_W("hpl.log"));
static cLogWriter gUpdateLogWriter(_W("hpl_update.log"));

//-----------------------------------------------------------------------

cLogWriter::cLogWriter(const tWString &asFileName) {
	msFileName = asFileName;
}

cLogWriter::~cLogWriter() {
	if (mpFile)
		fclose(mpFile);
}

void cLogWriter::Write(const tString &asMessage) {
	if (!mpFile)
		ReopenFile();

	if (mpFile) {
		fprintf(mpFile, asMessage.c_str());
		fflush(mpFile);
	}
}

void cLogWriter::Clear() {
	ReopenFile();
	if (mpFile)
		fflush(mpFile);
}

//-----------------------------------------------------------------------

void cLogWriter::SetFileName(const tWString &asFile) {
	if (msFileName == asFile)
		return;

	msFileName = asFile;
	ReopenFile();
}

//-----------------------------------------------------------------------

void cLogWriter::ReopenFile() {
#if 0
  		if(mpFile) fclose(mpFile);

#ifdef WIN32
			mpFile = _wfopen(msFileName.c_str(),_W("w"));
#else
			mpFile = fopen(cString::To8Char(msFileName).c_str(),"w");
#endif
#endif
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLowLevelSystemSDL::cLowLevelSystemSDL() {
	mpScriptEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	mpScriptOutput = hplNew(cScriptOutput, ());
	mpScriptEngine->SetMessageCallback(asMETHOD(cScriptOutput, AddMessage), mpScriptOutput, asCALL_THISCALL);

#ifdef AS_MAX_PORTABILITY
	RegisterScriptString(mpScriptEngine);
#else
	RegisterStdString(mpScriptEngine);
#endif

	mlHandleCount = 0;

	Log("-------- THE HPL ENGINE LOG ------------\n\n");
}

//-----------------------------------------------------------------------

cLowLevelSystemSDL::~cLowLevelSystemSDL() {
	/*Release all runnings contexts */

	mpScriptEngine->Release();
	hplDelete(mpScriptOutput);

	// perhaps not the best thing to skip :)
	// if(gpLogWriter)	hplDelete(gpLogWriter);
	// gpLogWriter = NULL;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void SetLogFile(const tWString &asFile) {
	gLogWriter.SetFileName(asFile);
}

//-----------------------------------------------------------------------

void FatalError(const char *fmt, ...) {
#if 0
  		char text[2048];
		va_list ap;
		if (fmt == NULL)
			return;
		va_start(ap, fmt);
			vsprintf(text, fmt, ap);
		va_end(ap);

		tString sMess = "FATAL ERROR: ";
		sMess += text;
		gLogWriter.Write(sMess);

#ifdef WIN32
		MessageBox( NULL, cString::To16Char(text).c_str(), _W("FATAL ERROR"), MB_ICONERROR);
#endif

		exit(1);
#endif
}

void Error(const char *fmt, ...) {
	char text[2048];
	va_list ap;
	if (fmt == NULL)
		return;
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);

	tString sMess = "ERROR: ";
	sMess += text;
	gLogWriter.Write(sMess);
}

void Warning(const char *fmt, ...) {
	char text[2048];
	va_list ap;
	if (fmt == NULL)
		return;
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);

	tString sMess = "WARNING: ";
	sMess += text;
	gLogWriter.Write(sMess);
}

void Log(const char *fmt, ...) {
	char text[2048];
	va_list ap;
	if (fmt == NULL)
		return;
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);

	tString sMess = "";
	sMess += text;
	gLogWriter.Write(sMess);
}

//-----------------------------------------------------------------------

static bool gbUpdateLogIsActive;
void SetUpdateLogFile(const tWString &asFile) {
	gUpdateLogWriter.SetFileName(asFile);
}

void ClearUpdateLogFile() {
	if (!gbUpdateLogIsActive)
		return;

	gUpdateLogWriter.Clear();
}

void SetUpdateLogActive(bool abX) {
	gbUpdateLogIsActive = abX;
}

void LogUpdate(const char *fmt, ...) {
	if (!gbUpdateLogIsActive)
		return;

	char text[2048];
	va_list ap;
	if (fmt == NULL)
		return;
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);

	tString sMess = "";
	sMess += text;
	gUpdateLogWriter.Write(sMess);
}

//-----------------------------------------------------------------------

void CopyTextToClipboard(const tWString &asText) {
#if 0 // def WIN32
		OpenClipboard(NULL);
		EmptyClipboard();

		HGLOBAL clipbuffer;
		wchar_t * pBuffer;
		EmptyClipboard();
		clipbuffer = GlobalAlloc(GMEM_DDESHARE, (asText.size()+1) *sizeof(wchar_t));
		pBuffer = (wchar_t*)GlobalLock(clipbuffer);
		wcscpy(pBuffer, asText.c_str());
		//GlobalUnlock(clipbuffer);

		SetClipboardData(CF_UNICODETEXT, clipbuffer);

		GlobalUnlock(clipbuffer);

		CloseClipboard();
#endif
}

tWString LoadTextFromClipboard() {
#if 0 // def WIN32
		tWString sText=_W("");
		OpenClipboard(NULL);

		HGLOBAL clipbuffer = GetClipboardData(CF_UNICODETEXT);

		wchar_t *pBuffer = (wchar_t*)GlobalLock(clipbuffer);

		if(pBuffer != NULL) sText = pBuffer;

		GlobalUnlock(clipbuffer);

		CloseClipboard();

		return sText;
#else
	return _W("");
#endif
}

//-----------------------------------------------------------------------

void CreateMessageBox(eMsgBoxType eType, const char *asCaption, const char *fmt, ...) {
	/*char text[2048];
	va_list ap;
	if (fmt == NULL)
		return;
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);

	tString sMess = "";
	sMess += text;


	#ifdef WIN32

	UINT lType = MB_OK;

	switch (eType)
	{
	case eMsgBoxType_Info:
		lType += MB_ICONINFORMATION;
		break;
	case eMsgBoxType_Error:
		lType += MB_ICONERROR;
		break;
	case eMsgBoxType_Warning:
		lType += MB_ICONWARNING;
		break;
	default:
		break;
	}


	MessageBox( NULL, sMess.c_str(), asCaption, lType );

	#endif*/
}

void CreateMessageBox(const char *asCaption, const char *fmt, ...) {
	/*char text[2048];
	va_list ap;
	if (fmt == NULL)
		return;
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);

	tString sMess = "";
	sMess += text;

	CreateMessageBox( eMsgBoxType_Default, asCaption, sMess.c_str() );*/
}

void CreateMessageBoxW(eMsgBoxType eType, const wchar_t *asCaption, const wchar_t *fmt, va_list ap) {
#if 0
		wchar_t text[2048];

		if (fmt == NULL)
			return;
		vswprintf(text, 2047, fmt, ap);

		tWString sMess = _W("");

#ifdef WIN32
		sMess += text;

		UINT lType = MB_OK;

		switch (eType)
		{
		case eMsgBoxType_Info:
			lType += MB_ICONINFORMATION;
			break;
		case eMsgBoxType_Error:
			lType += MB_ICONERROR;
			break;
		case eMsgBoxType_Warning:
			lType += MB_ICONWARNING;
			break;
		default:
			break;
		}

		MessageBox( NULL, sMess.c_str(), asCaption, lType );
#else
		sMess += asCaption;
		sMess +=_W("\n\n");
		sMess += text;
		fl_alert("%ls\n\n%ls",asCaption,text);
#endif
#endif
}

void CreateMessageBoxW(eMsgBoxType eType, const wchar_t *asCaption, const wchar_t *fmt, ...) {
	va_list ap;

	if (fmt == NULL)
		return;
	va_start(ap, fmt);
	CreateMessageBoxW(eType, asCaption, fmt, ap);
	va_end(ap);
}

void CreateMessageBoxW(const wchar_t *asCaption, const wchar_t *fmt, ...) {
	va_list ap;
	if (fmt == NULL)
		return;
	va_start(ap, fmt);
	CreateMessageBoxW(eMsgBoxType_Default, asCaption, fmt, ap);
	va_end(ap);
}

//-----------------------------------------------------------------------

static cDate DateFromGMTIme(struct tm *apClock) {
	cDate date;

	date.seconds = apClock->tm_sec;
	date.minutes = apClock->tm_min;
	date.hours = apClock->tm_hour;
	date.month_day = apClock->tm_mday;
	date.month = apClock->tm_mon;
	date.year = 1900 + apClock->tm_year;
	date.week_day = apClock->tm_wday;
	date.year_day = apClock->tm_yday;

	return date;
}

//-----------------------------------------------------------------------

void OpenBrowserWindow(const tWString &asURL) {
#if 0 // def WIN32
		ShellExecute ( NULL, _W("open"), asURL.c_str(), NULL, NULL, SW_SHOWNORMAL );
#elif defined(__linux__)
	tString asTemp = "./openurl.sh " + cString::To8Char(asURL);
	system(asTemp.c_str());
#elif defined(__APPLE__)
	tString asTemp = "open " + cString::To8Char(asURL);
	system(asTemp.c_str());
#endif
}

//-----------------------------------------------------------------------

tWString GetSystemSpecialPath(eSystemPath aPathType) {
#if 0
#if defined(WIN32)
		int type;
		switch(aPathType)
		{
		case eSystemPath_Personal:	type = CSIDL_PERSONAL;
			break;
		default: return _W("");
		}

		TCHAR sPath[1024];
		if(SUCCEEDED(SHGetFolderPath(NULL,
			type | CSIDL_FLAG_CREATE,
			NULL,0,sPath)))
		{
			return tWString(sPath);
		}
		else
		{
			return _W("");
		}
#else
		switch (aPathType)
		{
		case eSystemPath_Personal: {
			const char *home = getenv("HOME");
			return cString::To16Char(tString(home));
		}
		default:
			return _W("");
		}
#endif
#endif
	return _W("");
}

//-----------------------------------------------------------------------

bool FileExists(const tWString &asFileName) {
#if 0
#ifdef WIN32
		FILE *f = _wfopen(asFileName.c_str(),_W("r"));
#else
		FILE *f = fopen(cString::To8Char(asFileName).c_str(),"r");
#endif
		if(f==NULL)
		{
			return false;
		}

		fclose(f);
		return true;
#endif
	return false;
}

//-----------------------------------------------------------------------

void RemoveFile(const tWString &asFilePath) {
#if 0
#ifdef WIN32
		_wremove(asFilePath.c_str());
#else
		remove(cString::To8Char(asFilePath).c_str());
#endif
#endif
}

//-----------------------------------------------------------------------

bool CloneFile(const tWString &asSrcFileName, const tWString &asDestFileName,
			   bool abFailIfExists) {
#if 0
#ifdef WIN32
		return CopyFile(asSrcFileName.c_str(),asDestFileName.c_str(),abFailIfExists)==TRUE;
#else
		if (abFailIfExists && FileExists(asDestFileName)) {
			return true;
		}
		std::ifstream IN (cString::To8Char(asSrcFileName).c_str(), std::ios::binary);
		std::ofstream OUT (cString::To8Char(asDestFileName).c_str(), std::ios::binary);
		OUT << IN.rdbuf();
		OUT.flush();
		return true;
#endif
#endif
	return false;
}

//-----------------------------------------------------------------------

bool CreateFolder(const tWString &asPath) {
#if 0
#ifdef WIN32
		return CreateDirectory(asPath.c_str(),NULL)==TRUE;
#else
		return mkdir(cString::To8Char(asPath).c_str(),0755)==0;
#endif
#endif
	return false;
}

bool FolderExists(const tWString &asPath) {
#if 0
#ifdef WIN32
		return GetFileAttributes(asPath.c_str())==FILE_ATTRIBUTE_DIRECTORY;
#else
		struct stat statbuf;
		return (stat(cString::To8Char(asPath).c_str(), &statbuf) != -1);
#endif
#endif
	return false;
}

bool IsFileLink(const tWString &asPath) {
#if 0
		// Symbolic Links Not Supported under Windows
#ifndef WIN32
		struct stat statbuf;
		if (lstat(cString::To8Char(asPath).c_str(), &statbuf) == 0) {
			return statbuf.st_mode == S_IFLNK;
		} else {
			return false;
		}
#else
			return false;
#endif
#endif
	return false;
}

bool LinkFile(const tWString &asPointsTo, const tWString &asLink) {
#if 0
		// Symbolic Links Not Supported under Windows
#ifndef WIN32
		return (symlink(cString::To8Char(asPointsTo).c_str(), cString::To8Char(asLink).c_str()) == 0);
#else
		return false;
#endif
#endif
	return false;
}

bool RenameFile(const tWString &asFrom, const tWString &asTo) {
#if 0
#ifdef WIN32
		return false;
#else
		return (rename(cString::To8Char(asFrom).c_str(), cString::To8Char(asTo).c_str()) == 0);
#endif
#endif
	return false;
}

//-----------------------------------------------------------------------

cDate FileModifiedDate(const tWString &asFilePath) {
#if 0
  		struct tm* pClock;
#ifdef WIN32
		struct _stat attrib;
		_wstat(asFilePath.c_str(), &attrib);
#else
		struct stat attrib;
		stat(cString::To8Char(asFilePath).c_str(), &attrib);
#endif

		pClock = gmtime(&(attrib.st_mtime));	// Get the last modified time and put it into the time structure

		cDate date = DateFromGMTIme(pClock);

		return date;
#endif
	return {};
}

//-----------------------------------------------------------------------

cDate FileCreationDate(const tWString &asFilePath) {
#if 0
  		struct tm* pClock;
#ifdef WIN32
		struct _stat attrib;
		_wstat(asFilePath.c_str(), &attrib);
#else
		struct stat attrib;
		stat(cString::To8Char(asFilePath).c_str(), &attrib);
#endif

		pClock = gmtime(&(attrib.st_ctime));	// Get the last modified time and put it into the time structure

		cDate date = DateFromGMTIme(pClock);

		return date;
#endif

	return {};
}

//-----------------------------------------------------------------------

void SetWindowCaption(const tString &asName) {
#if 0
  		SDL_WM_SetCaption(asName.c_str(),"");
#endif
}

//-----------------------------------------------------------------------

bool HasWindowFocus(const tWString &asWindowCaption) {
#if 0
#ifdef WIN32
			HWND pWindowHandle = FindWindow(NULL, asWindowCaption.c_str());
			return (pWindowHandle == GetFocus());
#endif
#endif

	return true;
}

//-----------------------------------------------------------------------

unsigned long GetApplicationTime() {
#if 0
  		return SDL_GetTicks();
#endif
	return 0;
}

//-----------------------------------------------------------------------

void cScriptOutput::AddMessage(const asSMessageInfo *msg) {
	char sMess[1024];

	tString type = "ERR ";
	if (msg->type == asMSGTYPE_WARNING)
		type = "WARN";
	else if (msg->type == asMSGTYPE_INFORMATION)
		type = "INFO";

	sprintf(sMess, "%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type.c_str(), msg->message);

	msMessage += sMess;
}
void cScriptOutput::Display() {
	if (msMessage.size() > 500) {
		while (msMessage.size() > 500) {
			tString sSub = msMessage.substr(0, 500);
			msMessage = msMessage.substr(500);
			Log(sSub.c_str());
		}
		Log(msMessage.c_str());
	} else {
		Log(msMessage.c_str());
	}
}
void cScriptOutput::Clear() {
	msMessage = "";
}

//-----------------------------------------------------------------------

unsigned long cLowLevelSystemSDL::GetTime() {
#if 0
  		return SDL_GetTicks();
#endif
	return 0;
}

//-----------------------------------------------------------------------

cDate cLowLevelSystemSDL::GetDate() {
	return {};
}

//-----------------------------------------------------------------------

iScript *cLowLevelSystemSDL::CreateScript(const tString &asName) {
	return hplNew(cSqScript, (asName, mpScriptEngine, mpScriptOutput, mlHandleCount++));
}

//-----------------------------------------------------------------------

bool cLowLevelSystemSDL::AddScriptFunc(const tString &asFuncDecl, void *pFunc, int callConv) {
	if (mpScriptEngine->RegisterGlobalFunction(asFuncDecl.c_str(),
											   asFUNCTION(pFunc), callConv) < 0) {
		Error("Couldn't add func '%s'\n", asFuncDecl.c_str());
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------

bool cLowLevelSystemSDL::AddScriptVar(const tString &asVarDecl, void *pVar) {
	if (mpScriptEngine->RegisterGlobalProperty(asVarDecl.c_str(), pVar) < 0) {
		Error("Couldn't add var '%s'\n", asVarDecl.c_str());
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------

void cLowLevelSystemSDL::Sleep(const unsigned int alMillisecs) {
}

//-----------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// STATIC PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

} // namespace hpl
