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

#include "engines/advancedDetector.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "efh/efh.h"

namespace Efh {

uint32 EfhEngine::getFeatures() const {
	return _gameDescription->flags;
}

const char *EfhEngine::getGameId() const {
	return _gameDescription->gameId;
}

void EfhEngine::initGame(const ADGameDescription *gd) {
	_platform = gd->platform;
}

bool EfhEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher) || (f == kSupportsLoadingDuringRuntime) || (f == kSupportsSavingDuringRuntime);
}

const char *EfhEngine::getCopyrightString() const {
	return "Escape From Hell (C) Electronic Arts, 1990";
}

Common::Platform EfhEngine::getPlatform() const {
	return _platform;
}
} // End of namespace Efh

namespace Efh {

class EfhMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "efh";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const override;
	bool hasFeature(MetaEngineFeature f) const override;

	int getMaximumSaveSlot() const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	bool removeSaveState(const char *target, int slot) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

Common::Error EfhMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	*engine = new EfhEngine(syst, gd);
	((EfhEngine *)*engine)->initGame(gd);
	return Common::kNoError;
}

bool EfhMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate);
}

int EfhMetaEngine::getMaximumSaveSlot() const {
	return 99;
}

SaveStateList EfhMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String pattern = target;
	pattern += ".###";

	Common::StringArray filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	char slot[3];
	for (const auto &filename : filenames) {
		slot[0] = filename.c_str()[filename.size() - 2];
		slot[1] = filename.c_str()[filename.size() - 1];
		slot[2] = '\0';
		// Obtain the last 2 digits of the filename (without extension), since they correspond to the save slot
		int slotNum = atoi(slot);
		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::InSaveFile *file = saveFileMan->openForLoading(filename);
			if (file) {
				uint32 sign = file->readUint32LE();
				uint8 saveVersion = file->readByte();

				if (sign != EFH_SAVE_HEADER || saveVersion > kSavegameVersion) {
					warning("Incompatible savegame");
					delete file;
					continue;
				}

				// read name
				uint16 nameSize = file->readUint16LE();
				if (nameSize >= 255) {
					delete file;
					continue;
				}
				char name[256];
				file->read(name, nameSize);
				name[nameSize] = 0;

				saveList.push_back(SaveStateDescriptor(this, slotNum, name));
				delete file;
			}
		}
	}

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor EfhMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(fileName);

	if (file) {
		uint32 sign = file->readUint32LE();
		uint8 saveVersion = file->readByte();

		if (sign != EFH_SAVE_HEADER || saveVersion > kSavegameVersion) {
			warning("Incompatible savegame");
			delete file;
			return SaveStateDescriptor();
		}

		uint32 saveNameLength = file->readUint16LE();
		Common::String saveName;
		for (uint32 i = 0; i < saveNameLength; ++i) {
			char curChr = file->readByte();
			saveName += curChr;
		}

		SaveStateDescriptor desc(this, slot, saveName);

		Graphics::Surface *thumbnail;
		if (!Graphics::loadThumbnail(*file, thumbnail)) {
			delete file;
			return SaveStateDescriptor();
		}
		desc.setThumbnail(thumbnail);

		// Read in save date/time
		int16 year = file->readSint16LE();
		int16 month = file->readSint16LE();
		int16 day = file->readSint16LE();
		int16 hour = file->readSint16LE();
		int16 minute = file->readSint16LE();
		desc.setSaveDate(year, month, day);
		desc.setSaveTime(hour, minute);

		desc.setDeletableFlag(slot != 0);
		desc.setWriteProtectedFlag(slot == 0);

		delete file;
		return desc;
	}
	return SaveStateDescriptor();
}

bool EfhMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(fileName);
}

