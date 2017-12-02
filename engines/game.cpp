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

#include "engines/game.h"
#include "common/gui_options.h"
#include "common/translation.h"


const PlainGameDescriptor *findPlainGameDescriptor(const char *gameid, const PlainGameDescriptor *list) {
	const PlainGameDescriptor *g = list;
	while (g->gameId) {
		if (0 == scumm_stricmp(gameid, g->gameId))
			return g;
		g++;
	}
	return 0;
}

GameDescriptor::GameDescriptor() {
	setVal("gameid", "");
	setVal("description", "");
}

GameDescriptor::GameDescriptor(const PlainGameDescriptor &pgd, Common::String guioptions) {
	setVal("gameid", pgd.gameId);
	setVal("description", pgd.description);

	if (!guioptions.empty())
		setVal("guioptions", Common::getGameGUIOptionsDescription(guioptions));
}

GameDescriptor::GameDescriptor(const Common::String &g, const Common::String &d, Common::Language l, Common::Platform p, Common::String guioptions, GameSupportLevel gsl) {
	setVal("gameid", g);
	setVal("description", d);
	if (l != Common::UNK_LANG)
		setVal("language", Common::getLanguageCode(l));
	if (p != Common::kPlatformUnknown)
		setVal("platform", Common::getPlatformCode(p));
	if (!guioptions.empty())
		setVal("guioptions", Common::getGameGUIOptionsDescription(guioptions));

	setSupportLevel(gsl);
}

void GameDescriptor::setGUIOptions(Common::String guioptions) {
	if (!guioptions.empty())
		setVal("guioptions", Common::getGameGUIOptionsDescription(guioptions));
	else
		erase("guioptions");
}

void GameDescriptor::appendGUIOptions(const Common::String &str) {
	setVal("guioptions", getVal("guioptions", "") + " " + str);
}

void GameDescriptor::updateDesc(const char *extra) {
	const bool hasCustomLanguage = (language() != Common::UNK_LANG);
	const bool hasCustomPlatform = (platform() != Common::kPlatformUnknown);
	const bool hasExtraDesc = (extra && extra[0]);

	// Adapt the description string if custom platform/language is set.
	if (hasCustomLanguage || hasCustomPlatform || hasExtraDesc) {
		Common::String descr = description();

		descr += " (";
		if (hasExtraDesc)
			descr += extra;
		if (hasCustomPlatform) {
			if (hasExtraDesc)
				descr += "/";
			descr += Common::getPlatformDescription(platform());
		}
		if (hasCustomLanguage) {
			if (hasExtraDesc || hasCustomPlatform)
				descr += "/";
			descr += Common::getLanguageDescription(language());
		}
		descr += ")";
		setVal("description", descr);
	}
}

GameSupportLevel GameDescriptor::getSupportLevel() {
	GameSupportLevel gsl = kStableGame;
	if (contains("gsl")) {
		Common::String gslString = getVal("gsl");
		if (gslString.equals("unstable"))
			gsl = kUnstableGame;
		else if (gslString.equals("testing"))
			gsl = kTestingGame;
	}
	return gsl;
}

void GameDescriptor::setSupportLevel(GameSupportLevel gsl) {
	switch (gsl) {
	case kUnstableGame:
		setVal("gsl", "unstable");
		break;
	case kTestingGame:
		setVal("gsl", "testing");
		break;
	case kStableGame:
		// Fall Through intended
	default:
		erase("gsl");
	}
}

DetectionResults::DetectionResults(const DetectedGames &detectedGames) :
		_detectedGames(detectedGames) {
}

bool DetectionResults::foundUnknownGames() const {
	for (uint i = 0; i < _detectedGames.size(); i++) {
		if (_detectedGames[i].hasUnknownFiles) {
			return true;
		}
	}
	return false;
}

DetectedGames DetectionResults::listRecognizedGames() {
	DetectedGames candidates;
	for (uint i = 0; i < _detectedGames.size(); i++) {
		if (_detectedGames[i].canBeAdded) {
			candidates.push_back(_detectedGames[i]);
		}
	}
	return candidates;
}

Common::String DetectionResults::generateUnknownGameReport(bool translate, uint32 wordwrapAt) const {
	assert(!_detectedGames.empty());

	const char *reportStart = _s("The game in '%s' seems to be an unknown game variant.\n\n"
	                             "Please report the following data to the ScummVM team at %s "
	                             "along with the name of the game you tried to add and "
	                             "its version, language, etc.:");
	const char *reportEngineHeader = _s("Matched game IDs for the %s engine:");

	Common::String report = Common::String::format(
			translate ? _(reportStart) : reportStart, _detectedGames[0].matchedGame["path"].c_str(),
			"https://bugs.scummvm.org/"
	);
	report += "\n";

	FilePropertiesMap matchedFiles;

	const char *currentEngineName = nullptr;
	for (uint i = 0; i < _detectedGames.size(); i++) {
		const DetectedGame &game = _detectedGames[i];

		if (!game.hasUnknownFiles) continue;

		if (!currentEngineName || strcmp(currentEngineName, game.engineName) != 0) {
			currentEngineName = game.engineName;

			// If the engine is not the same as for the previous entry, print an engine line header
			report += "\n";
			report += Common::String::format(
					translate ? _(reportEngineHeader) : reportEngineHeader,
					game.engineName
			);
			report += " ";

		} else {
			report += ", ";
		}

		// Add the gameId to the list of matched games for the engine
		// TODO: Use the gameId here instead of the preferred target.
		// This is currently impossible due to the AD singleId feature losing the information.
		report += game.matchedGame["preferredtarget"];

		// Consolidate matched files across all engines and detection entries
		for (FilePropertiesMap::const_iterator it = game.matchedFiles.begin(); it != game.matchedFiles.end(); it++) {
			matchedFiles.setVal(it->_key, it->_value);
		}
	}

	if (wordwrapAt) {
		report.wordWrap(wordwrapAt);
	}

	report += "\n\n";

	for (FilePropertiesMap::const_iterator file = matchedFiles.begin(); file != matchedFiles.end(); ++file)
		report += Common::String::format("  {\"%s\", 0, \"%s\", %d},\n", file->_key.c_str(), file->_value.md5.c_str(), file->_value.size);

	report += "\n";

	return report;
}
