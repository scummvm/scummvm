/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* 
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/package/physfspackagemanager.h"

#include "sword25/kernel/memlog_off.h"
#include <vector>
#include <sstream>
#include "sword25/kernel/memlog_on.h"

using namespace std;

// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "PHYSFSPACKAGEMANAGER"

// -----------------------------------------------------------------------------
// Hilfsfunktionen
// -----------------------------------------------------------------------------

namespace
{
	const char * SafeGetLastError()
	{
		const char * ErrorMessage = PHYSFS_getLastError();
		return ErrorMessage ? ErrorMessage : "unknown";
	}

	// -------------------------------------------------------------------------

	void LogPhysfsError(const char * FunctionName)
	{
		BS_LOG_ERRORLN("%s() failed. Reason: %s.", FunctionName, SafeGetLastError());
	}

	// -------------------------------------------------------------------------

	void LogPhysfsError(const char * FunctionName, const char * FileName)
	{
		BS_LOG_ERRORLN("%s() on file \"%s\" failed. Reason: %s.", FunctionName, FileName, SafeGetLastError());
	}

	// -------------------------------------------------------------------------

	const char PATH_SEPARATOR = '/';
	const char NAVIGATION_CHARACTER = '.';

	// -------------------------------------------------------------------------

	std::string RemoveRedundantPathSeparators(const std::string & Path)
	{
		std::string Result;

		// Über alle Zeichen des Eingabepfades iterieren.
		std::string::const_iterator It = Path.begin();
		while (It != Path.end())
		{
			if (*It == PATH_SEPARATOR)
			{
				// Verzeichnistrenner gefunden.

				// Folgen von Verzeichnistrennern überspringen.
				while (It != Path.end() && *It == PATH_SEPARATOR) ++It;

				// Einzelnen Verzeichnistrenner ausgeben, nur am Ende des Pfades wird kein Verzeichnistrenner mehr ausgegeben.
				if (It != Path.end()) Result.push_back(PATH_SEPARATOR);
			}
			else
			{
				// Normales Zeichen gefunden, wird unverändert ausgegeben.
				Result.push_back(*It);
				++It;
			}
		}

		return Result;
	}

	// ---------------------------------------------------------------------

	struct PathElement
	{
	public:
		PathElement(std::string::const_iterator Begin, std::string::const_iterator End) : m_Begin(Begin), m_End(End) {}

		std::string::const_iterator GetBegin() const { return m_Begin; }
		std::string::const_iterator GetEnd() const { return m_End; }

	private:
		std::string::const_iterator m_Begin;
		std::string::const_iterator m_End;
	};

	// -------------------------------------------------------------------------

