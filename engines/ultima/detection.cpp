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

#include "ultima/detection.h"
#include "ultima/detection_tables.h"
#include "ultima/ultima.h"

const DebugChannelDef UltimaMetaEngineDetection::debugFlagList[] = {
	{Ultima::kDebugPath, "Path", "Pathfinding debug level"},
	{Ultima::kDebugGraphics, "Graphics", "Graphics debug level"},
	{Ultima::kDebugVideo, "Video", "Video playback debug level"},
	{Ultima::kDebugActor, "Actor", "Actor debug level"},
	{Ultima::kDebugObject, "Object", "Object debug level"},
	{Ultima::kDebugCollision, "Collision", "Collision debug level"},
	{Ultima::kDebugImGui, "imgui", "Imgui debug output"},
	DEBUG_CHANNEL_END
};

UltimaMetaEngineDetection::UltimaMetaEngineDetection() : AdvancedMetaEngineDetection(Ultima::GAME_DESCRIPTIONS,
	        Ultima::ULTIMA_GAMES) {
	static const char *const DIRECTORY_GLOBS[2] = { "usecode", 0 };
	_maxScanDepth = 2;
	_directoryGlobs = DIRECTORY_GLOBS;
	_flags = kADFlagMatchFullPaths;
}

REGISTER_PLUGIN_STATIC(ULTIMA_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, UltimaMetaEngineDetection);
