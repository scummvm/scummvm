/* AWE - Another World Engine
 * Copyright (C) 2004 Gregory Montoir
 * Copyright (C) 2004 The ScummVM project
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
 */

#ifndef __BANK_H__
#define __BANK_H__

#include "stdafx.h"

#include "intern.h"

namespace Awe {

struct MemEntry;

struct UnpackContext {
	uint16 size;
	uint32 crc;
	uint32 chk;
	int32 datasize;
};

struct Bank {
	UnpackContext _unpCtx;
	const char *_dataDir;
	uint8 *_iBuf, *_oBuf, *_startBuf;

	Bank(const char *dataDir);

	bool read(const MemEntry *me, uint8 *buf);
	void decUnk1(uint8 numChunks, uint8 addCount);
	void decUnk2(uint8 numChunks);
	bool unpack();
	uint16 getCode(uint8 numChunks);
	bool nextChunk();
	bool rcr(bool CF);
};

}

#endif