	std::string NormalizePath(const std::string & Path, const std::string & CurrentDirectory)
	{
		// Feststellen, ob der Pfad absolut (beginnt mit /) oder relativ ist und im relativen Fall dem Gesamtpfad das aktuelle Verzeichnis
		// voranstellen.
		std::string WholePath = (Path.size() >= 1 && Path[0] == PATH_SEPARATOR) ? "" : CurrentDirectory + PATH_SEPARATOR;

		// Alle gedoppelten und nachfolgende Verzeichnistrenner aus dem übergebenen Pfad entfernen und den Gesamtpfad zusammensetzen.
		// CurrentDirectory wird nicht auf diese Weise gesäubert. Es wird vorrausgesetzt, dass CurrentDirectory keine überflüssigen
		// Verzeichnistrenner beinhaltet.
		WholePath += RemoveRedundantPathSeparators(Path);

		// Gesamtpfad parsen und in Einzelelemente aufteilen. Dabei werden Vorkommen von ".." und "." korrekt behandelt.
		vector<PathElement> PathElements;
		size_t SeparatorPos = 0;
		while (SeparatorPos < WholePath.size())
		{
			// Nächsten Verzeichnistrenner finden.
			size_t NextSeparatorPos = WholePath.find(PATH_SEPARATOR, SeparatorPos + 1);
			if (NextSeparatorPos == std::string::npos) NextSeparatorPos = WholePath.size();

			// Anfang und Ende vom Pfadelement berechnen.
			std::string::const_iterator ElementBegin = WholePath.begin() + SeparatorPos + 1;
			std::string::const_iterator ElementEnd = WholePath.begin() + NextSeparatorPos;

			if (ElementEnd - ElementBegin == 2 &&
				ElementBegin[0] == NAVIGATION_CHARACTER &&
				ElementBegin[1] == NAVIGATION_CHARACTER)
			{
				// Element ist "..", daher wird das vorangegangene Pfadelement aus dem vector entfernt.
				if (PathElements.size()) PathElements.pop_back();
			}
			else if (ElementEnd - ElementBegin == 1 &&
				ElementBegin[0] == NAVIGATION_CHARACTER)
			{
				// Element ist ".", wir tun gar nichts.
			}
			else
			{
				// Normales Element in den vector einfügen.
				PathElements.push_back(PathElement(WholePath.begin() + SeparatorPos + 1, WholePath.begin() + NextSeparatorPos));
			}

			SeparatorPos = NextSeparatorPos;
		}

		if (PathElements.size())
		{
			// Die einzelnen Pfadelemente werden durch Verzeichnistrenner getrennt aneinandergesetzt.
			// Der so entstandene String wird als Ergebnis zurückgegeben.
			ostringstream PathBuilder;
			vector<PathElement>::const_iterator It = PathElements.begin();
			while (It != PathElements.end())
			{
				PathBuilder << PATH_SEPARATOR << std::string(It->GetBegin(), It->GetEnd());
				++It;
			}

			return PathBuilder.str();
		}
		else
		{
			// Nach dem Parsen sind keine Pfadelemente mehr übrig geblieben, daher wird des Root-Verzeichnis zurückgegeben.
			return std::string(1, PATH_SEPARATOR);
		}
	}

	// -------------------------------------------------------------------------
	// RAII-Klasse für PHYSFS-Filehandles.
	// -------------------------------------------------------------------------

	class PhysfsHandleHolder
	{
	public:
		PhysfsHandleHolder(PHYSFS_File * Handle) : m_Handle(Handle) {}
		~PhysfsHandleHolder()
		{
			if (m_Handle)
			{
				if (!PHYSFS_close(m_Handle)) LogPhysfsError("PHYSFS_close");
			}
		}

		PHYSFS_File * Get() { return m_Handle; }
		PHYSFS_File * Release()
		{
			PHYSFS_File * Result = m_Handle;
			m_Handle = 0;
			return Result;
		}

	private:
		PHYSFS_File * m_Handle;
	};

	// -------------------------------------------------------------------------
	// RAII-Klasse für PHYSFS-Listen.
	// -------------------------------------------------------------------------

	template<typename T>
	class PhysfsListHolder
	{
	public:
		PhysfsListHolder(T List) : m_List(List) {};
		~PhysfsListHolder() { if (m_List) PHYSFS_freeList(m_List); }

		T Get() { return m_List; }

	private:
		T m_List;
	};

	// -------------------------------------------------------------------------

	PHYSFS_File * OpenFileAndGetSize(const std::string & FileName, const std::string & CurrentDirectory, unsigned int & FileSize)
	{
		// Datei öffnen.
		PhysfsHandleHolder Handle(PHYSFS_openRead(NormalizePath(FileName, CurrentDirectory).c_str()));
		if (!Handle.Get())
		{
			LogPhysfsError("PHYSFS_openRead", FileName.c_str());
			return 0;
		}

		// Dateigröße bestimmen.
		PHYSFS_sint64 LongFileSize = PHYSFS_fileLength(Handle.Get());
		if (LongFileSize == -1)
		{
			BS_LOG_ERRORLN("Unable to determine filelength on PhysicsFS file \"%s\".", FileName.c_str());
			return 0;
		}
		if (LongFileSize >= UINT_MAX)
		{
			BS_LOG_ERRORLN("File \"%s\" is too big.", FileName.c_str());
			return 0;
		}

		// Rückgabewerte setzen.
		FileSize = static_cast<unsigned int>(LongFileSize);
		return Handle.Release();
	}
}

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

BS_PhysfsPackageManager::BS_PhysfsPackageManager(BS_Kernel * KernelPtr) :
	BS_PackageManager(KernelPtr),
	m_CurrentDirectory(1, PATH_SEPARATOR)
{
	if (!PHYSFS_init(0)) LogPhysfsError("PHYSFS_init");
}

