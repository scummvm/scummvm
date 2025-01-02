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

#include "mediastation/detection.h"
#include "mediastation/detection_tables.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

const DebugChannelDef MediaStationMetaEngineDetection::debugFlagList[] = {
	{ MediaStation::kDebugGraphics, "graphics", "Graphics debug level" },
	{ MediaStation::kDebugPath, "path", "Pathfinding debug level" },
	{ MediaStation::kDebugScan, "scan", "Scan for unrecognised games" },
	{ MediaStation::kDebugScript, "script", "Enable debug script dump" },
	{ MediaStation::kDebugEvents, "events", "Events processing" },
	{ MediaStation::kDebugLoading, "loading", "File loading" },
	DEBUG_CHANNEL_END
};

MediaStationMetaEngineDetection::MediaStationMetaEngineDetection() : AdvancedMetaEngineDetection(
	    MediaStation::gameDescriptions, MediaStation::mediastationGames) {
	_maxScanDepth = 3;
	_directoryGlobs = MediaStation::directoryGlobs;
}

REGISTER_PLUGIN_STATIC(MEDIASTATION_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, MediaStationMetaEngineDetection);
