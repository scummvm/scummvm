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

#include "titanic/core/project_item.h"
#include "titanic/events.h"
#include "titanic/support/simple_file.h"
#include "titanic/titanic.h"
#include "base/plugins.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/memstream.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "titanic/detection.h"

#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

namespace Titanic {

uint32 TitanicEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language TitanicEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

} // End of namespace Titanic

class TitanicMetaEngine : public AdvancedMetaEngine<Titanic::TitanicGameDescription> {
public:
	const char *getName() const override {
		return "titanic";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const Titanic::TitanicGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool TitanicMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSimpleSavesNames);
}

bool Titanic::TitanicEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error TitanicMetaEngine::createInstance(OSystem *syst, Engine **engine, const Titanic::TitanicGameDescription *desc) const {
	*engine = new Titanic::TitanicEngine(syst,desc);
	return Common::kNoError;
}

SaveStateList TitanicMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);
	Titanic::TitanicSavegameHeader header;

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot <= MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				Titanic::CompressedFile cf;
				cf.open(in);

				if (Titanic::CProjectItem::readSavegameHeader(&cf, header))
					saveList.push_back(SaveStateDescriptor(this, slot, header._saveName));

				cf.close();
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int TitanicMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

bool TitanicMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor TitanicMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {
		Titanic::CompressedFile file;
		file.open(f);

		Titanic::TitanicSavegameHeader header;
		if (!Titanic::CProjectItem::readSavegameHeader(&file, header, false)) {
			file.close();
			return SaveStateDescriptor();
		}

		file.close();

		// Create the return descriptor
		SaveStateDescriptor desc(this, slot, header._saveName);

		if (header._version) {
			desc.setThumbnail(header._thumbnail);
			desc.setSaveDate(header._year, header._month, header._day);
			desc.setSaveTime(header._hour, header._minute);
			desc.setPlayTime(header._totalFrames * GAME_FRAME_TIME);
		}

		return desc;
	}

	return SaveStateDescriptor();
}

