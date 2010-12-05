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
 * $URL$
 * $Id$
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/system.h"

#include "picture/picture.h"


namespace Picture {

struct PictureGameDescription {
	ADGameDescription desc;
};

uint32 PictureEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language PictureEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

}

static const PlainGameDescriptor pictureGames[] = {
	{"toltecs", "3 Skulls of the Toltecs"},
	{0, 0}
};


namespace Picture {

static const PictureGameDescription gameDescriptions[] = {

	{
		// 3 Skulls of the Toltecs English version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "05472037e9cfde146e953c434e74f0f4", 337643527),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
	},

	{
		// 3 Skulls of the Toltecs Russian version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "ba1742d3193b68ceb9434e2ab7a09a9b", 391462783),
			Common::RU_RUS,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
	},

	{
		// 3 Skulls of the Toltecs German version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "1a3292bad8e0bb5701800c73531dd75e", 345176617),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
	},

	{
		// 3 Skulls of the Toltecs German Demo version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "1c85e82712d24f1d5c1ea2a66ddd75c2", 47730038),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
	},

	{
		// 3 Skulls of the Toltecs French version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "4fb845635cbdac732453fe23be350df9", 327269545),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
	},

	{
		// 3 Skulls of the Toltecs Spanish version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "479f468beccc1b0ce5873ec523d1380e", 308391018),
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Picture

static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Picture::gameDescriptions,
	// Size of that superset structure
	sizeof(Picture::PictureGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	pictureGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"toltecs",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0,
	// Additional GUI options (for every game}
	Common::GUIO_NONE,
	// Maximum directory depth
	1,
	// List of directory globs
	0
};

class PictureMetaEngine : public AdvancedMetaEngine {
public:
	PictureMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "Picture Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Picture Engine Revistronic (C) 1996";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool PictureMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
//		(f == kSupportsDeleteSave) ||
	   	(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail);
}

bool Picture::PictureEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool PictureMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Picture::PictureGameDescription *gd = (const Picture::PictureGameDescription *)desc;
	if (gd) {
		*engine = new Picture::PictureEngine(syst, gd);
	}
	return gd != 0;
}

SaveStateList PictureMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Picture::PictureEngine::SaveHeader header;
	Common::String pattern = target;
	pattern += ".???";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); file++) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Picture::PictureEngine::readSaveHeader(in, false, header) == Picture::PictureEngine::kRSHENoError) {
					saveList.push_back(SaveStateDescriptor(slotNum, header.description));
				}
				delete in;
			}
		}
	}

	return saveList;
}

int PictureMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void PictureMetaEngine::removeSaveState(const char *target, int slot) const {
	// Slot 0 can't be deleted, it's for restarting the game(s)
	if (slot == 0)
		return;

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String filename = Picture::PictureEngine::getSavegameFilename(target, slot);

	saveFileMan->removeSavefile(filename.c_str());

	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".???";
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		// Rename every slot greater than the deleted slot,
		// Also do not rename quicksaves.
		if (slotNum > slot && slotNum < 990) {
			// FIXME: Our savefile renaming done here is inconsitent with what we do in
			// GUI_v2::deleteMenu. While here we rename every slot with a greater equal
			// number of the deleted slot to deleted slot, deleted slot + 1 etc.,
			// we only rename the following slots in GUI_v2::deleteMenu until a slot
			// is missing.
			saveFileMan->renameSavefile(file->c_str(), filename.c_str());

			filename = Picture::PictureEngine::getSavegameFilename(target, ++slot);
		}
	}

}

SaveStateDescriptor PictureMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Picture::PictureEngine::getSavegameFilename(target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

	if (in) {
		Picture::PictureEngine::SaveHeader header;
		Picture::PictureEngine::kReadSaveHeaderError error;

		error = Picture::PictureEngine::readSaveHeader(in, true, header);
		delete in;

		if (error == Picture::PictureEngine::kRSHENoError) {
			SaveStateDescriptor desc(slot, header.description);

			desc.setDeletableFlag(false);
			desc.setWriteProtectedFlag(false);
			desc.setThumbnail(header.thumbnail);

			return desc;
		}
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(PICTURE)
	REGISTER_PLUGIN_DYNAMIC(PICTURE, PLUGIN_TYPE_ENGINE, PictureMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(PICTURE, PLUGIN_TYPE_ENGINE, PictureMetaEngine);
#endif
