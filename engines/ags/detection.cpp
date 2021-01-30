/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "base/plugins.h"
#include "common/file.h"
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

AGSMetaEngineDetection::AGSMetaEngineDetection() : AdvancedMetaEngineDetection(AGS::GAME_DESCRIPTIONS,
	        sizeof(AGS::AGSGameDescription), AGS::GAME_NAMES) {
}

ADDetectedGame AGSMetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	// Set the default values for the fallback descriptor's ADGameDescription part.
	AGS::g_fallbackDesc.desc.language = Common::UNK_LANG;
	AGS::g_fallbackDesc.desc.platform = Common::kPlatformDOS;
	AGS::g_fallbackDesc.desc.flags = ADGF_NO_FLAGS;

	// // Set the defaults for gameid and extra
	_gameid = "ags";
	_extra.clear();

	// Scan for AGS games
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String filename = file->getName();
		if (!filename.hasSuffixIgnoreCase(".exe") &&
				!filename.hasSuffixIgnoreCase(".ags") &&
				!filename.equalsIgnoreCase("ac2game.dat"))
			// Neither, so move on
			continue;

		Common::File f;
		if (!f.open(allFiles[filename]))
			continue;

		if (AGS3::isAGSFile(f)) {
			_filename = filename;

			AGS::g_fallbackDesc.desc.gameId = _gameid.c_str();
			AGS::g_fallbackDesc.desc.extra = _extra.c_str();
			AGS::g_fallbackDesc.desc.filesDescriptions[0].fileName = _filename.c_str();
			AGS::g_fallbackDesc.desc.filesDescriptions[0].fileSize = f.size();

			return ADDetectedGame(&AGS::g_fallbackDesc.desc);
		}
	}

	return ADDetectedGame();
}

REGISTER_PLUGIN_STATIC(AGS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, AGSMetaEngineDetection);
