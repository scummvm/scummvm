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


#include "common/config-manager.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "audio/mididrv.h"

#include "snatcher/detection.h"
#include "snatcher/detection_tables.h"
#include "snatcher/fantranslations.h"

namespace Snatcher {

class SnatcherMetaEngineDetection : public AdvancedMetaEngineDetection<SnatcherGameDescription> {
public:
	SnatcherMetaEngineDetection() : AdvancedMetaEngineDetection(adGameDescs, gameList) {
		_md5Bytes = 1024 * 1024;
		_maxScanDepth = 2;
		_directoryGlobs = 0;
	}

	const char *getName() const override {
		return "snatcher";
	}

	const char *getEngineName() const override {
		return "snatcher";
	}

	const char *getOriginalCopyright() const override {
		return "Snatcher (C) Konami";
	}

	DetectedGames detectGames(const Common::FSList &fslist, uint32 skipADFlags, bool skipIncomplete) override;

	Common::Error identifyGame(DetectedGame &game, const void **descriptor) override;
};

DetectedGames SnatcherMetaEngineDetection::detectGames(const Common::FSList &fslist, uint32 skipADFlags, bool skipIncomplete) {
	DetectedGames games = AdvancedMetaEngineDetection::detectGames(fslist, skipADFlags, skipIncomplete);

	for (DetectedGame &game : games) {
		if (!game.gameId.equals("snatcher"))
			continue;

		Common::List<Common::Language> langList = FanTranslationHandler::listTranslations(game.platform);

		if (!langList.empty()) {
			langList.push_back(game.language);
			game.appendGUIOptions(getGameGUIOptionsDescriptionLanguages(langList));
		}
	}

	return games;
}

Common::Error SnatcherMetaEngineDetection::identifyGame(DetectedGame &game, const void **descriptor) {
	Common::Language selectedlang = Common::UNK_LANG;
	Common::Language defaultLang = Common::UNK_LANG;
	bool supportedFanLang = false;

	if (ConfMan.hasKey("gameid") && ConfMan.get("gameid") == "snatcher" && ConfMan.hasKey("language")) {
		selectedlang = Common::parseLanguage(ConfMan.get("language"));
		Common::Platform platform = Common::parsePlatform(ConfMan.get("platform"));

		if (selectedlang != Common::UNK_LANG && platform != Common::kPlatformUnknown) {
			Common::List<Common::Language> langList = FanTranslationHandler::listTranslations(platform, &defaultLang);
			for (Common::List<Common::Language>::const_iterator i = langList.begin(); i != langList.end(); ++i) {
				if (*i == selectedlang)
					supportedFanLang = true;
			}
		}
	}

	if (supportedFanLang && defaultLang != Common::UNK_LANG)
		ConfMan.set("language", Common::getLanguageCode(defaultLang));

	Common::Error err = AdvancedMetaEngineDetection::identifyGame(game, descriptor);

	if (supportedFanLang && defaultLang != Common::UNK_LANG && selectedlang != Common::UNK_LANG)
		ConfMan.set("language", Common::getLanguageCode(selectedlang));

	return err;
}

} // End of namespace Snatcher	

REGISTER_PLUGIN_STATIC(SNATCHER_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Snatcher::SnatcherMetaEngineDetection);
