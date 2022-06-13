/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef HPL_LOWLEVELSYSTEM_H
#define HPL_LOWLEVELSYSTEM_H

#include "hpl1/engine/system/MemoryManager.h"
#include "hpl1/engine/system/SystemTypes.h"


namespace hpl {

#ifdef UPDATE_TIMING_ENABLED
	#define START_TIMING_EX(x,y)	LogUpdate("Updating %s in file %s at line %d\n",x,__FILE__,__LINE__); \
									unsigned int y##_lTime = GetApplicationTime();
	#define START_TIMING(x)	LogUpdate("Updating %s in file %s at line %d\n",#x,__FILE__,__LINE__); \
								unsigned int x##_lTime = GetApplicationTime();
	#define STOP_TIMING(x)	LogUpdate(" Time spent: %d ms\n",GetApplicationTime() - x##_lTime);
	#define START_TIMING_TAB(x)	LogUpdate("\tUpdating %s in file %s at line %d\n",#x,__FILE__,__LINE__); \
							unsigned int x##_lTime = GetApplicationTime();
	#define STOP_TIMING_TAB(x)	LogUpdate("\t Time spent: %d ms\n",GetApplicationTime() - x##_lTime);
#else
	#define START_TIMING_EX(x,y)
	#define START_TIMING(x)
	#define STOP_TIMING(x)
	#define START_TIMING_TAB(x)
	#define STOP_TIMING_TAB(x)
#endif

	class iScript;

	extern void SetLogFile(const tWString &asFile);
	extern void FatalError(const char* fmt,... );
	extern void Error(const char* fmt, ...);
	extern void Warning(const char* fmt, ...);
	extern void Log(const char* fmt, ...);

	extern void SetUpdateLogFile(const tWString &asFile);
	extern void ClearUpdateLogFile();
	extern void SetUpdateLogActive(bool abX);
	extern void LogUpdate(const char* fmt, ...);

//	extern void CreateMessageBox ( const char* asCaption, const char *fmt, ...);
//	extern void CreateMessageBox ( eMsgBoxType eType, const char* asCaption, const char *fmt, ...);

	extern void CreateMessageBoxW( const wchar_t* asCaption, const wchar_t* fmt, ...);
	extern void CreateMessageBoxW( eMsgBoxType eType, const wchar_t* asCaption, const wchar_t* fmt, ...);

	extern void OpenBrowserWindow ( const tWString& asURL );

	extern void CopyTextToClipboard(const tWString &asText);
	extern tWString LoadTextFromClipboard();

	extern tWString GetSystemSpecialPath(eSystemPath aPathType);

	extern bool FileExists(const tWString& asFileName);
	extern void RemoveFile(const tWString& asFileName);
	extern bool CloneFile(const tWString& asSrcFileName,const tWString& asDestFileName,
					bool abFailIfExists);
	extern bool CreateFolder(const tWString& asPath);
	extern bool FolderExists(const tWString& asPath);
	extern bool IsFileLink(const tWString& asPath);
	extern bool LinkFile(const tWString& asPointsTo, const tWString& asLink);
	extern bool RenameFile(const tWString& asFrom, const tWString& asTo);
	extern cDate FileModifiedDate(const tWString& asFilePath);
	extern cDate FileCreationDate(const tWString& asFilePath);

	extern void SetWindowCaption(const tString &asName);

	extern bool HasWindowFocus(const tWString &asWindowCaption);

	extern unsigned long GetApplicationTime();

	class iLowLevelSystem
	{
	public:
		virtual ~iLowLevelSystem(){}

		/**
		 * Remark: Usually not finer then 10ms accuracy.
		 * \return Number of millisecs since start of app.
		 */
		virtual unsigned long GetTime()=0;

		/**
		* Gets the current date.
		*/
		virtual cDate GetDate()=0;

		/**
		 * Creates a ne script
		 * \param asName name of the script.
		 * \return
		 */
		virtual iScript* CreateScript(const tString& asName)=0;

		/**
		 * Add a function to the script vm. Example: "void test(float x)"
		 * \param asFuncDecl the declaration.
		 * \return
		 */
		virtual bool AddScriptFunc(const tString& asFuncDecl, void* pFunc, int callConv)=0;

		/**
		 * Add a variable to the script vm. Example: "int MyVar"
		 * \param asVarDecl the declartion
		 * \param *pVar the variable
		 * \return
		 */
		virtual bool AddScriptVar(const tString& asVarDecl, void *pVar)=0;

		/**
		* Sets the main thread to rest for a number of milliseconds.
		* \param alMillisecs
		*/
		virtual void Sleep ( const unsigned int alMillisecs )=0 ;

	};
};
#endif // HPL_LOWLEVELSYSTEM_H
