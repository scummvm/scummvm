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

#include "nancy/nancy.h"

namespace Nancy {

struct NancyGameDescription {
	ADGameDescription desc;
	GameType gameType;
};

uint32 NancyEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

const char *NancyEngine::getGameId() const {
	return _gameDescription->desc.gameid;
}


const char *const directoryGlobs[] = {
	"game",
	"datafiles",
	0
};

static const PlainGameDescriptor nancyGames[] = {
	// Games
	{"nancy1", "Nancy Drew 1: Secrets Can Kill"},
	{"nancy2", "Nancy Drew 2: Stay Tuned for Danger"},
	{"nancy3", "Nancy Drew 3: Message in a Haunted Mansion"},
	{"nancy4", "Nancy Drew 4: Treasure in the Royal Tower"},
	{"nancy5", "Nancy Drew 5: The Final Scene"},
	{"nancy6", "Nancy Drew 6: Secret of the Scarlet Hand"},
	{0, 0}
};

static const NancyGameDescription gameDescriptions[] = {

	{ // MD5 by waltervn
		{
			"nancy1", 0,
			{
				{"ciftree.dat", 0, "9f89e0b53717515ae0eb82d14ffe0e88", 4317962},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		kGameTypeNancy1
	},
	{ // MD5 by waltervn
		{
			"nancy2", 0,
			{
				{"ciftree.dat", 0, "fa4293d728a1b31407961cd82e86a015", 7784516},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		kGameTypeNancy1
	},
	{ // MD5 by waltervn
		{
			"nancy3", 0,
			{
				{"ciftree.dat", 0, "ee5f8832226567c3610556497c451b09", 16256355},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		kGameTypeNancy1
	},
	{ // MD5 by waltervn
		{
			"nancy3", 0,
			{
				{"data1.hdr", 0, "44906f3d2242f73f16feb8eb6a5161cb", 207327},
				{"data1.cab", 0, "e258cc871e5de5ae004d03c4e31431c7", 1555916},
				{"data2.cab", 0, "364dfd25677026da505f1fa6edd5571f", 137373135},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		kGameTypeNancy1
	},
	{ // MD5 by waltervn
		{
			"nancy4", 0,
			{
				{"ciftree.dat", 0, "e9d45f7db453b0d8f37d202fc979537c", 8742289},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		kGameTypeNancy1
	},
	{ // MD5 by waltervn
		{
			"nancy4", 0,
			{
				{"data1.hdr", 0, "fa4e7a1c411053557169a7731f287012", 263443},
				{"data1.cab", 0, "8f689f92fcca443d6a03faa5de7e2f1c", 1568756},
				{"data2.cab", 0, "5525aa428041f3f1421a6fb5d1b8dba1", 140518758},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		kGameTypeNancy1
	},
	{ // MD5 by waltervn
		{
			"nancy5", 0,
			{
				{"ciftree.dat", 0, "21fa81f322595c3100d8d58d100852d5", 8187692},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		kGameTypeNancy1
	},
	{ // MD5 by waltervn
		{
			"nancy5", 0,
			{
				{"data1.hdr", 0, "261105fba2a1226eedb090c2ce79fd35", 284091},
				{"data1.cab", 0, "7d27bb947ef7305831f1faaf1512a598", 1446301},
				{"data2.cab", 0, "00719c86cab733c1094b27079ce030f3", 145857935},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		kGameTypeNancy1
	},
	{ // MD5 by Strangerke
		{
			"nancy6", 0,
			{
				{"ciftree.dat", 0, "a97b848651fdcf38f5cad7092d98e4a1", 28888006},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		kGameTypeNancy1
	},
	{AD_TABLE_END_MARKER, kGameTypeNone}
};

class NancyMetaEngine : public AdvancedMetaEngine {
public:
	NancyMetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(NancyGameDescription), nancyGames) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getName() const {
		return "Nancy Drew";
	}

	const char *getOriginalCopyright() const {
		return "Nancy Drew Engine copyright Her Interactive, 1995-2012";
	}

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;
	bool hasFeature(MetaEngineFeature f) const;

	int getMaximumSaveSlot() const;
	SaveStateList listSaves(const char *target) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	void removeSaveState(const char *target, int slot) const;
};

bool NancyMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (gd) {
		*engine = new NancyEngine(syst, (const NancyGameDescription *)gd);
		((NancyEngine *)*engine)->initGame((const NancyGameDescription *)gd);
	}
	return gd != 0;
}

bool NancyMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSavesSupportCreationDate);
}

int NancyMetaEngine::getMaximumSaveSlot() const { return 99; }

SaveStateList NancyMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += "-??.SAV";

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)

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

	return saveList;
}

SaveStateDescriptor NancyMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
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
		char saveName[256];
		file->read(saveName, saveNameLength);
		saveName[saveNameLength] = 0;

		SaveStateDescriptor desc(slot, saveName);

		Graphics::Surface *const thumbnail = Graphics::loadThumbnail(*file);
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

		// Slot 0 is used for the 'restart game' save in all Nancy games, thus
		// we prevent it from being deleted.
		desc.setDeletableFlag(slot != 0);
		desc.setWriteProtectedFlag(slot == 0);

		delete file;
		return desc;
	}
	return SaveStateDescriptor();
}

void NancyMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s-%02d.SAV", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}
} // End of namespace Nancy

#if PLUGIN_ENABLED_DYNAMIC(NANCY)
REGISTER_PLUGIN_DYNAMIC(NANCY, PLUGIN_TYPE_ENGINE, Nancy::NancyMetaEngine);
#else
REGISTER_PLUGIN_STATIC(NANCY, PLUGIN_TYPE_ENGINE, Nancy::NancyMetaEngine);
#endif

namespace Nancy {

void NancyEngine::initGame(const NancyGameDescription *gd) {
	_gameType = gd->gameType;
	_platform = gd->desc.platform;
}

} // End of namespace Nancy
