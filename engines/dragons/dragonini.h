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
 */
#ifndef DRAGONS_DRAGONINI_H
#define DRAGONS_DRAGONINI_H

#include "common/system.h"
#include "dragons/bigfile.h"

namespace Dragons {

class Actor;

enum IniFlags {
	INI_FLAG_1 = 0x1,
	INI_FLAG_2 = 0x2,
	INI_FLAG_4 = 0x4,
	INI_FLAG_8 = 0x8,
	INI_FLAG_10 = 0x10,
	INI_FLAG_20 = 0x20,
	INI_FLAG_40 = 0x40,
	INI_FLAG_80 = 0x80
};


struct DragonINI {
	uint16 id;
	int16 iptIndex_maybe;
	int16 imgId;
	int16 actorResourceId;
	uint16 sequenceId;
	int16 inventorySequenceId;
	Actor *actor;
	uint16 sceneId;
	int16 direction;
	int16 counter;
	int16 objectState;
	uint16 objectState2;
	int16 x;
	int16 y;
	uint16 flags;
	int16 baseXOffset;
	int16 baseYOffset;
	int16 direction2;
};

class DragonINIResource {
private:
	BigfileArchive *_bigfileArchive;
	DragonINI *_dragonINI;
	uint16 _count;
	DragonINI *_flickerINI;

public:
	DragonINIResource(BigfileArchive *bigfileArchive);
	void reset();
	uint16 totalRecords() { return _count; }
	DragonINI *getRecord(uint16 index);
	void setFlickerRecord(DragonINI *dragonINI);
	DragonINI *getFlickerRecord() {
		return _flickerINI;
	}
	bool isFlicker(uint16 index);
	bool isFlicker(DragonINI *ini) {
		return isFlicker(ini->id);
	}

};

} // End of namespace Dragons

#endif //DRAGONS_DRAGONINI_H
