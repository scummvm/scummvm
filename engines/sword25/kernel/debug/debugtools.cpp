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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
using namespace std;

#include "sword25/kernel/md5.h"
#include "sword25/kernel/filesystemutil.h"
#include "sword25/kernel/debug/debugtools.h"

// -----------------------------------------------------------------------------
// Konstanten und Hilfsfunktionen
// -----------------------------------------------------------------------------

namespace
{
	const char *		VERSION_ID_ERROR = "???";
	const unsigned int	READBUFFER_SIZE = 1024 * 100;

	const char *		SUBVERSION_ENTRIES_FILENAME = ".svn\\entries";

	// -------------------------------------------------------------------------

	unsigned int ParseUnsignedInt(const string & Str, bool & Success)
	{
		istringstream iss(Str);

		unsigned int Result = 0;
		iss >> Result;

		Success = !iss.fail();
		
		return Result;
	}
}

// -----------------------------------------------------------------------------

const char * BS_Debugtools::GetVersionID()
{
	// Falls die Versions-ID noch nicht bekannt ist, muss sie bestimmt werden
	static string VersionIDString;
	if (VersionIDString.size() == 0)
	{
		// Dateinamen der EXE-Datei bestimmen
		char FileName[MAX_PATH + 1];
		if (GetModuleFileName(0, FileName, sizeof(FileName)) == 0) return VERSION_ID_ERROR;

		// Datei öffnen
		HANDLE FileHandle = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (INVALID_HANDLE_VALUE == FileHandle) return VERSION_ID_ERROR;

		// Datei stückchenweise Einlesen und MD5-Hash bilden
		BS_MD5 md5;
		std::vector<unsigned char> ReadBuffer(READBUFFER_SIZE);
		DWORD BytesRead = 0;
		do
		{
			// MD5-Hash für das eingelesene Dateistück berechnen.
			md5.Update(&ReadBuffer[0], BytesRead);

			if (ReadFile(FileHandle, &ReadBuffer[0], READBUFFER_SIZE, &BytesRead, 0) == FALSE)
			{
				CloseHandle(FileHandle);
				return VERSION_ID_ERROR;
			}
		} while (BytesRead > 0);

		// Datei schließen
		CloseHandle(FileHandle);

		// Falls sich das aktuelle Verzeichnis in einem Subversion-Repository befindet, wird auch die Subversion-Revision mit in die ID gehasht.
		// Dieses stellt im Beta-Test sicher, dass jede Änderung einer Datei, und nicht nur der EXE, zu einer neuen Versions-ID führt.
		unsigned int SubversionRevision = GetSubversionRevision();
		if (SubversionRevision != 0) md5.Update(reinterpret_cast<unsigned char *>(&SubversionRevision), sizeof(unsigned int));

		// MD5 abschließen
		unsigned char Digest[16];
		md5.GetDigest(Digest);
		
		// VersionsID-String erstellen
		std::ostringstream VersionIDBuf;
		VersionIDBuf << std::hex;
		for (unsigned int i = 0; i < sizeof(Digest); i++)
		{
			VersionIDBuf << (unsigned int) Digest[i];
		}
		VersionIDString = VersionIDBuf.str();
	}

	return VersionIDString.c_str();
}

// -----------------------------------------------------------------------------

unsigned int BS_Debugtools::GetSubversionRevision()
{
	// Existiert eine entries Datei?
	if (BS_FileSystemUtil::GetInstance().FileExists(SUBVERSION_ENTRIES_FILENAME))
	{
		bool Success;
		char Buffer[512];

		// entries Datei öffnen.
		ifstream File(SUBVERSION_ENTRIES_FILENAME);
		if (File.fail()) return 0;

		// Das Format auslesen und feststellen, ob wir es unterstützen.
		File.getline(Buffer, sizeof(Buffer), 0x0A);
		unsigned int FormatVersion = ParseUnsignedInt(Buffer, Success);
		if (File.fail() || !Success || FormatVersion < 7) return 0;

		// Den Namen des ersten Eintrages auslesen. Dieses muss ein leerer String sein und somit das aktuelle Verzeichnis benennen.
		File.getline(Buffer, sizeof(Buffer), 0x0A);
		if (File.fail() || strlen(Buffer) != 0) return 0;

		// Den Typ des Eintrages auslesen. Dieser muss "dir" sein.
		File.getline(Buffer, sizeof(Buffer), 0x0A);
		if (File.fail() || strcmp(Buffer, "dir") != 0) return 0;

		// Die Revision des Eintrages auslesen.
		File.getline(Buffer, sizeof(Buffer), 0x0A);
		unsigned int Revision = ParseUnsignedInt(Buffer, Success);
		if (File.fail() || !Success) return 0;

		return Revision;
	}

	return 0;
}
