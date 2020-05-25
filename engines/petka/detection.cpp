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
#include "common/savefile.h"

#include "engines/advancedDetector.h"

#include "petka/petka.h"

static const PlainGameDescriptor petkaGames[] = {
	{"petka_demo", "Red Comrades Demo"},
	{"petka1", "Red Comrades 1: Save the Galaxy"},
	{"petka2", "Red Comrades 2: For the Great Justice"},
	{0, 0}
};

#include "detection_tables.h"

class PetkaMetaEngine : public AdvancedMetaEngine {
public:
	PetkaMetaEngine() : AdvancedMetaEngine(Petka::gameDescriptions, sizeof(ADGameDescription), petkaGames) {
		_gameIds = petkaGames;
		_maxScanDepth = 2;
	}

	const char *getEngineId() const override {
		return "petka";
	}

	virtual const char *getName() const override {
		return "Red Comrades";
	}

	virtual const char *getOriginalCopyright() const override {
		return "Red Comrades (C) S.K.I.F";
	}

	virtual bool hasFeature(MetaEngineFeature f) const override;
	virtual int getMaximumSaveSlot() const override { return 18; }
	virtual SaveStateList listSaves(const char *target) const override;
	virtual void removeSaveState(const char *target, int slot) const override;
	virtual SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

bool PetkaMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime);
}

SaveStateList PetkaMetaEngine::listSaves(const char *target) const {
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
				if (Petka::readSaveHeader(*in.get(), desc))
					saveList.push_back(desc);
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

void PetkaMetaEngine::removeSaveState(const char *target, int slot) const {
	g_system->getSavefileManager()->removeSavefile(Petka::generateSaveName(slot, target));
}

SaveStateDescriptor PetkaMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::ScopedPtr<Common::InSaveFile> f(g_system->getSavefileManager()->openForLoading(Petka::generateSaveName(slot, target)));

	if (f) {
		SaveStateDescriptor desc;
		if (!Petka::readSaveHeader(*f.get(), desc, false))
			return SaveStateDescriptor();

		return desc;
	}

	return SaveStateDescriptor();
}

bool PetkaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc)
		*engine = new Petka::PetkaEngine(syst, desc);

	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(PETKA)
	REGISTER_PLUGIN_DYNAMIC(PETKA, PLUGIN_TYPE_ENGINE, PetkaMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(PETKA, PLUGIN_TYPE_ENGINE, PetkaMetaEngine);
#endif
