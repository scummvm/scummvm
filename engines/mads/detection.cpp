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
 *
 */

#include "base/plugins.h"
#include "common/str-array.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "common/translation.h"

#include "mads/detection.h"

static const PlainGameDescriptor MADSGames[] = {
	{"dragonsphere", "Dragonsphere"},
	{"nebular", "Rex Nebular and the Cosmic Gender Bender"},
	{"phantom", "Return of the Phantom"},
	{0, 0}
};

#define GAMEOPTION_EASY_MOUSE          GUIO_GAMEOPTIONS1
#define GAMEOPTION_ANIMATED_INVENTORY  GUIO_GAMEOPTIONS2
#define GAMEOPTION_ANIMATED_INTERFACE  GUIO_GAMEOPTIONS3
#define GAMEOPTION_NAUGHTY_MODE        GUIO_GAMEOPTIONS4
//#define GAMEOPTION_GRAPHICS_DITHERING  GUIO_GAMEOPTIONS5

#ifdef USE_TTS
#define GAMEOPTION_TTS_NARRATOR 	GUIO_GAMEOPTIONS5
#endif

#include "mads/detection_tables.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_EASY_MOUSE,
		{
			_s("Easy mouse interface"),
			_s("Shows object names when hovering the mouse over them"),
			"EasyMouse",
			true
		}
	},

	{
		GAMEOPTION_ANIMATED_INVENTORY,
		{
			_s("Animated inventory items"),
			_s("Animated inventory items"),
			"InvObjectsAnimated",
			true
		}
	},

	{
		GAMEOPTION_ANIMATED_INTERFACE,
		{
			_s("Animated game interface"),
			_s("Animated game interface"),
			"TextWindowAnimated",
			true
		}
	},

	{
		GAMEOPTION_NAUGHTY_MODE,
		{
			_s("Naughty game mode"),
			_s("Naughty game mode"),
			"NaughtyMode",
			true
		}
	},

	/*{
		GAMEOPTION_GRAPHICS_DITHERING,
		{
			_s("Graphics dithering"),
			_s("Graphics dithering"),
			"GraphicsDithering",
			true
		}
	},*/

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

class MADSMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	MADSMetaEngineDetection() : AdvancedMetaEngineDetection(MADS::gameDescriptions, sizeof(MADS::MADSGameDescription), MADSGames, optionsList) {
		_maxScanDepth = 3;
	}

	const char *getEngineId() const override {
		return "mads";
	}

	const char *getName() const override {
		return "MADS";
	}

	const char *getOriginalCopyright() const override {
		return "MADS (C) Microprose";
	}
};

REGISTER_PLUGIN_STATIC(MADS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, MADSMetaEngineDetection);
