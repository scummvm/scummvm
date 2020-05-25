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

#ifndef ULTIMA8_KERNEL_OBJECT_H
#define ULTIMA8_KERNEL_OBJECT_H

#include "ultima/ultima8/misc/p_dynamic_cast.h"
#include "ultima/ultima8/misc/pent_include.h"

namespace Ultima {
namespace Ultima8 {

class Usecode;

class Object {
public:
	Object() : _objId(0xFFFF) {}
	virtual ~Object();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE_BASE()

	//! get this Object's objID
	inline ObjId getObjId() const {
		return _objId;
	}

	//! Assign self and contents (if any) an objID
	//! \return the assiged ID
	virtual ObjId assignObjId();

	//! Clear objID of self and contents (if any)
	virtual void clearObjId();

	//! dump some info about this object to pout
	virtual void dumpInfo() const;

	//! Spawn a usecode function on this object
	//! \param classid The usecode class to run
	//! \param offset The offset in that class to run
	//! \param u The Usecode object containing the class
	//! \param args Optional arguments to the spawned process
	//! \param argsize The size (in bytes) of the optional arguments
	//! \return the PID of the spawned process
	ProcId callUsecode(uint16 classid, uint16 offset,
	                   const uint8 *args = 0, int argsize = 0);

	bool loadData(Common::ReadStream *rs, uint32 version);
	virtual void saveData(Common::WriteStream *ws);

protected:
	ObjId _objId;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
