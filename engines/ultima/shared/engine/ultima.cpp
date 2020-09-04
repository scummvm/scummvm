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

#include "ultima/shared/engine/ultima.h"
#include "ultima/shared/engine/data_archive.h"
#include "ultima/shared/engine/events.h"
#include "audio/mixer.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/file.h"
#include "common/translation.h"

namespace Ultima {
namespace Shared {

UltimaEngine * g_ultima;

UltimaEngine::UltimaEngine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _randomSource("Ultima"),
		_dataArchive(nullptr), _events(nullptr) {
	g_ultima = this;
}

UltimaEngine::~UltimaEngine() {
	g_ultima = nullptr;
}

bool UltimaEngine::initialize() {
	Common::String folder;
	int reqMajorVersion, reqMinorVersion;

	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugGraphics, "Graphics", "Graphics debug level");

	// Call syncSoundSettings to get default volumes set
	syncSoundSettings();

	// Check if the game uses data from te ultima.dat archive
	if (!isDataRequired(folder, reqMajorVersion, reqMinorVersion))
		return true;

	// Try and set up the data archive
	Common::U32String errorMsg;
	if (!UltimaDataArchive::load(folder, reqMajorVersion, reqMinorVersion, errorMsg)) {
		GUIError(errorMsg);
		return false;
	}

	return true;
}

void UltimaEngine::GUIError(const Common::U32String &msg) {
	GUIErrorMessage(msg);
}

bool UltimaEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

uint32 UltimaEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Language UltimaEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

GameId UltimaEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::FSNode UltimaEngine::getGameDirectory() const {
	return Common::FSNode(ConfMan.get("path"));
}

} // End of namespace Shared
} // End of namespace Ultima
