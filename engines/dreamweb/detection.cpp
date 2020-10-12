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

#include "common/algorithm.h"
#include "common/system.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"

#include "dreamweb/detection.h"

static const PlainGameDescriptor dreamWebGames[] = {
	{ "dreamweb", "DreamWeb" },
	{ 0, 0 }
};

#include "dreamweb/detection_tables.h"

static const ADExtraGuiOptionsMap gameGuiOptions[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false
		}
	},

	{
		GAMEOPTION_BRIGHTPALETTE,
		{
			_s("Use bright palette mode"),
			_s("Display graphics using the game's bright palette"),
			"bright_palette",
			true
		}
	},

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

	const char *getEngineId() const override {
		return "dreamweb";
	}

	const char *getName() const override {
		return "DreamWeb";
	}

	const char *getOriginalCopyright() const override {
		return "DreamWeb (C) Creative Reality";
	}
};

REGISTER_PLUGIN_STATIC(DREAMWEB_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, DreamWebMetaEngineDetection);
