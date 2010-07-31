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

#include "common/config-manager.h"
#include "engines/util.h"

#include "sword25/sword25.h"
#include "kernel/kernel.h"

namespace Sword25 {

#define BS_LOG_PREFIX "MAIN"

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
	// Alle Lognachrichten werden auch auf die Standardausgabe ausgegeben.
	BS_Log::RegisterLogListener(LogToStdout);

	// Kernel initialisieren.
	if (!BS_Kernel::GetInstance()->GetInitSuccess())
	{
		BS_LOG_ERRORLN("Kernel initialization failed.");
		return false;
	}
/*
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
*/

	return true;
}

bool Sword25Engine::AppMain() {
	return false;
}

bool Sword25Engine::AppEnd() {

	// Free the log file if it was used
	BS_Log::_CloseLog();

	return false;
}

} // End of namespace Sword25
