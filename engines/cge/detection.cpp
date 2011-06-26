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

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "base/plugins.h"
#include "graphics/thumbnail.h"
#include "cge/cge.h"

static const PlainGameDescriptor CGEGames[] = {
	{ "soltys", "Soltys" },
	{ 0, 0 }
};

namespace CGE {

using Common::GUIO_NONE;

static const ADGameDescription gameDescriptions[] = {

	{
		"soltys", "",
		{
			{"vol.cat", 0, "0c33e2c304821a2444d297fc5e2d67c6", 50176},
			{"vol.dat", 0, "f9ae2e7f8f7cac91378cdafca43faf1e", 8437572},
			AD_LISTEND
		},
		Common::PL_POL, Common::kPlatformPC, ADGF_NO_FLAGS, GUIO_NONE
	},
	{
		"soltys", "Soltys Freeware",
		{
			{"vol.cat", 0, "0c33e2c304821a2444d297fc5e2d67c6", 50176},
			{"vol.dat", 0, "f9ae2e7f8f7cac91378cdafca43faf1e", 8437676},
			AD_LISTEND
		},
		Common::PL_POL, Common::kPlatformPC, ADGF_NO_FLAGS, GUIO_NONE
	},
	{
		"soltys", "Soltys Demo",
		{
			{"vol.cat", 0, "1e077c8ff58109a187f07ac54b0c873a", 18788},
			{"vol.dat", 0, "75d385a6074c58b69f7730481f256051", 1796710},
			AD_LISTEND
		},
		Common::PL_POL, Common::kPlatformPC, ADGF_NO_FLAGS, GUIO_NONE
	},
	{
		"soltys", "Soltys Demo",
		{
			{"vol.cat", 0, "f17987487fab1ebddd781d8d02fedecc", 7168},
			{"vol.dat", 0, "c5d9b15863cab61dc125551576dece04", 1075272},
			AD_LISTEND
		},
		Common::PL_POL, Common::kPlatformPC, ADGF_NO_FLAGS, GUIO_NONE
	},
	AD_TABLE_END_MARKER
};

static const ADFileBasedFallback fileBasedFallback[] = {
	{ &gameDescriptions[0], { "vol.cat", "vol.dat", 0 } },
	{ 0, { 0 } }
};

} // End of namespace CGE

static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)CGE::gameDescriptions,
	// Size of that superset structure
	sizeof(ADGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	CGEGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"Soltys",
	// List of files for file-based fallback detection (optional)
	CGE::fileBasedFallback,
	// Flags
	0,
	// Additional GUI options (for every game}
	Common::GUIO_NONE,
	// Maximum directory depth
	0,
	// List of directory globs
	NULL
};

class CGEMetaEngine : public AdvancedMetaEngine {
public:
	CGEMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "CGE";
	}

	virtual const char *getOriginalCopyright() const {
		return "Soltys (c) 1994-1996 L.K. Avalon";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual int getMaximumSaveSlot() const;
	virtual SaveStateList listSaves(const char *target) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	virtual void removeSaveState(const char *target, int slot) const;
};

bool CGEMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSavesSupportCreationDate);
}

void CGEMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

int CGEMetaEngine::getMaximumSaveSlot() const {
	return 99;
}

SaveStateList CGEMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".???";

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	int slotNum = 0;
	for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); filename++) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(filename->c_str() + filename->size() - 3);

		if (slotNum >= 0 && slotNum <= 99) {
			Common::InSaveFile *file = saveFileMan->openForLoading(*filename);
			if (file) {
				int32 version = file->readSint32BE();
				if (version != CGE_SAVEGAME_VERSION) {
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

	return saveList;
}

SaveStateDescriptor CGEMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(fileName);

	if (file) {

		int32 version = file->readSint32BE();
		if (version != CGE_SAVEGAME_VERSION) {
			delete file;
			return SaveStateDescriptor();
		}

		uint32 saveNameLength = file->readUint16BE();
		char saveName[256];
		file->read(saveName, saveNameLength);
		saveName[saveNameLength] = 0;

		SaveStateDescriptor desc(slot, saveName);

		Graphics::Surface *thumbnail = new Graphics::Surface();
		assert(thumbnail);
		if (!Graphics::loadThumbnail(*file, *thumbnail)) {
			delete thumbnail;
			thumbnail = 0;
		}
		desc.setThumbnail(thumbnail);

		desc.setDeletableFlag(true);
		desc.setWriteProtectedFlag(false);

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

	return SaveStateDescriptor();
}

bool CGEMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new CGE::CGEEngine(syst, desc);
	}
	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(CGE)
REGISTER_PLUGIN_DYNAMIC(CGE, PLUGIN_TYPE_ENGINE, CGEMetaEngine);
#else
REGISTER_PLUGIN_STATIC(CGE, PLUGIN_TYPE_ENGINE, CGEMetaEngine);
#endif
