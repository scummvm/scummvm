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

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

#include "kernel/common.h"
#include "kernel/kernel.h"
#include "kernel/filesystemutil.h"
#include "kernel/debug/memorydumper.h"
#include "script/script.h"
#include "package/packagemanager.h"

#define BS_LOG_PREFIX "MAIN"

// -----------------------------------------------------------------------------

namespace
{
	const char * const PACKAGE_MANAGER = "physfs";
	const char * const DEFAULT_SCRIPT_FILE = "/system/boot.lua";
	const char * const MOUNT_DIR_PARAMETER = "-mount-dir";

	// -------------------------------------------------------------------------

	void LogToStdout(const char * Message)
	{
		printf("%s", Message);
	}

	// -------------------------------------------------------------------------

	bool LoadPackages()
	{
		BS_PackageManager * PackageManagerPtr = reinterpret_cast<BS_PackageManager *>(BS_Kernel::GetInstance()->GetService("package"));
		BS_ASSERT(PackageManagerPtr);

		// Das Hauptpaket laden.
		if (!PackageManagerPtr->LoadPackage("data.b25c", "/")) return false;

		// Den Inhalt des Programmverzeichnisses bestimmen und alphabetisch sortieren.
		vector<string> Filenames = BS_FileSystemUtil::GetInstance().GetFilesInDirectory(".");
		sort(Filenames.begin(), Filenames.end());

		// Alle Patch-Pakete identifizieren und mounten.
		// Die Dateinamen der Patch-Pakete besitzen die Form: patch???.b25c, wobei die Fragezeichen Platzhalter für Ziffern sind.
		// Da die Dateinamen sortiert wurden, werden Patches mit niedrigen Zahlen vor Patches mit hohen Zahlen gemounted.
		// Dies ist wichtig, da neu gemountete Pakete vorhandene Dateien im virtuellen Dateisystem überschreiben, wenn sie
		// gleichnamige Dateien beinhalten.
		for (vector<string>::const_iterator it = Filenames.begin(); it != Filenames.end(); ++it)
		{
			const string & CurFilename = *it;

			// Ist aktuelle Datei ein Patch-Paket?
			static const string PatchPattern = "patch???.b25c";
			if (CurFilename.size() == PatchPattern.size())
			{
				// Pattern Zeichen für Zeichen mit dem Dateinamen vergleichen.
				string::const_iterator PatchPatternIt = PatchPattern.begin();
				string::const_iterator CurFilenameIt = CurFilename.begin();
				for (; PatchPatternIt != PatchPattern.end(); ++PatchPatternIt, ++CurFilenameIt)
				{
					if (*PatchPatternIt == '?')
					{
						if (*CurFilenameIt < '0' || *CurFilenameIt > '9') break;
					}
					else
					{
						if (*PatchPatternIt != *CurFilenameIt) break;
					}
				}

				if (PatchPatternIt == PatchPattern.end())
				{
					// Pattern stimmt, Datei muss gemountet werden.
					if (!PackageManagerPtr->LoadPackage(CurFilename, "/")) return false;
				}
			}
		}

		// Alle Sprach-Pakete identifizieren und mounten.
		// Die Dateinamen der Sprach-Pakete besitzen die Form: lang_*.b25c (z.B. lang_de.b25c).
		for (vector<string>::const_iterator it = Filenames.begin(); it != Filenames.end(); ++it)
		{
			const string & CurFilename = *it;

			static const string Prefix = "lang_";
			static const string Suffix = ".b25c";

			if ((CurFilename.size() >= Prefix.size() && string(CurFilename.begin(), CurFilename.begin() + Prefix.size()) == Prefix) &&	// Präfix testen.
				(CurFilename.size() >= Suffix.size() && string(CurFilename.end() - Suffix.size(), CurFilename.end()) == Suffix) &&		// Suffix testen.
				(CurFilename.size() > Prefix.size() + Suffix.size())) // Sicherstellen, dass der Dateiname weitere Buchstaben zwischen Präfix und Suffix besitzt.
			{
				// Pattern stimmt, Datei muss gemountet werden.
				if (!PackageManagerPtr->LoadPackage(CurFilename, "/")) return false;
			}
		}	

		return true;
	}
}

// -----------------------------------------------------------------------------

bool AppStart(const vector<string> & CommandLineParameters)
{
	// Alle Lognachrichten werden auch auf die Standardausgabe ausgegeben.
	BS_Log::RegisterLogListener(LogToStdout);

	// Kernel initialisieren.
	if (!BS_Kernel::GetInstance()->GetInitSuccess())
	{
		BS_LOG_ERRORLN("Kernel initialization failed.");
		return false;
	}

	// Package-Manager starten, damit die Packfiles geladen werden können.
	BS_PackageManager * PackageManagerPtr = static_cast<BS_PackageManager *>(BS_Kernel::GetInstance()->NewService("package", PACKAGE_MANAGER));
	if (!PackageManagerPtr)
	{
		BS_LOG_ERRORLN("Packagemanager initialization failed.");
		return false;
	}

	// Packages laden oder das aktuelle Verzeichnis mounten, wenn das über Kommandozeile angefordert wurde.
	if (find(CommandLineParameters.begin(), CommandLineParameters.end(), MOUNT_DIR_PARAMETER) != CommandLineParameters.end())
	{
		if (!PackageManagerPtr->LoadDirectoryAsPackage(".", "/")) return false;
	}
	else
	{
		if (!LoadPackages()) return false;
	}

	// Einen Pointer auf den Skript-Engine holen.
	BS_ScriptEngine * ScriptPtr = static_cast<BS_ScriptEngine *>(BS_Kernel::GetInstance()->GetService("script"));
	if (!ScriptPtr)
	{
		BS_LOG_ERRORLN("Skript intialization failed.");
		return false;
	}

	// Die Kommandozeilen-Parameter der Skriptumgebung zugänglich machen.
	ScriptPtr->SetCommandLine(CommandLineParameters);

	return true;
}

// -----------------------------------------------------------------------------

bool AppEnd()
{
	// Den Kernel herunterfahren und alle Subsysteme deinitialisieren.
	BS_Kernel::DeleteInstance();

	return true;
}

// -----------------------------------------------------------------------------

bool AppMain()
{
	// Das Hauptskript starten. Dieses Skript lädt alle anderen Skripte und startet das eigentliche Spiel.
	BS_ScriptEngine * ScriptPtr = static_cast<BS_ScriptEngine *>(BS_Kernel::GetInstance()->GetService("script"));
	BS_ASSERT(ScriptPtr);
	ScriptPtr->ExecuteFile(DEFAULT_SCRIPT_FILE);

	return true;
}
