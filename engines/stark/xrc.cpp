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
#include "common/streamdebug.h"
#include "common/util.h"

namespace Stark {

XRCNode::XRCNode() :
		_dataType(0),
		_nodeOrder(0),
		_subType(0),
		_dataLength(0),
		_unknown3(0),
		_parent(nullptr) {
}

XRCNode::~XRCNode() {
	// Delete the children nodes
	Common::Array<XRCNode *>::iterator i = _children.begin();
	while (i != _children.end()) {
		delete *i;
		i++;
	}
}

XRCNode *XRCNode::read(Common::ReadStream *stream) {
	// Read the resource type
	byte dataType = stream->readByte();
	byte subType = stream->readByte();

	// Create a new node
	XRCNode *node;
	switch (dataType) {
	case kCamera:
		node = new CameraXRCNode();
		break;
	case kFloor:
		node = new FloorXRCNode();
		break;
	case kFace:
		node = new FaceXRCNode();
		break;
	case kScript:
		node = new ScriptXRCNode();
		break;
	default:
		node = new UnimplementedXRCNode();
		break;
	}

	node->_dataType = dataType;
	node->_subType = subType;

	// Read the node contents
	node->readCommon(stream);

	// Read the node type specific data using a memory stream
	if (node->_dataLength > 0) {
		Common::SeekableReadStream *dataStream = stream->readStream(node->_dataLength);

		node->readData(dataStream);

		if (isDataLeft(dataStream)) {
			warning("Not all XRC data was read. Type %s, subtype %d", node->getTypeName(), node->_subType);
		}

		delete dataStream;
	}

	node->readChildren(stream);

	return node;
}

void XRCNode::readCommon(Common::ReadStream *stream) {
	// Read node order
	_nodeOrder = stream->readUint16LE();

	// Read the resource name
	_name = readString(stream);

	// Read the data length
	_dataLength = stream->readUint32LE();

	// Show a first batch of information
	debugC(10, kDebugXRC, "Stark::XRCNode: Type 0x%02X, Name: \"%s\", %d bytes", _dataType, _name.c_str(), _dataLength);
}

void XRCNode::readChildren(Common::ReadStream *stream) {
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

		// Set the child's parent to the current node
		child->_parent = this;

		// Save all children read correctly
		_children.push_back(child);
	}
}

Common::String XRCNode::readString(Common::ReadStream *stream) {
	// Read the string length
	uint16 length = stream->readUint16LE();

	// Read the string
	char *data = new char[length];
	stream->read(data, length);
	Common::String string(data, length);
	delete[] data;

	return string;
}

XRCNode::DataMap XRCNode::readMap(Common::ReadStream *stream) {
	// TODO: Is this really a map?
	Common::HashMap<byte, uint16> map;

	uint32 size = stream->readUint32LE();
	for (uint i = 0; i < size; i++) {
		byte b = stream->readByte();
		uint16 w = stream->readUint16LE();

		map[b] = w;
	}

	return map;
}

Math::Vector3d XRCNode::readVector3(Common::ReadStream *stream) {
	Math::Vector3d v;
	v.readFromStream(stream);
	return v;
}

Math::Vector4d XRCNode::readVector4(Common::ReadStream *stream) {
	Math::Vector4d v;
	v.readFromStream(stream);
	return v;
}

float XRCNode::readFloat(Common::ReadStream *stream) {
	float f;
	stream->read(&f, sizeof(float));
	return f;
}

bool XRCNode::isDataLeft(Common::SeekableReadStream *stream) {
	return stream->pos() < stream->size();
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
	description += Common::String::format(" %s - (%s) - (sub=%d, order=%d)", _name.c_str(), type.c_str(), _subType, _nodeOrder);

	// Print tge node description
	debug(description.c_str());

	// Print the node data
	printData();

	// Recursively print the children nodes
	for (uint i = 0; i < _children.size(); i++) {
		_children[i]->print(depth + 1);
	}
}

