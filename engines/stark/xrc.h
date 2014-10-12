/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_XRC_H
#define STARK_XRC_H

#include "common/array.h"
#include "common/str.h"
#include "common/stream.h"

namespace Stark {

class XRCNode {
public:
	enum Type {
		kLevel = 2,
		kRoom = 3
	};

	virtual ~XRCNode();

	static XRCNode *read(Common::ReadStream *stream);

	Common::String getName() const { return _name; }
	Type getType() const {return (Type) _dataType; }
	Common::Array<XRCNode *> getChildren() const { return _children; }

	/**
	 * Get the archive file name containing the data for this node.
	 * Only Levels and Rooms have archives.
	 */
	Common::String getArchive();

	void print(uint depth = 0);

protected:
	XRCNode();

	void readCommon(Common::ReadStream *stream);
	virtual void readData(Common::ReadStream *stream) = 0;
	void readChildren(Common::ReadStream *stream);

	virtual void printData() = 0;

	const char *getTypeName();

	byte _dataType;
	byte _unknown1;
	uint16 _nodeOrder;	// Node order inside the parent node
	Common::String _name;
	uint16 _unknown3;

	XRCNode *_parent;
	Common::Array<XRCNode *> _children;
};

class UnimplementedXRCNode : public XRCNode {
public:
	virtual ~UnimplementedXRCNode();

	const byte *getData() const { return _data; }

protected:
	UnimplementedXRCNode();

	void readData(Common::ReadStream *stream) override;
	void printData() override;

	uint32 _dataLength;
	byte *_data;

	friend class XRCNode;
};

} // End of namespace Stark

#endif // STARK_XRC_H
