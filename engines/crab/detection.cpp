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
#include "engines/advancedDetector.h"

#include "crab/detection.h"

const DebugChannelDef CrabMetaEngineDetection::debugFlagList[] = {
	{Crab::kDebugGraphics, "Graphics", "Graphics debug level"},
	{Crab::kDebugPath, "Path", "Pathfinding debug level"},
	{Crab::kDebugFilePath, "FilePath", "File path debug level"},
	{Crab::kDebugScan, "Scan", "Scan for unrecognised games"},
	{Crab::kDebugScript, "Script", "Enable debug script dump"},
	DEBUG_CHANNEL_END};

static const PlainGameDescriptor crabGames[] = {
	{"unrest", "Unrest"},
	{nullptr, nullptr}};

namespace Crab {

static const ADGameDescription gameDescriptions[] = {
	{
		"unrest",
		"",
		AD_ENTRY2s("characters/backer_chars.xml", "d9028280f75836192d93e56df7313d8c", 2517,
				   "traits/tanya_intelligent.png", "441221becc2b70e7c9d7838d6a8bd990", 12593),
		Common::EN_ANY,
		Common::kPlatformUnknown, // These files are same on Windows, Linux and MacOS. Therefore, do not specify platform.
		ADGF_DROPLANGUAGE | ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO3(GUIO_NOSPEECH, GUIO_NOLAUNCHLOAD, GUIO_NOMIDI),
	},
	{
		"unrest",
		"Demo",
		AD_ENTRY2s("characters/backer_chars.xml", "5ba20126349bc57b3774623a1ec89627", 2127,
				   "traits/tanya_intelligent.png", "441221becc2b70e7c9d7838d6a8bd990", 12593),
		Common::EN_ANY,
		Common::kPlatformUnknown, // These files are same on Windows, Linux and MacOS. Therefore, do not specify platform.
		ADGF_DEMO | ADGF_TESTING,
		GUIO3(GUIO_NOSPEECH, GUIO_NOLAUNCHLOAD, GUIO_NOMIDI),
	},
	AD_TABLE_END_MARKER};

}

CrabMetaEngineDetection::CrabMetaEngineDetection() : AdvancedMetaEngineDetection(Crab::gameDescriptions,
																				 crabGames) {
	_flags = kADFlagMatchFullPaths;
}

REGISTER_PLUGIN_STATIC(CRAB_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, CrabMetaEngineDetection);
