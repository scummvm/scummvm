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

#include "common/algorithm.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/text-to-speech.h"

#include "engines/advancedDetector.h"

#include "dreamweb/detection.h"
#include "dreamweb/dreamweb.h"

#define GAMEOPTION_TTS_THINGS 	GUIO_GAMEOPTIONS1
#define GAMEOPTION_TTS_SPEECH 	GUIO_GAMEOPTIONS2

static const PlainGameDescriptor dreamWebGames[] = {
	{ "dreamweb", "DreamWeb" },
	{ nullptr, nullptr }
};

static const DebugChannelDef debugFlagList[] = {
	{DreamWeb::kDebugAnimation, "Animation", "Animation Debug Flag"},
	{DreamWeb::kDebugSaveLoad, "SaveLoad", "Track Save/Load Function"},
	DEBUG_CHANNEL_END
};

#include "dreamweb/detection_tables.h"

static const ADExtraGuiOptionsMap gameGuiOptions[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_BRIGHTPALETTE,
		{
			_s("Use bright palette mode"),
			_s("Display graphics using the game's bright palette"),
			"bright_palette",
			true,
			0,
			0
		}
	},

#ifdef USE_TTS
	{
		GAMEOPTION_TTS_THINGS,
		{
			_s("Enable Text to Speech for Objects, Options, and the Bible Quote"),
			_s("Use TTS to read the descriptions (if TTS is available)"),
			"tts_enabled_objects",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_TTS_SPEECH,
		{
			_s("Enable Text to Speech for Subtitles"),
			_s("Use TTS to read the subtitles (if TTS is available)"),
			"tts_enabled_speech",
			false,
			0,
			0
		}
	},
#endif

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class DreamWebMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	DreamWebMetaEngineDetection():
	AdvancedMetaEngineDetection(DreamWeb::gameDescriptions,
	sizeof(DreamWeb::DreamWebGameDescription), dreamWebGames,
	gameGuiOptions) {
		_guiOptions = GUIO1(GUIO_NOMIDI);
	}

	const char *getName() const override {
		return "dreamweb";
	}

	const char *getEngineName() const override {
		return "DreamWeb";
	}

	const char *getOriginalCopyright() const override {
		return "DreamWeb (C) Creative Reality";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(DREAMWEB_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, DreamWebMetaEngineDetection);
