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
#include "common/textconsole.h"

#include "mohawk/detection.h"

#include "mohawk/riven_metaengine.h"
#include "mohawk/myst_metaengine.h"


static const PlainGameDescriptor mohawkGames[] = {
	{"myst", "Myst"},
	{"makingofmyst", "The Making of Myst"},
	{"riven", "Riven: The Sequel to Myst"},
	{"zoombini", "Logical Journey of the Zoombinis"},
	{"alientales", "Alien Tales"},
	{"orly", "Orly's Draw-A-Story"},
	{"cstime", "Where in Time is Carmen Sandiego?"},
	{"csworld", "Where in the World is Carmen Sandiego?"},
	{"csamtrak", "Where in America is Carmen Sandiego? (The Great Amtrak Train Adventure)"},
	{"carmentq", "Carmen Sandiego's ThinkQuick Challenge"},
	{"carmentqc", "Carmen Sandiego's ThinkQuick Challenge Custom Question Creator"},
	{"maggiesfa", "Maggie's Farmyard Adventure"},
	{"jamesmath", "James Discovers/Explores Math"},
	{"treehouse", "The Treehouse"},
	{"greeneggs", "Green Eggs and Ham"},
	{"seussabc", "Dr. Seuss's ABC"},
	{"1stdegree", "In the 1st Degree"},
	{"csusa", "Where in the USA is Carmen Sandiego?"},
	{"seussps", "Dr. Seuss's Preschool"},
	{"tortoise", "Aesop's Fables: The Tortoise and the Hare"},
	{"arthur", "Arthur's Teacher Trouble"},
	{"grandma", "Just Grandma and Me"},
	{"ruff", "Ruff's Bone"},
	{"newkid", "The New Kid on the Block"},
	{"arthurrace", "Arthur's Reading Race"},
	{"arthurbday", "Arthur's Birthday"},
	{"lilmonster", "Little Monster at School"},
	{"catinthehat", "The Cat in the Hat"},
	{"rugrats", "Rugrats Adventure Game"},
	{"lbsampler", "Living Books Sampler"},
	{"bearfight", "The Berenstain Bears Get in a Fight"},
	{"beardark", "The Berenstain Bears In The Dark"},
	{"arthurcomp", "Arthur's Computer Adventure"},
	{"create", "The Story of Creation"},
	{"daniel", "Daniel in the Lions' Den"},
	{"harryhh","Harry and the Haunted House"},
	{"noah", "Noah's Ark"},
	{"stellaluna", "Stellaluna"},
	{"sheila", "Sheila Rae, the Brave"},
	{"rugratsps", "Rugrats Print Shop" },
	{"drseussreading", "Dr. Seuss Reading Games"},
	{"arthurreading", "Arthur's Reading Games"},
	{"wsg", "Williams-Sonoma Guide to Good Cooking" },
	{"mathsworkshop", "Maths Workshop" },
	{"mathsworkshopdx", "Maths Workshop Deluxe" },
	{"wricamact", "Write, Camera, Action!"},
	{"amazingwriting", "The Amazing Writing Machine"},
	{nullptr, nullptr}
};

#include "mohawk/detection_tables.h"

static const char *const directoryGlobs[] = {
	"all",
	"assets1",
	"data",
	"program",
	"95instal",
	"Rugrats Adventure Game",
	nullptr
};

class MohawkMetaEngineDetection : public AdvancedMetaEngineDetection<Mohawk::MohawkGameDescription> {
public:
	MohawkMetaEngineDetection() : AdvancedMetaEngineDetection(Mohawk::gameDescriptions, mohawkGames) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const override {
		return detectGameFilebased(allFiles, Mohawk::fileBased);
	}

	const char *getName() const override {
		return "mohawk";
	}

	const char *getEngineName() const override {
		return "Mohawk";
	}

	const char *getOriginalCopyright() const override {
		return "Myst and Riven (C) Cyan Worlds\nMohawk OS (C) Ubisoft";
	}

	DetectedGame toDetectedGame(const ADDetectedGame &adGame, ADDetectedGameExtraInfo *extraInfo) const override;
};

DetectedGame MohawkMetaEngineDetection::toDetectedGame(const ADDetectedGame &adGame, ADDetectedGameExtraInfo *extraInfo) const {
	DetectedGame game = AdvancedMetaEngineDetection::toDetectedGame(adGame);

	// The AdvancedDetector model only allows specifying a single supported
	// game language. The 25th anniversary edition Myst games are multilanguage.
	// Here we amend the detected games to set the list of supported languages.
	if (game.gameId == "myst"
			&& Common::checkGameGUIOption(GAMEOPTION_25TH, game.getGUIOptions())
			&& Common::checkGameGUIOption(GAMEOPTION_ME, game.getGUIOptions())) {
		const Mohawk::MystLanguage *languages = Mohawk::MohawkMetaEngine_Myst::listLanguages();
		while (languages->language != Common::UNK_LANG) {
			game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(languages->language));
			languages++;
		}
	}

	if (game.gameId == "riven"
			&& Common::checkGameGUIOption(GAMEOPTION_25TH, game.getGUIOptions())) {
		const Mohawk::RivenLanguage *languages = Mohawk::MohawkMetaEngine_Riven::listLanguages();
		while (languages->language != Common::UNK_LANG) {
			game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(languages->language));
			languages++;
		}
	}

	return game;
}

REGISTER_PLUGIN_STATIC(MOHAWK_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, MohawkMetaEngineDetection);
