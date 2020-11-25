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

#ifndef SCUMM_DETECTION_STEAM_H
#define SCUMM_DETECTION_STEAM_H

namespace Scumm {

// The following table includes all the index files, which are embedded in the
// main game executables in Steam versions.
static const SteamIndexFile steamIndexFiles[] = {
	{ GID_INDY3, Common::kPlatformWindows,   "%02d.LFL",      "00.LFL",        "Indiana Jones and the Last Crusade.exe",     162056,  6295 },
	{ GID_INDY3, Common::kPlatformMacintosh, "%02d.LFL",      "00.LFL",        "The Last Crusade",                           150368,  6295 },
	{ GID_INDY4, Common::kPlatformWindows,   "atlantis.%03d", "ATLANTIS.000",  "Indiana Jones and the Fate of Atlantis.exe", 224336, 12035 },
	{ GID_INDY4, Common::kPlatformMacintosh, "atlantis.%03d", "ATLANTIS.000",  "The Fate of Atlantis",                       260224, 12035 },
	{ GID_LOOM,  Common::kPlatformWindows,   "%03d.LFL",      "000.LFL",       "Loom.exe",                                   187248,  8307 },
	{ GID_LOOM,  Common::kPlatformMacintosh, "%03d.LFL",      "000.LFL",       "Loom",                                       170464,  8307 },
	{ GID_DIG,   Common::kPlatformWindows,   "dig.la%d",      "DIG.LA0",       "The Dig.exe",                                340632, 16304 },
	{ GID_DIG,   Common::kPlatformMacintosh, "dig.la%d",      "DIG.LA0",       "The Dig",                                    339744, 16304 },
	{ 0,         Common::kPlatformUnknown,   nullptr,         nullptr,         nullptr,                                           0,     0 }
};

static const SteamIndexFile *lookUpSteamIndexFile(Common::String pattern, Common::Platform platform) {
	for (const SteamIndexFile *indexFile = steamIndexFiles; indexFile->len; ++indexFile) {
		if (platform == indexFile->platform && pattern.equalsIgnoreCase(indexFile->pattern))
			return indexFile;
	}

	return nullptr;
}

} // End of namespace Scumm

#endif // SCUMM_DETECTION_STEAM_H
