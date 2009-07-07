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
#include "graphics/thumbnail.h"

#include "kyra/kyra_v1.h"
#include "kyra/util.h"

#define CURRENT_SAVE_VERSION 16

#define GF_FLOPPY  (1 <<  0)
#define GF_TALKIE  (1 <<  1)
#define GF_FMTOWNS (1 <<  2)

namespace Kyra {

KyraEngine_v1::kReadSaveHeaderError KyraEngine_v1::readSaveHeader(Common::SeekableReadStream *in, bool loadThumbnail, SaveHeader &header) {
	uint32 type = in->readUint32BE();
	header.originalSave = false;
	header.oldHeader = false;
	header.flags = 0;
	header.thumbnail = 0;

	if (type == MKID_BE('KYRA') || type == MKID_BE('ARYK')) { // old Kyra1 header ID
		header.gameID = GI_KYRA1;
		header.oldHeader = true;
	} else if (type == MKID_BE('HOFS')) { // old Kyra2 header ID
		header.gameID = GI_KYRA2;
		header.oldHeader = true;
	} else if (type == MKID_BE('WWSV')) {
		header.gameID = in->readByte();
	} else {
		// try checking for original save header
		const int descriptionSize[2] = { 30, 80 };
		char descriptionBuffer[81];

		bool saveOk = false;

		for (uint i = 0; i < ARRAYSIZE(descriptionSize) && !saveOk; ++i) {
			in->seek(0, SEEK_SET);
			in->read(descriptionBuffer, descriptionSize[i]);
			descriptionBuffer[descriptionSize[i]] = 0;

			Util::convertDOSToISO(descriptionBuffer);

			type = in->readUint32BE();
			header.version = in->readUint16LE();
			if (type == MKID_BE('MBL3') && header.version == 100) {
				saveOk = true;
				header.description = descriptionBuffer;
				header.gameID = GI_KYRA2;
				break;
			} else if (type == MKID_BE('MBL4') && header.version == 102) {
				saveOk = true;
				header.description = descriptionBuffer;
				header.gameID = GI_KYRA3;
				break;
			}
		}

		if (saveOk) {
			header.originalSave = true;
			header.description = descriptionBuffer;
			return kRSHENoError;
		} else {
			return kRSHEInvalidType;
		}
	}

	header.version = in->readUint32BE();
	if (header.version > CURRENT_SAVE_VERSION || (header.oldHeader && header.version > 8) || (type == MKID_BE('ARYK') && header.version > 3))
		return kRSHEInvalidVersion;

	// Versions prior to 9 are using a fixed length description field
	if (header.version <= 8) {
		char buffer[31];
		in->read(buffer, 31);
		// WORKAROUND: Old savegames could contain a missing termination 0 at the
		// end so we manually add it.
		buffer[30] = 0;
		header.description = buffer;
	} else {
		header.description = "";
		for (char c = 0; (c = in->readByte()) != 0;)
			header.description += c;
	}

	if (header.version >= 2)
		header.flags = in->readUint32BE();

	if (header.version >= 14) {
		if (loadThumbnail) {
			header.thumbnail = new Graphics::Surface();
			assert(header.thumbnail);
			if (!Graphics::loadThumbnail(*in, *header.thumbnail)) {
				delete header.thumbnail;
				header.thumbnail = 0;
			}
		} else {
			Graphics::skipThumbnailHeader(*in);
		}
	}

	return (in->ioFailed() ? kRSHEIoError : kRSHENoError);
}

Common::SeekableReadStream *KyraEngine_v1::openSaveForReading(const char *filename, SaveHeader &header) {
	Common::SeekableReadStream *in = 0;
	if (!(in = _saveFileMan->openForLoading(filename)))
		return 0;

	kReadSaveHeaderError errorCode = KyraEngine_v1::readSaveHeader(in, false, header);
	if (errorCode != kRSHENoError) {
		if (errorCode == kRSHEInvalidType)
			warning("No ScummVM Kyra engine savefile header.");
		else if (errorCode == kRSHEInvalidVersion)
			warning("Savegame is not the right version (%u, '%s')", header.version, header.oldHeader ? "true" : "false");
		else if (errorCode == kRSHEIoError)
			warning("Load failed '%s'", filename);

		delete in;
		return 0;
	}

	if (!header.originalSave) {
		if (!header.oldHeader) {
			if (header.gameID != _flags.gameID) {
				warning("Trying to load game state from other game (save game: %u, running game: %u)", header.gameID, _flags.gameID);
				delete in;
				return 0;
			}
		}

		if (header.version < 2) {
			warning("Make sure your savefile was from this version! (too old savefile version to detect that)");
		} else {
			if ((header.flags & GF_FLOPPY) && (_flags.isTalkie || _flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)) {
				warning("Can not load DOS Floppy savefile for this (non DOS Floppy) gameversion");
				delete in;
				return 0;
			} else if ((header.flags & GF_TALKIE) && !(_flags.isTalkie)) {
				warning("Can not load DOS CD-ROM savefile for this (non DOS CD-ROM) gameversion");
				delete in;
				return 0;
			} else if ((header.flags & GF_FMTOWNS) && !(_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)) {
				warning("Can not load FM-TOWNS/PC98 savefile for this (non FM-TOWNS/PC98) gameversion");
				delete in;
				return 0;
			}
		}
	}

	return in;
}

Common::WriteStream *KyraEngine_v1::openSaveForWriting(const char *filename, const char *saveName, const Graphics::Surface *thumbnail) const {
	if (shouldQuit())
		return 0;

	Common::WriteStream *out = 0;
	if (!(out = _saveFileMan->openForSaving(filename))) {
		warning("Can't create file '%s', game not saved", filename);
		return 0;
	}

	// Savegame version
	out->writeUint32BE(MKID_BE('WWSV'));
	out->writeByte(_flags.gameID);
	out->writeUint32BE(CURRENT_SAVE_VERSION);
	out->write(saveName, strlen(saveName)+1);
	if (_flags.isTalkie)
		out->writeUint32BE(GF_TALKIE);
	else if (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)
		out->writeUint32BE(GF_FMTOWNS);
	else
		out->writeUint32BE(GF_FLOPPY);

	if (out->err()) {
		warning("Can't write file '%s'. (Disk full?)", filename);
		delete out;
		return 0;
	}

	if (thumbnail)
		Graphics::saveThumbnail(*out, *thumbnail);
	else
		Graphics::saveThumbnail(*out);

	return out;
}

const char *KyraEngine_v1::getSavegameFilename(int num) {
	static Common::String filename;
	filename = getSavegameFilename(_targetName, num);
	return filename.c_str();
}

Common::String KyraEngine_v1::getSavegameFilename(const Common::String &target, int num) {
	assert(num >= 0 && num <= 999);

	char extension[5];
	sprintf(extension, "%03d", num);

	return target + "." + extension;
}

bool KyraEngine_v1::saveFileLoadable(int slot) {
	if (slot < 0 || slot > 999)
		return false;

	SaveHeader header;
	Common::SeekableReadStream *in = openSaveForReading(getSavegameFilename(slot), header);

	if (in) {
		delete in;
		return true;
	}

	return false;
}

void KyraEngine_v1::checkAutosave() {
	if (shouldPerformAutoSave(_lastAutosave)) {
		saveGameState(999, "Autosave", 0);
		_lastAutosave = _system->getMillis();
	}
}

void KyraEngine_v1::loadGameStateCheck(int slot) {
	if (loadGameState(slot) != Common::kNoError) {
		const char *filename = getSavegameFilename(slot);
		Common::String errorMessage = "Could not load savegame: '";
		errorMessage += filename;
		errorMessage += "'";

		GUIErrorMessage(errorMessage);
		error("%s", errorMessage.c_str());
	}
}

} // end of namespace Kyra