Common::KeymapArray EfhMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;

	Keymap *keymap = new Keymap(Keymap::kKeymapTypeGame, "efh", _("Game keymappings"));

	Action *act;

	act = new Action(kStandardActionSave, _("Save game"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("Ctrl+s");
	keymap->addAction(act);

	act = new Action(kStandardActionLoad, _("Load game"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("F7");
	act->addDefaultInputMapping("Ctrl+l");
	keymap->addAction(act);

	act = new Action("A", _("A"));
	act->setCustomEngineActionEvent(kActionA);
	act->addDefaultInputMapping("a");
	keymap->addAction(act);

	act = new Action("H",_( "H"));
	act->setCustomEngineActionEvent(kActionH);
	act->addDefaultInputMapping("h");
	keymap->addAction(act);

	act = new Action("D", _("D"));
	act->setCustomEngineActionEvent(kActionD);
	act->addDefaultInputMapping("d");
	keymap->addAction(act);

	act = new Action("R", _("R"));
	act->setCustomEngineActionEvent(kActionR);
	act->addDefaultInputMapping("r");
	keymap->addAction(act);

	act = new Action("S", _("S"));
	act->setCustomEngineActionEvent(kActionS);
	act->addDefaultInputMapping("s");
	keymap->addAction(act);

	act = new Action("T", _("T"));
	act->setCustomEngineActionEvent(kActionT);
	act->addDefaultInputMapping("t");
	keymap->addAction(act);

	act = new Action("L", _("L"));
	act->setCustomBackendActionAxisEvent(kActionL);
	act->addDefaultInputMapping("l");
	keymap->addAction(act);

	act = new Action("Q", _("Q"));
	act->setCustomBackendActionAxisEvent(kActionQ);
	act->addDefaultInputMapping("q");
	keymap->addAction(act);

	act = new Action("ESC", _("ESC"));
	act->setCustomEngineActionEvent(kActionEscape);
	act->addDefaultInputMapping("KEYCODE_ESCAPE");
	keymap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("goSouth"));
	act->setCustomEngineActionEvent(kActionMoveDown);
	act->addDefaultInputMapping("KEYCODE_DOWN");
	act->addDefaultInputMapping("KEYCODE_KP2");
	keymap->addAction(act);

	act = new Action("Movement Down left", _("goSouthEast"));
	act->setCustomEngineActionEvent(kActionMoveDownLeft);
	act->addDefaultInputMapping("KEYCODE_END");
	keymap->addAction(act);

	act = new Action("Movement Down Right", _("goSouthWest"));
	act->setCustomEngineActionEvent(kActionMoveDownRight);
	act->addDefaultInputMapping("KEYCODE_PAGEDOWN");
	keymap->addAction(act);

	act = new Action(kStandardActionMoveUp, _("goNorth"));
	act->setCustomEngineActionEvent(kActionMoveUp);
	act->addDefaultInputMapping("KEYCODE_UP");
	act->addDefaultInputMapping("KEYCODE_KP8");
	keymap->addAction(act);

	act = new Action("Movement Up left", _("goNorthEast"));
	act->setCustomEngineActionEvent(kActionMoveUpLeft);
	act->addDefaultInputMapping("KEYCODE_HOME");
	keymap->addAction(act);

	act = new Action("Movement Up Right", _("goNorthWest"));
	act->setCustomEngineActionEvent(kActionMoveUpRight);
	act->addDefaultInputMapping("KEYCODE_PAGEUP");
	keymap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("goEast"));
	act->setCustomEngineActionEvent(kActionMoveRight);
	act->addDefaultInputMapping("KEYCODE_RIGHT");
	act->addDefaultInputMapping("KEYCODE_KP6");
	keymap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("goWest"));
	act->setCustomEngineActionEvent(kActionMoveLeft);
	act->addDefaultInputMapping("KEYCODE_LEFT");
	act->addDefaultInputMapping("KEYCODE_KP4");
	keymap->addAction(act);

	act = new Action("ShowCharacterPortraitsOne", _("Display Character Summary One"));
	act->setCustomEngineActionEvent(kActionShowCharacterPortraitsOne);
	act->addDefaultInputMapping("1");
	act->addDefaultInputMapping("F1");
	keymap->addAction(act);

	act = new Action("ShowCharacterPortraitsTwo", _("Display Character Summary Two"));
	act->setCustomEngineActionEvent(kActionShowCharacterPortraitsTwo);
	act->addDefaultInputMapping("2");
	act->addDefaultInputMapping("F2");
	keymap->addAction(act);

	act = new Action("ShowCharacterPortraitsThree", _("Display Character Summary Three"));
	act->setCustomEngineActionEvent(kActionShowCharacterPortraitsThree);
	act->addDefaultInputMapping("3");
	act->addDefaultInputMapping("F3");
	keymap->addAction(act);

	return Keymap::arrayOf(keymap);
}

} // End of namespace Efh

#if PLUGIN_ENABLED_DYNAMIC(EFH)
	REGISTER_PLUGIN_DYNAMIC(EFH, PLUGIN_TYPE_ENGINE, Efh::EfhMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(EFH, PLUGIN_TYPE_ENGINE, Efh::EfhMetaEngine);
#endif
