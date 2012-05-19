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
 *
 */

#ifndef TONY_MPAL_MPALUTILS
#define TONY_MPAL_MPALUTILS

#include "common/scummsys.h"
#include "tony/mpal/memory.h"

namespace Tony {

namespace MPAL {

class RMRes {
protected:
	HGLOBAL m_h;
    byte *m_buf;

public:
	RMRes(uint32 resID);
    virtual ~RMRes();

    // Attributes
	unsigned int Size();
    const byte *DataPointer();
	bool IsValid() { return m_h != NULL; }

    // Casting for access to data
    operator const byte*();
};

class RMResRaw : public RMRes {
public:
	RMResRaw(uint32 resID);
	virtual ~RMResRaw();
		
	const byte *DataPointer();
	operator const byte*();

		int Width();
		int Height();
};

} // end of namespace MPAL

} // end of namespace Tony

#endif
