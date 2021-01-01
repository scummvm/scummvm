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

PlainGameDescriptor PlainGameDescriptor::empty() {
	PlainGameDescriptor pgd;
	pgd.gameId = nullptr;
	pgd.description = nullptr;
	return pgd;
}

PlainGameDescriptor PlainGameDescriptor::of(const char *gameId, const char *description) {
	PlainGameDescriptor pgd;
	pgd.gameId = gameId;
	pgd.description = description;
	return pgd;
}

QualifiedGameDescriptor::QualifiedGameDescriptor(const char *engine, const PlainGameDescriptor &pgd) :
		engineId(engine),
		gameId(pgd.gameId),
		description(pgd.description) {
}

DetectedGame::DetectedGame() :
		hasUnknownFiles(false),
		canBeAdded(true),
		language(Common::UNK_LANG),
		platform(Common::kPlatformUnknown),
		gameSupportLevel(kStableGame) {
}

DetectedGame::DetectedGame(const Common::String &engine, const PlainGameDescriptor &pgd) :
		engineId(engine),
		hasUnknownFiles(false),
		canBeAdded(true),
		language(Common::UNK_LANG),
		platform(Common::kPlatformUnknown),
		gameSupportLevel(kStableGame) {

	gameId = pgd.gameId;
	preferredTarget = pgd.gameId;
	description = pgd.description;
}

DetectedGame::DetectedGame(const Common::String &engine, const Common::String &id, const Common::String &d, Common::Language l, Common::Platform p, const Common::String &ex, bool unsupported) :
		engineId(engine),
		hasUnknownFiles(false),
		canBeAdded(true),
		gameSupportLevel(kStableGame) {

	gameId = id;
	preferredTarget = id;
	description = d;
	language = l;
	platform = p;
	extra = ex;

	// Append additional information, if set, to the description.
	description += updateDesc(unsupported);
}

void DetectedGame::setGUIOptions(const Common::String &guioptions) {
	_guiOptions = Common::getGameGUIOptionsDescription(guioptions);
}

void DetectedGame::appendGUIOptions(const Common::String &str) {
	if (!_guiOptions.empty())
		_guiOptions += " ";

	_guiOptions += str;
}

Common::String DetectedGame::updateDesc(bool skipExtraField) const {
	const bool hasCustomLanguage = (language != Common::UNK_LANG);
	const bool hasCustomPlatform = (platform != Common::kPlatformUnknown);
	const bool hasExtraDesc = (!extra.empty() && !skipExtraField);

	// Adapt the description string if custom platform/language is set.
	Common::String descr;
	if (!hasCustomLanguage && !hasCustomPlatform && !hasExtraDesc)
		return descr;

	descr += " (";

	if (hasExtraDesc)
		descr += extra;
	if (hasCustomPlatform) {
		if (hasExtraDesc)
			descr += "/";
		descr += Common::getPlatformDescription(platform);
	}
	if (hasCustomLanguage) {
		if (hasExtraDesc || hasCustomPlatform)
			descr += "/";
		descr += Common::getLanguageDescription(language);
	}

	descr += ")";

	return descr;
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

DetectedGames DetectionResults::listRecognizedGames() const {
	DetectedGames candidates;
	for (uint i = 0; i < _detectedGames.size(); i++) {
		if (_detectedGames[i].canBeAdded) {
			candidates.push_back(_detectedGames[i]);
		}
	}
	return candidates;
}

DetectedGames DetectionResults::listDetectedGames() const {
	return _detectedGames;
}

Common::U32String DetectionResults::generateUnknownGameReport(bool translate, uint32 wordwrapAt) const {
	return ::generateUnknownGameReport(_detectedGames, translate, false, wordwrapAt);
}

Common::U32String generateUnknownGameReport(const DetectedGames &detectedGames, bool translate, bool fullPath, uint32 wordwrapAt) {
	assert(!detectedGames.empty());

	const char *reportStart = _s("The game in '%s' seems to be an unknown game variant.\n\n"
	                             "Please report the following data to the ScummVM team at %s "
	                             "along with the name of the game you tried to add and "
	                             "its version, language, etc.:");
	const char *reportEngineHeader = _s("Matched game IDs for the %s engine:");

	Common::U32String report = Common::U32String::format(
			translate ? _(reportStart) : Common::U32String(reportStart),
			fullPath ? detectedGames[0].path.c_str() : detectedGames[0].shortPath.c_str(),
			"https://bugs.scummvm.org/"
	);
	report += Common::U32String("\n");

	FilePropertiesMap matchedFiles;

	Common::String currentEngineId;
	for (uint i = 0; i < detectedGames.size(); i++) {
		const DetectedGame &game = detectedGames[i];

		if (!game.hasUnknownFiles) continue;

		if (currentEngineId.empty() || currentEngineId != game.engineId) {
			currentEngineId = game.engineId;

			// If the engine is not the same as for the previous entry, print an engine line header
			report += Common::U32String("\n");
			report += Common::U32String::format(
					translate ? _(reportEngineHeader) : Common::U32String(reportEngineHeader),
					game.engineId.c_str()
			);
			report += Common::U32String(" ");

		} else {
			report += Common::U32String(", ");
		}

		// Add the gameId to the list of matched games for the engine
		// TODO: Use the gameId here instead of the preferred target.
		// This is currently impossible due to the AD singleId feature losing the information.
		report += game.preferredTarget;

		// Consolidate matched files across all engines and detection entries
		for (FilePropertiesMap::const_iterator it = game.matchedFiles.begin(); it != game.matchedFiles.end(); it++) {
			matchedFiles.setVal(it->_key, it->_value);
		}
	}

	if (wordwrapAt) {
		report.wordWrap(wordwrapAt);
	}

	report += Common::U32String("\n\n");

	for (FilePropertiesMap::const_iterator file = matchedFiles.begin(); file != matchedFiles.end(); ++file)
		report += Common::String::format("  {\"%s\", 0, \"%s\", %d},\n", file->_key.c_str(), file->_value.md5.c_str(), file->_value.size);

	report += Common::U32String("\n");

	return report;
}

Common::U32String generateUnknownGameReport(const DetectedGame &detectedGame, bool translate, bool fullPath, uint32 wordwrapAt) {
	DetectedGames detectedGames;
	detectedGames.push_back(detectedGame);

	return generateUnknownGameReport(detectedGames, translate, fullPath, wordwrapAt);
}
