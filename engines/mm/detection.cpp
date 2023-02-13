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
#include "mm/detection.h"
#include "mm/mm.h"

static const PlainGameDescriptor MIGHT_AND_MAGIC_GAMES[] = {
#ifdef ENABLE_MM1
	{ "mm1", "Might and Magic: Book One - Secret of the Inner Sanctum"},
	{ "mm1_enh", "Might and Magic: Book One - Secret of the Inner Sanctum - Enhanced"},
#endif
#ifdef ENABLE_XEEN
	{ "cloudsofxeen", "Might and Magic IV: Clouds of Xeen" },
	{ "darksideofxeen", "Might and Magic V: Darkside of Xeen" },
	{ "worldofxeen", "Might and Magic: World of Xeen" },
	{ "swordsofxeen", "Might and Magic: Swords of Xeen" },
#endif
	{ 0, 0 }
};

static const DebugChannelDef DEBUG_FLAT_LIST[] = {
	{ MM::kDebugPath, "Path", "Pathfinding debug level" },
	{ MM::kDebugScripts, "scripts", "Game scripts" },
	{ MM::kDebugGraphics, "graphics", "Graphics handling" },
	{ MM::kDebugSound, "sound", "Sound processing" },
	DEBUG_CHANNEL_END
};

#include "mm/detection_tables.h"

class MMMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	MMMetaEngineDetection() : AdvancedMetaEngineDetection(MM::GAME_DESCRIPTIONS,
		sizeof(MM::MightAndMagicGameDescription), MIGHT_AND_MAGIC_GAMES) {
		_maxScanDepth = 3;
	}

	const char *getName() const override {
		return "mm";
	}

	const char *getEngineName() const override {
		return "Might & Magic";
	}

	const char *getOriginalCopyright() const override {
		return "Might And Magic games (C) 1986-1993 New World Computing, Inc.";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return DEBUG_FLAT_LIST;
	}
};

REGISTER_PLUGIN_STATIC(MM_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, MMMetaEngineDetection);
