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
	BS_PackageManager
	-------------
	Dies ist das Package Manager Interface, dass alle Methoden enthält, die ein Package Manager
	implementieren muss.
	Beim Package Manager ist folgendes zu beachten:
	1. es wird ein komplett neuer (virtueller) Verzeichnisbaum erstellt, 
	   in den Packages und Verzeichnisse gemounted werden können.
	2. zum Trennen von Elementen eines Verzeichnisspfades wird '/' statt '\' verwendet.
	3. LoadDirectoryAsPackage sollte nur zum Testen benutzt werden. Im Final Release sollen sich
	   alle Dateien in echten Packages befinden.

	Autor: Malte Thiesen, $author$
*/

#ifndef BS_PACKAGE_MANAGER_H
#define BS_PACKAGE_MANAGER_H

// Includes
#include "kernel/common.h"
#include "kernel/kernel.h"
#include "kernel/service.h"

// Klassendefinition
/**
	@brief Das Package Manager Interface

	Beim Package Manager ist folgendes zu beachten:<br>
	1. es wird ein komplett neuer (virtueller) Verzeichnisbaum erstellt, 
	in den Packages und Verzeichnisse gemounted werden können.<br>
	2. zum Trennen von Elementen eines Verzeichnisspfades wird '/' statt '\' verwendet.<br>
	3. LoadDirectoryAsPackage sollte nur zum Testen benutzt werden. Im Final Release sollen sich
	alle Dateien in echten Packages befinden.
*/
class BS_PackageManager : public BS_Service
{
public:
	BS_PackageManager(BS_Kernel* pKernel);
	virtual ~BS_PackageManager() {};

	enum FILE_TYPES
	{
		FT_DIRECTORY	= (1<<0),
		FT_FILE			= (1<<1)
	};

	/**
		@brief Mit Instanzen dieses Objektes wird nach Dateien gesucht.

		Objekte diesen Types wird mit BS_PackageManager::CreateSearch erzeugt.
	*/
	class FileSearch
	{
	public:
		virtual ~FileSearch(){};

		/**
			@brief Gibt den Dateinamen der aktuellen Datei zurück.
			@return Gibt den Dateinamen der aktuellen Datei zurück.
		*/
		virtual std::string GetCurFileName() = 0;
		/**
			@brief Gibt den Typ der aktuellen Datei zurück.
			@return Gibt den Typ der aktuellen Datei zurück.<br>
					Dieses ist entweder BS_PackageManager::FT_FILE oder BS_PackageManager::FT_DIRECTORY.
		*/
		virtual unsigned int GetCurFileType() = 0;
		/**
			@brief Gibt die Größe der aktuellen Datei zurück.
			@return Gibt die Größe der aktuellen Datei zurück.<br>
					Bei Verzeichnissen ist dieser Wert immer 0.
		*/
		virtual unsigned int GetCurFileSize() = 0;
		// Sucht die nächste Datei
		// Gibt false zurück, falls keine weitere Datei gefunden wurde
		/**
			@brief Sucht die nächste Datei.
			@return Gibt false zurück, falls keine weitere Datei die Suchkriterien erfüllte.
		*/
		virtual bool NextFile() = 0;
	};

