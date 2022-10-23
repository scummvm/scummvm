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
#include "qdengine/detection.h"
#include "qdengine/detection_tables.h"

const DebugChannelDef QDEngineMetaEngineDetection::debugFlagList[] = {
	{ QDEngine::kDebugGraphics, "Graphics", "Graphics debug level" },
	{ QDEngine::kDebugPath, "Path", "Pathfinding debug level" },
	{ QDEngine::kDebugFilePath, "FilePath", "File path debug level" },
	{ QDEngine::kDebugScan, "Scan", "Scan for unrecognised games" },
	{ QDEngine::kDebugScript, "Script", "Enable debug script dump" },
	DEBUG_CHANNEL_END
};

QDEngineMetaEngineDetection::QDEngineMetaEngineDetection() : AdvancedMetaEngineDetection(QDEngine::GAME_DESCRIPTIONS, QDEngine::GAME_NAMES) {
}

REGISTER_PLUGIN_STATIC(QDENGINE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, QDEngineMetaEngineDetection);
