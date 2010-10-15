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
#include "common/debug-channels.h"
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

void logToStdout(const char *Message) {
	debugN(0, "%s", Message);
}

Sword25Engine::Sword25Engine(OSystem *syst, const ADGameDescription *gameDesc):
	Engine(syst),
	_gameDescription(gameDesc) {

	DebugMan.addDebugChannel(kDebugScript, "Script", "Script debug level");
	DebugMan.addDebugChannel(kDebugScript, "Scripts", "Script debug level");
	DebugMan.addDebugChannel(kDebugSound, "Sound", "Sound debug level");
}

Sword25Engine::~Sword25Engine() {
}

Common::Error Sword25Engine::run() {
	// Engine initialisation
	Common::Error errorCode = appStart();
	if (errorCode != Common::kNoError) {
		appEnd();
		return errorCode;
	}

	// Run the game
	bool runSuccess = appMain();

	// Engine de-initialisation
	bool deinitSuccess = appEnd();

	return (runSuccess && deinitSuccess) ? Common::kNoError : Common::kUnknownError;
}

Common::Error Sword25Engine::appStart() {
	// All log messages will be sent to StdOut
	BS_Log::RegisterLogListener(logToStdout);

	// Initialise the graphics mode to ARGB8888
	Graphics::PixelFormat format = Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);
	initGraphics(800, 600, true, &format);
	if (format != g_system->getScreenFormat())
		return Common::kUnsupportedColorMode;

	// Kernel initialization
	if (!Kernel::GetInstance()->GetInitSuccess()) {
		BS_LOG_ERRORLN("Kernel initialization failed.");
		return Common::kUnknownError;
	}

	// Package-Manager starten, damit die Packfiles geladen werden können.
	PackageManager *packageManagerPtr = static_cast<PackageManager *>(Kernel::GetInstance()->NewService("package", PACKAGE_MANAGER));
	if (!packageManagerPtr) {
		BS_LOG_ERRORLN("PackageManager initialization failed.");
		return Common::kUnknownError;
	}

	// Packages laden oder das aktuelle Verzeichnis mounten, wenn das über Kommandozeile angefordert wurde.
	if (getGameFlags() & GF_EXTRACTED) {
		if (!packageManagerPtr->loadDirectoryAsPackage(ConfMan.get("path"), "/"))
			return Common::kUnknownError;
	} else {
		if (!loadPackages())
			return Common::kUnknownError;
	}

	// Einen Pointer auf den Skript-Engine holen.
	ScriptEngine *scriptPtr = Kernel::GetInstance()->GetScript();
	if (!scriptPtr) {
		BS_LOG_ERRORLN("Script intialization failed.");
		return Common::kUnknownError;
	}

	Common::StringArray commandParameters;
	scriptPtr->setCommandLine(commandParameters);

	return Common::kNoError;
}

bool Sword25Engine::appMain() {
	// The main script start. This script loads all the other scripts and starts the actual game.
	ScriptEngine *scriptPtr = Kernel::GetInstance()->GetScript();
	BS_ASSERT(scriptPtr);
	scriptPtr->executeFile(DEFAULT_SCRIPT_FILE);

	return true;
}

bool Sword25Engine::appEnd() {
	// The kernel is shutdown, and un-initialises all subsystems
	Kernel::DeleteInstance();

	// Free the log file if it was used
	BS_Log::_CloseLog();

	return true;
}

bool Sword25Engine::loadPackages() {
	PackageManager *packageManagerPtr = Kernel::GetInstance()->GetPackage();
	BS_ASSERT(packageManagerPtr);

	// Load the main package
	if (!packageManagerPtr->loadPackage("data.b25c", "/")) return false;

	// Get the contents of the main program directory and sort them alphabetically
	Common::FSNode dir(ConfMan.get("path"));
	Common::FSList files;
	if (!dir.isDirectory() || !dir.getChildren(files, Common::FSNode::kListAll)) {
		warning("Game data path does not exist or is not a directory");
		return false;
	}

	Common::sort(files.begin(), files.end());

	// Identify all patch packages
	// The filename of patch packages must have the form patch??.b25c, with the question marks
	// are placeholders for numbers.
	// Since the filenames have been sorted, patches are mounted with low numbers first, through
	// to ones with high numbers. This is important, because newly mount packages overwrite
	// existing files in the virtual file system, if they include files with the same name.
	for (Common::FSList::const_iterator it = files.begin(); it != files.end(); ++it) {
		if (it->getName().matchString("patch???.b25c", true))
			if (!packageManagerPtr->loadPackage(it->getName(), "/"))
				return false;
	}

	// Identify and mount all language packages
	// The filename of the packages have the form lang_*.b25c (eg. lang_de.b25c)
	for (Common::FSList::const_iterator it = files.begin(); it != files.end(); ++it) {
		if (it->getName().matchString("lang_*.b25c", true))
			if (!packageManagerPtr->loadPackage(it->getName(), "/"))
				return false;
	}

	return true;
}

} // End of namespace Sword25
