/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "hpl1/hpl1.h"
#include "audio/mixer.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/savefile.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engine/engine.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "hpl1/debug.h"
#include "hpl1/detection.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MainMenu.h"
#include "hpl1/penumbra-overture/SaveHandler.h"

namespace Hpl1 {

Hpl1Engine *g_engine;

Hpl1Engine::Hpl1Engine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst), _gameDescription(gameDesc), _randomSource("Hpl1") {
	g_engine = this;
}

Hpl1Engine::~Hpl1Engine() {
	delete _screen;
}

uint32 Hpl1Engine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String Hpl1Engine::getGameId() const {
	return _gameDescription->gameId;
}

static Common::String getStartupSave(MetaEngine *meta, const char *target) {
	if (ConfMan.hasKey("save_slot")) {
		const int saveSlot = ConfMan.getInt("save_slot");
		const SaveStateDescriptor saveInfo = meta->querySaveMetaInfos(target, saveSlot);
		return saveInfo.getDescription();
	}
	return "";
}

Common::Error Hpl1Engine::run() {
	_gameInit = new cInit(); // TODO: remove allocation
	if (!_gameInit->Init(getStartupSave(getMetaEngine(), _targetName.c_str()).c_str())) {
		delete _gameInit;
		return Common::kUnknownError; // TODO: better errors
	};
	_gameInit->Run();
	_gameInit->Exit();
	delete _gameInit;
	return Common::kNoError;
}

void Hpl1Engine::pauseEngineIntern(bool pause) {
	_mixer->pauseAll(pause);
	g_system->lockMouse(!pause);
	// if a save is deleted from the global main menu,
	// it would still appear on the game's menu
	if (!pause)
		_gameInit->mpMainMenu->UpdateWidgets();
}

static Common::String freeSaveSlot(const Engine *engine, const int maxSaves) {
	for (int i = 0; i < maxSaves; ++i) {
		const Common::String name = engine->getSaveStateName(i);
		if (!g_system->getSavefileManager()->exists(name))
			return name;
	}
	return "";
}

Common::String Hpl1Engine::createSaveFile(const Common::String &internalName) {
	const Common::String freeSlot = freeSaveSlot(this, getMetaEngine()->getMaximumSaveSlot());
	if (freeSlot == "")
		warning("game out of save slots");
	return freeSlot;
}

static Common::String findSaveFile(const SaveStateList &saves, const Common::String &internalName) {
	for (auto &s : saves) {
		if (s.getDescription() == internalName)
			return g_engine->getSaveStateName(s.getSaveSlot());
	}
	logWarning(kDebugSaves | kDebugFilePath, "save file for save %s does not exist", internalName.c_str());
	return "";
}

void Hpl1Engine::removeSaveFile(const Common::String &internalName) {
	Common::String filename = findSaveFile(g_engine->getMetaEngine()->listSaves(_targetName.c_str()), internalName);
	if (filename != "")
		_saveFileMan->removeSavefile(filename);
}

Common::String Hpl1Engine::mapInternalSaveToFile(const Common::String &internalName) {
	return findSaveFile(g_engine->getMetaEngine()->listSaves(_targetName.c_str()), internalName);
}

Common::StringArray Hpl1Engine::listInternalSaves(const Common::String &pattern) {
	Common::StringArray internalSaves;
	SaveStateList saves = g_engine->getMetaEngine()->listSaves(_targetName.c_str());
	for (auto &save : saves) {
		const Common::String saveDesc = save.getDescription();
		if (saveDesc.matchString(pattern))
			internalSaves.push_back(saveDesc);
	}
	return internalSaves;
}

Common::Error Hpl1Engine::loadGameState(int slot) {
	SaveStateDescriptor a = getMetaEngine()->querySaveMetaInfos(_targetName.c_str(), slot);
	_gameInit->mpMainMenu->SetActive(false);
	_gameInit->mpSaveHandler->LoadGameFromFile(a.getDescription());
	return Common::kNoError;
}

Common::Error Hpl1Engine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // namespace Hpl1
