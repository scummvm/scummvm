/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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
 */

#include "stdafx.h"
#include "common/scummsys.h"
#include "scumm/scumm.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

namespace Scumm {

BundleMgr::BundleMgr() {
	_compTable = NULL;
	_bundleTable = NULL;
	_numFiles = 0;
	_curSample = -1;
}

BundleMgr::~BundleMgr() {
	free(_bundleTable);
	free(_compTable);
}

bool BundleMgr::openFile(const char *filename, const char *directory) {
	int32 tag, offset;

	if (_file.isOpen())
		return true;

	if (_file.open(filename, directory) == false) {
		warning("BundleMgr::openFile() Can't open bundle file: %s", filename);
		return false;
	}

/*
 TODO / FIXME
This is another spot were lots and lots of time is wasted, because the same data is read over
and over again from the disk. Disk I/O is *slooow*.
This function by itself actually isn't what is really slow - normally we would call
it once and then be done with it.
However, for whatever strange reasons, the higher level code constantly keeps creating
new BundleMgr, uses them to open a file, play a piece of sound, then delete the BundleMgr.
Repeat ad infinitum -> extremly slow.
*/

	tag = _file.readUint32BE();
	offset = _file.readUint32BE();
	_numFiles = _file.readUint32BE();

	_bundleTable = (AudioTable *) malloc(_numFiles * sizeof(AudioTable));

	_file.seek(offset, SEEK_SET);

	for (int32 i = 0; i < _numFiles; i++) {
		char name[13], c;
		int32 z = 0;
		int32 z2;

		for (z2 = 0; z2 < 8; z2++)
			if ((c = _file.readByte()) != 0)
				name[z++] = c;
		name[z++] = '.';
		for (z2 = 0; z2 < 4; z2++)
			if ((c = _file.readByte()) != 0)
				name[z++] = c;
		name[z] = '\0';
		strcpy(_bundleTable[i].filename, name);
		_bundleTable[i].offset = _file.readUint32BE();
		_bundleTable[i].size = _file.readUint32BE();
	}

	return true;
}

void BundleMgr::closeFile() {
	if (_file.isOpen()) {
		_file.close();
		free(_bundleTable);
		_bundleTable = NULL;
	}
}

int32 BundleMgr::decompressSampleByCurIndex(int32 offset, int32 size, byte **comp_final, int header_size) {
	return decompressSampleByIndex(_curSample, offset, size, comp_final, header_size);
}

int32 BundleMgr::decompressSampleByIndex(int32 index, int32 offset, int32 size, byte **comp_final, int header_size) {
	int32 i, tag, num, final_size, output_size;
	byte *comp_input, *comp_output;
	
	if (index != -1)
		_curSample = index;

	if (_file.isOpen() == false) {
		warning("BundleMgr::decompressSampleByIndex() File is not open!");
		return 0;
	}

/*
FIXME / TODO
 This function is a major speed hog. It re-reads the same data over and over and over again.
 Disk I/O is about the slowest thing on any modern computer. We used to cache all this data,
 with good reason... this will have to be done again.
*/

	_file.seek(_bundleTable[index].offset, SEEK_SET);
	tag = _file.readUint32BE();
	num = _file.readUint32BE();
	_file.readUint32BE();
	_file.readUint32BE();

	if (tag != MKID_BE('COMP')) {
		warning("BundleMgr::decompressSampleByIndex() Compressed sound %d invalid (%s)", index, tag2str(tag));
		return 0;
	}

	free(_compTable);
	_compTable = (CompTable *)malloc(sizeof(CompTable) * num);
	for (i = 0; i < num; i++) {
		_compTable[i].offset = _file.readUint32BE();
		_compTable[i].size = _file.readUint32BE();
		_compTable[i].codec = _file.readUint32BE();
		_file.readUint32BE();
	}

	int first_block = (offset + header_size) / 0x2000;
	int last_block = (offset + size + header_size - 1) / 0x2000;

	comp_output = (byte *)malloc(0x2000);
	*comp_final = (byte *)malloc(0x2000 * (1 + last_block - first_block));
	final_size = 0;

	int skip = offset - (first_block * 0x2000) + header_size;

	for (i = first_block; i <= last_block; i++) {
		assert(size);
		// CMI hack: one more zero byte at the end of input buffer
		comp_input = (byte *)malloc(_compTable[i].size + 1);
		comp_input[_compTable[i].size] = 0;

		_file.seek(_bundleTable[index].offset + _compTable[i].offset, SEEK_SET);
		_file.read(comp_input, _compTable[i].size);

		output_size = BundleCodecs::decompressCodec(_compTable[i].codec, comp_input, comp_output, _compTable[i].size);
		assert(output_size <= 0x2000);
		if ((header_size != 0) && (skip > header_size))
			output_size -= skip;
		if (output_size > size)
			output_size = size;
		memcpy(*comp_final + final_size, comp_output + skip, output_size);
		final_size += output_size;
		size -= output_size;
		if (skip > 0)
			skip = 0;

		free(comp_input);
	}
	free(comp_output);

	return final_size;
}

int32 BundleMgr::decompressSampleByName(const char *name, int32 offset, int32 size, byte **comp_final) {
	int32 final_size = 0, i;

	if (_file.isOpen() == false) {
		warning("BundleMgr::decompressSampleByName() File is not open!");
		return 0;
	}

	for (i = 0; i < _numFiles; i++) {
		if (!scumm_stricmp(name, _bundleTable[i].filename)) {
			final_size = decompressSampleByIndex(i, offset, size, comp_final, 0);
			return final_size;
		}
	}
	debug(2, "BundleMgr::decompressSampleByName() Failed finding voice %s", name);
	return final_size;
}

} // End of namespace Scumm
