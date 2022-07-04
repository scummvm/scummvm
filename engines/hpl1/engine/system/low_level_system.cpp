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

#include <stdarg.h>

#include "hpl1/engine/libraries/angelscript/angelscript.h"
#include "hpl1/engine/impl/SqScript.h"
#include "hpl1/engine/impl/scriptstring.h"
#include "hpl1/engine/impl/stdstring.h"
#include "hpl1/engine/system/String.h"

#include "hpl1/debug.h"
#include "hpl1/hpl1.h"

#include "common/system.h"

namespace hpl {

LowLevelSystem::LowLevelSystem() {
#if 0
	mpScriptEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
#endif
	//_scriptOutput = hplNew(cScriptOutput, ());
	//_scriptEngine->SetMessageCallback(asMETHOD(cScriptOutput, AddMessage), _scriptOutput, asCALL_THISCALL);
#if 0
#ifdef AS_MAX_PORTABILITY
	RegisterScriptString(mpScriptEngine);
#else
	RegisterStdString(mpScriptEngine);
#endif
#endif
	_handleCount = 0;
}

LowLevelSystem::~LowLevelSystem() {
	/*Release all runnings contexts */

	_scriptEngine->Release();
	hplDelete(_scriptOutput);

	// perhaps not the best thing to skip :)
	// if(gpLogWriter)	hplDelete(gpLogWriter);
	// gpLogWriter = NULL;
}

void FatalError(const char *fmt, ...) {
	va_list vl;
	va_start(vl, fmt);
	error(fmt, vl);
	va_end(vl);
}

void Error(const char *fmt, ...) {
	va_list vl;
	va_start(vl, fmt);
	debugN(Hpl1::kDebugLevelError, fmt, vl);
	va_end(vl);
}

void Warning(const char *fmt, ...) {
	va_list vl;
	va_start(vl, fmt);
	debugN(Hpl1::kDebugLevelWarning, fmt, vl);
	va_end(vl);
}

void Log(const char *fmt, ...) {
	va_list vl;
	va_start(vl, fmt);
	debugN(Hpl1::kDebugLevelLog, fmt, vl);
	va_end(vl);
}

static bool gbUpdateLogIsActive;
void SetUpdateLogActive(bool abX) {
	gbUpdateLogIsActive = abX;
}

void LogUpdate(const char *fmt, ...) {
#if 0
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
#endif
}

//-----------------------------------------------------------------------

void CopyTextToClipboard(const tWString &text) {
	//FIXME: string types
	g_system->setTextInClipboard(Common::String(cString::To8Char(text.c_str()).c_str()));
}

tWString LoadTextFromClipboard() {
	//FIXME: string types
	Common::U32String text = g_system->getTextFromClipboard();
	return cString::To16Char(Common::String(text).c_str());
}

//-----------------------------------------------------------------------

void CreateMessageBoxW(eMsgBoxType eType, const wchar_t *caption, const wchar_t *fmt, va_list ap) {
}

void CreateMessageBoxW(eMsgBoxType eType, const wchar_t *caption, const wchar_t *fmt, ...) {
}

void CreateMessageBoxW(const wchar_t *caption, const wchar_t *fmt, ...) {
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

void OpenBrowserWindow(const tWString &URL) {
	// FIXME: use proper string types
	g_system->openUrl(cString::To8Char(URL).c_str());
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

bool FileExists(const tWString &fileName) {
#if 0
#ifdef WIN32
		FILE *f = _wfopen(fileName.c_str(),_W("r"));
#else
		FILE *f = fopen(cString::To8Char(fileName).c_str(),"r");
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

void RemoveFile(const tWString &filePath) {
#if 0
#ifdef WIN32
		_wremove(filePath.c_str());
#else
		remove(cString::To8Char(filePath).c_str());
#endif
#endif
}

//-----------------------------------------------------------------------

bool CloneFile(const tWString &srcFileName, const tWString &destFileName,
			   bool abFailIfExists) {
#if 0
#ifdef WIN32
		return CopyFile(srcFileName.c_str(),destFileName.c_str(),abFailIfExists)==TRUE;
#else
		if (abFailIfExists && FileExists(destFileName)) {
			return true;
		}
		std::ifstream IN (cString::To8Char(srcFileName).c_str(), std::ios::binary);
		std::ofstream OUT (cString::To8Char(destFileName).c_str(), std::ios::binary);
		OUT << IN.rdbuf();
		OUT.flush();
		return true;
#endif
#endif
	return false;
}

//-----------------------------------------------------------------------

bool CreateFolder(const tWString &path) {
#if 0
#ifdef WIN32
		return CreateDirectory(path.c_str(),NULL)==TRUE;
#else
		return mkdir(cString::To8Char(path).c_str(),0755)==0;
#endif
#endif
	return false;
}

bool FolderExists(const tWString &path) {
#if 0
#ifdef WIN32
		return GetFileAttributes(path.c_str())==FILE_ATTRIBUTE_DIRECTORY;
#else
		struct stat statbuf;
		return (stat(cString::To8Char(path).c_str(), &statbuf) != -1);
#endif
#endif
	return false;
}

bool IsFileLink(const tWString &path) {
#if 0
		// Symbolic Links Not Supported under Windows
#ifndef WIN32
		struct stat statbuf;
		if (lstat(cString::To8Char(path).c_str(), &statbuf) == 0) {
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

bool LinkFile(const tWString &pointsTo, const tWString &link) {
#if 0
		// Symbolic Links Not Supported under Windows
#ifndef WIN32
		return (symlink(cString::To8Char(pointsTo).c_str(), cString::To8Char(link).c_str()) == 0);
#else
		return false;
#endif
#endif
	return false;
}

bool RenameFile(const tWString &from, const tWString &to) {
#if 0
#ifdef WIN32
		return false;
#else
		return (rename(cString::To8Char(from).c_str(), cString::To8Char(to).c_str()) == 0);
#endif
#endif
	return false;
}

//-----------------------------------------------------------------------

cDate FileModifiedDate(const tWString &filePath) {
#if 0
  		struct tm* pClock;
#ifdef WIN32
		struct _stat attrib;
		_wstat(filePath.c_str(), &attrib);
#else
		struct stat attrib;
		stat(cString::To8Char(filePath).c_str(), &attrib);
#endif

		pClock = gmtime(&(attrib.st_mtime));	// Get the last modified time and put it into the time structure

		cDate date = DateFromGMTIme(pClock);

		return date;
#endif
	return {};
}

//-----------------------------------------------------------------------

cDate FileCreationDate(const tWString &filePath) {
#if 0
  		struct tm* pClock;
#ifdef WIN32
		struct _stat attrib;
		_wstat(filePath.c_str(), &attrib);
#else
		struct stat attrib;
		stat(cString::To8Char(filePath).c_str(), &attrib);
#endif

		pClock = gmtime(&(attrib.st_ctime));	// Get the last modified time and put it into the time structure

		cDate date = DateFromGMTIme(pClock);

		return date;
#endif

	return {};
}

//-----------------------------------------------------------------------

void SetWindowCaption(const tString &name) {
	g_system->setWindowCaption(Common::U32String(name.c_str()));
}

unsigned long GetApplicationTime() {
	return g_engine->getTotalPlayTime();
}

unsigned long LowLevelSystem::getTime() {
	return g_engine->getTotalPlayTime();
}

cDate LowLevelSystem::getDate() {
	return {};
}

iScript *LowLevelSystem::createScript(const tString &name) {
	return hplNew(cSqScript, (name, _scriptEngine, _scriptOutput, _handleCount++));
}

bool LowLevelSystem::addScriptFunc(const tString &funcDecl, void *pFunc, int callConv) {
#if 0
  	if (_scriptEngine->RegisterGlobalFunction(funcDecl.c_str(),
											   asFUNCTION(pFunc), callConv) < 0) {
		Error("Couldn't add func '%s'\n", funcDecl.c_str());
		return false;
	}

	return true;
#endif
	return false;
}

bool LowLevelSystem::addScriptVar(const tString &varDecl, void *pVar) {
	if (_scriptEngine->RegisterGlobalProperty(varDecl.c_str(), pVar) < 0) {
		Error("Couldn't add var '%s'\n", varDecl.c_str());
		return false;
	}

	return true;
}

void LowLevelSystem::sleep(const unsigned int alMillisecs) {
}

} // namespace hpl
