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

#include "ultima8/detection.h"
#include "ultima8/detection_tables.h"
#include "ultima8/debug.h"
#include "ultima8/ultima8.h"

namespace Ultima8 {

static const PlainGameDescriptor ULTIMA8_GAMES[] = {
	{ "ultima8", "Ultima VIII: Pagan" },
	{ "remorse", "Crusader: No Remorse" },
	{ "regret", "Crusader: No Regret" },
	{ 0, 0 }
};

} // End of namespace Ultima8

const DebugChannelDef Ultima8MetaEngineDetection::debugFlagList[] = {
	{Ultima8::kDebugPath, "Path", "Pathfinding debug level"},
	{Ultima8::kDebugGraphics, "Graphics", "Graphics debug level"},
	{Ultima8::kDebugVideo, "Video", "Video playback debug level"},
	{Ultima8::kDebugActor, "Actor", "Actor debug level"},
	{Ultima8::kDebugObject, "Object", "Object debug level"},
	{Ultima8::kDebugCollision, "Collision", "Collision debug level"},
	DEBUG_CHANNEL_END
};

Ultima8MetaEngineDetection::Ultima8MetaEngineDetection() : AdvancedMetaEngineDetection(Ultima8::GAME_DESCRIPTIONS,
	        sizeof(Ultima8::Ultima8GameDescription), Ultima8::ULTIMA8_GAMES) {
	static const char *const DIRECTORY_GLOBS[2] = { "usecode", 0 };
	_maxScanDepth = 2;
	_directoryGlobs = DIRECTORY_GLOBS;
}

REGISTER_PLUGIN_STATIC(ULTIMA8_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Ultima8MetaEngineDetection);
