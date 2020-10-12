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

#include "common/translation.h"
#include "engines/advancedDetector.h"

#include "sherlock/detection.h"

static const PlainGameDescriptor sherlockGames[] = {
	{ "scalpel", "The Case of the Serrated Scalpel" },
	{ "rosetattoo", "The Case of the Rose Tattoo" },
	{0, 0}
};


#define GAMEOPTION_ORIGINAL_SAVES	GUIO_GAMEOPTIONS1
#define GAMEOPTION_FADE_STYLE		GUIO_GAMEOPTIONS2
#define GAMEOPTION_HELP_STYLE		GUIO_GAMEOPTIONS3
#define GAMEOPTION_PORTRAITS_ON		GUIO_GAMEOPTIONS4
#define GAMEOPTION_WINDOW_STYLE		GUIO_GAMEOPTIONS5
#define GAMEOPTION_TRANSPARENT_WINDOWS	GUIO_GAMEOPTIONS6

#ifdef USE_TTS
#define GAMEOPTION_TTS_NARRATOR 	GUIO_GAMEOPTIONS7
#endif

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVES,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false
		}
	},

	{
		GAMEOPTION_FADE_STYLE,
		{
			_s("Pixellated scene transitions"),
			_s("When changing scenes, a randomized pixel transition is done"),
			"fade_style",
			true
		}
	},

	{
		GAMEOPTION_HELP_STYLE,
		{
			_s("Don't show hotspots when moving mouse"),
			_s("Only show hotspot names after you actually click on a hotspot or action button"),
			"help_style",
			false
		}
	},

	{
		GAMEOPTION_PORTRAITS_ON,
		{
			_s("Show character portraits"),
			_s("Show portraits for the characters when conversing"),
			"portraits_on",
			true
		}
	},

	{
		GAMEOPTION_WINDOW_STYLE,
		{
			_s("Slide dialogs into view"),
			_s("Slide UI dialogs into view, rather than simply showing them immediately"),
			"window_style",
			true
		}
	},

	{
		GAMEOPTION_TRANSPARENT_WINDOWS,
		{
			_s("Transparent windows"),
			_s("Show windows with a partially transparent background"),
			"transparent_windows",
			true
		}
	},

#ifdef USE_TTS
	{
		GAMEOPTION_TTS_NARRATOR,
		{
			_s("TTS Narrator"),
			_s("Use TTS to read the descriptions (if TTS is available)"),
			"tts_narrator",
			false
		}
	},
#endif
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};


#include "sherlock/detection_tables.h"

class SherlockMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	SherlockMetaEngineDetection() : AdvancedMetaEngineDetection(Sherlock::gameDescriptions, sizeof(Sherlock::SherlockGameDescription),
		sherlockGames, optionsList) {}

	const char *getEngineId() const override {
		return "sherlock";
	}

	const char *getName() const override {
		return "Sherlock";
	}

	const char *getOriginalCopyright() const override {
		return "Sherlock (C) 1992-1996 Mythos Software, (C) 1992-1996 Electronic Arts";
	}
};


REGISTER_PLUGIN_STATIC(SHERLOCK_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, SherlockMetaEngineDetection);
