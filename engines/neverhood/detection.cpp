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
#include "common/file.h"
#include "common/translation.h"

#include "neverhood/neverhood.h"


namespace Neverhood {

struct NeverhoodGameDescription {
	ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

const char *NeverhoodEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

uint32 NeverhoodEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform NeverhoodEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

Common::Language NeverhoodEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

uint16 NeverhoodEngine::getVersion() const {
	return _gameDescription->version;
}

bool NeverhoodEngine::isDemo() const {
	return _gameDescription->desc.flags & ADGF_DEMO;
}

bool NeverhoodEngine::applyResourceFixes() const {
	return getLanguage() == Common::RU_RUS;
}

}

static const PlainGameDescriptor neverhoodGames[] = {
	{"neverhood", "The Neverhood Chronicles"},
	{0, 0}
};

namespace Neverhood {

static const NeverhoodGameDescription gameDescriptions[] = {

	{
		// Neverhood English version
		{
			"neverhood",
			0,
			AD_ENTRY1s("hd.blb", "22958d968458c9ff221aee38577bb2b2", 4279716),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		0,
		0,
		0,
		0,
	},

	{
		// Neverhood English demo version
		{
			"neverhood",
			"Demo",
			AD_ENTRY1s("nevdemo.blb", "05b735cfb1086892bec79b54dca5545b", 22564568),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
		0,
		0,
		0,
		0,
	},

	{
		// Neverhood earlier English demo version
		{
			"neverhood",
			"Demo",
			AD_ENTRY1s("nevdemo.blb", "9cbc33bc8ebacacfc8071f3e26a9c85f", 22357020),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
		0,
		0,
		0,
		0,
	},

	{
		// Neverhood Russian version. Dyadyushka Risech
		{
			"neverhood",
			"DR",
			AD_ENTRY1s("hd.blb", "787951bf094aad9962291e69a707bdde", 4248635),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		0,
		0,
		0,
		0,
	},

// FIXME: Disabled for now, as it has broken resources that corrupt the heap
// (e.g. the menu header).
#if 0
	{
		// Neverhood Russian version. Fargus
		{
			"neverhood",
			"Fargus",
			AD_ENTRY1s("hd.blb", "c87c69db423f560d3708e9de78751a7f", 4425816),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		0,
		0,
		0,
		0,
	},
#endif

	{ AD_TABLE_END_MARKER, 0, 0, 0, 0 }
};

} // End of namespace Neverhood

static const ExtraGuiOption neverhoodExtraGuiOption1 = {
	_s("Use original save/load screens"),
	_s("Use the original save/load screens instead of the ScummVM ones"),
	"originalsaveload",
	false
};

static const ExtraGuiOption neverhoodExtraGuiOption2 = {
	_s("Skip the Hall of Records storyboard scenes"),
	_s("Allows the player to skip past the Hall of Records storyboard scenes"),
	"skiphallofrecordsscenes",
	false
};

static const ExtraGuiOption neverhoodExtraGuiOption3 = {
	_s("Scale the making of videos to full screen"),
	_s("Scale the making of videos, so that they use the whole screen"),
	"scalemakingofvideos",
	false
};


class NeverhoodMetaEngine : public AdvancedMetaEngine {
public:
	NeverhoodMetaEngine() : AdvancedMetaEngine(Neverhood::gameDescriptions, sizeof(Neverhood::NeverhoodGameDescription), neverhoodGames) {
		_singleId = "neverhood";
		_guiOptions = GUIO2(GUIO_NOSUBTITLES, GUIO_NOMIDI);
	}

	virtual const char *getName() const {
		return "The Neverhood Chronicles";
	}

	virtual const char *getOriginalCopyright() const {
		return "The Neverhood Chronicles (C) The Neverhood, Inc.";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const;
	SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;

};

bool NeverhoodMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSimpleSavesNames);
}

bool Neverhood::NeverhoodEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool NeverhoodMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Neverhood::NeverhoodGameDescription *gd = (const Neverhood::NeverhoodGameDescription *)desc;
	if (gd) {
		*engine = new Neverhood::NeverhoodEngine(syst, gd);
	}
	return gd != 0;
}

const ExtraGuiOptions NeverhoodMetaEngine::getExtraGuiOptions(const Common::String &target) const {
	ExtraGuiOptions options;
	options.push_back(neverhoodExtraGuiOption1);
	options.push_back(neverhoodExtraGuiOption2);
	options.push_back(neverhoodExtraGuiOption3);
	return options;
}

SaveStateList NeverhoodMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Neverhood::NeverhoodEngine::SaveHeader header;
	Common::String pattern = target;
	pattern += ".###";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); file++) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);
		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Neverhood::NeverhoodEngine::readSaveHeader(in, header) == Neverhood::NeverhoodEngine::kRSHENoError) {
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

int NeverhoodMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void NeverhoodMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String filename = Neverhood::NeverhoodEngine::getSavegameFilename(target, slot);
	saveFileMan->removeSavefile(filename.c_str());
}

SaveStateDescriptor NeverhoodMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Neverhood::NeverhoodEngine::getSavegameFilename(target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

	if (in) {
		Neverhood::NeverhoodEngine::SaveHeader header;
		Neverhood::NeverhoodEngine::kReadSaveHeaderError error;

		error = Neverhood::NeverhoodEngine::readSaveHeader(in, header, false);
		delete in;

		if (error == Neverhood::NeverhoodEngine::kRSHENoError) {
			SaveStateDescriptor desc(slot, header.description);

			desc.setDeletableFlag(false);
			desc.setWriteProtectedFlag(false);
			desc.setThumbnail(header.thumbnail);
			int day = (header.saveDate >> 24) & 0xFF;
			int month = (header.saveDate >> 16) & 0xFF;
			int year = header.saveDate & 0xFFFF;
			desc.setSaveDate(year, month, day);
			int hour = (header.saveTime >> 16) & 0xFF;
			int minutes = (header.saveTime >> 8) & 0xFF;
			desc.setSaveTime(hour, minutes);
			desc.setPlayTime(header.playTime * 1000);
			return desc;
		}
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(NEVERHOOD)
	REGISTER_PLUGIN_DYNAMIC(NEVERHOOD, PLUGIN_TYPE_ENGINE, NeverhoodMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(NEVERHOOD, PLUGIN_TYPE_ENGINE, NeverhoodMetaEngine);
#endif
