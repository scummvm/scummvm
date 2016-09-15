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

#include "engines/advancedDetector.h"
#include "common/system.h"
#include "common/savefile.h"

#include "wage/wage.h"

namespace Wage {

const char *WageEngine::getGameFile() const {
	return _gameDescription->filesDescriptions[0].fileName;
}

}

static const PlainGameDescriptor wageGames[] = {
	{"afm", "Another Fine Mess"},
	{"amot", "A Mess O' Trouble"},
	{"cantitoe", "Camp Cantitoe"},
	{"drakmythcastle", "Drakmyth Castle"},
	{"raysmaze", "Ray's Maze"},
	{"scepters", "Enchanted Scepters"},
	{"twisted", "Twisted!"},
	{"wage", "WAGE"},
	{0, 0}
};

#include "wage/detection_tables.h"

class WageMetaEngine : public AdvancedMetaEngine {
public:
	WageMetaEngine() : AdvancedMetaEngine(Wage::gameDescriptions, sizeof(ADGameDescription), wageGames) {
		_md5Bytes = 2 * 1024 * 1024;
		_singleId = "wage";
		_guiOptions = GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI);
	}

	virtual const char *getEngineId() const {
		return "wage";
	}


	virtual const char *getName() const {
		return "World Adventure Game Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "World Builder (C) Silicon Beach Software";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
};

bool WageMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSimpleSavesNames);
}

bool Wage::WageEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool WageMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Wage::WageEngine(syst, desc);
	}
	return desc != 0;
}

SaveStateList WageMetaEngine::listSaves(const char *target) const {
	const uint32 WAGEflag = MKTAG('W','A','G','E');
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	char saveDesc[128] = {0};
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
				saveDesc[0] = 0;
				in->seek(in->size() - 8);
				uint32 offset = in->readUint32BE();
				uint32 type = in->readUint32BE();
				if (type == WAGEflag) {
					in->seek(offset);

					type = in->readUint32BE();
					if (type == WAGEflag) {
						in->read(saveDesc, 127);
					}
				}
				saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int WageMetaEngine::getMaximumSaveSlot() const { return 999; }

void WageMetaEngine::removeSaveState(const char *target, int slot) const {
	g_system->getSavefileManager()->removeSavefile(Common::String::format("%s.%03d", target, slot));
}

#if PLUGIN_ENABLED_DYNAMIC(WAGE)
	REGISTER_PLUGIN_DYNAMIC(WAGE, PLUGIN_TYPE_ENGINE, WageMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(WAGE, PLUGIN_TYPE_ENGINE, WageMetaEngine);
#endif

namespace Wage {

bool WageEngine::canLoadGameStateCurrently() {
	return true;
}

bool WageEngine::canSaveGameStateCurrently() {
	return true;
}

} // End of namespace Wage
