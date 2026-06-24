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

#ifndef EEM_INSTALLER_H
#define EEM_INSTALLER_H

#include "common/archive.h"

namespace Common {
class FSNode;
}

namespace EEM {

// The Macintosh floppy release of Eagle Eye Mysteries ships a custom
// self-extracting installer spread across six files:
//
//     "Eagle Eye Installer"   (disk 1: catalog + first payload span)
//     "EEM Install Data 2".."EEM Install Data 6"   (disks 2-6: payload)
//
// The catalog lives in the data fork of "Eagle Eye Installer"; each entry is
// flagged "DATASFS1" (a data file) or "APPLSFS1" (the game application) and the
// fork payloads are Compact Pro compressed (an RLE escape layer over an LZH
// stage). createInstallerArchive() parses that catalog and returns an archive
// that decompresses members on demand, so ScummVM can run straight from the
// installer files with no manual extraction step.
//
// Data files (PICS.DBD, ANI.DBX, MysteryData, ...) are exposed as plain
// members; the application and "EEM Sound&Music" resource forks are exposed as
// Mac resource-fork alt streams so Common::MacResManager can read them.
//
// Returns nullptr when the installer files are absent or cannot be parsed.
Common::Archive *createInstallerArchive(const Common::FSNode &dir);

} // End of namespace EEM

#endif
