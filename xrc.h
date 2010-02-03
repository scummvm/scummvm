/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef STARK_XRC_H
#define STARK_XRC_H

#include "common/str.h"
#include "common/stream.h"

namespace Stark {

class XRCNode {
private:
	XRCNode();

public:
	~XRCNode();

	static XRCNode *read(Common::ReadStream *stream);

	Common::String getName() const { return _name; }
	const byte *getData() const { return _data; }
	Common::Array<XRCNode *> getChildren() const { return _children; }

protected:
	bool readInternal(Common::ReadStream *stream);

	byte _dataType;
	byte _unknown1;
	uint16 _unknown2;	// Scene ID? Maps to corresponding XARC in sub folders specific to the scene/room
	Common::String _name;
	uint32 _dataLength;
	byte *_data;
	Common::Array<XRCNode *> _children;
	uint16 _unknown3;
};

} // End of namespace Stark

#endif // STARK_XRC_H