const char *XRCNode::getTypeName() {
	static const struct {
		Type type;
		const char *name;
	} typeNames[] = {
			{ kLevel,      "Level"      },
			{ kRoom,       "Room"       },
			{ kCamera,     "Camera"     },
			{ kFloor,      "Floor"      },
			{ kFace,       "Face"       },
			{ kScript,     "Script"     },
			{ kLight,      "Light"      },
			{ kBoneMesh,   "BoneMesh"   },
			{ kTextureSet, "TextureSet" }
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
		switch (_subType) {
		case 1:
			archive = Common::String::format("%s/%s.xarc", _name.c_str(), _name.c_str());
			break;
		case 2:
			archive = Common::String::format("%02x/%02x.xarc", _nodeOrder, _nodeOrder);
			break;
		default:
			error("Unknown level archive type %d", _subType);
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

UnimplementedXRCNode::UnimplementedXRCNode() :
		XRCNode(),
		_data(nullptr) {
}

UnimplementedXRCNode::~UnimplementedXRCNode() {
	// Delete this node's data
	delete[] _data;
}

void UnimplementedXRCNode::readData(Common::ReadStream *stream) {
	// Read the data
	if (_dataLength) {
		_data = new byte[_dataLength];
		uint32 bytesRead = stream->read(_data, _dataLength);

		// Verify the whole array could be read
		if (bytesRead != _dataLength) {
			error("Stark::XRCNode: data length mismatch (%d != %d)", bytesRead, _dataLength);
		}
	}
}

void UnimplementedXRCNode::printData() {
	// Print the node data
	if (_data) {
		Common::hexdump(_data, _dataLength);
	}
}

ScriptXRCNode::~ScriptXRCNode() {
}

ScriptXRCNode::ScriptXRCNode() {
}

void ScriptXRCNode::readData(Common::ReadStream* stream) {
	uint32 count = stream->readUint32LE();
	for (uint i = 0; i < count; i++) {
		Argument argument;
		argument.type = stream->readUint32LE();

		switch (argument.type) {
		case Argument::kTypeInteger1:
		case Argument::kTypeInteger2:
			argument.intValue = stream->readUint32LE();
			break;

		case Argument::kTypeDataMap:
			argument.mapValue = readMap(stream);
			break;
		case Argument::kTypeString:
			argument.stringValue = readString(stream);
			break;
		default:
			error("Unknown argument type %d", argument.type);
		}

		_arguments.push_back(argument);
	}
}

void ScriptXRCNode::printData() {
	for (uint i = 0; i < _arguments.size(); i++) {
		switch (_arguments[i].type) {
		case Argument::kTypeInteger1:
		case Argument::kTypeInteger2:
			debug("%d: %d", i, _arguments[i].intValue);
			break;

		case Argument::kTypeDataMap: {
			Common::String desc;

			DataMap map = _arguments[i].mapValue;
			for (DataMap::const_iterator it = map.begin(); it != map.end(); it++) {
				desc += Common::String::format("(%d => %d) ", it->_key, it->_value);
			}

			debug("%d: %s", i, desc.c_str());
		}
			break;
		case Argument::kTypeString:
			debug("%d: %s", i, _arguments[i].stringValue.c_str());
			break;
		default:
			error("Unknown argument type %d", _arguments[i].type);
		}
	}
}

CameraXRCNode::~CameraXRCNode() {
}

CameraXRCNode::CameraXRCNode() :
	_fov(0),
	_f2(0) {
}

void CameraXRCNode::readData(Common::ReadStream* stream) {
	_position = readVector3(stream);
	_lookAt = readVector3(stream);
	_fov = readFloat(stream);
	_f2 = readFloat(stream);
	_v3 = readVector4(stream);
	_v4 = readVector3(stream);
}

void CameraXRCNode::printData() {
	Common::Debug debug = streamDbg();
	debug << "position: " << _position << "\n";
	debug << "lookAt: " << _lookAt << "\n";
	debug << "fov: " << _fov << "\n";
	debug << "f1: " << _f2 << "\n";
	debug << "v3: " << _v3 << "\n";
	debug << "v4: " << _v4 << "\n";
}

FloorXRCNode::FloorXRCNode() :
	_facesCount(0) {
}

FloorXRCNode::~FloorXRCNode() {
}

void FloorXRCNode::readData(Common::ReadStream* stream) {
	_facesCount = stream->readUint32LE();
	uint32 positionsCount = stream->readUint32LE();

	for (uint i = 0; i < positionsCount; i++) {
		Math::Vector3d v = readVector3(stream);
		_positions.push_back(v);
	}
}

void FloorXRCNode::printData() {
	debug("face count: %d", _facesCount);

	Common::Debug debug = streamDbg();
	for (uint i = 0; i < _positions.size(); i++) {
		debug << i << ": " << _positions[i] << "\n";
	}
}

FaceXRCNode::FaceXRCNode() :
	_unk1(0),
	_unk2(0) {
	for (uint i = 0; i < ARRAYSIZE(_indices); i++) {
		_indices[i] = 0;
	}
}

FaceXRCNode::~FaceXRCNode() {
}

void FaceXRCNode::readData(Common::ReadStream* stream) {
	for (uint i = 0; i < ARRAYSIZE(_indices); i++) {
		_indices[i] = stream->readSint16LE();
	}

	_unk1 = readFloat(stream);

	for (uint i = 0; i < ARRAYSIZE(_indices); i++) {
		stream->readSint16LE(); // Skipped in the original
	}

	_unk2 = readFloat(stream);
}

void FaceXRCNode::printData() {
	debug("indices: %d %d %d, unk1 %f, unk2 %f", _indices[0], _indices[1], _indices[2], _unk1, _unk2);
}

} // End of namespace Stark
