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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "base/plugins.h"

#include "ripper/detection.h"
#include "ripper/detection_tables.h"

const DebugChannelDef RipperMetaEngineDetection::debugFlagList[] = {
	{ Ripper::kDebugGeneral, "general", "Engine lifecycle and general state" },
	{ Ripper::kDebugResources, "resources", "Resource library access" },
	{ Ripper::kDebugScripts, "scripts", "Compiled script parsing and execution" },
	{ Ripper::kDebugScene, "scene", "Scene transitions and interactions" },
	{ Ripper::kDebugVideo, "video", "Media selection and playback" },
	{ Ripper::kDebugInput, "input", "Keyboard and mouse translation" },
	{ Ripper::kDebugCursor, "cursor", "Cursor assets, selection, and animation" },
	{ Ripper::kDebugAudio, "audio", "Music, speech, and sound-effect playback" },
	{ Ripper::kDebugDialogue, "dialogue", "Dialogue choices, presentation, and selection state" },
	{ Ripper::kDebugWac, "wac", "WAC database, presentation, and puzzle state" },
	DEBUG_CHANNEL_END
};

RipperMetaEngineDetection::RipperMetaEngineDetection() : AdvancedMetaEngineDetection(
	Ripper::gameDescriptions, Ripper::ripperGames) {
	_maxScanDepth = 2;
}

REGISTER_PLUGIN_STATIC(RIPPER_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, RipperMetaEngineDetection);
