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
#include "hpl1/detection.h"
#include "hpl1/detection_tables.h"
#include "hpl1/debug.h"

const DebugChannelDef Hpl1MetaEngineDetection::debugFlagList[] = {
	{ Hpl1::kDebugGraphics, "Graphics", "Graphics debug level" },
	{ Hpl1::kDebugResourceLoading, "ResourceLoading", "Resource loading debug level" },
	{ Hpl1::kDebugFilePath, "FilePath", "File path debug level" }, 
	DEBUG_CHANNEL_END
};

Hpl1MetaEngineDetection::Hpl1MetaEngineDetection() : AdvancedMetaEngineDetection(Hpl1::GAME_DESCRIPTIONS,
	sizeof(ADGameDescription), Hpl1::GAME_NAMES) {
}

DetectedGames Hpl1MetaEngineDetection::detectGames(const Common::FSList &fslist, uint32 skipADFlags, bool skipIncomplete) {
	DetectedGames games; 
	for(auto file : fslist) {
		if (file.getName() == "Penumbra.exe")
			games.push_back(DetectedGame(getName(), Hpl1::GAME_NAMES[0])); 
	}
	return games; 
} 

PlainGameList Hpl1MetaEngineDetection::getSupportedGames() const {
	return PlainGameList(Hpl1::GAME_NAMES); 	
} 

PlainGameDescriptor Hpl1MetaEngineDetection::findGame(const char *gameid) const {
	return Hpl1::GAME_NAMES[0]; 
}

REGISTER_PLUGIN_STATIC(HPL1_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Hpl1MetaEngineDetection);
