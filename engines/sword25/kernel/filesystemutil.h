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

/*
	Die Klasse BS_FileSystemUtil stellt einen Wrapper für dateisystemspezifische Operationen dar, die nicht über die C/C++ Bibliotheken
	abgedeckt werden.

	Jede unterstützte Plattform muss dieses Interface implementieren und die Singleton-Methode BS_FileSystemUtil::GetInstance()
	implementieren.
*/

#ifndef BS_FILESYSTEMUTIL_H
#define BS_FILESYSTEMUTIL_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/common.h"
#include "kernel/bs_stdint.h"

#include "kernel/memlog_off.h"
#include <string>
#include <vector>
#include <ctime>
#include "kernel/memlog_on.h"

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_FileSystemUtil
{
public:
	static BS_FileSystemUtil & GetInstance();
	virtual ~BS_FileSystemUtil() {};

	/*
		Diese Funktion gibt den Namen des Verzeichnisses zurück, in dem sämtliche Benutzerdaten gespeichert werden sollen.

		Dieses sind z.B. Screenshots, Spielstände, Konfigurationsdateien, Logdateien, ...
		Unter Windows Vista ist dieses beispielsweise: C:\Users\Malte\Documents\Broken Sword 2.5
		Falls dieses Verzeichnis noch nicht existiert, wird es automatisch erstellt.

		@return Gibt den Namen des Verzeichnisses für Benutzerdaten zurück.
	*/
	virtual std::string GetUserdataDirectory() = 0;
	/*
		@return Gibt den Pfadtrenner zurück (z.B. \ unter Windows und / unter Linux).
	*/
	virtual std::string	GetPathSeparator() = 0;
	/*
		@param Filename der Pfad zu einer Datei.
		@return Gibt die Größe der angegebenen Datei zurück. Falls die Größe nicht bestimmt werden konnte, oder die Datei nicht existiert wird -1 zurückgegeben.
	*/
	virtual uint64_t GetFileSize(const std::string & Filename) = 0;
	/*
		@param Filename der Pfad zu einer Datei.
		@return Gibt den Zeitstempel des Zeitpunktes zurück an dem die Datei zuletzt verändert wurde. Bei einem Fehler wird 0 zurückgegeben.
	*/
	virtual time_t GetFileTime(const std::string & Filename) = 0;
	/*
		@param Filename der Pfad zu einer Datei.
		@return Gibt true zurück, wenn die Datei existiert.
	*/
	virtual bool FileExists(const std::string & Filename) = 0;
	/*
		Diese Funktion erstellt einen Verzeichnis mit sämtlichen Unterverzeichnissen.

		Wenn des Parameter "\b\c\d\e" ist und der Pfad "\b\c" bereits existiert, werden die Verzeichnisse
		"d" und "e" erstellt.

		@param DirectoryName der Name des zu erstellenden Verzeichnisses.
		@return Gibt true zurück, wenn die Verzeichnisse erstellt werden konnten, ansonsten false.
	*/
	virtual bool CreateDirectory(const std::string & DirectoryName) = 0;
	/*
		Erstellt eine Liste aller Dateinamen in einem Verzeichnis.

		@param Directory das zu durchsuchende Verzeichnis.
		@return Gibt einen vector mit allen gefundenen Dateinamen zurück.
	*/
	virtual std::vector<std::string> GetFilesInDirectory(const std::string & Path) = 0;
};

#endif
