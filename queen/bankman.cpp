/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 The ScummVM project
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

#include "stdafx.h"
#include "queen/bankman.h"

#include "queen/resource.h"

namespace Queen {

BankManager::BankManager(Resource *res) 
	: _res(res) {
	memset(_frames, 0, sizeof(_frames));
	memset(_banks, 0, sizeof(_banks));
	memset(_loadedBanks, 0, sizeof(_loadedBanks));
}

BankManager::~BankManager() {
	for (uint32 i = 0; i < MAX_BANKS_NUMBER; ++i) {
		close(i);
	}
	eraseFrames(true);
}

void BankManager::load(const char *bankname, uint32 bankslot) {
	debug(9, "BankManager::load(%s, %d)", bankname, bankslot);
	assert(bankslot < MAX_BANKS_NUMBER);

	if (!scumm_stricmp(bankname, _loadedBanks[bankslot])) {
		debug(9, "BankManager::load() bank '%s' already loaded", bankname);
		return;
	}

	close(bankslot);
	_banks[bankslot].data = _res->loadFile(bankname);

	uint16 entries = READ_LE_UINT16(_banks[bankslot].data);
	assert(entries < MAX_BANK_SIZE);
	debug(9, "BankManager::load() entries = %d", entries); 

	uint32 offset = 2;
	const uint8 *p = _banks[bankslot].data;
	for (uint16 i = 1; i <= entries; ++i) {
		_banks[bankslot].indexes[i] = offset;
		uint16 w = READ_LE_UINT16(p + offset + 0);
		uint16 h = READ_LE_UINT16(p + offset + 2);
		// jump to next entry, skipping data & header
		offset += w * h + 8; 
	}

	// mark this bank as loaded
	strcpy(_loadedBanks[bankslot], bankname);
}

void BankManager::unpack(uint32 srcframe, uint32 dstframe, uint32 bankslot) {
	debug(9, "BankManager::unpack(%d, %d, %d)", srcframe, dstframe, bankslot);
	assert(bankslot < MAX_BANKS_NUMBER);
	assert(_banks[bankslot].data != NULL);
		
	BobFrame *pbf = &_frames[dstframe];
	const uint8 *p = _banks[bankslot].data + _banks[bankslot].indexes[srcframe];
	pbf->width    = READ_LE_UINT16(p + 0);
	pbf->height   = READ_LE_UINT16(p + 2);
	pbf->xhotspot = READ_LE_UINT16(p + 4);
	pbf->yhotspot = READ_LE_UINT16(p + 6);

	uint32 size = pbf->width * pbf->height;
	delete[] pbf->data;
	pbf->data = new uint8[ size ];
	memcpy(pbf->data, p + 8, size);
}

void BankManager::overpack(uint32 srcframe, uint32 dstframe, uint32 bankslot) {
	debug(9, "BankManager::overpack(%d, %d, %d)", srcframe, dstframe, bankslot);
	assert(bankslot < MAX_BANKS_NUMBER);
	assert(_banks[bankslot].data != NULL);

	const uint8 *p = _banks[bankslot].data + _banks[bankslot].indexes[srcframe];
	uint16 src_w = READ_LE_UINT16(p + 0);
	uint16 src_h = READ_LE_UINT16(p + 2);

	// unpack if destination frame is smaller than source one
	if (_frames[dstframe].width < src_w || _frames[dstframe].height < src_h) {
		unpack(srcframe, dstframe, bankslot);
	} else {
		// copy data 'over' destination frame (without changing frame header)
		memcpy(_frames[dstframe].data, p + 8, src_w * src_h);
	}
}

void BankManager::close(uint32 bankslot) {
	debug(9, "BankManager::close(%d)", bankslot);
	assert(bankslot < MAX_BANKS_NUMBER);
	delete[] _banks[bankslot].data;
	memset(&_banks[bankslot], 0, sizeof(PackedBank));
	_loadedBanks[bankslot][0] = '\0';
}

BobFrame *BankManager::fetchFrame(uint32 index) {
	debug(9, "BankManager::fetchFrame(%d)", index);
	assert(index < MAX_FRAMES_NUMBER);
	BobFrame *pbf = &_frames[index];
	assert(pbf->data != 0);
	return pbf;
}

void BankManager::eraseFrame(uint32 index) {
	debug(9, "BankManager::eraseFrame(%d)", index);
	assert(index < MAX_FRAMES_NUMBER);
	BobFrame *pbf = &_frames[index];
	delete[] pbf->data;
	memset(pbf, 0, sizeof(BobFrame));
}

void BankManager::eraseFrames(bool joe) {
	uint32 i = 0;
	if (!joe) {
		i = FRAMES_JOE;
	}
	while (i < MAX_FRAMES_NUMBER) {
		eraseFrame(i);
		++i;
	}
}

} // End of namespace Queen
