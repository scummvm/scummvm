/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "scumm/util.h"
#include "common/util.h"

namespace Scumm {

#pragma mark -
#pragma mark --- ScummFile ---
#pragma mark -


ScummFile::ScummFile() : _encbyte(0), _subFileStart(0), _subFileLen(0) {
}

void ScummFile::setEnc(byte value) {
	_encbyte = value;
}

void ScummFile::setSubfileRange(uint32 start, uint32 len) {
	// TODO: Add sanity checks
	const uint32 fileSize = File::size();
	assert(start <= fileSize);
	assert(start + len <= fileSize);
	_subFileStart = start;
	_subFileLen = len;
	seek(0, SEEK_SET);
}

void ScummFile::resetSubfile() {
	_subFileStart = 0;
	_subFileLen = 0;
	seek(0, SEEK_SET);
}

bool ScummFile::open(const char *filename, AccessMode mode) {
	if (File::open(filename, mode)) {
		resetSubfile();
		return true;
	} else {
		return false;
	}
}

bool ScummFile::openSubFile(const char *filename) {
	assert(isOpen());

	// Disable the XOR encryption and reset any current subfile range
	setEnc(0);
	resetSubfile();

	// Read in the filename table and look for the specified file

	unsigned long file_off, file_len;
	char file_name[0x20+1];
	unsigned long i;

	// Get the length of the data file to use for consistency checks
	const uint32 data_file_len = size();
	
	// Read offset and length to the file records */
	const uint32 file_record_off = readUint32BE();
	const uint32 file_record_len = readUint32BE();

	// Do a quick check to make sure the offset and length are good
	if (file_record_off + file_record_len > data_file_len) {
		return false;
	}

	// Do a little consistancy check on file_record_length
	if (file_record_len % 0x28) {
		return false;
	}

	// Scan through the files
	for (i = 0; i < file_record_len; i += 0x28) {
		// read a file record
		seek(file_record_off + i, SEEK_SET);
		file_off = readUint32BE();
		file_len = readUint32BE();
		read(file_name, 0x20);
		file_name[0x20] = 0;

		assert(file_name[0]);
		//debug(7, "  extracting \'%s\'", file_name);
		
		// Consistency check. make sure the file data is in the file
		if (file_off + file_len > data_file_len) {
			return false;
		}
		
		if (scumm_stricmp(file_name, filename) == 0) {
			// We got a match!
			setSubfileRange(file_off, file_len);
			return true;
		}
	}

	return false;
}


bool ScummFile::eof() {
	return _subFileLen ? (pos() >= _subFileLen) : File::eof();
}

uint32 ScummFile::pos() {
	return File::pos() - _subFileStart;
}

uint32 ScummFile::size() {
	return _subFileLen ? _subFileLen : File::size();
}

void ScummFile::seek(int32 offs, int whence) {
	if (_subFileLen) {
		// Constrain the seek to the subfile
		switch (whence) {
		case SEEK_END:
			offs = _subFileStart + _subFileLen - offs;
			break;
		case SEEK_SET:
			offs += _subFileStart;
			break;
		case SEEK_CUR:
			offs += File::pos();
			break;
		}
		assert((int32)_subFileStart <= offs && offs <= (int32)(_subFileStart + _subFileLen));
		whence = SEEK_SET;
	}
	File::seek(offs, whence);
}

uint32 ScummFile::read(void *ptr, uint32 len) {
	uint32 realLen;
	
	if (_subFileLen) {
		// Limit the amount we read by the subfile boundaries.
		const uint32 curPos = pos();
		assert(_subFileLen >= curPos);
		uint32 newPos = curPos + len;
		if (newPos > _subFileLen) {
			len = _subFileLen - curPos;
			_ioFailed = true;
		}
	}
	
	realLen = File::read(ptr, len);
	
	
	// If an encryption byte was specified, XOR the data we just read by it.
	// This simple kind of "encryption" was used by some of the older SCUMM
	// games.
	if (_encbyte) {
		byte *p = (byte *)ptr;
		byte *end = p + realLen;
		while (p < end)
			*p++ ^= _encbyte;
	}

	return realLen;
}

uint32 ScummFile::write(const void *, uint32) {
	error("ScummFile does not support writing!");
}

#pragma mark -
#pragma mark --- Utilities ---
#pragma mark -

void checkRange(int max, int min, int no, const char *str) {
	if (no < min || no > max) {
		char buf[256];
		snprintf(buf, sizeof(buf), str, no);
		error("Value %d is out of bounds (%d,%d) (%s)", no, min, max, buf);
	}
}

/**
 * Convert an old style direction to a new style one (angle),
 */
int newDirToOldDir(int dir) {
	if (dir >= 71 && dir <= 109)
		return 1;
	if (dir >= 109 && dir <= 251)
		return 2;
	if (dir >= 251 && dir <= 289)
		return 0;
	return 3;
}

/**
 * Convert an new style (angle) direction to an old style one.
 */
int oldDirToNewDir(int dir) {
	assert(0 <= dir && dir <= 3);
	const int new_dir_table[4] = { 270, 90, 180, 0 };
	return new_dir_table[dir];
}

/**
 * Convert an angle to a simple direction.
 */
int toSimpleDir(int dirType, int dir) {
	if (dirType) {
		const int16 directions[] = { 22,  72, 107, 157, 202, 252, 287, 337 };
		for (int i = 0; i < 7; i++)
			if (dir >= directions[i] && dir <= directions[i+1])
				return i+1;
	} else {
		const int16 directions[] = { 71, 109, 251, 289 };
		for (int i = 0; i < 3; i++)
			if (dir >= directions[i] && dir <= directions[i+1])
				return i+1;
	}

	return 0;
}

/**
 * Convert a simple direction to an angle.
 */
int fromSimpleDir(int dirType, int dir) {
	if (dirType)
		return dir * 45;
	else
		return dir * 90;
}

/**
 * Normalize the given angle - that means, ensure it is positive, and
 * change it to the closest multiple of 45 degree by abusing toSimpleDir.
 */
int normalizeAngle(int angle) {
	int temp;

	temp = (angle + 360) % 360;

	return toSimpleDir(1, temp) * 45;
}

const char *tag2str(uint32 tag) {
	static char str[5];
	str[0] = (char)(tag >> 24);
	str[1] = (char)(tag >> 16);
	str[2] = (char)(tag >> 8);
	str[3] = (char)tag;
	str[4] = '\0';
	return str;
}

} // End of namespace Scumm
