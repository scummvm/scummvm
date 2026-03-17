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

#include "waynesworld/waynesworld.h"

#include "engines/advancedDetector.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "waynesworld/detection.h"

namespace WaynesWorld {
static const ADExtraGuiOptionsMap optionsList[] = {
	{GAMEOPTION_ORIGINAL_SAVELOAD,
	 {
	 		_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
	 	"originalsaveload",
	 	true,
	 	0,
	 	0
			}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // namespace WaynesWorld

class WaynesWorldMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "waynesworld";
	}
	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return WaynesWorld::optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool WaynesWorldMetaEngine::hasFeature(MetaEngineFeature f) const {
	return (f == kSupportsLoadingDuringStartup) ||
		   checkExtendedSaves(f);
}

Common::Error WaynesWorldMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new WaynesWorld::WaynesWorldEngine(syst,desc);
	return Common::kNoError;
}

SaveStateList WaynesWorldMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	const Common::String pattern = "ww##.sav";
	Common::StringArray filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (const auto &filename : filenames) {
		// Obtain the 2 digits from the filename, since they correspond to the save slot
		Common::String numb = filename.substr(2, 2);
		const int slotNum = atoi(numb.c_str());

		if (slotNum > 0 && slotNum <= 99) {

			Common::InSaveFile *file = saveFileMan->openForLoading(filename);
			if (file) {
				WaynesWorld::SavegameHeader header;

				// Check to see if it's a ScummVM savegame or an original format savegame
				char buffer[kWWSavegameStrSize + 1];
				if (file->size() < 1140) {
					saveList.push_back(SaveStateDescriptor(this, slotNum, "Unknown"));
				} else {
					file->seek(1135);
					file->read(buffer, kWWSavegameStrSize + 1);

					if (!strncmp(buffer, WaynesWorld::savegameStr, kWWSavegameStrSize + 1)) {
						// Valid savegame
						if (WaynesWorld::WaynesWorldEngine::readSavegameHeader(file, header)) {
							saveList.push_back(SaveStateDescriptor(this, slotNum, header.saveName));
						}
					} else {
						// Unexpected savegame format? 
					}
				}
				delete file;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor WaynesWorldMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("ww%02d.sav", slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(fileName);

	if (f) {
		WaynesWorld::SavegameHeader header;

		// Check to see if it's a ScummVM savegame or not
		char buffer[kWWSavegameStrSize + 1];
		if (f->size() < 1140)
			buffer[0] = 0;
		else {
			f->seek(1135);
			f->read(buffer, kWWSavegameStrSize + 1);
		}

		bool hasHeader = !strncmp(buffer, WaynesWorld::savegameStr, kWWSavegameStrSize + 1) &&
						 WaynesWorld::WaynesWorldEngine::readSavegameHeader(f, header, false);
		delete f;

		if (!hasHeader) {
			// Original savegame perhaps?
			SaveStateDescriptor desc(this, slot, "Unknown");
			return desc;
		}

		// Create the return descriptor
		SaveStateDescriptor desc(this, slot, header.saveName);
		desc.setThumbnail(header.thumbnail);
		desc.setSaveDate(header.saveYear, header.saveMonth, header.saveDay);
		desc.setSaveTime(header.saveHour, header.saveMinutes);

		if (header.playTime) {
			desc.setPlayTime(header.playTime * 1000);
		}

		return desc;
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(WAYNESWORLD)
	REGISTER_PLUGIN_DYNAMIC(WAYNESWORLD, PLUGIN_TYPE_ENGINE, WaynesWorldMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(WAYNESWORLD, PLUGIN_TYPE_ENGINE, WaynesWorldMetaEngine);
#endif