Common::KeymapArray TitanicMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Titanic;

	Keymap *engineKeymap = new Keymap(Keymap::kKeymapTypeGame, "titanic-default", _("Default keymappings"));
	Keymap *petKeymap = new Keymap(Keymap::kKeymapTypeGame, "pet", _("PET keymappings"));
	Keymap *realLifeKeymap = new Keymap(Keymap::kKeymapTypeGame, "real-life", _("Real Life keymappings"));
	Keymap *invShortcutKeymap = new Keymap(Keymap::kKeymapTypeGame, "inv-shortcut", _("Inventory shortcut"));
	Keymap *contSaveKeymap = new Keymap(Keymap::kKeymapTypeGame, "cont-save", _("Continue game dialogue keymappings"));
	Keymap *starMapKeymap = new Keymap(Keymap::kKeymapTypeGame, "star-map", _("Star Map keymappings"));
	Keymap *movementKeymap = new Keymap(Keymap::kKeymapTypeGame, "movement", _("Movement keymappings"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Select / Interact / Move"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeymap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Select / Interact / Quick move"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeymap->addAction(act);

	act = new Action("CHEAT", _("Cheat menu"));
	act->setCustomEngineActionEvent(kActionCheat);
	act->addDefaultInputMapping("C+c");
	engineKeymap->addAction(act);

	act = new Action("SAVE", _("Save game"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	engineKeymap->addAction(act);

	act = new Action("LOAD", _("Load game"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("F7");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	engineKeymap->addAction(act);

	act = new Action("SHIFT", _("Shift"));
	act->setCustomEngineActionEvent(kActionShift);
	act->addDefaultInputMapping("LSHIFT");
	act->addDefaultInputMapping("RSHIFT");
	act->addDefaultInputMapping("JOY_X");
	engineKeymap->addAction(act);

	act = new Action("UP", _("Scroll up"));
	act->setCustomEngineActionEvent(kActionUp);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("KP8");
	act->addDefaultInputMapping("JOY_UP");
	realLifeKeymap->addAction(act);

	act = new Action("DOWN", _("Scroll down"));
	act->setCustomEngineActionEvent(kActionDown);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("TAB");
	act->addDefaultInputMapping("KP2");
	act->addDefaultInputMapping("JOY_DOWN");
	realLifeKeymap->addAction(act);

	act = new Action("SELECT", _("Select"));
	act->setCustomEngineActionEvent(kActionSelect);
	act->addDefaultInputMapping("Return");
	act->addDefaultInputMapping("KP_ENTER");
	realLifeKeymap->addAction(act);

	act = new Action("CONVERSATION", _("Conversation"));
	act->setCustomEngineActionEvent(kActionPETConversation);
	act->addDefaultInputMapping("F1");
	petKeymap->addAction(act);

	act = new Action("INVENTORY", _("Inventory"));
	act->setCustomEngineActionEvent(kActionPETInventory);
	act->addDefaultInputMapping("F2");
	petKeymap->addAction(act);

	act = new Action("REMOTE", _("Remote"));
	act->setCustomEngineActionEvent(kActionPETRemote);
	act->addDefaultInputMapping("F3");
	petKeymap->addAction(act);

	act = new Action("ROOMS", _("Rooms"));
	act->setCustomEngineActionEvent(kActionPETRooms);
	act->addDefaultInputMapping("F4");
	petKeymap->addAction(act);

	act = new Action("REALLIFE", _("Real life"));
	act->setCustomEngineActionEvent(kActionPETRealLife);
	act->addDefaultInputMapping("F6");
	petKeymap->addAction(act);

	act = new Action("SCROLLUP", _("Previous page"));
	act->setCustomEngineActionEvent(kActionPETScrollPageUp);
	act->addDefaultInputMapping("PAGEUP");
	act->addDefaultInputMapping("KP9");
	act->addDefaultInputMapping("JOY_UP");
	petKeymap->addAction(act);

	act = new Action("SCROLLDOWN", _("Next page"));
	act->setCustomEngineActionEvent(kActionPETScrollPageDown);
	act->addDefaultInputMapping("PAGEDOWN");
	act->addDefaultInputMapping("KP3");
	act->addDefaultInputMapping("JOY_DOWN");
	petKeymap->addAction(act);

	act = new Action("SCROLLTOP", _("Scroll to top"));
	act->setCustomEngineActionEvent(kActionPETScrollTop);
	act->addDefaultInputMapping("HOME");
	act->addDefaultInputMapping("KP7");
	petKeymap->addAction(act);

	act = new Action("SCROLLBOTTOM", _("Scroll to bottom"));
	act->setCustomEngineActionEvent(kActionPeTScrollBottom);
	act->addDefaultInputMapping("END");
	act->addDefaultInputMapping("KP1");
	petKeymap->addAction(act);

	if (Common::parseLanguage(ConfMan.get("language", target)) == Common::DE_DEU) {
		act = new Action("TRANSLATION", _("Translation"));
		act->setCustomEngineActionEvent(kActionPETTranslation);
		act->addDefaultInputMapping("F8");
		petKeymap->addAction(act);
	}

	act = new Action("INVENTORY", _("Inventory"));
	act->setCustomEngineActionEvent(kActionPETInventory);
	act->addDefaultInputMapping("TAB");
	invShortcutKeymap->addAction(act);

	act = new Action("QUIT", _("Quit"));
	act->setCustomEngineActionEvent(kActionQuit);
	act->addDefaultInputMapping("ESCAPE");
	contSaveKeymap->addAction(act);

	act = new Action("TOGGLE", _("Toggle between Star Map and photo of your home"));
	act->setCustomEngineActionEvent(kActionStarMapToggle);
	act->addDefaultInputMapping("TAB");
	starMapKeymap->addAction(act);

	act = new Action("LEFT", _("Move left"));
	act->setCustomEngineActionEvent(kActionStarMapLeft);
	act->allowKbdRepeats();
	act->addDefaultInputMapping("z");
	starMapKeymap->addAction(act);

	act = new Action("RIGHT", _("Move right"));
	act->setCustomEngineActionEvent(kActionStarMapRight);
	act->allowKbdRepeats();
	act->addDefaultInputMapping("x");
	starMapKeymap->addAction(act);

	act = new Action("UP", _("Move up"));
	act->setCustomEngineActionEvent(kActionStarMapUp);
	act->allowKbdRepeats();
	act->addDefaultInputMapping("QUOTE");
	starMapKeymap->addAction(act);

	act = new Action("DOWN", _("Move down"));
	act->setCustomEngineActionEvent(kActionStarMapDown);
	act->allowKbdRepeats();
	act->addDefaultInputMapping("SLASH");
	starMapKeymap->addAction(act);

	act = new Action("FORWARD", _("Move forward"));
	act->setCustomEngineActionEvent(kActionStarMapForward);
	act->allowKbdRepeats();
	act->addDefaultInputMapping("SEMICOLON");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	starMapKeymap->addAction(act);

	act = new Action("BACKWARD", _("Move backward"));
	act->setCustomEngineActionEvent(kActionStarMapBackward);
	act->allowKbdRepeats();
	act->addDefaultInputMapping("PERIOD");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	starMapKeymap->addAction(act);

	act = new Action("STOP", _("Stop moving"));
	act->setCustomEngineActionEvent(kActionStarMapStop);
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_Y");
	starMapKeymap->addAction(act);

	act = new Action("LOCK", _("Lock coordinate"));
	act->setCustomEngineActionEvent(kActionStarMapLock);
	act->addDefaultInputMapping("l");
	act->addDefaultInputMapping("JOY_UP");
	starMapKeymap->addAction(act);

	act = new Action("UNLOCK", _("Unlock coordinate"));
	act->setCustomEngineActionEvent(kActionStarMapUnlock);
	act->addDefaultInputMapping("d");
	act->addDefaultInputMapping("JOY_DOWN");
	starMapKeymap->addAction(act);

	act = new Action("CONSTELLATIONS", _("View constellations"));
	act->setCustomEngineActionEvent(kActionStarMapConstellations);
	act->addDefaultInputMapping("c");
	starMapKeymap->addAction(act);

	act = new Action("BOUNDARIES", _("View boundaries"));
	act->setCustomEngineActionEvent(kActionStarMapBoundaries);
	act->addDefaultInputMapping("b");
	starMapKeymap->addAction(act);

	act = new Action("MFORWARD", _("Move forward"));
	act->setCustomEngineActionEvent(kActionMovementForwards);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	movementKeymap->addAction(act);

	act = new Action("MBACKWARD", _("Move backward"));
	act->setCustomEngineActionEvent(kActionMovementBackwards);
	act->addDefaultInputMapping("DOWN");
	 act->addDefaultInputMapping("JOY_DOWN");
	movementKeymap->addAction(act);

	act = new Action("MLEFT", _("Move left"));
	act->setCustomEngineActionEvent(kActionMovementLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	movementKeymap->addAction(act);

	act = new Action("MRIGHT", _("Move right"));
	act->setCustomEngineActionEvent(kActionMovementRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	movementKeymap->addAction(act);

	KeymapArray keymaps(7);

	keymaps[0] = engineKeymap;
	keymaps[1] = realLifeKeymap;
	keymaps[2] = petKeymap;
	keymaps[3] = invShortcutKeymap;
	keymaps[4] = contSaveKeymap;
	keymaps[5] = starMapKeymap;
	keymaps[6] = movementKeymap;

	contSaveKeymap->setEnabled(false);
	realLifeKeymap->setEnabled(false);
	starMapKeymap->setEnabled(false);

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(TITANIC)
	REGISTER_PLUGIN_DYNAMIC(TITANIC, PLUGIN_TYPE_ENGINE, TitanicMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TITANIC, PLUGIN_TYPE_ENGINE, TitanicMetaEngine);
#endif
