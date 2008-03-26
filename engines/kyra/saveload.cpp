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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"
#include "common/savefile.h"
#include "common/system.h"

#include "kyra/kyra.h"

#define CURRENT_SAVE_VERSION 8

#define GF_FLOPPY (1 <<  0)
#define GF_TALKIE (1 <<  1)
#define GF_FMTOWNS (1 <<  2)

namespace Kyra {

Common::InSaveFile *KyraEngine::openSaveForReading(const char *filename, uint32 &version, char *saveName) {
	debugC(9, kDebugLevelMain, "KyraEngine::openSaveForReading('%s', %p, %p)", filename, (const void*)&version, saveName);

	Common::InSaveFile *in = 0;
	if (!(in = _saveFileMan->openForLoading(filename))) {
		warning("Can't open file '%s', game not loaded", filename);
		return 0;
	}

	uint32 type = in->readUint32BE();

	// FIXME: The kyra savegame code used to be endian unsafe. Uncomment the
	// following line to graciously handle old savegames from LE machines.
	// if (type != MKID_BE('KYRA') && type != MKID_BE('ARYK')) {
	if (type != MKID_BE(saveGameID())) {
		warning("No ScummVM Kyra engine savefile header.");
		delete in;
		return 0;
	}

	version = in->readUint32BE();
	if (version > CURRENT_SAVE_VERSION) {
		warning("Savegame is not the right version (%u)", version);
		delete in;
		return 0;
	}

	char saveNameBuffer[31];
	if (!saveName)
		saveName = saveNameBuffer;
	in->read(saveName, 31);

	if (_flags.gameID == GI_KYRA1 && version < 2) {
		warning("Make sure your savefile was from this version! (too old savefile version to detect that)");
	} else {
		uint32 flags = in->readUint32BE();
		if ((flags & GF_FLOPPY) && (_flags.isTalkie || _flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)) {
			warning("Can not load DOS Floppy savefile for this (non DOS Floppy) gameversion");
			delete in;
			return 0;
		} else if ((flags & GF_TALKIE) && !(_flags.isTalkie)) {
			warning("Can not load DOS CD-ROM savefile for this (non DOS CD-ROM) gameversion");
			delete in;
			return 0;
		} else if ((flags & GF_FMTOWNS) && !(_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)) {
			warning("Can not load FM-Towns/PC98 savefile for this (non FM-Towns/PC98) gameversion");
			delete in;
			return 0;
		}
	}

	if (in->ioFailed()) {
		error("Load failed ('%s', '%s').", filename, saveName);
		delete in;
		return 0;
	}

	return in;
}

Common::OutSaveFile *KyraEngine::openSaveForWriting(const char *filename, const char *saveName) const {
	debugC(9, kDebugLevelMain, "KyraEngine::openSaveForWriting('%s', '%s')", filename, saveName);
	if (_quitFlag)
		return 0;

	Common::OutSaveFile *out = 0;
	if (!(out = _saveFileMan->openForSaving(filename))) {
		warning("Can't create file '%s', game not saved", filename);
		return 0;
	}

	// Savegame version
	out->writeUint32BE(saveGameID());
	out->writeUint32BE(CURRENT_SAVE_VERSION);
	out->write(saveName, 31);
	if (_flags.isTalkie)
		out->writeUint32BE(GF_TALKIE);
	else if (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)
		out->writeUint32BE(GF_FMTOWNS);
	else
		out->writeUint32BE(GF_FLOPPY);

	if (out->ioFailed()) {
		warning("Can't write file '%s'. (Disk full?)", filename);
		delete out;
		return 0;
	}

	return out;
}

const char *KyraEngine::getSavegameFilename(int num) {
	static Common::String filename;
	char extension[5];
	sprintf(extension, "%.3d", num);

	filename = _targetName + "." + extension;

	return filename.c_str();
}

} // end of namespace Kyra

