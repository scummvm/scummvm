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
#include "common/util.h"
#include "m4/detection.h"
#include "m4/detection_tables.h"

const DebugChannelDef M4MetaEngineDetection::debugFlagList[] = {
	{ M4::kDebugScripts, "scripts", "Script debug level" },
	{ M4::kDebugMessages, "messages", "Messages debug level" },
	{ M4::kDebugCore, "core", "Core debug level" },
	DEBUG_CHANNEL_END
};

static const char *const DIRECTORY_GLOBS[] = {
	"goodstuf", "resource", nullptr
};

M4MetaEngineDetection::M4MetaEngineDetection() : AdvancedMetaEngineDetection(M4::gameDescriptions,
		M4::m4Games) {
	_guiOptions = GUIO1(GAMEOPTION_ORIGINAL_SAVELOAD);
	_maxScanDepth = 2;
	_directoryGlobs = DIRECTORY_GLOBS;
}

REGISTER_PLUGIN_STATIC(M4_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, M4MetaEngineDetection);
