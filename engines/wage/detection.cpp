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
	{"scepters", "Enchanced Scepters"},
	{"wage", "WAGE"},
	{0, 0}
};

namespace Wage {

static const ADGameDescription gameDescriptions[] = {
	{
		"afm",
		"v1.8",
		AD_ENTRY1s("Another Fine Mess 1.8", "8e5aa915f3253efb2aab52435647b25e", 1456000),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"amot",
		"v1.8",
		AD_ENTRY1s("A Mess O' Trouble 1.8", "b3ef53afed282671b704e45df829350c", 1895552),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"cantitoe",
		"",
		AD_ENTRY1s("Camp Cantitoe", "098aa5c11c58e1ef274a30a9e01b4755", 621440),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"wage",
		"Escape from School!",
		AD_ENTRY1s("Escape from School!", "a854be48d4af20126d18a9cad93a969b", 51840),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_USEEXTRAASTITLE,
		GUIO0()
	},
	{
		"wage",
		"Queen Quest",
		AD_ENTRY1s("Queen Quest", "730605d312efedb5e3ff108522fcac18", 59776),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_USEEXTRAASTITLE,
		GUIO0()
	},
	{
		"scepters",
		"",
		AD_ENTRY1s("Scepters", "b80bff315897776dda7689cdf829fab4", 360832),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"wage",
		"Zoony",
		AD_ENTRY1s("Zoony", "e6cc8a914a4215dafbcce6315dd12cf5", 160256),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_USEEXTRAASTITLE,
		GUIO0()
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Wage

class WageMetaEngine : public AdvancedMetaEngine {
public:
	WageMetaEngine() : AdvancedMetaEngine(Wage::gameDescriptions, sizeof(ADGameDescription), wageGames) {
		_singleid = "wage";
		_guioptions = GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI);
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
		(f == kSupportsDeleteSave);
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
	char saveDesc[31];
	Common::String pattern = target;
	pattern += ".???";

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				uint32 type = in->readUint32BE();
				if (type == WAGEflag)
					in->read(saveDesc, 31);
				saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
				delete in;
			}
		}
	}

	return saveList;
}

int WageMetaEngine::getMaximumSaveSlot() const { return 999; }

void WageMetaEngine::removeSaveState(const char *target, int slot) const {
	char fileName[MAXPATHLEN];
	sprintf(fileName, "%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

#if PLUGIN_ENABLED_DYNAMIC(WAGE)
	REGISTER_PLUGIN_DYNAMIC(WAGE, PLUGIN_TYPE_ENGINE, WageMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(WAGE, PLUGIN_TYPE_ENGINE, WageMetaEngine);
#endif

namespace Wage {

bool WageEngine::canLoadGameStateCurrently() {
	return false;
}

bool WageEngine::canSaveGameStateCurrently() {
	return false;
}

} // End of namespace Wage
