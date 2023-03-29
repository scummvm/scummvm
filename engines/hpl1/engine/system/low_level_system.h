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

#ifndef HPL_LOWLEVELSYSTEM_H
#define HPL_LOWLEVELSYSTEM_H

#include "hpl1/engine/libraries/angelscript/angelscript.h"
#include "hpl1/engine/system/MemoryManager.h"
#include "hpl1/engine/system/SystemTypes.h"

class asIScriptEngine;

namespace hpl {

#ifdef UPDATE_TIMING_ENABLED
#define START_TIMING_EX(x, y)                                                \
	LogUpdate("Updating %s in file %s at line %d\n", x, __FILE__, __LINE__); \
	unsigned int y##_lTime = GetApplicationTime();
#define START_TIMING(x)                                                       \
	LogUpdate("Updating %s in file %s at line %d\n", #x, __FILE__, __LINE__); \
	unsigned int x##_lTime = GetApplicationTime();
#define STOP_TIMING(x) LogUpdate(" Time spent: %d ms\n", GetApplicationTime() - x##_lTime);
#define START_TIMING_TAB(x)                                                     \
	LogUpdate("\tUpdating %s in file %s at line %d\n", #x, __FILE__, __LINE__); \
	unsigned int x##_lTime = GetApplicationTime();
#define STOP_TIMING_TAB(x) LogUpdate("\t Time spent: %d ms\n", GetApplicationTime() - x##_lTime);
#else
#define START_TIMING_EX(x, y)
#define START_TIMING(x)
#define STOP_TIMING(x)
#define START_TIMING_TAB(x)
#define STOP_TIMING_TAB(x)
#endif

class iScript;

extern void SetLogFile(const tWString &File);
extern void Error(const char *fmt, ...);
extern void Warning(const char *fmt, ...);
extern void Log(const char *fmt, ...);

extern void SetUpdateLogActive(bool abX);
extern void LogUpdate(const char *fmt, ...);

extern void CreateMessageBoxW(const wchar_t *caption, const wchar_t *fmt, ...);
extern void CreateMessageBoxW(eMsgBoxType eType, const wchar_t *caption, const wchar_t *fmt, ...);

extern void OpenBrowserWindow(const tWString &URL);

extern void CopyTextToClipboard(const tWString &text);
extern tWString LoadTextFromClipboard();

extern tWString GetSystemSpecialPath(eSystemPath pathType);

extern bool FileExists(const tWString &fileName);
extern void RemoveFile(const tWString &fileName);
extern bool CloneFile(const tWString &srcFileName, const tWString &destFileName,
					  bool failIfExists);
extern bool CreateFolder(const tWString &path);
extern bool FolderExists(const tWString &path);
extern bool IsFileLink(const tWString &path);
extern bool LinkFile(const tWString &pointsTo, const tWString &link);
extern bool RenameFile(const tWString &from, const tWString &to);
extern cDate FileModifiedDate(const tWString &filePath);
extern cDate FileCreationDate(const tWString &filePath);

extern void SetWindowCaption(const tString &name);

extern unsigned long GetApplicationTime();

class LowLevelSystem {
public:
	LowLevelSystem();
	~LowLevelSystem();

	/**
	 * Remark: Usually not finer then 10ms accuracy.
	 * \return Number of millisecs since start of app.
	 */
	unsigned long getTime();

	/**
	 * Gets the current date.
	 */
	cDate getDate();

	/**
	 * Creates a ne script
	 * \param name name of the script.
	 * \return
	 */
	iScript *createScript(const tString &name);

	/**
	 * Add a function to the script vm. Example: "void test(float x)"
	 * \param funcDecl the declaration.
	 * \return
	 */
	bool addScriptFunc(const tString &funcDecl, asGENFUNC_t func, int callConv);

	/**
	 * Add a variable to the script vm. Example: "int MyVar"
	 * \param varDecl the declartion
	 * \param *pVar the variable
	 * \return
	 */
	bool addScriptVar(const tString &varDecl, void *var);

	/**
	 * Sets the main thread to rest for a number of milliseconds.
	 * \param alMillisecs
	 */
	void sleep(const unsigned int millisecs);

private:
	asIScriptEngine *_scriptEngine;
	int _handleCount;
};

} // namespace hpl

#endif // HPL_LOWLEVELSYSTEM_H
