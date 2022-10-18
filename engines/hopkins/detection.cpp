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
#include "common/translation.h"

#include "hopkins/detection.h"
#include "hopkins/hopkins.h"

static const DebugChannelDef debugFlagList[] = {
	{Hopkins::kDebugPath, "Path", "Pathfinding debug level"},
	{Hopkins::kDebugGraphics, "Graphics", "Graphics debug level"},
	DEBUG_CHANNEL_END
};

static const PlainGameDescriptor hopkinsGames[] = {
	{"hopkins", "Hopkins FBI"},
	{nullptr, nullptr}
};

#include "hopkins/detection_tables.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_GORE_DEFAULT_OFF,
		{
			_s("Gore Mode"),
			_s("Enable Gore Mode when available"),
			"enable_gore",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_GORE_DEFAULT_ON,
		{
			_s("Gore Mode"),
			_s("Enable Gore Mode when available"),
			"enable_gore",
			true,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

const static char *directoryGlobs[] = {
	"voice",
	"link",
	nullptr
};

class HopkinsMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	HopkinsMetaEngineDetection() : AdvancedMetaEngineDetection(Hopkins::gameDescriptions, sizeof(Hopkins::HopkinsGameDescription), hopkinsGames, optionsList) {
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
	}

	const char *getName() const override {
		return "hopkins";
	}

	const char *getEngineName() const override {
		return "Hopkins FBI";
	}

	const char *getOriginalCopyright() const override {
		return "Hopkins FBI (C) 1997-2003 MP Entertainment";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};


REGISTER_PLUGIN_STATIC(HOPKINS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, HopkinsMetaEngineDetection);
