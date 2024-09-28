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
#include "common/config-manager.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/str-array.h"
#include "common/translation.h"
#include "common/util.h"

#include "engines/advancedDetector.h"

#include "qdengine/qdengine.h"
#include "qdengine/detection_tables.h"

static const DebugChannelDef debugFlagList[] = {
	{ QDEngine::kDebugGraphics, "graphics", "Graphics debug level" },
	{ QDEngine::kDebugImGui, "imgui", "Imgui debug output"},
	{ QDEngine::kDebugInput, "input", "Enable user input tracing"},
	{ QDEngine::kDebugLoad, "load", "Enable load tracing" },
	{ QDEngine::kDebugLog, "log", "See log messages"},
	{ QDEngine::kDebugMovement, "movement", "Enable movement tracing"},
	{ QDEngine::kDebugSave, "save", "Enable save tracing"},
	{ QDEngine::kDebugSound, "sound", "Sound debug level"},
	{ QDEngine::kDebugTemp, "temp", "Enable temporary tracing" },
	{ QDEngine::kDebugQuant, "quant", "Enable quant tracing"},
	{ QDEngine::kDebugMinigames, "minigames", "Enable minigames tracing"},
	{ QDEngine::kDebugText, "text", "Enable text tracing"},
	DEBUG_CHANNEL_END
};

class QDEngineMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {

public:
	QDEngineMetaEngineDetection() : AdvancedMetaEngineDetection(QDEngine::GAME_DESCRIPTIONS, QDEngine::GAME_NAMES) {
	}

	~QDEngineMetaEngineDetection() override {}

	const char *getName() const override {
		return "qdengine";
	}

	const char *getEngineName() const override {
		return "QD Engine";
	}

	const char *getOriginalCopyright() const override {
		return "QD Engine (C) 2003-8 K-D LAB";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};



REGISTER_PLUGIN_STATIC(QDENGINE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, QDEngineMetaEngineDetection);
