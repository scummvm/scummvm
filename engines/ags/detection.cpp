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
#include "common/punycode.h"
#include "ags/detection.h"
#include "ags/detection_tables.h"

namespace AGS3 {

static const char *const HEAD_SIG = "CLIB\x1a";
static const char *const TAIL_SIG = "CLIB\x1\x2\x3\x4SIGE";
#define HEAD_SIG_SIZE 5
#define TAIL_SIG_SIZE 12

/**
 * Detect the presence of an AGS game
 * TODO: This is a compact version of MFLUtil::ReadSigsAndVersion. I didn't
 * use the full version due to the complexities of including it when
 * plugins are enabled. In the future, though, it would be nice to figure
 * out, since the full version can handle not detecting on files that are
 * AGS, but only contain sounds, etc. rather than a game
 */
static bool isAGSFile(Common::File &f) {
	// Check for signature at beginning of file
	char buffer[16];
	if (f.read(buffer, HEAD_SIG_SIZE) == HEAD_SIG_SIZE &&
	        !memcmp(buffer, HEAD_SIG, HEAD_SIG_SIZE))
		return true;

	// Check for signature at end of EXE files
	f.seek(-TAIL_SIG_SIZE, SEEK_END);
	if (f.read(buffer, TAIL_SIG_SIZE) == TAIL_SIG_SIZE &&
	        !memcmp(buffer, TAIL_SIG, TAIL_SIG_SIZE))
		return true;

	return false;
}

} // namespace AGS3

const DebugChannelDef AGSMetaEngineDetection::debugFlagList[] = {
	{AGS::kDebugGraphics, "Graphics", "Graphics debug level"},
	{AGS::kDebugPath, "Path", "Pathfinding debug level"},
	{AGS::kDebugFilePath, "FilePath", "File path debug level"},
	{AGS::kDebugScan, "Scan", "Scan for unrecognised games"},
	{AGS::kDebugScript, "Script", "Enable debug script dump"},
	DEBUG_CHANNEL_END
};

AGSMetaEngineDetection::AGSMetaEngineDetection() : AdvancedMetaEngineDetection(AGS::GAME_DESCRIPTIONS, AGS::GAME_NAMES) {
	_flags = kADFlagCanPlayUnknownVariants;
}

DetectedGames AGSMetaEngineDetection::detectGames(const Common::FSList &fslist, uint32 skipADFlags, bool skipIncomplete) {
	FileMap allFiles;

	if (fslist.empty())
		return DetectedGames();

	// Compose a hashmap of all files in fslist.
	composeFileHashMap(allFiles, fslist, (_maxScanDepth == 0 ? 1 : _maxScanDepth));

	// Run the detector on this
	ADDetectedGames matches = detectGame(fslist.begin()->getParent(), allFiles, Common::UNK_LANG, Common::kPlatformUnknown, "", skipADFlags, skipIncomplete);

	cleanupPirated(matches);

	bool foundKnownGames = false;
	DetectedGames detectedGames;
	for (uint i = 0; i < matches.size(); i++) {
		DetectedGame game = toDetectedGame(matches[i]);
		if (game.hasUnknownFiles) {
			// Check the game is an AGS game
			for (FilePropertiesMap::const_iterator it = game.matchedFiles.begin(); it != game.matchedFiles.end(); it++) {
				Common::File f;
				if (f.open(allFiles[it->_key]) && AGS3::isAGSFile(f)) {
					detectedGames.push_back(game);
					break;
				}
			}
		} else {
			detectedGames.push_back(game);
			foundKnownGames = true;
		}
	}

	// If we didn't find a known game, also add a fallback detection
	if (!foundKnownGames) {
		// Use fallback detector if there were no matches by other means
		ADDetectedGame fallbackDetectionResult = fallbackDetect(allFiles, fslist);
		if (fallbackDetectionResult.desc) {
			DetectedGame fallbackDetectedGame = toDetectedGame(fallbackDetectionResult);
			fallbackDetectedGame.preferredTarget += "-fallback";

			detectedGames.push_back(fallbackDetectedGame);
		}
	}
	return detectedGames;
}

ADDetectedGame AGSMetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const {
	// Set the default values for the fallback descriptor's ADGameDescription part.
	AGS::g_fallbackDesc.desc.language = Common::UNK_LANG;
	AGS::g_fallbackDesc.desc.platform = Common::kPlatformUnknown;
	AGS::g_fallbackDesc.desc.flags = ADGF_NO_FLAGS;

	// FIXME: Hack to return match without checking for game data,
	// so that the command line game scanner will work
	if (ConfMan.get("gameid") == "ags-scan") {
		_gameid = "ags-scan";
		AGS::g_fallbackDesc.desc.gameId = "ags-scan";
		return ADDetectedGame(&AGS::g_fallbackDesc.desc);
	}

	// Set the defaults for gameid and extra
	_gameid = "ags";
	_extra.clear();
	bool hasUnknownFiles = true;

	// Scan for AGS games
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::Path filename = file->getPathInArchive();
		if (!filename.baseName().hasSuffixIgnoreCase(".exe") &&
		        !filename.baseName().hasSuffixIgnoreCase(".ags") &&
		        !filename.equalsIgnoreCase("ac2game.dat"))
			// Neither, so move on
			continue;

		filename = filename.punycodeEncode();
		Common::File f;
		if (!allFiles.contains(filename) || !f.open(allFiles[filename]))
			continue;

		if (AGS3::isAGSFile(f)) {
			f.seek(0);
			_md5 = Common::computeStreamMD5AsString(f, 5000);

			// Check whether the game is in the detection list with a different filename
			for (const ::AGS::AGSGameDescription *gameP = ::AGS::GAME_DESCRIPTIONS;
			        gameP->desc.gameId; ++gameP) {
				if (_md5 == gameP->desc.filesDescriptions[0].md5 &&
				        f.size() == gameP->desc.filesDescriptions[0].fileSize &&
					AD_NO_SIZE != gameP->desc.filesDescriptions[0].fileSize) {
					hasUnknownFiles = false;
					_gameid = gameP->desc.gameId;
					break;
				}
			}

			AGS::g_fallbackDesc.desc.gameId = _gameid.c_str();
			AGS::g_fallbackDesc.desc.extra = _extra.c_str();

			_filenameStr = filename.toString('/');
			AGS::g_fallbackDesc.desc.filesDescriptions[0].fileName = _filenameStr.c_str();
			AGS::g_fallbackDesc.desc.filesDescriptions[0].fileSize = (f.size() >= 0xffffffff) ? AD_NO_SIZE : f.size();
			AGS::g_fallbackDesc.desc.filesDescriptions[0].md5 = _md5.c_str();

			ADDetectedGame game(&AGS::g_fallbackDesc.desc);
			game.matchedFiles[filename].md5 = _md5;
			game.matchedFiles[filename].size = f.size();

			game.hasUnknownFiles = hasUnknownFiles;
			return game;
		}
	}

	return ADDetectedGame();
}

REGISTER_PLUGIN_STATIC(AGS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, AGSMetaEngineDetection);