// -----------------------------------------------------------------------------

BS_PhysfsPackageManager::~BS_PhysfsPackageManager()
{
	if (!PHYSFS_deinit()) LogPhysfsError("PHYSFS_deinit");
}

// -----------------------------------------------------------------------------

BS_Service * BS_PhysfsPackageManager_CreateObject(BS_Kernel * KernelPtr) { return new BS_PhysfsPackageManager(KernelPtr); }

// -----------------------------------------------------------------------------

bool BS_PhysfsPackageManager::LoadPackage(const std::string & FileName, const std::string & MountPosition)
{
	if (!PHYSFS_mount(FileName.c_str(), NormalizePath(MountPosition, m_CurrentDirectory).c_str(), 0))
	{
		BS_LOG_ERRORLN("Unable to mount file \"%s\" to \"%s\". Reason: %s.", FileName.c_str(), MountPosition.c_str(), SafeGetLastError());
		return false;
	}
	else
	{
		BS_LOGLN("Package '%s' mounted as '%s'.", FileName.c_str(), MountPosition.c_str());
		return true;
	}
}

// -----------------------------------------------------------------------------

bool BS_PhysfsPackageManager::LoadDirectoryAsPackage(const std::string & DirectoryName, const std::string & MountPosition)
{
	if (!PHYSFS_mount(DirectoryName.c_str(), NormalizePath(MountPosition, m_CurrentDirectory).c_str(), 0))
	{
		BS_LOG_ERRORLN("Unable to mount directory \"%s\" to \"%s\". Reason: %s.", DirectoryName.c_str(), MountPosition.c_str(), SafeGetLastError());
		return false;
	}
	else
	{
		BS_LOGLN("Directory '%s' mounted as '%s'.", DirectoryName.c_str(), MountPosition.c_str());
		return true;
	}
}

// -----------------------------------------------------------------------------

void * BS_PhysfsPackageManager::GetFile(const std::string & FileName, unsigned int * FileSizePtr)
{
	// Datei öffnen und deren Größe bestimmen.
	unsigned int FileSize;
	PhysfsHandleHolder Handle(OpenFileAndGetSize(FileName, m_CurrentDirectory, FileSize));
	if (!Handle.Get()) return 0;

	// Falls gewünscht, die Größe der Datei zurückgeben.
	if (FileSizePtr) *FileSizePtr = FileSize;

	// Datei einlesen.
	char * Buffer = new char[FileSize];
	if (PHYSFS_read(Handle.Get(), Buffer, 1, FileSize) <= 0)
	{
		LogPhysfsError("PHYSFS_read", FileName.c_str());
		delete [] Buffer;
		return 0;
	}

	return Buffer;
}

// -----------------------------------------------------------------------------

std::string BS_PhysfsPackageManager::GetCurrentDirectory()
{
	return m_CurrentDirectory;
}

// -----------------------------------------------------------------------------

bool BS_PhysfsPackageManager::ChangeDirectory(const std::string & Directory)
{
	// Pfad normalisieren.
	std::string CleanedDirectory = NormalizePath(Directory, m_CurrentDirectory);

	// Interne Variable setzen, wenn das Verzeichnis tatsächlich existiert oder das Wurzelverzeichnis ist.
	if (CleanedDirectory == std::string(1, PATH_SEPARATOR) || PHYSFS_isDirectory(CleanedDirectory.c_str()))
	{
		m_CurrentDirectory = CleanedDirectory;
		return true;
	}
	// Fehler ausgeben, wenn das Verzeichnis nicht existiert.
	else
	{
		BS_LOG_ERRORLN("Tried to change to non-existing directory \"%s\". Call is ignored", Directory.c_str());
		return false;
	}
}

// -----------------------------------------------------------------------------

std::string BS_PhysfsPackageManager::GetAbsolutePath(const std::string & FileName)
{
	return NormalizePath(FileName, m_CurrentDirectory);
}

// -----------------------------------------------------------------------------

