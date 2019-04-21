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

#include "common/system.h"

#include "engines/advancedDetector.h"

#include "pink/pink.h"

namespace Pink {

Common::Language PinkEngine::getLanguage() const {
	return _desc->language;
}

} // End of Namespace Pink

static const PlainGameDescriptor pinkGames[] = {
	{"peril", "The Pink Panther: Passport to Peril"},
	{"pokus", "The Pink Panther: Hokus Pokus Pink"},
	{0, 0}
};

#include "detection_tables.h"

static const char *directoryGlobs[] = {
	"install",
	nullptr
};


class PinkMetaEngine : public AdvancedMetaEngine {
public:
	PinkMetaEngine() : AdvancedMetaEngine(Pink::gameDescriptions, sizeof(ADGameDescription), pinkGames) {
		_gameIds = pinkGames;
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	virtual const char *getName() const {
		return "Pink Panther";
	}

	virtual const char *getOriginalCopyright() const {
		return "Pink Panther (C) Wanderlust Interactive";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual int getMaximumSaveSlot() const { return 99; }
	virtual SaveStateList listSaves(const char *target) const;
	virtual void removeSaveState(const char *target, int slot) const;
	virtual SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool PinkMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSimpleSavesNames);
}

SaveStateList PinkMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String pattern = Common::String::format("%s.s##", target);
	Common::StringArray filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);
		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::ScopedPtr<Common::InSaveFile> in(saveFileMan->openForLoading(*file));
			if (in) {
				SaveStateDescriptor desc;
				desc.setSaveSlot(slotNum);
				if (Pink::readSaveHeader(*in.get(), desc))
					saveList.push_back(desc);
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

void PinkMetaEngine::removeSaveState(const char *target, int slot) const {
	g_system->getSavefileManager()->removeSavefile(Pink::generateSaveName(slot, target));
}

SaveStateDescriptor PinkMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::ScopedPtr<Common::InSaveFile> f(g_system->getSavefileManager()->openForLoading(Pink::generateSaveName(slot, target)));

	if (f) {
		SaveStateDescriptor desc;
		if (!Pink::readSaveHeader(*f.get(), desc, false))
			return SaveStateDescriptor();

		return desc;
	}

	return SaveStateDescriptor();
}

bool PinkMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc)
		*engine = new Pink::PinkEngine(syst, desc);

	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(PINK)
REGISTER_PLUGIN_DYNAMIC(PINK, PLUGIN_TYPE_ENGINE, PinkMetaEngine);
#else
REGISTER_PLUGIN_STATIC(PINK, PLUGIN_TYPE_ENGINE, PinkMetaEngine);
#endif
