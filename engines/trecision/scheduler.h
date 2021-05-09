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

#ifndef TRECISION_SCHEDULER_H
#define TRECISION_SCHEDULER_H

#include "trecision/trecision.h"

namespace Trecision {

class Scheduler {
private:
	TrecisionEngine *_vm;
	int maxmesg, maxmesh, maxmesa;

public:
	Scheduler(TrecisionEngine *vm);
	~Scheduler();

	void process();
	void doEvent(uint8 cls, uint8 event, uint8 priority, uint16 u16Param1, uint16 u16Param2, uint8 u8Param, uint32 u32Param);

}; // end of class

} // end of namespace
#endif