unsigned int BS_PhysfsPackageManager::GetFileSize(const std::string & FileName)
{
	// Datei öffnen und deren Größe bestimmen.
	unsigned int FileSize;
	PhysfsHandleHolder Handle(OpenFileAndGetSize(FileName, m_CurrentDirectory, FileSize));
	if (!Handle.Get()) return 0xffffffff;

	// Größe der Datei zurückgeben.
	return FileSize;
}

// -----------------------------------------------------------------------------

unsigned int BS_PhysfsPackageManager::GetFileType(const std::string & FileName)
{
	std::string NormalizedPath = NormalizePath(FileName, m_CurrentDirectory);

	if (PHYSFS_exists(NormalizedPath.c_str()))
	{
		return PHYSFS_isDirectory(NormalizedPath.c_str()) ? BS_PackageManager::FT_DIRECTORY : BS_PackageManager::FT_FILE;
	}
	else
	{
		BS_LOG_ERRORLN("Cannot determine type of non-existant file \"%s\".", NormalizedPath.c_str());
		return 0;
	}
}

// -----------------------------------------------------------------------------

bool BS_PhysfsPackageManager::FileExists(const std::string & FileName)
{
	std::string NormalizedPath = NormalizePath(FileName, m_CurrentDirectory);
	return PHYSFS_exists(NormalizedPath.c_str()) != 0;
}

// -----------------------------------------------------------------------------
// Dateien suchen
// -----------------------------------------------------------------------------

class PhysfsFileSearch : public BS_PackageManager::FileSearch
{
public:
	// Path muss normalisiert sein.
	PhysfsFileSearch(BS_PackageManager & PackageManager, const vector<std::string> & FoundFiles) :
		m_PackageManager(PackageManager),
		m_FoundFiles(FoundFiles),
		m_FoundFilesIt(m_FoundFiles.begin())
	{
	}

	virtual std::string GetCurFileName()
	{
		return *m_FoundFilesIt;
	}

	virtual unsigned int GetCurFileType()
	{
		return m_PackageManager.GetFileType(*m_FoundFilesIt);
	}

	virtual unsigned int GetCurFileSize()
	{
		return m_PackageManager.GetFileSize(*m_FoundFilesIt);
	}

	virtual bool NextFile()
	{
		++m_FoundFilesIt;
		return m_FoundFilesIt != m_FoundFiles.end();
	}

	BS_PackageManager &					m_PackageManager;
	vector<std::string>					m_FoundFiles;
	vector<std::string>::const_iterator	m_FoundFilesIt;
};

// -----------------------------------------------------------------------------

BS_PackageManager::FileSearch * BS_PhysfsPackageManager::CreateSearch(const std::string& Filter, const std::string& Path, unsigned int TypeFilter)
{
	std::string NormalizedPath = NormalizePath(Path, m_CurrentDirectory);

	// Nach Wildcards gefilterte Ergebnisliste erstellen.
	PhysfsListHolder<char **> FilesPtr(PHYSFSEXT_enumerateFilesWildcard(NormalizedPath.c_str(), Filter.c_str(), 1));

	// Diese Liste muss nun wiederum nach den gewünschten Dateitype gefiltert werden. Das Ergebnis wird in einem vector gespeichert, der dann
	// einem PhysfsFileSearch-Objekt übergeben wird.
	vector<std::string> FoundFiles;
	for (char ** CurFilePtr = FilesPtr.Get(); *CurFilePtr != 0; ++CurFilePtr)
	{
		// Vollständigen Pfad zur gefunden Datei konstruieren.
		std::string FullFilePath = NormalizedPath + std::string(1, PATH_SEPARATOR) + *CurFilePtr;

		// Feststellen, ob der Dateityp erwünscht ist und nur dann den Dateinamen dem Ergebnisvektor hinzufügen.
		unsigned int FileType = GetFileType(FullFilePath);
		if (FileType & TypeFilter) FoundFiles.push_back(FullFilePath);
	}

	// Falls überhaupt eine Datei gefunden wurde, wird ein FileSearch-Objekt zurückgegeben mit dem über die gefundenen Dateien iteriert werden kann.
	// Anderenfalls wird 0 zurückgegeben.
	if (FoundFiles.size())
	{
		return new PhysfsFileSearch(*this, FoundFiles);
	}
	else
	{
		return 0;
	}
}
