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

#include "base/plugins.h"
#include "engines/advancedDetector.h"
#include "common/translation.h"

#include "hopkins/detection.h"

static const PlainGameDescriptor hopkinsGames[] = {
	{"hopkins", "Hopkins FBI"},
	{0, 0}
};

#include "hopkins/detection_tables.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_GORE_DEFAULT_OFF,
		{
			_s("Gore Mode"),
			_s("Enable Gore Mode when available"),
			"enable_gore",
			false
		}
	},

	{
		GAMEOPTION_GORE_DEFAULT_ON,
		{
			_s("Gore Mode"),
			_s("Enable Gore Mode when available"),
			"enable_gore",
			true
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

const static char *directoryGlobs[] = {
	"voice",
	"link",
	0
};

class HopkinsMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	HopkinsMetaEngineDetection() : AdvancedMetaEngineDetection(Hopkins::gameDescriptions, sizeof(Hopkins::HopkinsGameDescription), hopkinsGames, optionsList) {
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "hopkins";
	}

	const char *getName() const override {
		return "Hopkins FBI";
	}

	const char *getOriginalCopyright() const override {
		return "Hopkins FBI (C) 1997-2003 MP Entertainment";
	}
};


REGISTER_PLUGIN_STATIC(HOPKINS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, HopkinsMetaEngineDetection);
