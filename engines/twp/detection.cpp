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
#include "common/file.h"
#include "common/translation.h"
#include "twp/detection.h"
#include "twp/detection_tables.h"

const DebugChannelDef TwpMetaEngineDetection::debugFlagList[] = {
	{Twp::kDebugText, "Text", "Text debug level"},
	{Twp::kDebugGGPack, "GGPack", "GGPack debug level"},
	{Twp::kDebugRes, "Res", "Resource debug level"},
	{Twp::kDebugDialog, "Dialog", "Dialog debug level"},
	{Twp::kDebugGenScript, "General", "Enable debug general script dump"},
	{Twp::kDebugObjScript, "Object", "Enable debug object script dump"},
	{Twp::kDebugSysScript, "System", "Enable debug system script dump"},
	{Twp::kDebugRoomScript, "Room", "Enable debug room script dump"},
	{Twp::kDebugActScript, "Actor", "Enable debug actor script dump"},
	{Twp::kDebugSndScript, "Sound", "Enable debug sound script dump"},
	{Twp::kDebugGame, "Game", "Game debug level"},
	DEBUG_CHANNEL_END};

TwpMetaEngineDetection::TwpMetaEngineDetection() : AdvancedMetaEngineDetection(Twp::gameDescriptions,
																			   sizeof(ADGameDescription), Twp::twpGames) {
}

ADDetectedGame TwpMetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const {
	for (const auto & allFile : allFiles) {
		if (allFile._key.toString().hasSuffixIgnoreCase(".ggpack1")) {
			return ADDetectedGame(Twp::gameDescriptions);
		}
	}
	return ADDetectedGame();
}

REGISTER_PLUGIN_STATIC(TWP_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, TwpMetaEngineDetection);
