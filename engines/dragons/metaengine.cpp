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
 */

#include "dragons/dragons.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"
#include "base/plugins.h"
#include "graphics/thumbnail.h"

class DragonsMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "dragons";
	}

	virtual bool hasFeature(MetaEngineFeature f) const override;
	virtual Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	virtual int getMaximumSaveSlot() const override;
	virtual SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	virtual void removeSaveState(const char *target, int slot) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool DragonsMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
			(f == kSupportsListSaves) ||
			(f == kSupportsDeleteSave) ||
			(f == kSupportsLoadingDuringStartup) ||
			(f == kSavesSupportMetaInfo) ||
			(f == kSavesSupportThumbnail) ||
			(f == kSavesSupportCreationDate);
}

void DragonsMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

int DragonsMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

SaveStateList DragonsMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Dragons::SaveHeader header;
	Common::String pattern = target;
	pattern += ".???";
	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);
		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Dragons::DragonsEngine::readSaveHeader(in, header) == Dragons::kRSHENoError) {
					saveList.push_back(SaveStateDescriptor(slotNum, header.description));
				}
				delete in;
			}
		}
	}
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor DragonsMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Dragons::DragonsEngine::getSavegameFilename(target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());
	if (in) {
		Dragons::SaveHeader header;
		Dragons::kReadSaveHeaderError error;
		error = Dragons::DragonsEngine::readSaveHeader(in, header, false);
		delete in;
		if (error == Dragons::kRSHENoError) {
			SaveStateDescriptor desc(slot, header.description);
			// Slot 0 is used for the "Continue" save
			desc.setDeletableFlag(slot != 0);
			desc.setWriteProtectedFlag(slot == 0);
			desc.setThumbnail(header.thumbnail);
			desc.setSaveDate(header.saveDate & 0xFFFF, (header.saveDate >> 16) & 0xFF, (header.saveDate >> 24) & 0xFF);
			desc.setSaveTime((header.saveTime >> 16) & 0xFF, (header.saveTime >> 8) & 0xFF);
			desc.setPlayTime(header.playTime * 1000);
			return desc;
		}
	}
	return SaveStateDescriptor();
}

Common::Error DragonsMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Dragons::DragonsGameDescription *gd = (const Dragons::DragonsGameDescription *)desc;

	switch (gd->gameId) {
	case Dragons::kGameIdDragons:
		*engine = new Dragons::DragonsEngine(syst, desc);
		break;
	case Dragons::kGameIdDragonsBadExtraction:
		GUIErrorMessageWithURL(_("Error: It appears that the game data files were extracted incorrectly.\n\nYou should only extract STR and XA files using the special method. The rest should be copied normally from your game CD.\n\n See https://wiki.scummvm.org/index.php?title=Datafiles#Blazing_Dragons"),
		                       "https://wiki.scummvm.org/index.php?title=Datafiles#Blazing_Dragons");
		break;
	default:
		return Common::kUnsupportedGameidError;
	}
	return Common::kNoError;
}

Common::KeymapArray DragonsMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "dragons", "Blazing Dragons");

	Action *act;

	act = new Action("LCLK", _("Action"));
	act->setCustomEngineActionEvent(Dragons::kDragonsActionSelect);
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action("CHANGECOMMAND", _("Change Command"));
	act->setCustomEngineActionEvent(Dragons::kDragonsActionChangeCommand);
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("INVENTORY", _("Inventory"));
	act->setCustomEngineActionEvent(Dragons::kDragonsActionInventory);
	act->addDefaultInputMapping("i");
	engineKeyMap->addAction(act);

	act = new Action("ENTER", _("Enter"));
	act->setCustomEngineActionEvent(Dragons::kDragonsActionEnter);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("KP_ENTER");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveUp, _("Up"));
	act->setCustomEngineActionEvent(Dragons::kDragonsActionUp);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Down"));
	act->setCustomEngineActionEvent(Dragons::kDragonsActionDown);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Left"));
	act->setCustomEngineActionEvent(Dragons::kDragonsActionLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Right"));
	act->setCustomEngineActionEvent(Dragons::kDragonsActionRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeyMap->addAction(act);

	act = new Action("SQUARE", _("Square"));
	act->setCustomEngineActionEvent(Dragons::kDragonsActionSquare);
	act->addDefaultInputMapping("a");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	act = new Action("TRIANGLE", _("Triangle"));
	act->setCustomEngineActionEvent(Dragons::kDragonsActionTriangle);
	act->addDefaultInputMapping("w");
	act->addDefaultInputMapping("JOY_Y");
	engineKeyMap->addAction(act);

	act = new Action("CIRCLE", _("Circle"));
	act->setCustomEngineActionEvent(Dragons::kDragonsActionCircle);
	act->addDefaultInputMapping("d");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("CROSS", _("Cross"));
	act->setCustomEngineActionEvent(Dragons::kDragonsActionCross);
	act->addDefaultInputMapping("s");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action("L1", _("Left Shoulder"));
	act->setCustomEngineActionEvent(Dragons::kDragonsActionL1);
	act->addDefaultInputMapping("o");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	engineKeyMap->addAction(act);

	act = new Action("R1", _("Right Shoulder"));
	act->setCustomEngineActionEvent(Dragons::kDragonsActionR1);
	act->addDefaultInputMapping("p");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	engineKeyMap->addAction(act);

	act = new Action("DEBUGGFX", _("Debug Graphics"));
	act->setCustomEngineActionEvent(Dragons::kDragonsActionDebugGfx);
	act->addDefaultInputMapping("TAB");
	engineKeyMap->addAction(act);

	act = new Action("QUIT", _("Quit Game"));
	act->setCustomEngineActionEvent(Dragons::kDragonsActionQuit);
	act->addDefaultInputMapping("C+q");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

#if PLUGIN_ENABLED_DYNAMIC(DRAGONS)
	REGISTER_PLUGIN_DYNAMIC(DRAGONS, PLUGIN_TYPE_ENGINE, DragonsMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DRAGONS, PLUGIN_TYPE_ENGINE, DragonsMetaEngine);
#endif
