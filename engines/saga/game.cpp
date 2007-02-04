/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Game detection, general game parameters

#include "saga/saga.h"

#include "common/config-manager.h"
#include "common/advancedDetector.h"

#include "saga/rscfile.h"
#include "saga/interface.h"
#include "saga/scene.h"
#include "saga/sagaresnames.h"


namespace Saga {
struct SAGAGameDescription {
	Common::ADGameDescription desc;

	int gameType;
	int gameId;
	uint32 features;
	const GameDisplayInfo *gameDisplayInfo;
	int startSceneNumber;
	const GameResourceDescription *resourceDescription;
	int fontsCount;
	const GameFontDescription *fontDescriptions;
	const GameSoundInfo *voiceInfo;
	const GameSoundInfo *sfxInfo;
	const GameSoundInfo *musicInfo;
	int patchesCount;
	const GamePatchDescription *patchDescriptions;
};

const bool SagaEngine::isBigEndian() const { return (_gameDescription->features & GF_BIG_ENDIAN_DATA) != 0; }
const bool SagaEngine::isMacResources() const { return (getPlatform() == Common::kPlatformMacintosh); }
const GameResourceDescription *SagaEngine::getResourceDescription() { return _gameDescription->resourceDescription; }
const GameSoundInfo *SagaEngine::getVoiceInfo() const { return _gameDescription->voiceInfo; }
const GameSoundInfo *SagaEngine::getSfxInfo() const { return _gameDescription->sfxInfo; }
const GameSoundInfo *SagaEngine::getMusicInfo() const { return _gameDescription->musicInfo; }

const GameFontDescription *SagaEngine::getFontDescription(int index) {
	assert(index < _gameDescription->fontsCount);
	return &_gameDescription->fontDescriptions[index];
}
int SagaEngine::getFontsCount() const { return _gameDescription->fontsCount; }

int SagaEngine::getGameId() const { return _gameDescription->gameId; }
int SagaEngine::getGameType() const { return _gameDescription->gameType; }
uint32 SagaEngine::getFeatures() const { return _gameDescription->features; }
Common::Language SagaEngine::getLanguage() const { return _gameDescription->desc.language; }
Common::Platform SagaEngine::getPlatform() const { return _gameDescription->desc.platform; }
int SagaEngine::getGameNumber() const { return _gameNumber; }
int SagaEngine::getStartSceneNumber() const { return _gameDescription->startSceneNumber; }

int SagaEngine::getPatchesCount() const { return _gameDescription->patchesCount; }
const GamePatchDescription *SagaEngine::getPatchDescriptions() const { return _gameDescription->patchDescriptions; }
const Common::ADGameFileDescription *SagaEngine::getFilesDescriptions() const { return _gameDescription->desc.filesDescriptions; }

static GameList GAME_detectGames(const FSList &fslist);
}

static const PlainGameDescriptor sagaGames[] = {
	{"saga", "SAGA Engine game"},
	{"ite", "Inherit the Earth: Quest for the Orb"},
	{"ihnm", "I Have No Mouth and I Must Scream"},
	{0, 0}
};

static const Common::ADObsoleteGameID obsoleteGameIDsTable[] = {
	{"ite", "saga", Common::kPlatformUnknown},
	{"ihnm", "saga", Common::kPlatformUnknown},
	{0, 0, Common::kPlatformUnknown}
};

namespace Saga {

#include "sagagame.cpp"

}

static const Common::ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Saga::gameDescriptions,
	// Size of that superset structure
	sizeof(Saga::SAGAGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	sagaGames,
	// Structure for autoupgrading obsolete targets
	obsoleteGameIDsTable,
	// Name of single gameid (optional)
	"saga",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	Common::kADFlagComplexID
};

ADVANCED_DETECTOR_DEFINE_PLUGIN(SAGA, Saga::SagaEngine, Saga::GAME_detectGames, detectionParams);

REGISTER_PLUGIN(SAGA, "SAGA Engine", "Inherit the Earth (C) Wyrmkeep Entertainment");

namespace Saga {

bool SagaEngine::initGame() {
	int i = Common::AdvancedDetector::detectBestMatchingGame(detectionParams);
	if (i < 0)
		return false;

	_gameDescription = &gameDescriptions[i];

	_gameDisplayInfo = *_gameDescription->gameDisplayInfo;
	_displayClip.right = _gameDisplayInfo.logicalWidth;
	_displayClip.bottom = _gameDisplayInfo.logicalHeight;

	return _resource->createContexts();
}

GameList GAME_detectGames(const FSList &fslist) {
	return Common::AdvancedDetector::detectAllGames(fslist, detectionParams);
}

} // End of namespace Saga
