/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "queen/graphics.h"

QueenGraphics::QueenGraphics(QueenResource *resource)
	:_resource(resource) {
		
	memset(_frames, 0, sizeof(_frames));
	memset(_banks, 0, sizeof(_banks));	
}


void QueenGraphics::bankLoad(const char *bankname, uint32 bankslot) {
	
	int16 i;
	
	if (!_resource->exists(bankname)) {
	  error("Unable to open bank '%s'", bankname);	
	}
	bankErase(bankslot);
	_banks[bankslot].data = _resource->loadFile(bankname);
	int16 entries = (int16)READ_LE_UINT16(_banks[bankslot].data);
	if (entries < 0 || entries >= MAX_BANK_SIZE) {
	  error("Maximum bank size exceeded or negative bank size : %d", entries);
	}
	
	uint32 offset = 2;
	uint8 *p = _banks[bankslot].data;
	for (i = 1; i <= entries; ++i) {
		_banks[bankslot].indexes[i] = offset;
		uint16 w = READ_LE_UINT16(p + offset + 0);
		uint16 h = READ_LE_UINT16(p + offset + 2);
		// jump to next entry, skipping data & header
		offset += w * h + 8; 
	}
	
	debug(5, "Loaded bank '%s' in slot %d, %d entries", bankname, bankslot, entries); 
}

void QueenGraphics::bankUnpack(uint32 srcframe, uint32 dstframe, uint32 bankslot) {
	
	uint8* p = _banks[bankslot].data + _banks[bankslot].indexes[srcframe];
		
	ObjectFrame* pof = &_frames[dstframe];
	delete[] pof->data;

	pof->width    = READ_LE_UINT16(p + 0);
	pof->height   = READ_LE_UINT16(p + 2);
	pof->xhotspot = READ_LE_UINT16(p + 4);
	pof->yhotspot = READ_LE_UINT16(p + 6);
	
	uint size = pof->width * pof->height;
	pof->data = new uint8[ size ];
	memcpy(pof->data, p, size);
	
	debug(5, "Unpacked frame %d from bank slot %d to frame slot %d", srcframe, bankslot, dstframe);
}


void QueenGraphics::bankOverpack(uint32 srcframe, uint32 dstframe, uint32 bankslot) {
	
	uint8 *p = _banks[bankslot].data + _banks[bankslot].indexes[srcframe];
	uint16 src_w = READ_LE_UINT16(p + 0);
	uint16 src_h = READ_LE_UINT16(p + 2);
	
	// unpack if destination frame is smaller than source one
	if (_frames[dstframe].width < src_w || _frames[dstframe].height < src_h) {
		bankUnpack(srcframe, dstframe, bankslot);
	}
	else {
		// copy data 'over' destination frame (without changing frame header)
		memcpy(_frames[dstframe].data, p, src_w * src_h);
	}
	
	debug(5, "Overpacked frame %d from bank slot %d to frame slot %d", srcframe, bankslot, dstframe);
}

void QueenGraphics::bankErase(uint32 bankslot) {
	
	delete[] _banks[bankslot].data;
	_banks[bankslot].data = 0;
	
	debug(5, "Erased bank in slot %d", bankslot);
}

