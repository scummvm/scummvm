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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/shared/core/ultima.h"
#include "ultima/shared/core/events.h"
#include "audio/mixer.h"
#include "common/file.h"
#include "common/translation.h"
#include "common/unzip.h"

namespace Ultima {
namespace Shared {

#define DATA_FILENAME "ultima.dat"

UltimaEngine * g_ultima;

UltimaEngine::UltimaEngine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _randomSource("Ultima"),
		_dataArchive(nullptr), _debugger(nullptr), _events(nullptr) {
	g_ultima = this;
}

UltimaEngine::~UltimaEngine() {
	g_ultima = nullptr;
}

bool UltimaEngine::initialize() {
	Common::File f;
	Common::String folder;
	int reqMajorVersion, reqMinorVersion;

	if (!isDataRequired(folder, reqMajorVersion, reqMinorVersion))
		return true;

	if (!Common::File::exists(DATA_FILENAME) ||
		(_dataArchive = Common::makeZipArchive(DATA_FILENAME)) == 0 ||
		!f.open(Common::String::format("%s/version.txt", folder.c_str()), *_dataArchive)) {
		delete _dataArchive;
		GUIError(Common::String::format(_("Could not locate engine data %s"), DATA_FILENAME));
		return false;
	}

	// Validate the version
	char buffer[5];
	f.read(buffer, 4);
	buffer[4] = '\0';

	int major = 0, minor = 0;
	if (buffer[1] == '.') {
		major = buffer[0] - '0';
		minor = atoi(&buffer[2]);
	}

	if (major != reqMajorVersion || minor != reqMinorVersion) {
		delete _dataArchive;
		GUIError(Common::String::format(_("Out of date engine data. Expected %d.%d, but got version %d.%d"),
			reqMajorVersion, reqMinorVersion, major, minor));
		return false;
	}

	SearchMan.add("data", _dataArchive);
	return true;
}

void UltimaEngine::GUIError(const Common::String &msg) {
	GUIErrorMessage(msg);
}

bool UltimaEngine::autoSaveCheck(int lastSaveTime) {
	if (shouldPerformAutoSave(lastSaveTime) && canSaveGameStateCurrently()) {
		saveGameState(0, _("Autosave"));
		return true;
	}

	return false;
}

} // End of namespace Shared
} // End of namespace Ultima
