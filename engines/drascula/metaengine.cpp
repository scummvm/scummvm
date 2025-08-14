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

#include "engines/savestate.h"
#include "engines/advancedDetector.h"

#include "common/translation.h"

#include "graphics/thumbnail.h"

#include "drascula/drascula.h"
#include "drascula/detection.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

namespace Drascula {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false,
			0,
			0
		}
	},

#ifdef USE_TTS
	{
		GAMEOPTION_TTS,
		{
			_s("Enable Text to Speech"),
			_s("Use TTS to read text in the game (if TTS is available)"),
			"tts_enabled",
			false,
			0,
			0
		}
	},
#endif

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

uint32 DrasculaEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language DrasculaEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

void DrasculaEngine::loadArchives() {
	const ADGameFileDescription *ag;

	if (getFeatures() & GF_PACKED) {
		for (ag = _gameDescription->desc.filesDescriptions; ag->fileName; ag++) {
			if (!_archives.hasArchive(ag->fileName))
				_archives.registerArchive(ag->fileName, ag->fileType);
		}
	}

	_archives.enableFallback(true);
}

} // End of namespace Drascula


namespace Drascula {

SaveStateDescriptor loadMetaData(Common::ReadStream *s, int slot, bool setPlayTime);

class DrasculaMetaEngine : public AdvancedMetaEngine<Drascula::DrasculaGameDescription> {
public:
	const char *getName() const override {
		return "drascula";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Drascula::optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const Drascula::DrasculaGameDescription *gd) const override;
	bool hasFeature(MetaEngineFeature f) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool DrasculaMetaEngine::hasFeature(MetaEngineFeature f) const {
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

SaveStateList DrasculaMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String pattern = target;
	pattern += ".###";

	Common::StringArray filenames = saveFileMan->listSavefiles(pattern);
	SaveStateList saveList;
	int slotNum = 0;

	for (const auto &filename : filenames) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(filename.c_str() + filename.size() - 3);

		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::InSaveFile *in = saveFileMan->openForLoading(filename);
			if (in) {
				SaveStateDescriptor desc = loadMetaData(in, slotNum, false);
				if (desc.getSaveSlot() != slotNum) {
					// invalid
					delete in;
					continue;
				}
				saveList.push_back(desc);
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor DrasculaMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);

	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fileName);

	SaveStateDescriptor desc(this, slot, Common::U32String());
	if (in) {
		desc = Drascula::loadMetaData(in, slot, false);
		if (desc.getSaveSlot() != slot) {
			delete in;
			return SaveStateDescriptor();
		}

		Graphics::Surface *thumbnail;
		if (!Graphics::loadThumbnail(*in, thumbnail)) {
			delete in;
			return SaveStateDescriptor();
		}
		desc.setThumbnail(thumbnail);

		delete in;
	}

	return desc;
}

int DrasculaMetaEngine::getMaximumSaveSlot() const { return 999; }

bool DrasculaMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(fileName);
}

Common::Error DrasculaMetaEngine::createInstance(OSystem *syst, Engine **engine, const Drascula::DrasculaGameDescription *desc) const {
	*engine = new Drascula::DrasculaEngine(syst,desc);
	return Common::kNoError;
}

