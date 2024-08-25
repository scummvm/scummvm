/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/advancedDetector.h"

#include "common/system.h"
#include "common/savefile.h"
#include "common/file.h"
#include "common/translation.h"

#include "graphics/thumbnail.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"

#include "adl/adl.h"
#include "adl/detection.h"
#include "adl/disk_image_helpers.h"

namespace Adl {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_NTSC,
		{
			_s("TV emulation"),
			_s("Emulate composite output to an NTSC TV"),
			"ntsc",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_COLOR_DEFAULT_OFF,
		{
			_s("Color graphics"),
			_s("Use color graphics instead of monochrome"),
			"color",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_COLOR_DEFAULT_ON,
		{
			_s("Color graphics"),
			_s("Use color graphics instead of monochrome"),
			"color",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_SCANLINES,
		{
			_s("Show scanlines"),
			_s("Darken every other scanline to mimic the look of a CRT"),
			"scanlines",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_MONO_TEXT,
		{
			_s("Always use sharp monochrome text"),
			_s("Do not emulate NTSC artifacts for text"),
			"monotext",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_APPLE2E_CURSOR,
		{
			_s("Use checkered cursor"),
			_s("Use the checkered cursor from later Apple II models"),
			"apple2e_cursor",
			false,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

Common::Path getDiskImageName(const AdlGameDescription &adlDesc, byte volume) {
	const ADGameDescription &desc = adlDesc.desc;
	for (uint i = 0; desc.filesDescriptions[i].fileName; ++i) {
		const ADGameFileDescription &fDesc = desc.filesDescriptions[i];

		if (fDesc.fileType == volume) {
			for (uint e = 0; e < ARRAYSIZE(diskImageExts); ++e) {
				if (diskImageExts[e].platform == desc.platform) {
					Common::Path testFileName(fDesc.fileName);
					testFileName.appendInPlace(diskImageExts[e].extension);
					if (Common::File::exists(testFileName))
						return testFileName;
				}
			}

			error("Failed to find disk image '%s'", fDesc.fileName);
		}
	}

	error("Disk volume %d not found", volume);
}

GameType getGameType(const AdlGameDescription &adlDesc) {
	return adlDesc.gameType;
}

GameVersion getGameVersion(const AdlGameDescription &adlDesc) {
	return adlDesc.version;
}

Common::Language getLanguage(const AdlGameDescription &adlDesc) {
	return adlDesc.desc.language;
}

Common::Platform getPlatform(const AdlGameDescription &adlDesc) {
	return adlDesc.desc.platform;
}

class AdlMetaEngine : public AdvancedMetaEngine<AdlGameDescription> {
public:
	const char *getName() const override {
		return "adl";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	int getAutosaveSlot() const override { return 15; }
	int getMaximumSaveSlot() const override { return 15; }
	SaveStateList listSaves(const char *target) const override;
	void removeSaveState(const char *target, int slot) const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const AdlGameDescription *adlGd) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool AdlMetaEngine::hasFeature(MetaEngineFeature f) const {
	switch(f) {
	case kSupportsListSaves:
	case kSupportsLoadingDuringStartup:
	case kSupportsDeleteSave:
	case kSavesSupportMetaInfo:
	case kSavesSupportThumbnail:
	case kSavesSupportCreationDate:
	case kSavesSupportPlayTime:
		return true;
	default:
		return false;
	}
}

SaveStateDescriptor AdlMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.s%02d", target, slot);
	Common::InSaveFile *inFile = g_system->getSavefileManager()->openForLoading(fileName);

	if (!inFile)
		return SaveStateDescriptor();

	if (inFile->readUint32BE() != MKTAG('A', 'D', 'L', ':')) {
		delete inFile;
		return SaveStateDescriptor();
	}

	byte saveVersion = inFile->readByte();
	if (saveVersion != SAVEGAME_VERSION) {
		delete inFile;
		return SaveStateDescriptor();
	}

	char name[SAVEGAME_NAME_LEN] = { };
	inFile->read(name, sizeof(name) - 1);
	inFile->readByte();

	if (inFile->eos() || inFile->err()) {
		delete inFile;
		return SaveStateDescriptor();
	}

	SaveStateDescriptor sd(this, slot, name);

	int year = inFile->readUint16BE();
	int month = inFile->readByte();
	int day = inFile->readByte();
	sd.setSaveDate(year + 1900, month + 1, day);

	int hour = inFile->readByte();
	int minutes = inFile->readByte();
	sd.setSaveTime(hour, minutes);

	uint32 playTime = inFile->readUint32BE();
	sd.setPlayTime(playTime);

	if (inFile->eos() || inFile->err()) {
		delete inFile;
		return SaveStateDescriptor();
	}

	Graphics::Surface *thumbnail;
	if (!Graphics::loadThumbnail(*inFile, thumbnail)) {
		delete inFile;
		return SaveStateDescriptor();
	}
	sd.setThumbnail(thumbnail);

	delete inFile;
	return sd;
}

SaveStateList AdlMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray files = saveFileMan->listSavefiles(Common::String(target) + ".s##");

	SaveStateList saveList;

	for (uint i = 0; i < files.size(); ++i) {
		const Common::String &fileName = files[i];
		Common::InSaveFile *inFile = saveFileMan->openForLoading(fileName);
		if (!inFile) {
			warning("Cannot open save file '%s'", fileName.c_str());
			continue;
		}

		if (inFile->readUint32BE() != MKTAG('A', 'D', 'L', ':')) {
			warning("No header found in '%s'", fileName.c_str());
			delete inFile;
			continue;
		}

		byte saveVersion = inFile->readByte();
		if (saveVersion != SAVEGAME_VERSION) {
			warning("Unsupported save game version %i found in '%s'", saveVersion, fileName.c_str());
			delete inFile;
			continue;
		}

		char name[SAVEGAME_NAME_LEN] = { };
		inFile->read(name, sizeof(name) - 1);
		delete inFile;

		int slotNum = atoi(fileName.c_str() + fileName.size() - 2);
		SaveStateDescriptor sd(this, slotNum, name);
		saveList.push_back(sd);
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

void AdlMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.s%02d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

Engine *HiRes1Engine_create(OSystem *syst, const AdlGameDescription *gd);
Engine *HiRes2Engine_create(OSystem *syst, const AdlGameDescription *gd);
Engine *HiRes0Engine_create(OSystem *syst, const AdlGameDescription *gd);
Engine *HiRes3Engine_create(OSystem *syst, const AdlGameDescription *gd);
Engine *HiRes4Engine_create(OSystem *syst, const AdlGameDescription *gd);
Engine *HiRes5Engine_create(OSystem *syst, const AdlGameDescription *gd);
Engine *HiRes6Engine_create(OSystem *syst, const AdlGameDescription *gd);

Common::Error AdlMetaEngine::createInstance(OSystem *syst, Engine **engine, const AdlGameDescription *adlGd) const {
	switch (adlGd->gameType) {
	case GAME_TYPE_HIRES1:
		*engine = HiRes1Engine_create(syst, adlGd);
		break;
	case GAME_TYPE_HIRES2:
		*engine = HiRes2Engine_create(syst, adlGd);
		break;
	case GAME_TYPE_HIRES0:
		*engine = HiRes0Engine_create(syst, adlGd);
		break;
	case GAME_TYPE_HIRES3:
		*engine = HiRes3Engine_create(syst, adlGd);
		break;
	case GAME_TYPE_HIRES4:
		*engine = HiRes4Engine_create(syst, adlGd);
		break;
	case GAME_TYPE_HIRES5:
		*engine = HiRes5Engine_create(syst, adlGd);
		break;
	case GAME_TYPE_HIRES6:
		*engine = HiRes6Engine_create(syst, adlGd);
		break;
	default:
		return Common::kUnsupportedGameidError;
	}

	return Common::kNoError;
}

Common::KeymapArray AdlMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;

	Keymap *engineKeymap = new Keymap(Keymap::kKeymapTypeGame, "adl", "ADL");

	Action *act;

	act = new Action("QUIT", _("Quit"));
	act->setCustomEngineActionEvent(kADLActionQuit);
	act->addDefaultInputMapping("C+q");
	engineKeymap->addAction(act);

	return Keymap::arrayOf(engineKeymap);
}

} // End of namespace Adl

#if PLUGIN_ENABLED_DYNAMIC(ADL)
	REGISTER_PLUGIN_DYNAMIC(ADL, PLUGIN_TYPE_ENGINE, Adl::AdlMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ADL, PLUGIN_TYPE_ENGINE, Adl::AdlMetaEngine);
#endif

