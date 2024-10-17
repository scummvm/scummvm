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

// Game detection, general game parameters

#include "saga/saga.h"

#include "base/plugins.h"

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "common/translation.h"
#include "graphics/thumbnail.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "saga/animation.h"
#include "saga/displayinfo.h"
#include "saga/events.h"
#include "saga/resource.h"
#include "saga/interface.h"
#include "saga/scene.h"
#include "saga/detection.h"

namespace Saga {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_COPY_PROTECTION,
		{
			_s("Enable copy protection"),
			_s("Enable any copy protection that would otherwise be bypassed by default."),
			"copy_protection",
			false,
			0,
			0
		},
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

bool SagaEngine::isBigEndian() const { return (isMacResources() || (getPlatform() == Common::kPlatformAmiga)) && getGameId() == GID_ITE; }
bool SagaEngine::isMacResources() const { return (getPlatform() == Common::kPlatformMacintosh); }

GameResourceList SagaEngine::getResourceList() const { return _gameDescription->resourceList; }
GameFontList SagaEngine::getFontList() const { return _gameDescription->fontList; }
GamePatchList SagaEngine::getPatchList() const { return _gameDescription->patchList; }

int SagaEngine::getGameId() const { return _gameDescription->gameId; }

uint32 SagaEngine::getFeatures() const {
	uint32 result = _gameDescription->features;

	return result;
}

Common::Language SagaEngine::getLanguage() const { return _gameDescription->desc.language; }
Common::Platform SagaEngine::getPlatform() const { return _gameDescription->desc.platform; }
int SagaEngine::getGameNumber() const { return _gameNumber; }
int SagaEngine::getStartSceneNumber() const { return _gameDescription->startSceneNumber; }

const ADGameFileDescription *SagaEngine::getFilesDescriptions() const {
    return getFeatures() & GF_INSTALLER ? _gameDescription->filesInArchive : _gameDescription->desc.filesDescriptions;
}

const ADGameFileDescription *SagaEngine::getArchivesDescriptions() const {
  return getFeatures() & GF_INSTALLER ? _gameDescription->desc.filesDescriptions : nullptr;
}

} // End of namespace Saga

class SagaMetaEngine : public AdvancedMetaEngine<Saga::SAGAGameDescription> {
public:
	const char *getName() const override {
		return "saga";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Saga::optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const Saga::SAGAGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool SagaMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime);
}

