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
#include "engines/metaengine.h"
#include "common/system.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "common/translation.h"

static const PlainGameDescriptor skySetting =
	{"sky", "Beneath a Steel Sky" };

static const ExtraGuiOption skyExtraGuiOption = {
	_s("Floppy intro"),
	_s("Use the floppy version's intro (CD version only)"),
	"alt_intro",
	false
};

struct SkyVersion {
	int dinnerTableEntries;
	int dataDiskSize;
	const char *extraDesc;
	int version;
	const char *guioptions;
};

// TODO: Would be nice if Disk::determineGameVersion() used this table, too.
static const SkyVersion skyVersions[] = {
	{  232, -1, "floppy demo", 272, GUIO1(GUIO_NOSPEECH) }, // German
	{  243, -1, "pc gamer demo", 109, GUIO1(GUIO_NOSPEECH) },
	{  247, -1, "floppy demo", 267, GUIO1(GUIO_NOSPEECH) }, // English
	{ 1404, -1, "floppy", 288, GUIO1(GUIO_NOSPEECH) },
	{ 1413, -1, "floppy", 303, GUIO1(GUIO_NOSPEECH) },
	{ 1445, 8830435, "floppy", 348, GUIO1(GUIO_NOSPEECH) },
	{ 1445, -1, "floppy", 331, GUIO1(GUIO_NOSPEECH) },
	{ 1711, -1, "cd demo", 365, GUIO0() },
	{ 5099, -1, "cd", 368, GUIO0() },
	{ 5097, -1, "cd", 372, GUIO0() },
	{ 0, 0, 0, 0, 0 }
};

class SkyMetaEngineDetection : public MetaEngineDetection {
public:
	const char *getName() const override;
	const char *getOriginalCopyright() const override;

	const char *getEngineId() const override {
		return "sky";
	}

	PlainGameList getSupportedGames() const override;
	const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const override;
	PlainGameDescriptor findGame(const char *gameid) const override;
	DetectedGames detectGames(const Common::FSList &fslist) const override;
};

const char *SkyMetaEngineDetection::getName() const {
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

const ExtraGuiOptions SkyMetaEngineDetection::getExtraGuiOptions(const Common::String &target) const {
	Common::String guiOptions;
	ExtraGuiOptions options;

	if (target.empty()) {
		options.push_back(skyExtraGuiOption);
		return options;
	}

	if (ConfMan.hasKey("guioptions", target)) {
		guiOptions = ConfMan.get("guioptions", target);
		guiOptions = parseGameGUIOptions(guiOptions);
	}

	if (!guiOptions.contains(GUIO_NOSPEECH))
		options.push_back(skyExtraGuiOption);
	return options;
}

PlainGameDescriptor SkyMetaEngineDetection::findGame(const char *gameid) const {
	if (0 == scumm_stricmp(gameid, skySetting.gameId))
		return skySetting;
	return PlainGameDescriptor::empty();
}

DetectedGames SkyMetaEngineDetection::detectGames(const Common::FSList &fslist) const {
	DetectedGames detectedGames;
	bool hasSkyDsk = false;
	bool hasSkyDnr = false;
	int dinnerTableEntries = -1;
	int dataDiskSize = -1;

	// Iterate over all files in the given directory
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (!file->isDirectory()) {
			if (0 == scumm_stricmp("sky.dsk", file->getName().c_str())) {
				Common::File dataDisk;
				if (dataDisk.open(*file)) {
					hasSkyDsk = true;
					dataDiskSize = dataDisk.size();
				}
			}

			if (0 == scumm_stricmp("sky.dnr", file->getName().c_str())) {
				Common::File dinner;
				if (dinner.open(*file)) {
					hasSkyDnr = true;
					dinnerTableEntries = dinner.readUint32LE();
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

		if (sv->dinnerTableEntries) {
			Common::String extra = Common::String::format("v0.0%d %s", sv->version, sv->extraDesc);

			DetectedGame game = DetectedGame(getEngineId(), skySetting.gameId, skySetting.description, Common::UNK_LANG, Common::kPlatformUnknown, extra);
			game.setGUIOptions(sv->guioptions);

			detectedGames.push_back(game);
		} else {
			detectedGames.push_back(DetectedGame(getEngineId(), skySetting.gameId, skySetting.description));
		}
	}

	return detectedGames;
}

REGISTER_PLUGIN_STATIC(SKY_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, SkyMetaEngineDetection);
