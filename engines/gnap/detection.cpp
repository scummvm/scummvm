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

#include "gnap/gnap.h"

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "base/plugins.h"
#include "graphics/thumbnail.h"

static const PlainGameDescriptor gnapGames[] = {
	{ "gnap", "Gnap" },
	{ 0, 0 }
};

namespace Gnap {

static const ADGameDescription gameDescriptions[] = {
	{
		"gnap", "",
		{
			{"stock_n.dat", 0, "46819043d019a2f36b727cc2bdd6980f", 12515823},
			AD_LISTEND
		},
		Common::EN_ANY, Common::kPlatformWindows, ADGF_NO_FLAGS, GUIO0()
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Gnap

class GnapMetaEngine : public AdvancedMetaEngine {
public:
	GnapMetaEngine() : AdvancedMetaEngine(Gnap::gameDescriptions, sizeof(ADGameDescription), gnapGames) {
		_singleid = "gnap";
		_maxScanDepth = 3;
	}

	virtual const char *getName() const {
		return "Gnap";
	}

	virtual const char *getOriginalCopyright() const {
		return "Gnap (C) Artech Digital Entertainment 1997";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual int getMaximumSaveSlot() const;
	virtual SaveStateList listSaves(const char *target) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	virtual void removeSaveState(const char *target, int slot) const;
};

bool GnapMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		false;
#if 0		
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSavesSupportCreationDate);
#endif	    
}

void GnapMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

int GnapMetaEngine::getMaximumSaveSlot() const { return 99; }

SaveStateList GnapMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".???";

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
#if 0	
	int slotNum = 0;
	for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(filename->c_str() + filename->size() - 3);

		if (slotNum >= 0 && slotNum <= 99) {
			Common::InSaveFile *file = saveFileMan->openForLoading(*filename);
			if (file) {
				int32 version = file->readSint32BE();
				if (version != GNAP_SAVEGAME_VERSION) {
					delete file;
					continue;
				}

				// read name
				uint16 nameSize = file->readUint16BE();
				if (nameSize >= 255) {
					delete file;
					continue;
				}
				char name[256];
				file->read(name, nameSize);
				name[nameSize] = 0;

				saveList.push_back(SaveStateDescriptor(slotNum, name));
				delete file;
			}
		}
	}
#endif
	return saveList;
}

SaveStateDescriptor GnapMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
#if 0
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(fileName);
	if (file) {

		int32 version = file->readSint32BE();
		if (version != GNAP_SAVEGAME_VERSION) {
			delete file;
			return SaveStateDescriptor();
		}

		uint32 saveNameLength = file->readUint16BE();
		char saveName[256];
		file->read(saveName, saveNameLength);
		saveName[saveNameLength] = 0;

		SaveStateDescriptor desc(slot, saveName);

		Graphics::Surface *const thumbnail = Graphics::loadThumbnail(*file);
		desc.setThumbnail(thumbnail);

		uint32 saveDate = file->readUint32BE();
		uint16 saveTime = file->readUint16BE();

		int day = (saveDate >> 24) & 0xFF;
		int month = (saveDate >> 16) & 0xFF;
		int year = saveDate & 0xFFFF;

		desc.setSaveDate(year, month, day);

		int hour = (saveTime >> 8) & 0xFF;
		int minutes = saveTime & 0xFF;

		desc.setSaveTime(hour, minutes);

		delete file;
		return desc;
	}
#endif
	return SaveStateDescriptor();
}

bool GnapMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Gnap::GnapEngine(syst, desc);
	}
	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(GNAP)
	REGISTER_PLUGIN_DYNAMIC(GNAP, PLUGIN_TYPE_ENGINE, GnapMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GNAP, PLUGIN_TYPE_ENGINE, GnapMetaEngine);
#endif
