// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "filesystemutil.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ShlObj.h>
#include <memory.h>

using namespace std;

#define BS_LOG_PREFIX "FILESYSTEMUTILWIN32"

// -----------------------------------------------------------------------------
// Konstanten und Hilfsfunktionen
// -----------------------------------------------------------------------------

namespace
{
	const char * DIRECTORY_NAME = "Broken Sword 2.5";

	// -------------------------------------------------------------------------

	string GetAbsolutePath(const string & Path)
	{
		char Buffer[MAX_PATH];
		if (!::GetFullPathNameA(Path.c_str(), MAX_PATH, Buffer, 0))
		{
			// Bei der Ausführung von GetFullPathNameA() ist ein Fehler aufgetreten.
			// Wir können an dieser Stelle nichts andere machen, als einen leeren String zurückzugeben.
			BS_LOG_ERRORLN("A call to GetFullPathNameA() failed.");
			return "";
		}

		// Ergebnis zurückgeben.
		return string(Buffer);
	}
}

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_FileSystemUtilWin32 : public BS_FileSystemUtil
{
public:
	virtual string GetUserdataDirectory()
	{
		// Die C++ Dateisystemfunktionen können leider nicht mit Unicode-Dateinamen umgehen.
		// Für uns ist das problematisch, wenn wir in das %APPDATA%-Verzeichnis eines Benutzers zugreifen wollen,
		// dessen Name Unicode-Zeichen enthält, die sich mit der aktuellen Codepage nicht als ANSI-String darstellen 
		// lassen.
		// Wir behelfen uns damit, dass wir das Verzeichnis als Unicode-String abfragen, in die kurze
		// Verzeichnisdarstellung konvertieren und das Ergebnis in einen ANSI-String konvertieren.
		// Kurze Dateinamen sollten keine Unicode-Zeichen enthalten, ich habe allerdings keine offizielle Aussage dazu
		// gefunden.

		WCHAR PathBuffer[MAX_PATH];

		// Das %APPDATA%-Verzeichnis erfragen.
		if (::SHGetSpecialFolderPathW(0, PathBuffer, CSIDL_APPDATA, FALSE) == FALSE)
		{
			BS_LOG_ERRORLN("SHGetSpecialFolderPathW() failed");
			return "";
		}

		// Die kurze Variante des Verzeichnisses erfragen.
		if (::GetShortPathNameW(PathBuffer, PathBuffer, MAX_PATH) == 0)
		{
			BS_LOG_ERRORLN("GetShortPathNameW() failed");
			return "";
		}

		// Die Verzeichnisangabe in einen ANSI-String konvertieren.
		char AnsiPathBuffer[MAX_PATH];
		BOOL UsedDefaultChar = FALSE;
		if (::WideCharToMultiByte(CP_ACP, 0, PathBuffer, -1, AnsiPathBuffer, MAX_PATH, 0, &UsedDefaultChar) == 0)
		{
			BS_LOG_ERRORLN("WideCharToMultiByte() failed");
			return "";
		}

		// Falls bei der Konvertierung ein zum Einsatz kam, ist das Ergebnis nicht eindeutig und damit nicht
		// verwendbar.
		if (UsedDefaultChar)
		{
			BS_LOG_ERRORLN("Conversion from unicode to ANSI is ambiguous.");
			return "";
		}

		// Verzeichnis zurückgeben.
		return string(AnsiPathBuffer) + "\\" + DIRECTORY_NAME;
	}

	virtual string GetPathSeparator()
	{
		return string("\\");
	}

	virtual uint64_t GetFileSize(const std::string & Filename)
	{
		WIN32_FILE_ATTRIBUTE_DATA fileAttributeData;
		// Dateiattribute einlesen.
		if (::GetFileAttributesExA(Filename.c_str(), GetFileExInfoStandard, &fileAttributeData) != 0)
		{
			// Die Dateigröße wird von Windows in zwei 32-Bit Zahlen angegeben. Diese werden an dieser Stelle in eine 64-Bit Zahl umgewandelt.
			uint64_t fileSize = fileAttributeData.nFileSizeHigh;
			fileSize <<= 32;
			fileSize |= fileAttributeData.nFileSizeLow;
			return fileSize;
		}
		else
		{
			return -1;
		}	
	}

	virtual time_t GetFileTime(const std::string & Filename)
	{
		WIN32_FILE_ATTRIBUTE_DATA fileAttributeData;
		if (::GetFileAttributesExA(Filename.c_str(), GetFileExInfoStandard, &fileAttributeData) != 0)
		{
			__int64 timestamp;
			memcpy(&timestamp, &fileAttributeData.ftLastWriteTime, sizeof(FILETIME));
			return (timestamp - 0x19DB1DED53E8000) / 10000000; 
		}
		else
		{
			return 0;
		}
	}

	virtual bool FileExists(const std::string & Filename)
	{
		return ::GetFileAttributesA(Filename.c_str()) != INVALID_FILE_ATTRIBUTES;
	}

	// Windows.h enthält ein Makro mit dem Namen CreateDirectory. Dieses muss entfernt werden bevor die Definition von
	// unserem CreateDirectory() folgt.
	#ifdef CreateDirectory
		#undef CreateDirectory
	#endif

	virtual bool CreateDirectory(const string & DirectoryName)
	{
		return CreateDirectoryRecursive(GetAbsolutePath(DirectoryName));
	}

	virtual vector<string> GetFilesInDirectory(const std::string & Directory)
	{
		vector<string> Result;

		// Suchstring erstellen, dabei muss der leere String (aktuelles Verzeichnis) gesondert behandelt werden.
		string SearchPattern;
		if (Directory.empty())
			SearchPattern = "*";
		else
			SearchPattern = Directory + "\\*";

		// Die erste Datei suchen.
		WIN32_FIND_DATAA FindData;
		HANDLE FindHandle = ::FindFirstFileA(SearchPattern.c_str(), &FindData);

		while (FindHandle != INVALID_HANDLE_VALUE)
		{
			// Verzeichnisse ignorieren.
			// Beim erstellen des Ergebnispfades muss wieder der Sonderfall der leeren Verzeichnisangabe berücksichtigt werden.
			if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				Result.push_back(FindData.cFileName);

			// Solange weitermachen, bis keine Dateien mehr gefunden werden.
			if (FindNextFileA(FindHandle, &FindData) == 0)
			{
				FindClose(&FindHandle);
				break;
			}
		}

		return Result;
	}

private:
	bool CreateDirectoryRecursive(string & DirectoryName)
	{
		// 
		// http://www.codeguru.com/Cpp/W-P/files/article.php/c4439/
		// (c) Assaf Tzur-El
		//

		DWORD attr;
		int pos;
		bool result = true;

		// Check for trailing slash:
		pos = DirectoryName.find_last_of("\\");
		if (DirectoryName.length() == pos + 1)  // last character is "\"
		{
			DirectoryName.resize(pos);
		}

		// Look for existing object:
		attr = ::GetFileAttributesA(DirectoryName.c_str());
		if (0xFFFFFFFF == attr)  // doesn't exist yet - create it!
		{
			pos = DirectoryName.find_last_of("\\");
			if (0 < pos)
			{
				// Create parent dirs:
				result = CreateDirectoryRecursive(DirectoryName.substr(0, pos));
			}
			// Create node:
			result = result && ::CreateDirectoryA(DirectoryName.c_str(), NULL);
		}
		else if (!(FILE_ATTRIBUTE_DIRECTORY & attr))
		{  // object already exists, but is not a dir
			::SetLastError(ERROR_FILE_EXISTS);
			result = false;
		}

		return result;
	}
};

// -----------------------------------------------------------------------------
// Singleton-Methode der Elternklasse
// Hiermit wird sichergestellt, dass wenn immer diese Datei kompiliert wird,
// die Singleton-Methode der Oberklasse diese Klasse instanziiert.
// Unterscheidung zwischen den Plattformen wird so nur durch Linken gegen andere
// Dateien realisiert.
// -----------------------------------------------------------------------------

BS_FileSystemUtil & BS_FileSystemUtil::GetInstance()
{
	static BS_FileSystemUtilWin32 Instance;
	return Instance;
}
