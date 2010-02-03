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

#include "engines/stark/xrc.h"
#include "engines/stark/debug.h"

#include "common/util.h"

namespace Stark {

XRCNode::XRCNode() : _data(NULL) {
}

XRCNode::~XRCNode() {
	// Delete this node's data
	delete[] _data;

	// Delete the children nodes
	Common::Array<XRCNode *>::iterator i = _children.begin();
	while (i != _children.end()) {
		delete *i;
		i++;
	}
}

XRCNode *XRCNode::read(Common::ReadStream *stream) {
	// Create a new node
	XRCNode *node = new XRCNode();

	// Read the node contents
	if (!node->readInternal(stream)) {
		delete node;
		return NULL;
	}

	return node;
}

bool XRCNode::readInternal(Common::ReadStream *stream) {
	// Read the resource type
	_dataType = stream->readByte();

	// Read unknown data
	_unknown1 = stream->readByte();
	_unknown2 = stream->readUint16LE();

	// Read the resource name length
	uint16 nameLength = stream->readUint16LE();

	// Read the resource name
	char *name = new char[nameLength];
	stream->read(name, nameLength);
	_name = Common::String(name, nameLength);
	delete[] name;

	// Read the data length
	_dataLength = stream->readUint32LE();

	// Show a first batch of information
	debugC(10, kDebugXRC, "Stark::XRCNode: Type 0x%02X, Name: \"%s\", %d bytes", _dataType, _name.c_str(), _dataLength);

	// Read the data
	if (_dataLength) {
		_data = new byte[_dataLength];
		uint32 bytesRead = stream->read(_data, _dataLength);

		// Verify the whole array could be read
		if (bytesRead != _dataLength)
			return false;
	}

	// Get the number of children
	uint16 numChildren = stream->readUint16LE();

	// Read more unknown data
	_unknown3 = stream->readUint16LE();
	debugC(10, kDebugUnknown, "Stark::XRCNode:   \"%s\" has "
		"unknown1=0x%02X, unknown2=0x%04X, unknown3=0x%04X",
		_name.c_str(), _unknown1, _unknown2, _unknown3);
	if (_unknown3 != 0) {
		warning("Stark::XRCNode: \"%s\" has unknown3=0x%04X with unknown meaning", _name.c_str(), _unknown3);
	}


	// Show the resource data
	debugC(20, kDebugXRC, "Stark::XRCNode:   \"%s\" has %d bytes of data:", _name.c_str(), _dataLength);
	if ((gDebugLevel >= 20) && Common::isDebugChannelEnabled(kDebugXRC)) {
		Common::hexdump(_data, _dataLength);
	}
	debug(3, "   Raw: %X, %X, %X, %s, %d bytes, %X, %X", _dataType, _unknown1,
		_unknown2, _name.c_str(), _dataLength, numChildren, _unknown3);


	// Read the children nodes
	debugC(10, kDebugXRC, "Stark::XRCNode:   \"%s\" has %d children:\n", _name.c_str(), numChildren);
	_children.reserve(numChildren);
	for (int i = 0; i < numChildren; i++) {
		XRCNode *child = XRCNode::read(stream);

		// If we can't read a children node, consider this node as broken
		if (!child)
			return false;

		// Save all children read correctly
		_children.push_back(child);
	}

	return true;
}

} // End of namespace Stark
