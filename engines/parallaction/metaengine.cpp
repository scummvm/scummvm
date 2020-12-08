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

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "parallaction/parallaction.h"
#include "parallaction/detection.h"

namespace Parallaction {

int Parallaction::getGameType() const { return _gameDescription->gameType; }
uint32 Parallaction::getFeatures() const { return _gameDescription->features; }
Common::Language Parallaction::getLanguage() const { return _gameDescription->desc.language; }
Common::Platform Parallaction::getPlatform() const { return _gameDescription->desc.platform; }

} // End of namespace Parallaction

class ParallactionMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "parallaction";
	}

    bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
};

bool ParallactionMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave);
}

bool Parallaction::Parallaction::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher);
}

Common::Error ParallactionMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Parallaction::PARALLACTIONGameDescription *gd = (const Parallaction::PARALLACTIONGameDescription *)desc;

	switch (gd->gameType) {
	case Parallaction::GType_Nippon:
		*engine = new Parallaction::Parallaction_ns(syst, gd);
		break;
	case Parallaction::GType_BRA:
		*engine = new Parallaction::Parallaction_br(syst, gd);
		break;
	default:
		return Common::kUnsupportedGameidError;
	}

	return Common::kNoError;
}

SaveStateList ParallactionMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();

	Common::String pattern(ConfMan.getDomain(target)->getVal("gameid") + ".0##");
	Common::StringArray filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);

		if (slotNum >= 0 && slotNum <= 99) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				Common::String saveDesc = in->readLine();
				saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int ParallactionMetaEngine::getMaximumSaveSlot() const { return 99; }

void ParallactionMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = ConfMan.getDomain(target)->getVal("gameid");
	filename += Common::String::format(".0%02d", slot);

	g_system->getSavefileManager()->removeSavefile(filename);
}

#if PLUGIN_ENABLED_DYNAMIC(PARALLACTION)
	REGISTER_PLUGIN_DYNAMIC(PARALLACTION, PLUGIN_TYPE_ENGINE, ParallactionMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(PARALLACTION, PLUGIN_TYPE_ENGINE, ParallactionMetaEngine);
#endif
