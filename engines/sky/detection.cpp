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

#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/md5.h"
#include "engines/advancedDetector.h"
#include "engines/metaengine.h"
#include "common/system.h"
#include "common/file.h"
#include "common/textconsole.h"

static const PlainGameDescriptor skySetting =
	{"sky", "Beneath a Steel Sky" };

struct SkyVersion {
	int dinnerTableEntries;
	int dataDiskSize;
	const char *extraDesc;
	int version;
	const char *guioptions;
};

// TODO: Would be nice if Disk::determineGameVersion() used this table, too.
static const SkyVersion skyVersions[] = {
	{  232, 734425, "Floppy Demo", 272, GUIO1(GUIO_NOSPEECH) }, // German
	{  243, 1328979, "PC Gamer Demo", 109, GUIO1(GUIO_NOSPEECH) },
	{  247, 814147, "Floppy Demo", 267, GUIO1(GUIO_NOSPEECH) }, // English
	{ 1404, 8252443, "Floppy", 288, GUIO1(GUIO_NOSPEECH) },
	{ 1413, 8387069, "Floppy", 303, GUIO1(GUIO_NOSPEECH) },
	{ 1445, 8830435, "Floppy", 348, GUIO1(GUIO_NOSPEECH) },
	{ 1445,	-1, "Floppy", 331, GUIO1(GUIO_NOSPEECH) },
	{ 1711, 26623798, "CD Demo", 365, GUIO0() },
	{ 5099, 72429382, "CD", 368, GUIO0() },
	{ 5097, 72395713, "CD", 372, GUIO0() },
	{ 5097, 73123264, "CD", 372, GUIO0() },
	{ 0, 0, 0, 0, 0 }
};

class SkyMetaEngineDetection : public MetaEngineDetection {
public:
	const char *getEngineName() const override;
	const char *getOriginalCopyright() const override;

	const char *getName() const override {
		return "sky";
	}

	PlainGameList getSupportedGames() const override;
	PlainGameDescriptor findGame(const char *gameid) const override;
	DetectedGames detectGames(const Common::FSList &fslist, uint32 /*skipADFlags*/, bool /*skipIncomplete*/) override;

	uint getMD5Bytes() const override {
		return 0;
	}
};

const char *SkyMetaEngineDetection::getEngineName() const {
	return "Beneath a Steel Sky";
}

const char *SkyMetaEngineDetection::getOriginalCopyright() const {
	return "Beneath a Steel Sky (C) Revolution";
}

PlainGameList SkyMetaEngineDetection::getSupportedGames() const {
	PlainGameList games;
	games.push_back(skySetting);
	return games;
}

PlainGameDescriptor SkyMetaEngineDetection::findGame(const char *gameid) const {
	if (0 == scumm_stricmp(gameid, skySetting.gameId))
		return skySetting;
	return PlainGameDescriptor::empty();
}

DetectedGames SkyMetaEngineDetection::detectGames(const Common::FSList &fslist, uint32 /*skipADFlags*/, bool /*skipIncomplete*/) {
	DetectedGames detectedGames;
	bool hasSkyDsk = false;
	bool hasSkyDnr = false;
	int dinnerTableEntries = -1;
	int dataDiskSize = -1;
	Common::String dataDiskHeadMD5 = "";
	int exeSize = -1; 
	const Common::Language langs[] = {
		Common::EN_GRB,
		Common::DE_DEU,
		Common::FR_FRA,
		Common::EN_USA,
		Common::SE_SWE,
		Common::IT_ITA,
		Common::PT_BRA,
		Common::ES_ESP,
	};
	bool langTable[ARRAYSIZE(langs)];

	memset(langTable, 0, sizeof(langTable));

	// Iterate over all files in the given directory
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (!file->isDirectory()) {
			if (0 == scumm_stricmp("sky.dsk", file->getName().c_str())) {
				Common::File dataDisk;
				if (dataDisk.open(*file)) {
					hasSkyDsk = true;
					dataDiskSize = dataDisk.size();
					if (dataDiskSize == 73123264)
						dataDiskHeadMD5 = Common::computeStreamMD5AsString(dataDisk, 5000);
				}
			}

			if (0 == scumm_stricmp("sky.dnr", file->getName().c_str())) {
				Common::File dinner;
				if (dinner.open(*file)) {
					hasSkyDnr = true;
					dinnerTableEntries = dinner.readUint32LE();
					for (int i = 0; i < dinnerTableEntries; i++) {
						uint16 entry = dinner.readUint16LE();
						dinner.readUint16LE();
						if (entry >= 60600 && entry < 60600 + 8 * ARRAYSIZE(langs) && (entry % 8) == 0)
							langTable[(entry - 60600) / 8] = true;
					}
				}
			}

			if (0 == scumm_stricmp("sky.exe", file->getName().c_str())) {
				Common::File skyExe;
				if (skyExe.open(*file)) {
					exeSize = skyExe.size();
				}
			}
		}
	}

	if (hasSkyDsk && hasSkyDnr) {
		// Match found, add to list of candidates, then abort inner loop.
		// The game detector uses US English by default. We want British
		// English to match the recorded voices better.
		const SkyVersion *sv = skyVersions;
		while (sv->dinnerTableEntries) {
			if (dinnerTableEntries == sv->dinnerTableEntries &&
				(sv->dataDiskSize == dataDiskSize || sv->dataDiskSize == -1)) {
				break;
			}
			++sv;
		}

		DetectedGame game;
		Common::Language lang = Common::Language::UNK_LANG;
		if (dataDiskSize == 73123264 && dataDiskHeadMD5 == "886d6faecd97488be09b73f4f87b92d9")
			lang = Common::Language::RU_RUS;

		if (sv->dinnerTableEntries) {
			Common::String extra = Common::String::format("v0.0%d %s", sv->version, sv->extraDesc);

			game = DetectedGame(getName(), skySetting.gameId, skySetting.description, lang, Common::kPlatformDOS, extra);
			game.setGUIOptions(sv->guioptions);
		} else {
			game = DetectedGame(getName(), skySetting.gameId, skySetting.description);
		}

		if (lang == Common::Language::UNK_LANG) {
			for (uint i = 0; i < ARRAYSIZE(langs); i++) {
				if (langTable[i])
					game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(langs[i]));
			}

			if (exeSize == 575538)
				game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::ZH_TWN));
		} else
			game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(lang));

		detectedGames.push_back(game);
	}

	return detectedGames;
}

REGISTER_PLUGIN_STATIC(SKY_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, SkyMetaEngineDetection);
