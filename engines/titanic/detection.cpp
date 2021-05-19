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
#include "common/str-array.h"
#include "common/memstream.h"
#include "engines/advancedDetector.h"

#include "titanic/detection.h"
#include "titanic/debugger.h"

static const DebugChannelDef debugFlagList[] = {
	{Titanic::kDebugCore, "core", "Core engine debug level"},
	{Titanic::kDebugScripts, "scripts", "Game scripts"},
	{Titanic::kDebugGraphics, "graphics", "Graphics handling"},
	{Titanic::kDebugStarfield, "starfield", "Starfield logic"},
	DEBUG_CHANNEL_END
};

static const PlainGameDescriptor TitanicGames[] = {
	{"titanic", "Starship Titanic"},
	{0, 0}
};

#include "titanic/detection_tables.h"

class TitanicMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	TitanicMetaEngineDetection() : AdvancedMetaEngineDetection(Titanic::gameDescriptions, sizeof(Titanic::TitanicGameDescription), TitanicGames) {
		_maxScanDepth = 3;
	}

	const char *getEngineId() const override {
		return "titanic";
	}

	const char *getName() const override {
		return "Starship Titanic";
	}

	const char *getOriginalCopyright() const override {
		return "Starship Titanic (C) The Digital Village";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(TITANIC_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, TitanicMetaEngineDetection);
