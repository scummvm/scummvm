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

#ifndef QUEENBANKMAN_H
#define QUEENBANKMAN_H

#include "common/util.h"
#include "queen/structs.h"

namespace Queen {

class Resource;

class BankManager {
public:

	BankManager(Resource *res);
	~BankManager();

	//! load a bank into the specified slot
	void load(const char *bankname, uint32 bankslot);
	
	//! unpack a frame from a loaded bank
	void unpack(uint32 srcframe, uint32 dstframe, uint32 bankslot);
	
	//! unpack a frame over an existing one from a loaded bank
	void overpack(uint32 srcframe, uint32 dstframe, uint32 bankslot);
	
	//! close a bank
	void close(uint32 bankslot);
	
	//! get a reference to unpacked frame
	BobFrame *fetchFrame(uint32 index);
	
	//! erase a frame
	void eraseFrame(uint32 index);
	
	//! erase all unpacked frames
	void eraseFrames(bool joe);

	enum {
		MAX_BANK_SIZE     = 110,
		MAX_FRAMES_NUMBER = 256,
		MAX_BANKS_NUMBER  =  18
	};

private:

	struct PackedBank {
		uint32 indexes[MAX_BANK_SIZE];
		uint8 *data;
	};

	//! unpacked bob frames
	BobFrame _frames[MAX_FRAMES_NUMBER];

	 //! banked bob frames
	PackedBank _banks[MAX_BANKS_NUMBER];

	//! loaded banks names
	char _loadedBanks[MAX_BANKS_NUMBER][20];

	Resource *_res;
};

} // End of namespace Queen

#endif
