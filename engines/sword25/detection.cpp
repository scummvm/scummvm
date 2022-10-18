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
#include "common/translation.h"
#include "engines/advancedDetector.h"

#include "sword25/detection.h"
#include "sword25/detection_tables.h"
#include "sword25/sword25.h"

static const PlainGameDescriptor sword25Game[] = {
	{"sword25", "Broken Sword 2.5"},
	{0, 0}
};

static const DebugChannelDef debugFlagList[] = {
	{Sword25::kDebugScript, "Script", "Script debug level"},
	{Sword25::kDebugSound, "Sound", "Sound debug level"},
	DEBUG_CHANNEL_END
};

static const char *directoryGlobs[] = {
	"system", // Used by extracted dats
	0
};

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ENGLISH_SPEECH,
		{
			_s("Use English speech"),
			_s("Use English speech instead of German for every language other than German"),
			"english_speech",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class Sword25MetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	Sword25MetaEngineDetection() : AdvancedMetaEngineDetection(Sword25::gameDescriptions, sizeof(ADGameDescription), sword25Game, optionsList) {
		_guiOptions = GUIO2(GUIO_NOMIDI, GAMEOPTION_ENGLISH_SPEECH);
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getName() const override {
		return "sword25";
	}

	const char *getEngineName() const override {
		return "Broken Sword 2.5";
	}

	const char *getOriginalCopyright() const override {
		return "Broken Sword 2.5 (C) Malte Thiesen, Daniel Queteschiner and Michael Elsdorfer";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(SWORD25_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Sword25MetaEngineDetection);
