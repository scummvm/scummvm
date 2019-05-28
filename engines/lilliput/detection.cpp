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

#include "engines/advancedDetector.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "lilliput/lilliput.h"

namespace Lilliput {

struct LilliputGameDescription {
	ADGameDescription desc;
	GameType gameType;
};

uint32 LilliputEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

const char *LilliputEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}


static const PlainGameDescriptor lilliputGames[] = {
	// Games
	{"robin", "Adventures of Robin Hood"},
	{"rome", "Rome: Pathway to Power"},
	{0, 0}
};

static const LilliputGameDescription gameDescriptions[] = {

	// Robin Hood English
	{
		{
			"robin", 0,
			{
				{"erules.prg", 0, "92aaf84693a8948497ad57864fa31c2a", 71010},
				{"isomap.dta", 0, "bad97eae03a4db3e99565e39b0b3c06a", 16384},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		kGameTypeRobin
	},
	// Robin Hood French
	{
		{
			"robin", 0,
			{
				{"frules.prg", 0, "cf076c5ebfe8b3571e74a6a46d79426f", 76660},
				{"isomap.dta", 0, "bad97eae03a4db3e99565e39b0b3c06a", 16384},
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		kGameTypeRobin
	},
	// Robin Hood German
	{
		{
			"robin", 0,
			{
				{"grules.prg", 0, "b53b7353dc1e841b206a64851e7bc58c", 78050},
				{"isomap.dta", 0, "bad97eae03a4db3e99565e39b0b3c06a", 16384},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		kGameTypeRobin
	},
	// Robin Hood Italian
	{
		{
			"robin", 0,
			{
				{"irules.prg", 0, "4d69ed3cda1e1d73585905517ea705d1", 75654},
				{"isomap.dta", 0, "bad97eae03a4db3e99565e39b0b3c06a", 16384},
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		kGameTypeRobin
	},
	{AD_TABLE_END_MARKER, kGameTypeNone}
};

class LilliputMetaEngine : public AdvancedMetaEngine {
public:
	LilliputMetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(LilliputGameDescription), lilliputGames) {
	}

	const char *getName() const {
		return "Lilliput";
	}

	const char *getOriginalCopyright() const {
		return "Lilliput (C) S.L.Grand, Brainware, 1991-1992";
	}

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;
	bool hasFeature(MetaEngineFeature f) const;

	int getMaximumSaveSlot() const;
	SaveStateList listSaves(const char *target) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	void removeSaveState(const char *target, int slot) const;
};

bool LilliputMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (gd) {
		*engine = new LilliputEngine(syst, (const LilliputGameDescription *)gd);
		((LilliputEngine *)*engine)->initGame((const LilliputGameDescription *)gd);
	}
	return gd != 0;
}

bool LilliputMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate);
}

int LilliputMetaEngine::getMaximumSaveSlot() const {
	return 99;
}

SaveStateList LilliputMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += "-##.SAV";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	char slot[3];
	int slotNum = 0;
	for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
		slot[0] = filename->c_str()[filename->size() - 6];
		slot[1] = filename->c_str()[filename->size() - 5];
		slot[2] = '\0';
		// Obtain the last 2 digits of the filename (without extension), since they correspond to the save slot
		slotNum = atoi(slot);
		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::InSaveFile *file = saveFileMan->openForLoading(*filename);
			if (file) {
				int saveVersion = file->readByte();

				if (saveVersion != kSavegameVersion) {
					warning("Savegame of incompatible version");
					delete file;
					continue;
				}

				// read name
				uint16 nameSize = file->readUint16BE();
				if (nameSize >= 255) {
					delete file;
					continue;
				}
				char name[256];
				file->read(name, nameSize);
				name[nameSize] = 0;

				saveList.push_back(SaveStateDescriptor(slotNum, name));
				delete file;
			}
		}
	}

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor LilliputMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s-%02d.SAV", target, slot);
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(fileName);

	if (file) {
		int saveVersion = file->readByte();

		if (saveVersion != kSavegameVersion) {
			warning("Savegame of incompatible version");
			delete file;
			return SaveStateDescriptor();
		}

		uint32 saveNameLength = file->readUint16BE();
		Common::String saveName;
		for (uint32 i = 0; i < saveNameLength; ++i) {
			char curChr = file->readByte();
			saveName += curChr;
		}

		SaveStateDescriptor desc(slot, saveName);

		Graphics::Surface *thumbnail;
		if (!Graphics::loadThumbnail(*file, thumbnail)) {
			delete file;
			return SaveStateDescriptor();
		}
		desc.setThumbnail(thumbnail);

		desc.setDeletableFlag(true);
		desc.setWriteProtectedFlag(false);

		uint32 saveDate = file->readUint32BE();
		uint16 saveTime = file->readUint16BE();

		int day = (saveDate >> 24) & 0xFF;
		int month = (saveDate >> 16) & 0xFF;
		int year = saveDate & 0xFFFF;

		desc.setSaveDate(year, month, day);

		int hour = (saveTime >> 8) & 0xFF;
		int minutes = saveTime & 0xFF;

		desc.setSaveTime(hour, minutes);

		// Slot 0 is used for the 'restart game' save in all Robin games, thus
		// we prevent it from being deleted.
		desc.setDeletableFlag(slot != 0);
		desc.setWriteProtectedFlag(slot == 0);

		delete file;
		return desc;
	}
	return SaveStateDescriptor();
}

void LilliputMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s-%02d.SAV", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}
} // End of namespace Lilliput

#if PLUGIN_ENABLED_DYNAMIC(LILLIPUT)
REGISTER_PLUGIN_DYNAMIC(LILLIPUT, PLUGIN_TYPE_ENGINE, Lilliput::LilliputMetaEngine);
#else
REGISTER_PLUGIN_STATIC(LILLIPUT, PLUGIN_TYPE_ENGINE, Lilliput::LilliputMetaEngine);
#endif

namespace Lilliput {

void LilliputEngine::initGame(const LilliputGameDescription *gd) {
	_gameType = gd->gameType;
	_platform = gd->desc.platform;
}

} // End of namespace Lilliput
