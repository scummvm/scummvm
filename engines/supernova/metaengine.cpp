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

#include "base/plugins.h"
#include "common/file.h"
#include "common/gui_options.h"
#include "common/savefile.h"
#include "common/system.h"
#include "graphics/thumbnail.h"
#include "engines/advancedDetector.h"

#include "supernova/supernova.h"

class SupernovaMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "supernova";
	}

    bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	void removeSaveState(const char *target, int slot) const override;
	int getMaximumSaveSlot() const override {
		return 99;
	}
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool SupernovaMetaEngine::hasFeature(MetaEngineFeature f) const {
	switch (f) {
	case kSupportsLoadingDuringStartup:
	case kSupportsListSaves:
	case kSupportsDeleteSave:
	case kSavesSupportMetaInfo:
	case kSavesSupportThumbnail:
	case kSavesSupportCreationDate:
	case kSavesSupportPlayTime:
		return true;
	default:
		return false;
	}
}

Common::Error SupernovaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Supernova::SupernovaEngine(syst);
	return Common::kNoError;
}

SaveStateList SupernovaMetaEngine::listSaves(const char *target) const {
	Common::StringArray filenames;
	Common::String pattern;
	if (!strncmp(target, "msn1", 4))
		pattern = Common::String::format("msn_save.###");
	if (!strncmp(target, "msn2", 4))
		pattern = Common::String::format("ms2_save.###");

	filenames = g_system->getSavefileManager()->listSavefiles(pattern);

	SaveStateList saveFileList;
	for (Common::StringArray::const_iterator file = filenames.begin();
		 file != filenames.end(); ++file) {
		int saveSlot = atoi(file->c_str() + file->size() - 3);
		if (saveSlot >= 0 && saveSlot <= getMaximumSaveSlot()) {
			Common::InSaveFile *savefile = g_system->getSavefileManager()->openForLoading(*file);
			if (savefile) {
				uint saveHeader = savefile->readUint32LE();
				if ((saveHeader == SAVEGAME_HEADER && !strncmp(target, "msn1", 4)) ||
					(saveHeader == SAVEGAME_HEADER2 && !strncmp(target, "msn2", 4))) {
					byte saveVersion = savefile->readByte();
					if (saveVersion <= SAVEGAME_VERSION) {
						int saveFileDescSize = savefile->readSint16LE();
						char* saveFileDesc = new char[saveFileDescSize];
						savefile->read(saveFileDesc, saveFileDescSize);
						saveFileList.push_back(SaveStateDescriptor(saveSlot, saveFileDesc));
						delete [] saveFileDesc;
					}
				}
				delete savefile;
			}
		}
	}

	Common::sort(saveFileList.begin(), saveFileList.end(), SaveStateDescriptorSlotComparator());
	return saveFileList;
}

void SupernovaMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename;
	if (!strncmp(target, "msn1", 4))
		filename = Common::String::format("msn_save.%03d", slot);
	if (!strncmp(target, "msn2", 4))
		filename = Common::String::format("ms2_save.%03d", slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor SupernovaMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName;
	if (!strncmp(target, "msn1", 4))
		fileName = Common::String::format("msn_save.%03d", slot);
	if (!strncmp(target, "msn2", 4))
		fileName = Common::String::format("ms2_save.%03d", slot);
	Common::InSaveFile *savefile = g_system->getSavefileManager()->openForLoading(fileName);

	if (savefile) {
		uint saveHeader = savefile->readUint32LE();
		if ((!strncmp(target, "msn1", 4) && saveHeader != SAVEGAME_HEADER) ||
			(!strncmp(target, "msn2", 4) && saveHeader != SAVEGAME_HEADER2)) {
			delete savefile;
			return SaveStateDescriptor();
		}
		byte saveVersion = savefile->readByte();
		if (saveVersion > SAVEGAME_VERSION){
			delete savefile;
			return SaveStateDescriptor();
		}

		int descriptionSize = savefile->readSint16LE();
		char* description = new char[descriptionSize];
		savefile->read(description, descriptionSize);
		SaveStateDescriptor desc(slot, description);
		delete [] description;

		uint32 saveDate = savefile->readUint32LE();
		int day = (saveDate >> 24) & 0xFF;
		int month = (saveDate >> 16) & 0xFF;
		int year = saveDate & 0xFFFF;
		desc.setSaveDate(year, month, day);

		uint16 saveTime = savefile->readUint16LE();
		int hour = (saveTime >> 8) & 0xFF;
		int minutes = saveTime & 0xFF;
		desc.setSaveTime(hour, minutes);

		uint32 playTime =savefile->readUint32LE();
		desc.setPlayTime(playTime * 1000);

		if (Graphics::checkThumbnailHeader(*savefile)) {
			Graphics::Surface *thumbnail;
			if (!Graphics::loadThumbnail(*savefile, thumbnail)) {
				delete savefile;
				return SaveStateDescriptor();
			}
			desc.setThumbnail(thumbnail);
		}

		delete savefile;

		return desc;
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(SUPERNOVA)
    REGISTER_PLUGIN_DYNAMIC(SUPERNOVA, PLUGIN_TYPE_ENGINE, SupernovaMetaEngine);
#else
    REGISTER_PLUGIN_STATIC(SUPERNOVA, PLUGIN_TYPE_ENGINE, SupernovaMetaEngine);
#endif
