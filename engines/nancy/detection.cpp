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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"

#include "engines/nancy/detection.h"
#include "engines/nancy/detection_tables.h"

const char *const directoryGlobs[] = {
	"game",
	"iff",
	"cifTree",
	"datafiles",
	"cdvideo", // used to test for unpacked variants
	"hdvideo", // used to test for unpacked variants
	nullptr
};

static const DebugChannelDef debugFlagList[] = {
	{ Nancy::kDebugEngine, "Engine", "Engine general debug" },
	{ Nancy::kDebugActionRecord, "ActionRecord", "Action Record debug" },
	{ Nancy::kDebugScene, "Scene", "Scene debug" },
	{ Nancy::kDebugSound, "Sound", "Sound debug" },
	{ Nancy::kDebugHypertext, "Hypertext", "Hypertext rendering debug" },
	DEBUG_CHANNEL_END
};

class NancyMetaEngineDetection : public AdvancedMetaEngineDetection<Nancy::NancyGameDescription> {
public:
	NancyMetaEngineDetection() : AdvancedMetaEngineDetection(Nancy::gameDescriptions, Nancy::nancyGames) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
		_guiOptions = GUIO2(GUIO_NOMIDI, GUIO_NOASPECT);
	}

	const char *getName() const override {
		return "nancy";
	}

	const char *getEngineName() const override {
		return "Nancy Drew";
	}

	const char *getOriginalCopyright() const override {
		return "Nancy Drew Engine copyright Her Interactive, 1995-2012";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(NANCY_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, NancyMetaEngineDetection);
