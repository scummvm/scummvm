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

const char *const PACKAGE_MANAGER = "archiveFS";
const char *const DEFAULT_SCRIPT_FILE = "/system/boot.lua";

void LogToStdout(const char *Message) {
	debugN(0, Message);
}

Sword25Engine::Sword25Engine(OSystem *syst, const ADGameDescription *gameDesc):
	Engine(syst),
	_gameDescription(gameDesc) {
}

Sword25Engine::~Sword25Engine() {
}

Common::Error Sword25Engine::run() {
	// Engine initialisation
	Common::StringArray commandParameters;
	Common::Error errorCode = AppStart(commandParameters);
	if (errorCode != Common::kNoError) {
		AppEnd();
		return errorCode;
	}

	// Run the game
	bool RunSuccess = AppMain();

	// Engine de-initialisation
	bool DeinitSuccess = AppEnd();

	return (RunSuccess && DeinitSuccess) ? Common::kNoError : Common::kUnknownError;
}

Common::Error Sword25Engine::AppStart(const Common::StringArray &CommandParameters) {
	// All log messages will be sent to StdOut
	BS_Log::RegisterLogListener(LogToStdout);

	// Initialise the graphics mode to RGBA8888
	Graphics::PixelFormat format = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	initGraphics(800, 600, true, &format);
	if (format != g_system->getScreenFormat())
		return Common::kUnsupportedColorMode;

	// Kernel initialization
	if (!Kernel::GetInstance()->GetInitSuccess()) {
		BS_LOG_ERRORLN("Kernel initialization failed.");
		return Common::kUnknownError;
	}

	// Package-Manager starten, damit die Packfiles geladen werden können.
	PackageManager *PackageManagerPtr = static_cast<PackageManager *>(Kernel::GetInstance()->NewService("package", PACKAGE_MANAGER));
	if (!PackageManagerPtr) {
		BS_LOG_ERRORLN("Packagemanager initialization failed.");
		return Common::kUnknownError;
	}

	// Packages laden oder das aktuelle Verzeichnis mounten, wenn das über Kommandozeile angefordert wurde.
	if (getGameFlags() & GF_EXTRACTED) {
		if (!PackageManagerPtr->LoadDirectoryAsPackage(ConfMan.get("path"), "/"))
			return Common::kUnknownError;
	} else {
		if (!LoadPackages())
			return Common::kUnknownError;
	}

	// Einen Pointer auf den Skript-Engine holen.
	ScriptEngine *ScriptPtr = static_cast<ScriptEngine *>(Kernel::GetInstance()->GetService("script"));
	if (!ScriptPtr) {
		BS_LOG_ERRORLN("Script intialization failed.");
		return Common::kUnknownError;
	}

	// Die Kommandozeilen-Parameter der Skriptumgebung zugänglich machen.
	ScriptPtr->SetCommandLine(CommandParameters);

	return Common::kNoError;
}

bool Sword25Engine::AppMain() {
	// The main script start. This script loads all the other scripts and starts the actual game.
	ScriptEngine *ScriptPtr = static_cast<ScriptEngine *>(Kernel::GetInstance()->GetService("script"));
	BS_ASSERT(ScriptPtr);
	ScriptPtr->ExecuteFile(DEFAULT_SCRIPT_FILE);

	return true;
}

bool Sword25Engine::AppEnd() {
	// The kernel is shutdown, and un-initialises all subsystems
	Kernel::DeleteInstance();

	// Free the log file if it was used
	BS_Log::_CloseLog();

	return true;
}

bool Sword25Engine::LoadPackages() {
	PackageManager *PackageManagerPtr = reinterpret_cast<PackageManager *>(Kernel::GetInstance()->GetService("package"));
	BS_ASSERT(PackageManagerPtr);

	// Load the main package
	if (!PackageManagerPtr->LoadPackage("data.b25c", "/")) return false;

	// Get the contents of the main program directory and sort them alphabetically
	Common::StringArray Filenames = FileSystemUtil::GetInstance().GetFilesInDirectory(".");
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
		if ((CurFilename.size() >= Prefix.size() && Common::String(CurFilename.begin(), CurFilename.begin() + Prefix.size()) == Prefix) &&  // Prefix test
		        (CurFilename.size() >= Suffix.size() && Common::String(CurFilename.end() - Suffix.size(), CurFilename.end()) == Suffix) &&      // Suffix test
		        (CurFilename.size() > Prefix.size() + Suffix.size())) {
			// Pattern matches - the file should be mounted
			if (!PackageManagerPtr->LoadPackage(CurFilename, "/")) return false;
		}
	}

	return true;
}

} // End of namespace Sword25
