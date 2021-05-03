/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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
 */

#include "engines/metaengine.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/gui_options.h"
#include "common/savefile.h"
#include "common/system.h"

#include "sword2/sword2.h"
#include "sword2/saveload.h"
#include "sword2/detection_internal.h"

class Sword2MetaEngine : public MetaEngine {
public:
	const char *getName() const override {
		return "sword2";
	}

	bool hasFeature(MetaEngineFeature f) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;

	Common::Error createInstance(OSystem *syst, Engine **engine) const override;
};

bool Sword2MetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSimpleSavesNames);
}

bool Sword2::Sword2Engine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsSubtitleOptions) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsLoadingDuringRuntime);
}

SaveStateList Sword2MetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	char saveDesc[SAVE_DESCRIPTION_LEN];
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				in->readUint32LE();
				in->read(saveDesc, SAVE_DESCRIPTION_LEN);
				saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int Sword2MetaEngine::getMaximumSaveSlot() const { return 999; }

void Sword2MetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = target;
	filename += Common::String::format(".%03d", slot);

	g_system->getSavefileManager()->removeSavefile(filename);
}

Common::Error Sword2MetaEngine::createInstance(OSystem *syst, Engine **engine) const {
	assert(syst);
	assert(engine);

	Common::FSList fslist;
	Common::FSNode dir(ConfMan.get("path"));
	if (!dir.getChildren(fslist, Common::FSNode::kListAll)) {
		return Common::kNoGameDataFoundError;
	}

	// Invoke the detector
	Common::String gameid = ConfMan.get("gameid");
	DetectedGames detectedGames = detectGamesImpl(fslist);

	for (uint i = 0; i < detectedGames.size(); i++) {
		if (detectedGames[i].gameId == gameid) {
			*engine = new Sword2::Sword2Engine(syst);
			return Common::kNoError;
		}
	}

	return Common::kNoGameDataFoundError;
}

#if PLUGIN_ENABLED_DYNAMIC(SWORD2)
	REGISTER_PLUGIN_DYNAMIC(SWORD2, PLUGIN_TYPE_ENGINE, Sword2MetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SWORD2, PLUGIN_TYPE_ENGINE, Sword2MetaEngine);
#endif
