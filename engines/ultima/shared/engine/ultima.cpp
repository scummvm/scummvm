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
#include "gui/saveload.h"

namespace Ultima {
namespace Shared {

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
	Common::String folder;
	int reqMajorVersion, reqMinorVersion;

	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugGraphics, "Graphics", "Graphics debug level");

	// Check if the game uses data from te ultima.dat archive
	if (!isDataRequired(folder, reqMajorVersion, reqMinorVersion))
		return true;

	// Try and open the data archive
	Common::String errorMsg;
	_dataArchive = UltimaDataArchive::load(folder, reqMajorVersion, reqMinorVersion, errorMsg);
	if (_dataArchive) {
		SearchMan.add("data", _dataArchive);
		return true;
	} else {
		GUIError(errorMsg);
		return false;
	}
}

void UltimaEngine::GUIError(const Common::String &msg) {
	GUIErrorMessage(msg);
}

bool UltimaEngine::autoSaveCheck(int lastSaveTime) {
	if (shouldPerformAutoSave(lastSaveTime) && canSaveGameStateCurrently(true)) {
		saveGameState(0, _("Autosave"), true);
		return true;
	}

	return false;
}

bool UltimaEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

uint32 UltimaEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::FSNode UltimaEngine::getGameDirectory() const {
	return Common::FSNode(ConfMan.get("path"));
}

bool UltimaEngine::loadGame() {
	if (!canLoadGameStateCurrently())
		return false;

	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Load game:"), _("Load"), false);
	int slotNum = dialog->runModalWithCurrentTarget();
	delete dialog;

	if (slotNum != -1)
		return loadGameState(slotNum).getCode() == Common::kNoError;

	return false;
}

bool UltimaEngine::saveGame() {
	if (!canSaveGameStateCurrently())
		return false;

	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
	int slotNum = dialog->runModalWithCurrentTarget();
	Common::String saveName = dialog->getResultString();
	delete dialog;

	if (slotNum != -1)
		return saveGameState(slotNum, saveName).getCode() == Common::kNoError;

	return false;
}

UltimaMetaEngine *UltimaEngine::getMetaEngine() const {
	return g_metaEngine;
}

} // End of namespace Shared
} // End of namespace Ultima