bool Saga::SagaEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error SagaMetaEngine::createInstance(OSystem *syst, Engine **engine, const Saga::SAGAGameDescription *gd) const {
	switch (gd->gameId) {
	case Saga::GID_IHNM:
#ifndef ENABLE_IHNM
		return Common::Error(Common::kUnsupportedGameidError, _s("I Have No Mouth support not compiled in"));
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
				saveList.push_back(SaveStateDescriptor(this, slotNum, saveDesc));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int SagaMetaEngine::getMaximumSaveSlot() const { return MAX_SAVES - 1; }

bool SagaMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = target;
	filename += Common::String::format(".s%02d", slot);

	return g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor SagaMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	static char fileName[MAX_FILE_NAME];
	Common::sprintf_s(fileName, "%s.s%02d", target, slot);
	char title[TITLESIZE];

	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fileName);

	if (in) {
		uint32 type = in->readUint32BE();
		in->readUint32LE();		// size
		uint32 version = in->readUint32LE();
		char name[SAVE_TITLE_SIZE];
		in->read(name, sizeof(name));

		SaveStateDescriptor desc(this, slot, name);

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

Common::KeymapArray SagaMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Saga;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, engineKeyMapId, _("Default game keymap"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, gameKeyMapId, _("Game keymapping"));
	Keymap *optionKeyMap = new Keymap(Keymap::kKeymapTypeGame, optionKeyMapId, _("Option panel keymapping"));
	Keymap *saveKeyMap = new Keymap(Keymap::kKeymapTypeGame, saveKeyMapId, _("Save panel keymapping"));
	Keymap *loadKeyMap = new Keymap(Keymap::kKeymapTypeGame, loadKeyMapId, _("Load panel keymapping"));
	Keymap *quitKeyMap = new Keymap(Keymap::kKeymapTypeGame, quitKeyMapId, _("Quit panel keymapping"));
	Keymap *converseKeyMap = new Keymap(Keymap::kKeymapTypeGame, converseKeyMapId, _("Converse panel keymapping"));

	Action *act;

	{
		act = new Action(kStandardActionLeftClick, _("Left click"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		engineKeyMap->addAction(act);

		act = new Action(kStandardActionMiddleClick, _("Middle click"));
		act->addDefaultInputMapping("MOUSE_MIDDLE");
		act->setMiddleClickEvent();
		engineKeyMap->addAction(act);

		act = new Action(kStandardActionRightClick, _("Right click"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		engineKeyMap->addAction(act);

		// I18N: the boss key is a feature,
		// that allows players to quickly switch to a fake screen that looks like a business application,
		// typically to avoid detection if someone, like a boss, walks by while they are playing.
		act = new Action("BOSSKEY", _("Boss key"));
		act->setCustomEngineActionEvent(kActionBossKey);
		act->addDefaultInputMapping("F9");
		engineKeyMap->addAction(act);

		act = new Action("SHOWOPTION", _("Show options"));
		act->setCustomEngineActionEvent(kActionOptions);
		act->addDefaultInputMapping("F5");
		act->addDefaultInputMapping("C+o");
		engineKeyMap->addAction(act);
	}

	{
		act = new Action("EXITCONVO", _("Exit conversation"));
		act->setCustomEngineActionEvent(kActionConverseExit);
		act->addDefaultInputMapping("x");
		converseKeyMap->addAction(act);

		act = new Action("UPCONVO", _("Conversation position - Up"));
		act->setCustomEngineActionEvent(kActionConversePosUp);
		act->addDefaultInputMapping("u");
		converseKeyMap->addAction(act);

		act = new Action("DOWNCONVO", _("Conversation position - Down"));
		act->setCustomEngineActionEvent(kActionConversePosDown);
		act->addDefaultInputMapping("d");
		converseKeyMap->addAction(act);
	}

	{
		act = new Action("ESCAPE", _("Escape"));
		act->setCustomEngineActionEvent(kActionEscape);
		act->addDefaultInputMapping("ESCAPE");
		gameKeyMap->addAction(act);

		act = new Action("PAUSE", _("Pause game"));
		act->setCustomEngineActionEvent(kActionPause);
		act->addDefaultInputMapping("z");
		act->addDefaultInputMapping("PAUSE");
		gameKeyMap->addAction(act);

		act = new Action("ABRTSPEECH", _("Abort speech"));
		act->setCustomEngineActionEvent(kActionAbortSpeech);
		act->addDefaultInputMapping("SPACE");
		gameKeyMap->addAction(act);

		act = new Action("SHOWDILOG", _("Show dialogue"));
		act->setCustomEngineActionEvent(kActionShowDialogue);
		act->addDefaultInputMapping("r");
		gameKeyMap->addAction(act);

		act = new Action("WALK", _("Walk to"));
		act->setCustomEngineActionEvent(kActionWalkTo);
		act->addDefaultInputMapping("w");
		gameKeyMap->addAction(act);

		act = new Action("LOOK", _("Look at"));
		act->setCustomEngineActionEvent(kActionLookAt);
		act->addDefaultInputMapping("l");
		gameKeyMap->addAction(act);

		act = new Action("PICKUP", _("Pickup"));
		act->setCustomEngineActionEvent(kActionPickUp);
		act->addDefaultInputMapping("p");
		gameKeyMap->addAction(act);

		act = new Action("TALK", _("Talk to"));
		act->setCustomEngineActionEvent(kActionTalkTo);
		act->addDefaultInputMapping("t");
		gameKeyMap->addAction(act);

		act = new Action("OPEN", _("Open"));
		act->setCustomEngineActionEvent(kActionOpen);
		act->addDefaultInputMapping("o");
		gameKeyMap->addAction(act);

		act = new Action("CLOSE", _("Close"));
		act->setCustomEngineActionEvent(kActionClose);
		act->addDefaultInputMapping("c");
		gameKeyMap->addAction(act);

		act = new Action("USE", _("Use"));
		act->setCustomEngineActionEvent(kActionUse);
		act->addDefaultInputMapping("u");
		gameKeyMap->addAction(act);

		act = new Action("GIVE", _("Give"));
		act->setCustomEngineActionEvent(kActionGive);
		act->addDefaultInputMapping("g");
		gameKeyMap->addAction(act);

		act = new Action("PUSH", _("Push"));
		act->setCustomEngineActionEvent(kActionPush);
		act->addDefaultInputMapping("p");
		gameKeyMap->addAction(act);

		act = new Action("TAKE", _("Take"));
		act->setCustomEngineActionEvent(kActionTake);
		act->addDefaultInputMapping("k");
		gameKeyMap->addAction(act);

		act = new Action("SWALLOW", _("Swallow"));
		act->setCustomEngineActionEvent(kActionSwallow);
		act->addDefaultInputMapping("s");
		gameKeyMap->addAction(act);
	}

	{
		act = new Action("READSPEED", _("Reading speed"));
		act->setCustomEngineActionEvent(kActionOptionReadingSpeed);
		act->addDefaultInputMapping("r");
		optionKeyMap->addAction(act);

		act = new Action("MUSIC", _("Change music"));
		act->setCustomEngineActionEvent(kActionOptionMusic);
		act->addDefaultInputMapping("m");
		optionKeyMap->addAction(act);

		act = new Action("SOUND", _("Change sound"));
		act->setCustomEngineActionEvent(kActionOptionSound);
		act->addDefaultInputMapping("n");
		optionKeyMap->addAction(act);

		act = new Action("VOICES", _("Change voices"));
		act->setCustomEngineActionEvent(kActionOptionVoices);
		act->addDefaultInputMapping("v");
		optionKeyMap->addAction(act);

		act = new Action("CONTGAME", _("Continue game"));
		act->setCustomEngineActionEvent(kActionOptionContinue);
		act->addDefaultInputMapping("c");
		optionKeyMap->addAction(act);

		act = new Action("LOAD", _("Load game"));
		act->setCustomEngineActionEvent(kActionOptionLoad);
		act->addDefaultInputMapping("l");
		optionKeyMap->addAction(act);

		act = new Action("QUITGAME", _("Quit game"));
		act->setCustomEngineActionEvent(kActionOptionQuitGame);
		act->addDefaultInputMapping("q");
		optionKeyMap->addAction(act);

		act = new Action("SAVEGAME", _("Save game"));
		act->setCustomEngineActionEvent(kActionOptionSaveGame);
		act->addDefaultInputMapping("s");
		optionKeyMap->addAction(act);
	}

	{
		act = new Action("QUIT", _("Quit"));
		act->setCustomEngineActionEvent(kActionOptionQuit);
		act->addDefaultInputMapping("q");
		quitKeyMap->addAction(act);

		act = new Action("CNCLQUIT", _("Cancel quit"));
		act->setCustomEngineActionEvent(kActionOptionCancel);
		act->addDefaultInputMapping("c");
		quitKeyMap->addAction(act);

		act = new Action("OKAY", _("Okay"));
		act->setCustomEngineActionEvent(kActionOptionOkay);
		act->addDefaultInputMapping("o");
		loadKeyMap->addAction(act);

		act = new Action("CNCLLOAD", _("Cancel load"));
		act->setCustomEngineActionEvent(kActionOptionCancel);
		act->addDefaultInputMapping("c");
		loadKeyMap->addAction(act);

		act = new Action("SAVE", _("Save"));
		act->setCustomEngineActionEvent(kActionOptionSave);
		act->addDefaultInputMapping("s");
		saveKeyMap->addAction(act);

		act = new Action("CNCLSAVE", _("Cancel save"));
		act->setCustomEngineActionEvent(kActionOptionCancel);
		act->addDefaultInputMapping("c");
		saveKeyMap->addAction(act);
	}

	KeymapArray keymaps(7);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;
	keymaps[2] = optionKeyMap;
	keymaps[3] = saveKeyMap;
	keymaps[4] = loadKeyMap;
	keymaps[5] = quitKeyMap;
	keymaps[6] = converseKeyMap;

	gameKeyMap->setEnabled(false);
	optionKeyMap->setEnabled(false);
	saveKeyMap->setEnabled(false);
	loadKeyMap->setEnabled(false);
	quitKeyMap->setEnabled(false);
	converseKeyMap->setEnabled(false);

	return keymaps;
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
			if (getLanguage() == Common::ZH_TWN)
				return ITE_DisplayInfo_ZH;
			if (isECS()) {
				static GameDisplayInfo ITE_DisplayInfo_ECS;
				if (!ITE_DisplayInfo_ECS.width) {
					ITE_DisplayInfo_ECS = ITE_DisplayInfo;
					ITE_DisplayInfo_ECS.statusTextColor = kITEECSBottomColorGreen;
					ITE_DisplayInfo_ECS.statusBGColor = kITEECSColorBlack;
				}
				return ITE_DisplayInfo_ECS;
			}
			return ITE_DisplayInfo;
#ifdef ENABLE_IHNM
		case GID_IHNM:
			return IHNM_DisplayInfo;
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

bool SagaEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return !_scene->isInIntro() &&
		(_interface->getMode() == kPanelMain || _interface->getMode() == kPanelChapterSelection);
}

bool SagaEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return !_scene->isInIntro() &&
		(_interface->getMode() == kPanelMain || _interface->getMode() == kPanelChapterSelection);
}

} // End of namespace Saga
