/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "common/file.h"

#include "saga.h"

#include "resfile.h"
#include "binread.h"


ResourceFile::ResourceFile() {
	_resTblOffset = 0;
	_resTblCt     = 0;
	_resTblLoaded = false;
	_resTbl       = NULL;
}

ResourceFile::~ResourceFile() {
	close();
}

bool ResourceFile::open(const char *filename, const char *directory) {
	byte * temp_tbl;
	uint32 tbl_len;

	if (!File::open(filename, directory)) {
		return false;
	}

	/* Seek to end of file to read resource table 'trailer' */
	_file_len = size();

	seek(_file_len - RSC_TABLEINFO_SIZE, SEEK_SET);

	_resTblOffset = readSint32LE();
	_resTblCt     = readSint32LE();

	/* Check for sane values */
	if (_resTblOffset != _file_len - RSC_TABLEINFO_SIZE - 
		(RSC_TABLEENTRY_SIZE * _resTblCt)) {
		return false;
	}
	
	/* Load resource table */
	_resTbl = new Resource[_resTblCt];

	seek(_resTblOffset, SEEK_SET);

	tbl_len = _resTblCt * RSC_TABLEENTRY_SIZE;
	temp_tbl = new byte[tbl_len];

	if (read(temp_tbl, tbl_len) != tbl_len) {
		delete [] _resTbl;
		delete [] temp_tbl;
		return false;
	}

	BinReader bread(temp_tbl, tbl_len);

	for (int i = 0; i < _resTblCt; i++) {
		_resTbl[i].res_offset = bread.readSint32LE();
		_resTbl[i].res_len    = bread.readSint32LE();
	}

	delete[] temp_tbl;

	_resTblLoaded = true;

	return true;
}


void ResourceFile::close() {
	if ( _resTblLoaded) {
		delete [] _resTbl;
		_resTblLoaded = false;
		_resTbl = NULL;
	}

	_resTblOffset = 0;
	_resTblCt = 0;

	if (File::isOpen()) {
		File::close();
	}
}

int32 ResourceFile::getResourceCt() {
	return (_resTblLoaded) ? _resTblCt : -1;
}

int32 ResourceFile::getResourceOffset(int32 rn) {
	if (!R_PBOUNDS(rn, _resTblCt))
		return -1;

	return _resTbl[rn].res_offset;
}

int32 ResourceFile::getResourceLen(int32 rn) {
	if (!R_PBOUNDS(rn, _resTblCt))
		return -1;

	return _resTbl[rn].res_len;
}

bool ResourceFile::loadResource(int32 rn, byte **res, int32 *res_len) {
	byte *new_res;
	uint32 new_res_len;

	assert(res != NULL && res_len != NULL);
	*res = NULL;
	*res_len = NULL;

	if (!R_PBOUNDS( rn, _resTblCt)) {
		return false;
	}

	new_res_len = _resTbl[rn].res_len;
	new_res = new byte[new_res_len];

	if (!new_res) {
		return false;
	}

	seek(_resTbl[rn].res_offset, SEEK_SET);

	if (read(new_res, new_res_len) != new_res_len) {
		delete[] new_res;
		return false;
	}

	*res = new_res;
	*res_len = new_res_len;
		
	return true;
}

void ResourceFile::freeResource(byte *res) {
	delete[] res;
}











