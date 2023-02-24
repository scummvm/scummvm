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

#include "tetraedge/detection.h"
#include "tetraedge/metaengine.h"
#include "tetraedge/detection_tables.h"

const DebugChannelDef TetraedgeMetaEngineDetection::debugFlagList[] = {
	{ Tetraedge::kDebugGraphics, "Graphics", "Graphics debug level" },
	{ Tetraedge::kDebugPath, "Path", "Pathfinding debug level" },
	{ Tetraedge::kDebugFilePath, "FilePath", "File path debug level" },
	{ Tetraedge::kDebugScan, "Scan", "Scan for unrecognised games" },
	{ Tetraedge::kDebugScript, "Script", "Enable debug script dump" },
	DEBUG_CHANNEL_END
};

TetraedgeMetaEngineDetection::TetraedgeMetaEngineDetection() : AdvancedMetaEngineDetection(Tetraedge::GAME_DESCRIPTIONS,
	sizeof(ADGameDescription), Tetraedge::GAME_NAMES) {
	_flags = kADFlagMatchFullPaths;
}

REGISTER_PLUGIN_STATIC(TETRAEDGE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, TetraedgeMetaEngineDetection);
