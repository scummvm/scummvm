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

#include "common/config-manager.h"
#include "engines/util.h"

#include "sword25/sword25.h"
#include "sword25/kernel/filesystemutil.h"
#include "sword25/kernel/kernel.h"
#include "sword25/package/packagemanager.h"
#include "sword25/script/script.h"

namespace Sword25 {

#define BS_LOG_PREFIX "MAIN"

const char * const PACKAGE_MANAGER = "physfs";
const char * const DEFAULT_SCRIPT_FILE = "/system/boot.lua";
const char * const MOUNT_DIR_PARAMETER = "-mount-dir";


void LogToStdout(const char *Message) {
	warning(Message);
}

Sword25Engine::Sword25Engine(OSystem *syst, const Sword25GameDescription *gameDesc):
		Engine(syst),
		_gameDescription(gameDesc) {
}

Sword25Engine::~Sword25Engine() {
}

Common::Error Sword25Engine::run() {
	// Engine initialisation
	Common::StringArray commandParameters;
	if (!AppStart(commandParameters)) {
		AppEnd();
		error("A fatal error occured during engine startup");
	}

	// Run the game
	bool RunSuccess = AppMain();

	// Engine de-initialisation
	bool DeinitSuccess = AppEnd();

	return (RunSuccess && DeinitSuccess) ? Common::kNoError : Common::kUnknownError;
}

bool Sword25Engine::AppStart(const Common::StringArray &CommandParameters) {
	// All log messages will be sent to StdOut
	BS_Log::RegisterLogListener(LogToStdout);

	// Kernel initialisation
	if (!BS_Kernel::GetInstance()->GetInitSuccess()) {
		BS_LOG_ERRORLN("Kernel initialization failed.");
		return false;
	}

	// Package-Manager starten, damit die Packfiles geladen werden können.
	BS_PackageManager *PackageManagerPtr = static_cast<BS_PackageManager *>(BS_Kernel::GetInstance()->NewService("package", PACKAGE_MANAGER));
	if (!PackageManagerPtr) {
		BS_LOG_ERRORLN("Packagemanager initialization failed.");
		return false;
	}

	// Packages laden oder das aktuelle Verzeichnis mounten, wenn das über Kommandozeile angefordert wurde.
	if (find(CommandParameters.begin(), CommandParameters.end(), MOUNT_DIR_PARAMETER) != CommandParameters.end()) {
		if (!PackageManagerPtr->LoadDirectoryAsPackage(".", "/")) return false;
	} else {
		if (!LoadPackages()) return false;
	}

	// Einen Pointer auf den Skript-Engine holen.
	BS_ScriptEngine *ScriptPtr = static_cast<BS_ScriptEngine *>(BS_Kernel::GetInstance()->GetService("script"));
	if (!ScriptPtr) {
		BS_LOG_ERRORLN("Skript intialization failed.");
		return false;
	}

	// Die Kommandozeilen-Parameter der Skriptumgebung zugänglich machen.
	ScriptPtr->SetCommandLine(CommandParameters);

	return true;
}

bool Sword25Engine::AppMain() {
	// The main script start. This script loads all the other scripts and starts the actual game.
	BS_ScriptEngine * ScriptPtr = static_cast<BS_ScriptEngine *>(BS_Kernel::GetInstance()->GetService("script"));
	BS_ASSERT(ScriptPtr);
	ScriptPtr->ExecuteFile(DEFAULT_SCRIPT_FILE);

	return true;
}

bool Sword25Engine::AppEnd() {
	// The kernel is shutdown, and un-initialises all subsystems
	BS_Kernel::DeleteInstance();

	// Free the log file if it was used
	BS_Log::_CloseLog();

	return false;
}

bool Sword25Engine::LoadPackages() {
	BS_PackageManager *PackageManagerPtr = reinterpret_cast<BS_PackageManager *>(BS_Kernel::GetInstance()->GetService("package"));
	BS_ASSERT(PackageManagerPtr);

	// Load the main package
	if (!PackageManagerPtr->LoadPackage("data.b25c", "/")) return false;

	// Get the contents of the main program directory and sort them alphabetically
	Common::StringArray Filenames = BS_FileSystemUtil::GetInstance().GetFilesInDirectory(".");
	Common::sort(Filenames.begin(), Filenames.end());

	// Identity all patch packages
	// The filename of patch packages must have the form patch??.b25c, with the question marks
	// are placeholders for numbers.
	// Since the filenames have been sorted, patches are mounted with low numbers first, through
	// to ones with high numbers. This is important, because newly mount packages overwrite
	// existing files in the virtual file system, if they include files with the same name.
	for (Common::StringArray::const_iterator it = Filenames.begin(); it != Filenames.end(); ++it) {
		const Common::String &CurFilename = *it;

		// Is the current file a patch package?
		static const Common::String PatchPattern = "patch???.b25c";
		if (CurFilename.size() == PatchPattern.size()) {
			// Check the file against the patch pattern character by character
			Common::String::const_iterator PatchPatternIt = PatchPattern.begin();
			Common::String::const_iterator CurFilenameIt = CurFilename.begin();
			for (; PatchPatternIt != PatchPattern.end(); ++PatchPatternIt, ++CurFilenameIt) {
				if (*PatchPatternIt == '?') {
					if (*CurFilenameIt < '0' || *CurFilenameIt > '9') break;
				} else {
					if (*PatchPatternIt != *CurFilenameIt) break;
				}
			}

			if (PatchPatternIt == PatchPattern.end()) {
				// The pattern fits, so the file should be mounted
				if (!PackageManagerPtr->LoadPackage(CurFilename, "/")) return false;
			}
		}
	}

	// Identity and mount all language packages
	// The filename of the packages have the form lang_*.b25c (eg. lang_de.b25c)
	for (Common::StringArray::const_iterator it = Filenames.begin(); it != Filenames.end(); ++it) {
		const Common::String &CurFilename = *it;

		static const Common::String Prefix = "lang_";
		static const Common::String Suffix = ".b25c";

		// Make sure the filename prefix and suffix has characters between them
		if ((CurFilename.size() >= Prefix.size() && Common::String(CurFilename.begin(), CurFilename.begin() + Prefix.size()) == Prefix) &&	// Prefix test
			(CurFilename.size() >= Suffix.size() && Common::String(CurFilename.end() - Suffix.size(), CurFilename.end()) == Suffix) &&		// Suffix test
			(CurFilename.size() > Prefix.size() + Suffix.size())) 
		{
			// Pattern matches - the file should be mounted
			if (!PackageManagerPtr->LoadPackage(CurFilename, "/")) return false;
		}
	}

	return true;
}

} // End of namespace Sword25