	/**
		@brief Mounted den Inhalt eines Packages in das angegebene Verzeichnis im virtuellen Verzeichnisbaum.
		@param FileName der Dateiname des zu mountenden Packages
		@param MountPosition der Verzeichnisname, unter dem das Package gemounted werden soll
		@return Gibt true zurück falls das mounten erfolgreich war, andernfalls false.
	*/
	virtual bool LoadPackage(const std::string& FileName, const std::string& MountPosition) = 0;
	/**
		@brief Mounted den Inhalt eines Verzeichnisses in das angegebene Verzeichnis im virtuellen Verzeichnisbaum.
		@param DirectoryName der Name des zu mountenden Verzeichnisses
		@param MountPosition der Verzeichnisname, unter dem das Verzeichnis gemounted werden soll
		@return Gibt true zurück falls das mounten erfolgreich war, andernfalls false.
	*/
	virtual bool LoadDirectoryAsPackage(const std::string& DirectoryName, const std::string& MountPosition) = 0;
	/**
		@brief Lädt eine Datei aus dem virtuellen Verzeichnisbaum.
		@param FileName der Dateiname der zu ladenden Datei
		@param pFileSize Pointer auf die Variable, die die Größe der geladenen Datei enthalten soll<br>
						 Der Standardwert ist NULL.
		@return Gibt einen Pointer auf die Dateidaten zürück, oder NULL, wenn die Datei nicht geladen werden konnte.
		@remark Es darf nicht vergessen werden, die Dateidaten nach Benutzung mit BE_DELETE_A freizugeben.
	*/
	virtual void* GetFile(const std::string& FileName, unsigned int* pFileSize = NULL) = 0;
	/**
		@brief Gibt den Pfad zum aktuellen Verzeichnis zurück.
		@return Gibt einen String zurück, der den Pfad zum aktuellen Verzeichnis enthält.<br>
				Falls der Pfad nicht bestimmt werden konnte wird ein leerer String zurückgegeben.
		@remark Zum Trennen von Pfadelementen wird "/" und nicht "\" verwendet.
	*/
	virtual		std::string GetCurrentDirectory() = 0;
	/**
		@brief Wechselt das aktuelle Verzeichnis.
		@param Directory ein String der das Verzeichnis bezeichnet, in dass gewechselt werden soll.<br>
						 Die Pfadangabe darf relativ sein.
		@return Gibt true zurück, falls der Vorgang erfolgreich war, ansonsten false.
		@remark Zum Trennen von Pfadelementen wird "/" und nicht "\" verwendet.
	*/
	virtual bool ChangeDirectory(const std::string& Directory) = 0;
	/**
		@brief Gibt den absoluten Pfad zu einer Datei im virtuellen Verzeichnisbaum zurück.
		@param FileName der Dateiname der Datei, deren absoluter Pfad bestimmt werden soll.<br>
			   Diese Parameter kann sowohl relative als auch absolute Pfadangaben beinhalten.
		@return Gibt einen String zurück, der den absoluten Pfad zur übergebenen Datei enthält.<br>
				Falls der absolute Pfad nicht bestimmt werden konnte, wird ein leerer String zurückgegeben.
		@remark Zum Trennen von Pfadelementen wird "/" und nicht "\" verwendet.
	*/
	virtual std::string GetAbsolutePath(const std::string& FileName) = 0;
	/**
		@brief Erstellt ein BS_PackageManager::FileSearch Objekt mit dem Nach Dateien gesucht werden kann.
		@param Filter gibt den Suchstring an. Dieser darf die Wildcards '*' und '?' enthalten.
		@param Path gibt das Verzeichnis an, welches durchsucht werden soll.
		@param TypeFilter ist eine Kombination der Flags BS_PackageManager::FT_DIRECTORY und BS_PackageManager::FT_FILE.<br>
						  Diese Flags geben an, ob nach Dateien oder Verzeichnissen oder beiden gesucht werden soll.<br>
						  Der Standardwert ist BS_PackageManager::FT_DIRECTORY | BS_PackageManager::FT_FILE.
		@return Gibt einen Pointer auf ein BS_PackageManager::FileSearch Objekt zurück, oder NULL wenn keine Datei gefunden wurde.
		@remark Nicht vergessen, das Objekt nach Benutzung mit delete freizugeben.
	*/
	virtual FileSearch* CreateSearch(const std::string& Filter, const std::string& Path, unsigned int TypeFilter = FT_DIRECTORY | FT_FILE) = 0;

	/**
	 * @brief Gibt die Dateigröße zurück.
	 * @param FileName die Datei.
	 * @return die Dateigröße. Im Falle eines Fehlers wird 0xffffffff zurückgegeben.
	 * @remarks Bei komprimierten Containern wird die unkomprimierte Größe zurückgegeben.
	 **/
	virtual unsigned int GetFileSize(const std::string& FileName) = 0;

	/**
		@brief Gibt den Typ einer Datei zurück.
		@param FileName der Dateiname
		@return Gibt den Dateityp zurück (BS_PackageManager::FT_DIRECTORY oder BS_PackageManager::FT_FILE).<br>
				Falls die Datei nicht gefunden wurde wird 0 zurückgegeben.
	*/
	virtual unsigned int GetFileType(const std::string & FileName) = 0;

	/**
		@brief Bestimmt, ob eine Datei existiert.
		@param FileName der Dateiname
		@return Gibt true zurück, wenn die Datei existiert, ansonsten false.
	*/
	virtual bool FileExists(const std::string & FileName) = 0;

private:
	bool _RegisterScriptBindings();
};

#endif
