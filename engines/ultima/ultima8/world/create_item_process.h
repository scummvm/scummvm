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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA8_WORLD_CREATEITEMPROCESS_H
#define ULTIMA8_WORLD_CREATEITEMPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class CreateItemProcess : public Process {
public:
	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE()

	CreateItemProcess();
	CreateItemProcess(uint32 shape, uint32 frame, uint16 quality,
	                  uint16 flags, uint16 npcnum, uint16 mapnum,
	                  uint32 extendedflags, int32 x, int32 y, int32 z);
	~CreateItemProcess(void) override;

	void run() override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

protected:
	uint32 _shape;
	uint32 _frame;
	uint16 _quality;
	uint16 _flags;
	uint16 _npcNum;
	uint16 _mapNum;
	uint32 _extendedFlags;
	int32 _x, _y, _z;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