Common::KeymapArray DrasculaMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Drascula;

	bool originalSaveLoad = ConfMan.getBool("originalsaveload", target);

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "drascula-default", _("Default keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));
	Keymap *animationKeyMap = new Keymap(Keymap::kKeymapTypeGame, "animation", _("Animation keymappings"));
	Keymap *quitDialogKeyMap = new Keymap(Keymap::kKeymapTypeGame, "quit-dialog", _("Quit dialog keymappings"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Move / Interact / Select"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Inventory"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("SKIP", _("Skip"));
	act->setCustomEngineActionEvent(kActionSkip);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_X");
	animationKeyMap->addAction(act);

	act = new Action("LOOK", _("Look"));
	act->setCustomEngineActionEvent(kActionLook);
	act->addDefaultInputMapping("F1");
	gameKeyMap->addAction(act);

	act = new Action("PICK", _("Pick"));
	act->setCustomEngineActionEvent(kActionPick);
	act->addDefaultInputMapping("F2");
	gameKeyMap->addAction(act);

	act = new Action("OPEN", _("Open"));
	act->setCustomEngineActionEvent(kActionOpen);
	act->addDefaultInputMapping("F3");
	gameKeyMap->addAction(act);

	act = new Action("CLOSE", _("Close"));
	act->setCustomEngineActionEvent(kActionClose);
	act->addDefaultInputMapping("F4");
	gameKeyMap->addAction(act);

	act = new Action("TALK", _("Talk"));
	act->setCustomEngineActionEvent(kActionTalk);
	act->addDefaultInputMapping("F5");
	gameKeyMap->addAction(act);

	act = new Action("MOVE", _("Move"));
	act->setCustomEngineActionEvent(kActionMove);
	act->addDefaultInputMapping("F6");
	gameKeyMap->addAction(act);

	act = new Action("LOAD", _("Load game"));
	act->setCustomEngineActionEvent(kActionLoadGame);
	act->addDefaultInputMapping("F7");
	act->addDefaultInputMapping("JOY_LEFT");
	gameKeyMap->addAction(act);

	act = new Action("RESETVERBS", _("Reset selected verb"));
	act->setCustomEngineActionEvent(kActionVerbReset);
	act->addDefaultInputMapping("F8");
	act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
	gameKeyMap->addAction(act);

	act = new Action("VOLCONTROLS", _("Volume controls"));
	act->setCustomEngineActionEvent(kActionVolumeControls);
	act->addDefaultInputMapping("F9");
	act->addDefaultInputMapping("JOY_X");
	gameKeyMap->addAction(act);

	if (originalSaveLoad) {
		act = new Action("SAVELOAD", _("Save / load game"));
		act->setCustomEngineActionEvent(kActionSaveGame);
		act->addDefaultInputMapping("F10");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeyMap->addAction(act);
	} else {
		act = new Action("SAVE", _("Save game"));
		act->setCustomEngineActionEvent(kActionSaveGame);
		act->addDefaultInputMapping("F10");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeyMap->addAction(act);
	}

	act = new Action("SUBTITLESENABLE", _("Enable subtitles"));
	act->setCustomEngineActionEvent(kActionSubtitlesEnable);
	act->addDefaultInputMapping("v");
	act->addDefaultInputMapping("JOY_UP");
	gameKeyMap->addAction(act);

	act = new Action("SUBTITLESDISABLE", _("Disable subtitles"));
	act->setCustomEngineActionEvent(kActionSubtitlesDisable);
	act->addDefaultInputMapping("t");
	act->addDefaultInputMapping("JOY_DOWN");
	gameKeyMap->addAction(act);

	act = new Action("QUIT", _("Quit"));
	act->setCustomEngineActionEvent(kActionQuit);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	gameKeyMap->addAction(act);

	act = new Action(kStandardActionEE, _("???"));
	act->setCustomEngineActionEvent(kActionEasterEgg);
	act->addDefaultInputMapping("0");
	gameKeyMap->addAction(act);

	act = new Action("PAUSESPEECH", _("Pause speech"));
	act->setCustomEngineActionEvent(kActionPauseSpeech);
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("PAUSE");
	act->addDefaultInputMapping("JOY_Y");
	gameKeyMap->addAction(act);

	act = new Action("QUITCONFIRM", _("Confirm quit"));
	act->setCustomEngineActionEvent(kActionConfirmQuit);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	quitDialogKeyMap->addAction(act);

	KeymapArray keymaps(4);

	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;
	keymaps[2] = animationKeyMap;
	keymaps[3] = quitDialogKeyMap;

	animationKeyMap->setEnabled(false);
	quitDialogKeyMap->setEnabled(false);

	return keymaps;
}

} // End of namespace Drascula

#if PLUGIN_ENABLED_DYNAMIC(DRASCULA)
	REGISTER_PLUGIN_DYNAMIC(DRASCULA, PLUGIN_TYPE_ENGINE, Drascula::DrasculaMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DRASCULA, PLUGIN_TYPE_ENGINE, Drascula::DrasculaMetaEngine);
#endif
