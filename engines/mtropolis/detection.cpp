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
#include "engines/advancedDetector.h"

#include "common/config-manager.h"
#include "common/file.h"

#include "mtropolis/detection.h"

static const PlainGameDescriptor mTropolisGames[] = {
	{"obsidian", "Obsidian"},
	{"mti", "Muppet Treasure Island"},
	{"albert1", "Uncle Albert's Magical Album"},
	{"albert2", "Uncle Albert's Fabulous Voyage"},
	{"albert3", "Uncle Albert's Mysterious Island"},
	{"spqr", "SPQR: The Empire's Darkest Hour"},
	{"sttgs", "Star Trek: The Game Show"},
	{"unit", "Unit: Re-Booted"},
	{"mtropolis", "mTropolis Title"},
	{nullptr, nullptr}
};

#include "mtropolis/detection_tables.h"

static const char *const directoryGlobs[] = {
	"Obsidian",
	"RESOURCE",
	"Saved Games",
	"MTPLAY32",
	"Albert",
	"DATA",
	"GAME",
	nullptr
};

class MTropolisMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	MTropolisMetaEngineDetection() : AdvancedMetaEngineDetection(MTropolis::gameDescriptions, sizeof(MTropolis::MTropolisGameDescription), mTropolisGames) {
		_guiOptions = GUIO3(GAMEOPTION_DYNAMIC_MIDI, GAMEOPTION_LAUNCH_DEBUG, GAMEOPTION_ENABLE_SHORT_TRANSITIONS);
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
		_flags = kADFlagCanPlayUnknownVariants;
	}

	const char *getName() const override {
		return "mtropolis";
	}

	const char *getEngineName() const override {
		return "mTropolis";
	}

	const char *getOriginalCopyright() const override {
		return "mTropolis (C) mFactory/Quark";
	}
	
	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const override;

	static MTropolis::MTropolisGameDescription _globalFallbackDesc;
};

ADDetectedGame MTropolisMetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **outExtra) const {
	const char *winBootFileName = MTROPOLIS_WIN_BOOT_SCRIPT_NAME;
	const char *macBootFileName = MTROPOLIS_MAC_BOOT_SCRIPT_NAME;

	FileMap::const_iterator macBootScriptIt = allFiles.find(macBootFileName);
	FileMap::const_iterator winBootScriptIt = allFiles.find(winBootFileName);

	bool foundMac = (macBootScriptIt != allFiles.end());
	bool foundWin = (winBootScriptIt != allFiles.end());

	if (foundMac && foundWin) {
		warning("Found both %s and %s, need exactly one or the other to boot a mTropolis project", winBootFileName, macBootFileName);
		return ADDetectedGame();
	}

	if (!foundMac && !foundWin)
		return ADDetectedGame();

	MTropolis::MTropolisGameDescription *desc = &_globalFallbackDesc;

	if (foundWin) {
		desc->desc.platform = Common::kPlatformWindows;
	} else if (foundMac) {
		desc->desc.platform = Common::kPlatformMacintosh;
	}

	if (outExtra) {
		const Common::FSNode *bootScriptFile = nullptr;

		if (foundMac)
			bootScriptFile = &macBootScriptIt->_value;
		if (foundWin)
			bootScriptFile = &winBootScriptIt->_value;

		Common::File f;
		if (bootScriptFile && f.open(*bootScriptFile)) {
			Common::String targetID = "mtropolis-fallback";
			Common::String gameName = foundWin ? "Windows mTropolis Title" : "Macintosh mTropolis Title";

			Common::String lineStr;
			while (!f.err() && !f.eos()) {
				lineStr = f.readString('\n');
				lineStr.trim();

				const char *targetIDPrefix = "//targetID:";
				const char *gameNamePrefix = "//gameName:";

				if (lineStr.hasPrefix(targetIDPrefix))
					targetID = lineStr.substr(strlen(targetIDPrefix));
				else if (lineStr.hasPrefix(gameNamePrefix))
					gameName = lineStr.substr(strlen(gameNamePrefix));
			}

			ADDetectedGameExtraInfo *extraInfo = new ADDetectedGameExtraInfo();
			extraInfo->gameName = gameName;
			extraInfo->targetID = targetID;

			*outExtra = extraInfo;
		}
	}

	return ADDetectedGame(&desc->desc);
}

MTropolis::MTropolisGameDescription MTropolisMetaEngineDetection::_globalFallbackDesc = {
	{
		"mtropolis",
		"",
		{
			AD_LISTEND
		},
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	MTropolis::GID_GENERIC,
	0,
	MTropolis::MTBOOT_USE_BOOT_SCRIPT,
};

REGISTER_PLUGIN_STATIC(MTROPOLIS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, MTropolisMetaEngineDetection);
