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

#include "bbvs/bbvs.h"

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "base/plugins.h"
#include "graphics/thumbnail.h"

static const PlainGameDescriptor bbvsGames[] = {
	{ "bbvs", "Beavis and Butt-head in Virtual Stupidity" },
	{ 0, 0 }
};

namespace Bbvs {

static const ADGameDescription gameDescriptions[] = {
	{
		"bbvs",
		0,
		AD_ENTRY1s("vspr0001.vnm", "7ffe9b9e7ca322db1d48e86f5130578e", 1166628),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"bbvs",
		0,
		AD_ENTRY1s("vspr0001.vnm", "91c76b1048f93208cd7b1a05ebccb408", 1176976),
		Common::RU_RUS,
		Common::kPlatformWindows,
		GF_GUILANGSWITCH | ADGF_NO_FLAGS,
		GUIO0()
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Bbvs

static const char * const directoryGlobs[] = {
	"vnm",
	0
};

class BbvsMetaEngine : public AdvancedMetaEngine {
public:
	BbvsMetaEngine() : AdvancedMetaEngine(Bbvs::gameDescriptions, sizeof(ADGameDescription), bbvsGames) {
		_singleId = "bbvs";
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
	}

	virtual const char *getName() const {
		return "MTV's Beavis and Butt-head in Virtual Stupidity";
	}

	virtual const char *getOriginalCopyright() const {
		return "(C) 1995 Viacom New Media";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual int getMaximumSaveSlot() const;
	virtual SaveStateList listSaves(const char *target) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	virtual void removeSaveState(const char *target, int slot) const;
};

bool BbvsMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSavesSupportCreationDate) ||
		(f == kSimpleSavesNames);
}

void BbvsMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

int BbvsMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

SaveStateList BbvsMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Bbvs::BbvsEngine::SaveHeader header;
	Common::String pattern = target;
	pattern += ".###";
	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);
		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Bbvs::BbvsEngine::readSaveHeader(in, header) == Bbvs::BbvsEngine::kRSHENoError) {
					saveList.push_back(SaveStateDescriptor(slotNum, header.description));
				}
				delete in;
			}
		}
	}
	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor BbvsMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Bbvs::BbvsEngine::getSavegameFilename(target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());
	if (in) {
		Bbvs::BbvsEngine::SaveHeader header;
		Bbvs::BbvsEngine::kReadSaveHeaderError error;
		error = Bbvs::BbvsEngine::readSaveHeader(in, header, false);
		delete in;
		if (error == Bbvs::BbvsEngine::kRSHENoError) {
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

bool BbvsMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Bbvs::BbvsEngine(syst, desc);
	}
	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(BBVS)
	REGISTER_PLUGIN_DYNAMIC(BBVS, PLUGIN_TYPE_ENGINE, BbvsMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(BBVS, PLUGIN_TYPE_ENGINE, BbvsMetaEngine);
#endif
