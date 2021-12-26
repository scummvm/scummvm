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

#include "cine/detection.h"
#include "cine/cine.h"

static const PlainGameDescriptor cineGames[] = {
	{"fw", "Future Wars"},
	{"os", "Operation Stealth"},
	{nullptr, nullptr}
};

#include "cine/detection_tables.h"

static const DebugChannelDef debugFlagList[] = {
	{Cine::kCineDebugScript,    "Script",    "Script debug level"},
	{Cine::kCineDebugPart,      "Part",      "Part debug level"},
	{Cine::kCineDebugSound,     "Sound",     "Sound debug level"},
	{Cine::kCineDebugCollision, "Collision", "Collision debug level"},
	DEBUG_CHANNEL_END
};

static const ADExtraGuiOptionsMap optionsList[] = {
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
		GAMEOPTION_TRANSPARENT_DIALOG_BOXES,
		{
			_s("Use transparent dialog boxes in 16 color scenes"),
			_s("Use transparent dialog boxes in 16 color scenes even if the original game version did not support them"),
			"transparentdialogboxes",
			false
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class CineMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	CineMetaEngineDetection() : AdvancedMetaEngineDetection(Cine::gameDescriptions, sizeof(Cine::CINEGameDescription), cineGames, optionsList) {
		_guiOptions = GUIO3(GUIO_NOSPEECH, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_TRANSPARENT_DIALOG_BOXES);
	}

	const char *getEngineId() const override {
		return "cine";
	}

	const char *getName() const override {
		return "Cinematique evo 1";
	}

	const char *getOriginalCopyright() const override {
		return "Cinematique evo 1 (C) Delphine Software";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(CINE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, CineMetaEngineDetection);
