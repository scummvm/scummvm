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
#ifndef SCUMMVM_DRAGONINI_H
#define SCUMMVM_DRAGONINI_H

#include "common/system.h"
#include "bigfile.h"

namespace Dragons {

struct DragonINI
	{
		int16 iptIndex_maybe;
		int16 field_2;
		int16 actorResourceId;
		int16 frameIndexId_maybe;
		int16 field_8;
		uint16 actorId;
		uint16 sceneId;
		int16 field_e;
		uint16 field_10;
		int16 field_12;
		int16 field_14;
		int16 x;
		int16 y;
		uint16 field_1a_flags_maybe;
		int16 field_1c;
		int16 field_1e;
		uint16 field_20_actor_field_14;
	};

class DragonINIResource {
private:
	DragonINI *_dragonINI;
	uint16 _count;
	DragonINI *_flickerINI;
public:
	DragonINIResource(BigfileArchive *bigfileArchive);
	uint16 totalRecords() { return _count; }
	DragonINI *getRecord(uint16 index);
	void setFlickerRecord(DragonINI *dragonINI);
	DragonINI *getFlickerRecord() {
		return _flickerINI;
	}
};

} // End of namespace Dragons

#endif //SCUMMVM_DRAGONINI_H
