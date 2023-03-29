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

#include "hpl1/engine/impl/SqScript.h"
#include "hpl1/engine/libraries/angelscript/add-ons/scriptarray.h"
#include "hpl1/engine/libraries/angelscript/add-ons/scriptstdstring.h"
#include "hpl1/engine/libraries/angelscript/angelscript.h"
#include "hpl1/engine/system/String.h"

#include "hpl1/debug.h"
#include "hpl1/hpl1.h"

#include "common/system.h"

namespace hpl {

static void scriptEngineLog(const asSMessageInfo *msg) {
	switch (msg->type) {
	case asMSGTYPE_ERROR:
		Hpl1::logError(Hpl1::kDebugScripts, "%s (%d, %d) : %s\n", msg->section, msg->row, msg->col, msg->message);
		break;
	case asMSGTYPE_WARNING:
		Hpl1::logWarning(Hpl1::kDebugScripts, "%s (%d, %d) : %s\n", msg->section, msg->row, msg->col, msg->message);
		break;
	case asMSGTYPE_INFORMATION:
		Hpl1::logInfo(Hpl1::kDebugScripts, "%s (%d, %d) : %s\n", msg->section, msg->row, msg->col, msg->message);
	}
}

LowLevelSystem::LowLevelSystem() {
	_scriptEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptArray(_scriptEngine, true);
	_scriptEngine->SetMessageCallback(asFunctionPtr(scriptEngineLog), nullptr, asCALL_CDECL);
	RegisterStdString(_scriptEngine);
	_handleCount = 0;
}

LowLevelSystem::~LowLevelSystem() {
	/*Release all runnings contexts */
	cleanupRegisteredString();
	_scriptEngine->Release();
	// perhaps not the best thing to skip :)
	// if(gpLogWriter)	hplDelete(gpLogWriter);
	// gpLogWriter = NULL;
}

static void commonLog(int level, const char *fmt, va_list args) {
	char buffer[256];
	vsnprintf(buffer, 256, fmt, args);
	debugN(level, "%s", buffer);
}

void Error(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	commonLog(Hpl1::kDebugLevelError, fmt, args);
	va_end(args);
}

void Warning(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	commonLog(Hpl1::kDebugLevelWarning, fmt, args);
	va_end(args);
}

void Log(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	commonLog(Hpl1::kDebugLevelLog, fmt, args);
	va_end(args);
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
	vsnprintf(text, 2048, fmt, ap);
	va_end(ap);

	tString sMess = "";
	sMess += text;
	gUpdateLogWriter.Write(sMess);
#endif
}

//-----------------------------------------------------------------------

void CopyTextToClipboard(const tWString &text) {
	g_system->setTextInClipboard(text);
}

tWString LoadTextFromClipboard() {
	Common::U32String text = g_system->getTextFromClipboard();
	return text;
}

//-----------------------------------------------------------------------

void CreateMessageBoxW(eMsgBoxType eType, const wchar_t *caption, const wchar_t *fmt, va_list ap) {
}

void CreateMessageBoxW(eMsgBoxType eType, const wchar_t *caption, const wchar_t *fmt, ...) {
}

void CreateMessageBoxW(const wchar_t *caption, const wchar_t *fmt, ...) {
}

//-----------------------------------------------------------------------

/*static cDate DateFromGMTIme(struct tm *apClock) {
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
}*/

//-----------------------------------------------------------------------

void OpenBrowserWindow(const tWString &URL) {
	g_system->openUrl(URL);
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
	return Common::U32String("");
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
	TimeDate td;
	g_system->getTimeAndDate(td);
	return {td.tm_sec, td.tm_min, td.tm_hour, td.tm_mday, td.tm_mon, td.tm_year - 100, td.tm_wday, 0};
}

iScript *LowLevelSystem::createScript(const tString &name) {
	return hplNew(cSqScript, (name, _scriptEngine, _handleCount++));
}

bool LowLevelSystem::addScriptFunc(const tString &funcDecl, asGENFUNC_t pFunc, int callConv) {
	if (_scriptEngine->RegisterGlobalFunction(funcDecl.c_str(),
											  asFUNCTION(pFunc), callConv) < 0) {
		Hpl1::logError(Hpl1::kDebugScripts, "Couldn't add script function '%s'\n", funcDecl.c_str());
		return false;
	}

	return true;
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
