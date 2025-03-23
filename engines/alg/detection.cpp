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

#include "alg/alg.h"
#include "alg/detection.h"
#include "alg/detection_tables.h"

static const PlainGameDescriptor algGame[] = {
	{ "cpatrols", "Crime Patrol (lower video quality)" },
	{ "cpatrold", "Crime Patrol" },
	{ "cpatroldemo", "Crime Patrol Demo" },
	{ "dwarss", "Drug Wars (lower video quality)" },
	{ "dwarsd", "Drug Wars" },
	{ "dwarsdemo", "Drug Wars Demo" },
	{ "johnrocs", "Who Shot Johnny Rock? (lower video quality)" },
	{ "johnrocd", "Who Shot Johnny Rock?" },
	{ "lbhunter", "The Last Bounty Hunter" },
	{ "lbhunterdemo", "The Last Bounty Hunter Demo" },
	{ "maddog", "Mad Dog McCree" },
	{ "maddog2s", "Mad Dog II: The Lost Gold (lower video quality)" },
	{ "maddog2d", "Mad Dog II: The Lost Gold" },
	{ "spiratess", "Space Pirates (lower video quality)" },
	{ "spiratesd", "Space Pirates" },
	{ "spiratesdemo", "Space Pirates Demo" },
	{ nullptr, nullptr }
};

static const DebugChannelDef debugFlagList[] = {
        { Alg::kAlgDebugGeneral, "general", "General" },
        { Alg::kAlgDebugGraphics, "graphics", "Graphics" },
		DEBUG_CHANNEL_END
};

class AlgMetaEngineDetection : public AdvancedMetaEngineDetection<Alg::AlgGameDescription> {
public:
	AlgMetaEngineDetection() : AdvancedMetaEngineDetection(Alg::gameDescriptions, algGame) {
		_guiOptions = GUIO1(GUIO_NOMIDI);
		_maxScanDepth = 1;
	}

	const char *getName() const override {
		return "alg";
	}

	const char *getEngineName() const override {
		return "American Laser Games";
	}

	const char *getOriginalCopyright() const override {
		return "Copyright (C) American Laser Games";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(ALG_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, AlgMetaEngineDetection);
