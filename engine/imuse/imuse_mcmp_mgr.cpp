/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "stdafx.h"
#include "bits.h"
#include "debug.h"
#include "resource.h"
#include "engine.h"

#include "imuse/imuse_mcmp_mgr.h"

uint16 imuseDestTable[5786];

McmpMgr::McmpMgr() {
	_compTable = NULL;
	_numCompItems = 0;
	_curSample = -1;
	_compInput = NULL;
	_outputSize = 0;
	_file = NULL;
	_numCompItems = 0;
	_lastBlock = -1;
}

McmpMgr::~McmpMgr() {
	if (_file)
		fclose(_file);
	if (_compTable)
		free(_compTable);
	if (_compInput)
		free(_compInput);
}

bool McmpMgr::openSound(const char *filename, byte **resPtr, int &offsetData) {
	_file = g_resourceloader->openNewStream(filename);

	if (!_file) {
		warning("McmpMgr::openSound() Can't open sound MCMP file: %s", filename);
		return false;
	}

	uint32 tag;
	fread(&tag, 1, 4, _file);
	if (READ_BE_UINT32(&tag) != MKID_BE('MCMP')) {
		error("McmpMgr::openSound() Expected MCMP tag");
		return false;
	}

	fread(&_numCompItems, 1, 2, _file);
	_numCompItems = READ_BE_UINT16(&_numCompItems);
	assert(_numCompItems > 0);

	int offset = ftell(_file) + (_numCompItems * 9) + 2;
	_numCompItems--;
	_compTable = (CompTable *)malloc(sizeof(CompTable) * _numCompItems);
	fseek(_file, 5, SEEK_CUR);
	fread(&_compTable[0].decompSize, 1, 4, _file);
	int headerSize = _compTable[0].decompSize = READ_BE_UINT32(&_compTable[0].decompSize);
	int maxSize = headerSize;
	offset += headerSize;

	int i;
	for (i = 0; i < _numCompItems; i++) {
		fread(&_compTable[i].codec, 1, 1, _file);
		fread(&_compTable[i].decompSize, 1, 4, _file);
		_compTable[i].decompSize = READ_BE_UINT32(&_compTable[i].decompSize);
		fread(&_compTable[i].compSize, 1, 4, _file);
		_compTable[i].compSize = READ_BE_UINT32(&_compTable[i].compSize);
		_compTable[i].offset = offset;
		offset += _compTable[i].compSize;
		if (_compTable[i].compSize > maxSize)
			maxSize = _compTable[i].compSize;
	}
	int16 sizeCodecs;
	fread(&sizeCodecs, 1, 2, _file);
	sizeCodecs = READ_BE_UINT16(&sizeCodecs);
	for (i = 0; i < _numCompItems; i++) {
		_compTable[i].offset += sizeCodecs;
	}
	fseek(_file, sizeCodecs, SEEK_CUR);
	_compInput = (byte *)malloc(maxSize);
	fread(_compInput, 1, headerSize, _file);
	*resPtr = _compInput;
	offsetData = headerSize;
	return true;
}

int32 McmpMgr::decompressSample(int32 offset, int32 size, byte **comp_final) {
	int32 i, final_size, output_size;
	int skip, first_block, last_block;

	if (!_file) {
		error("McmpMgr::decompressSampleByName() File is not open!");
		return 0;
	}

	first_block = offset / 0x2000;
	last_block = (offset + size - 1) / 0x2000;
	skip = offset % 0x2000;

	// Clip last_block by the total number of blocks (= "comp items")
	if ((last_block >= _numCompItems) && (_numCompItems > 0))
		last_block = _numCompItems - 1;

	int32 blocks_final_size = 0x2000 * (1 + last_block - first_block);
	*comp_final = (byte *)malloc(blocks_final_size);
	final_size = 0;

	for (i = first_block; i <= last_block; i++) {
		if (_lastBlock != i) {
			fseek(_file, _compTable[i].offset, SEEK_SET);
			fread(_compInput, 1, _compTable[i].compSize, _file);
			decompressVima(_compInput, (int16 *)_compOutput, _compTable[i].decompSize, imuseDestTable);
			_outputSize = _compTable[i].decompSize;
			if (_outputSize > 0x2000) {
				error("McmpMgr::decompressSample() _outputSize: %d", _outputSize);
			}
			_lastBlock = i;
		}

		output_size = _outputSize - skip;

		if ((output_size + skip) > 0x2000) // workaround
			output_size -= (output_size + skip) - 0x2000;

		if (output_size > size)
			output_size = size;

		assert(final_size + output_size <= blocks_final_size);

		memcpy(*comp_final + final_size, _compOutput + skip, output_size);
		final_size += output_size;

		size -= output_size;
		assert(size >= 0);
		if (size == 0)
			break;

		skip = 0;
	}

	return final_size;
}
