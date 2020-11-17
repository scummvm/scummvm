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

// Game detection, general game parameters

#include "saga/saga.h"

#include "base/plugins.h"

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "common/translation.h"
#include "graphics/thumbnail.h"

#include "saga/animation.h"
#include "saga/displayinfo.h"
#include "saga/events.h"
#include "saga/resource.h"
#include "saga/interface.h"
#include "saga/scene.h"
#include "saga/detection.h"

namespace Saga {

bool SagaEngine::isBigEndian() const { return isMacResources() && getGameId() == GID_ITE; }
bool SagaEngine::isMacResources() const { return (getPlatform() == Common::kPlatformMacintosh); }
const GameResourceDescription *SagaEngine::getResourceDescription() const { return _gameDescription->resourceDescription; }

const GameFontDescription *SagaEngine::getFontDescription(int index) const {
	assert(index < _gameDescription->fontsCount);
	return &_gameDescription->fontDescriptions[index];
}
int SagaEngine::getFontsCount() const { return _gameDescription->fontsCount; }

int SagaEngine::getGameId() const { return _gameDescription->gameId; }

uint32 SagaEngine::getFeatures() const {
	uint32 result = _gameDescription->features;

	return result;
}

Common::Language SagaEngine::getLanguage() const { return _gameDescription->desc.language; }
Common::Platform SagaEngine::getPlatform() const { return _gameDescription->desc.platform; }
int SagaEngine::getGameNumber() const { return _gameNumber; }
int SagaEngine::getStartSceneNumber() const { return _gameDescription->startSceneNumber; }

const GamePatchDescription *SagaEngine::getPatchDescriptions() const { return _gameDescription->patchDescriptions; }
const ADGameFileDescription *SagaEngine::getFilesDescriptions() const { return _gameDescription->desc.filesDescriptions; }

} // End of namespace Saga

class SagaMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "saga";
	}

    bool hasFeature(MetaEngineFeature f) const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool SagaMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSimpleSavesNames);
}

bool Saga::SagaEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error SagaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Saga::SAGAGameDescription *gd = (const Saga::SAGAGameDescription *)desc;

	switch (gd->gameId) {
	case Saga::GID_IHNM:
#ifndef ENABLE_IHNM
		return Common::Error(Common::kUnsupportedGameidError, _s("I Have No Mouth support not compiled in"));
#endif
		break;
	case Saga::GID_DINO:
	case Saga::GID_FTA2:
#ifndef ENABLE_SAGA2
		return Common::Error(Common::kUnsupportedGameidError, _s("SAGA2 support not compiled in"));
#endif
		break;
	default:
		break;
	}

	*engine = new Saga::SagaEngine(syst, gd);
	return Common::kNoError;
}

SaveStateList SagaMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	char saveDesc[SAVE_TITLE_SIZE];
	Common::String pattern = target;
	pattern += ".s##";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	int slotNum = 0;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 2);

		if (slotNum >= 0 && slotNum < MAX_SAVES) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				for (int i = 0; i < 3; i++)
					in->readUint32BE();
				in->read(saveDesc, SAVE_TITLE_SIZE);
				saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int SagaMetaEngine::getMaximumSaveSlot() const { return MAX_SAVES - 1; }

void SagaMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = target;
	filename += Common::String::format(".s%02d", slot);

	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor SagaMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	static char fileName[MAX_FILE_NAME];
	sprintf(fileName, "%s.s%02d", target, slot);
	char title[TITLESIZE];

	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fileName);

	if (in) {
		uint32 type = in->readUint32BE();
		in->readUint32LE();		// size
		uint32 version = in->readUint32LE();
		char name[SAVE_TITLE_SIZE];
		in->read(name, sizeof(name));

		SaveStateDescriptor desc(slot, name);

		// Some older saves were not written in an endian safe fashion.
		// We try to detect this here by checking for extremely high version values.
		// If found, we retry with the data swapped.
		if (version > 0xFFFFFF) {
			warning("This savegame is not endian safe, retrying with the data swapped");
			version = SWAP_BYTES_32(version);
		}

		debug(2, "Save version: 0x%X", version);

		if (version < 4)
			warning("This savegame is not endian-safe. There may be problems");

		if (type != MKTAG('S','A','G','A')) {
			error("SagaEngine::load wrong save game format");
		}

		if (version > 4) {
			in->read(title, TITLESIZE);
			debug(0, "Save is for: %s", title);
		}

		if (version >= 6) {
			Graphics::Surface *thumbnail;
			if (!Graphics::loadThumbnail(*in, thumbnail)) {
				delete in;
				return SaveStateDescriptor();
			}
			desc.setThumbnail(thumbnail);

			uint32 saveDate = in->readUint32BE();
			uint16 saveTime = in->readUint16BE();

			int day = (saveDate >> 24) & 0xFF;
			int month = (saveDate >> 16) & 0xFF;
			int year = saveDate & 0xFFFF;

			desc.setSaveDate(year, month, day);

			int hour = (saveTime >> 8) & 0xFF;
			int minutes = saveTime & 0xFF;

			desc.setSaveTime(hour, minutes);

			if (version >= 8) {
				uint32 playTime = in->readUint32BE();
				desc.setPlayTime(playTime * 1000);
			}
		}

		delete in;

		return desc;
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(SAGA)
	REGISTER_PLUGIN_DYNAMIC(SAGA, PLUGIN_TYPE_ENGINE, SagaMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SAGA, PLUGIN_TYPE_ENGINE, SagaMetaEngine);
#endif

namespace Saga {

bool SagaEngine::initGame() {
	_displayClip.right = getDisplayInfo().width;
	_displayClip.bottom = getDisplayInfo().height;

	return _resource->createContexts();
}

const GameDisplayInfo &SagaEngine::getDisplayInfo() {
	switch (_gameDescription->gameId) {
		case GID_ITE:
			return ITE_DisplayInfo;
#ifdef ENABLE_IHNM
		case GID_IHNM:
			return IHNM_DisplayInfo;
#endif
#ifdef ENABLE_SAGA2
		case GID_DINO:
			return FTA2_DisplayInfo;	// TODO
		case GID_FTA2:
			return FTA2_DisplayInfo;
#endif
		default:
			error("getDisplayInfo: Unknown game ID");
			return ITE_DisplayInfo;		// for compilers that don't support NORETURN
	}
}

Common::Error SagaEngine::loadGameState(int slot) {
	// Init the current chapter to 8 (character selection) for IHNM
	if (getGameId() == GID_IHNM)
		_scene->changeScene(-2, 0, kTransitionFade, 8);

	// First scene sets up palette
	_scene->changeScene(getStartSceneNumber(), 0, kTransitionNoFade);
	_events->handleEvents(0); // Process immediate events

	if (getGameId() == GID_ITE)
		_interface->setMode(kPanelMain);
	else
		_interface->setMode(kPanelChapterSelection);

	load(calcSaveFileName((uint)slot));
	syncSoundSettings();

	return Common::kNoError;	// TODO: return success/failure
}

Common::Error SagaEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	save(calcSaveFileName((uint)slot), desc.c_str());
	return Common::kNoError;	// TODO: return success/failure
}

bool SagaEngine::canLoadGameStateCurrently() {
	return !_scene->isInIntro() &&
		(_interface->getMode() == kPanelMain || _interface->getMode() == kPanelChapterSelection);
}

bool SagaEngine::canSaveGameStateCurrently() {
	return !_scene->isInIntro() &&
		(_interface->getMode() == kPanelMain || _interface->getMode() == kPanelChapterSelection);
}

} // End of namespace Saga
