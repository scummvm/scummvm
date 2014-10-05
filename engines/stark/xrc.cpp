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

#include "engines/stark/xrc.h"
#include "engines/stark/debug.h"

#include "common/debug-channels.h"
#include "common/util.h"

namespace Stark {

XRCNode::XRCNode() :
		_data(nullptr),
		_parent(nullptr) {
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
	_nodeOrder = stream->readUint16LE();

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
		if (bytesRead != _dataLength) {
			warning("Stark::XRCNode: data length mismatch (%d != %d)", bytesRead, _dataLength);
			return false;
		}
	}

	// Get the number of children
	uint16 numChildren = stream->readUint16LE();

	// Read more unknown data
	_unknown3 = stream->readUint16LE();
	if (_unknown3 != 0) {
		warning("Stark::XRCNode: \"%s\" has unknown3=0x%04X with unknown meaning", _name.c_str(), _unknown3);
	}

	// Read the children nodes
	_children.reserve(numChildren);
	for (int i = 0; i < numChildren; i++) {
		XRCNode *child = XRCNode::read(stream);

		// If we can't read a children node, consider this node as broken
		if (!child)
			return false;

		// Set the child's parent to the current node
		child->_parent = this;

		// Save all children read correctly
		_children.push_back(child);
	}

	return true;
}

void XRCNode::print(uint depth) {
	// Display value for the node type
	Common::String type(getTypeName());
	if (type.empty()) {
		type = Common::String::format("%d", _dataType);
	}

	// Build the node description
	Common::String description;
	for (uint i = 0; i < depth; i++) {
		description += "-";
	}
	description += Common::String::format(" %s - (%s) - (unk1=%d, order=%x)", _name.c_str(), type.c_str(), _unknown1, _nodeOrder);

	// Print tge node description
	debug(description.c_str());

	// Print the node data
	if (_data) {
		Common::hexdump(_data, _dataLength);
	}

	// Recursively print the children nodes
	for (uint i = 0; i < _children.size(); i++) {
		_children[i]->print(depth + 1);
	}
}

const char *XRCNode::getTypeName() {
	static const struct {
		Type type;
		const char *name;
	} typeNames[] {
			{ kLevel, "Level" },
			{ kRoom,  "Room"  }
	};

	for (uint i = 0; i < ARRAYSIZE(typeNames); i++) {
		if (typeNames[i].type == _dataType) {
			return typeNames[i].name;
		}
	}

	return nullptr;
}

Common::String XRCNode::getArchive() {
	Common::String archive;

	switch (getType()) {
	case kLevel:
		switch (_unknown1) {
		case 1:
			archive = Common::String::format("%s/%s.xarc", _name.c_str(), _name.c_str());
			break;
		case 2:
			archive = Common::String::format("%02x/%02x.xarc", _nodeOrder, _nodeOrder);
			break;
		default:
			error("Unknown level archive type %d", _unknown1);
		}
		break;
	case kRoom:
		assert(_parent);
		archive = Common::String::format("%02x/%02x/%02x.xarc", _parent->_nodeOrder, _nodeOrder, _nodeOrder);
		break;
	default:
		error("This type of node cannot load children %d", _dataType);
	}

	return archive;
}

} // End of namespace Stark
