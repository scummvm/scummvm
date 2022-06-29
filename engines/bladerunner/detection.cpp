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


#include "bladerunner/bladerunner.h"
#include "bladerunner/detection_tables.h"
#include "bladerunner/savefile.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"

static const DebugChannelDef debugFlagList[] = {
	{BladeRunner::kDebugScript, "Script", "Debug the scripts"},
	DEBUG_CHANNEL_END
};

namespace BladeRunner {

static const PlainGameDescriptor bladeRunnerGames[] = {
	{"bladerunner", "Blade Runner"},
	{"bladerunner-final", "Blade Runner with restored content"},
	{"bladerunner-ee", "Blade Runner: Enhanced Edition"},
	{nullptr, nullptr}
};

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_SITCOM,
		{
			_s("Sitcom mode"),
			_s("Game will add laughter after actor's line or narration"),
			"sitcom",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_SHORTY,
		{
			_s("Shorty mode"),
			_s("Game will shrink the actors and make their voices high pitched"),
			"shorty",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_FRAMELIMITER_NODELAYMILLIS,
		{
			_s("Frame limiter high performance mode"),
			_s("This mode may result in high CPU usage! It avoids use of delayMillis() function."),
			"nodelaymillisfl",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_FRAMELIMITER_FPS,
		{
			_s("Max frames per second limit"),
			_s("This mode targets a maximum of 120 fps. When disabled, the game targets 60 fps"),
			"frames_per_secondfl",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_DISABLE_STAMINA_DRAIN,
		{
			_s("Disable McCoy's quick stamina drain"),
			_s("When running, McCoy won't start slowing down as soon as the player stops clicking the mouse"),
			"disable_stamina_drain",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_SHOW_SUBS_IN_CRAWL,
		{
			_s("Show subtitles during text crawl"),
			_s("During the intro cutscene, show subtitles during the text crawl"),
			"use_crawl_subs",
			true,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace BladeRunner

class BladeRunnerMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	BladeRunnerMetaEngineDetection();

	const char *getEngineId() const override;
	const char *getName() const override;
	const char *getOriginalCopyright() const override;
	const DebugChannelDef *getDebugChannels() const override;
};

BladeRunnerMetaEngineDetection::BladeRunnerMetaEngineDetection()
	: AdvancedMetaEngineDetection(
		BladeRunner::gameDescriptions,
		sizeof(BladeRunner::gameDescriptions[0]),
		BladeRunner::bladeRunnerGames,
		BladeRunner::optionsList) {
		// Setting this, allows the demo files to be copied in the BladeRunner
		// game data folder and be detected and subsequently launched without
		// any issues (eg. like ScummVM launching Blade Runner instead of the demo).
		// Although the demo files are not part of the original game's installation
		// or CD content, it's nice to support the use case whereby the user
		// manually copies the demo files in the Blade Runner game data folder
		// and expects ScummVM to detect both, offer a choice on which to add,
		// and finally launch the proper one depending on which was added.
		_flags = kADFlagUseExtraAsHint;
}

const char *BladeRunnerMetaEngineDetection::getEngineId() const {
	return "bladerunner";
}

const char *BladeRunnerMetaEngineDetection::getName() const {
	return "Blade Runner";
}

const char *BladeRunnerMetaEngineDetection::getOriginalCopyright() const {
	return "Blade Runner (C) 1997 Westwood Studios";
}

const DebugChannelDef *BladeRunnerMetaEngineDetection::getDebugChannels() const {
	return debugFlagList;
}

REGISTER_PLUGIN_STATIC(BLADERUNNER_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, BladeRunnerMetaEngineDetection);
