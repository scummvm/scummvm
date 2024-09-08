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
	{"mindgym", "Mind Gym"},
	{"architecture", "Fun With Architecture"},
	{"beatrix", "The Magic World of Beatrix Potter"},
	{"wtimpact", "Whitetail Impact"},
	{"worldbroke", "The Day The World Broke"},
	{"dilbert", "The Totally Techie World of Young Dilbert: Hi-Tech Hijinks"},
	{"freewilly", "Free Willy Activity Center"},
	{"hercules", "Hercules & Xena Learning Adventure: Quest for the Scrolls"},
	{"idino", "I Can Be a Dinosaur Finder"},
	{"idoctor", "I Can Be an Animal Doctor"},
	{"drawmarvelway", "How to Draw the Marvel Way"},
	{"ftts", "FairyTale: A True Story - Activity Center"},
	{"purplemoon", "Purple Moon Sampler"},
	{"chomp", "Chomp! The Video Game"},
	{"cyber24", "24 Hours in Cyberspace"},
	{"ivoclar", "IVOCLAR"},
	{"realwild", "Real Wild Child! Australian Rock Music 1950s-90s"},
	{"telemed", "How to Build a Telemedicine Program"},
	{"angelica", "Rugrats: Totally Angelica Boredom Buster"},
	{"babe", "Babe and Friends: Animated Early Reader"},
	{"biocellevo", "Biologia Cellulare Evoluzione E Variet\xc3\xa0 Della Vita"},
	{"easybake", "Easy-Bake Kitchen"},
	{"forgotten", "The Forgotten: It Begins"},
	{"greveholm2", "The Mystery at Greveholm 2: The Journey to Planutus"},
	{"itacante", "Itacante: La Cit\xc3\xa9 des Robots"},
	{"kingofdragonpass", "King of Dragon Pass"},
	{"ks1eng", "The Times Key Stage 1 English"},
	{"maisy", "Maisy's Playhouse"},
	{"msb_animal", "The Magic School Bus Explores the World of Animals"},
	{"msb_bugs", "The Magic School Bus Explores Bugs"},
	{"msb_concert", "The Magic School Bus In Concert"},
	{"msb_flight", "The Magic School Bus Discovers Flight"},
	{"msb_mars", "The Magic School Bus Lands on Mars"},
	{"msb_volcano", "The Magic School Bus Volcano Adventure"},
	{"msb_whales", "The Magic School Bus Whales & Dolphins"},
	{"mykropolis", "Mykropolis: Planet der Roboter"},
	{"notebook", "Your Notebook (with help from Amelia)"},
	{"pferdpony", "Pferd & Pony: Lass uns reiten"},
	{"mirodc10", "Pinnacle Systems miroVideo Studio DC10 Plus Tour"},
	{"poser3_sampler_zygote", "Poser 3 Zygote Sampler Disc"},
	{"poser3_zygote", "Poser 3 Zygote Promo"},
	{"wtextreme", "Whitetail Extreme"},
	{"c9sampler", "Cloud 9 CD Sampler Volume 2"},
	{"adobe24", "Adobe 24 Hours Tools Sampler"},
	{"byzantine", "Byzantine: The Betrayal"},


	{"mtropolis", "mTropolis Title"},
	{nullptr, nullptr}
};

#include "mtropolis/detection_tables.h"

static const char *const directoryGlobs[] = {
	"Obsidian",
	"RESOURCE",
	"mPlugins",
	"Saved Games",
	"MTPLAY32",
	"HIJINKS",
	"Albert",
	"DATA",
	"GAME",
	"Player",
	"mfx",
	"cdData",
	"Opal",
	"MSB",
	"System",
	"YN",
	"planutus",
	nullptr
};

class MTropolisMetaEngineDetection : public AdvancedMetaEngineDetection<MTropolis::MTropolisGameDescription> {
public:
	MTropolisMetaEngineDetection() : AdvancedMetaEngineDetection(MTropolis::gameDescriptions, mTropolisGames) {
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
