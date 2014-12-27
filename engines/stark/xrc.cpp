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

NodeType::NodeType(Type type) {
	_type = type;
}

NodeType::NodeType() :
		NodeType::NodeType(kInvalid) {
}

void NodeType::readFromStream(Common::ReadStream *stream) {
	byte rawType;
	rawType = stream->readByte();
	_type = (NodeType::Type) rawType;
}

const char *NodeType::getName() {
	static const struct {
		NodeType::Type type;
		const char *name;
	} typeNames[] = {
			{ NodeType::kInvalid,                "Invalid"                },
			{ NodeType::kRoot,                   "Root"                   },
			{ NodeType::kLevel,                  "Level"                  },
			{ NodeType::kRoom,                   "Room"                   },
			{ NodeType::kLayer,                  "Layer"                  },
			{ NodeType::kCamera,                 "Camera"                 },
			{ NodeType::kFloor,                  "Floor"                  },
			{ NodeType::kFace,                   "Face"                   },
			{ NodeType::kItem,                   "Item"                   },
			{ NodeType::kScript,                 "Script"                 },
			{ NodeType::kAnimHier,               "AnimHier"               },
			{ NodeType::kAnim,                   "Anim"                   },
			{ NodeType::kDirection,              "Direction"              },
			{ NodeType::kImage,                  "Image"                  },
			{ NodeType::kAnimScript,             "AnimScript"             },
			{ NodeType::kAnimScriptItem,         "AnimScriptItem"         },
			{ NodeType::kSoundItem,              "SoundItem"              },
			{ NodeType::kFloorField,             "FloorField"             },
			{ NodeType::kBookmark,               "Bookmark"               },
			{ NodeType::kKnowledgeSet,           "KnowledgeSet"           },
			{ NodeType::kKnowledge,              "Knowledge"              },
			{ NodeType::kCommand,                "Command"                },
			{ NodeType::kPATTable,               "PATTable"               },
			{ NodeType::kContainer,              "Container"              },
			{ NodeType::kDialog,                 "Dialog"                 },
			{ NodeType::kSpeech,                 "Speech"                 },
			{ NodeType::kLight,                  "Light"                  },
			{ NodeType::kCursor,                 "Cursor"                 },
			{ NodeType::kBoneMesh,               "BoneMesh"               },
			{ NodeType::kScroll,                 "Scroll"                 },
			{ NodeType::kFMV,                    "FMV"                    },
			{ NodeType::kLipSynch,               "LipSynch"               },
			{ NodeType::kAnimScriptBonesTrigger, "AnimScriptBonesTrigger" },
			{ NodeType::kString,                 "String"                 },
			{ NodeType::kTextureSet,             "TextureSet"             }
	};

	for (uint i = 0; i < ARRAYSIZE(typeNames); i++) {
		if (typeNames[i].type == _type) {
			return typeNames[i].name;
		}
	}

	return nullptr;
}

NodeType::Type NodeType::get() {
	return _type;
}

bool NodeType::is(NodeType::Type type) {
	return _type == type;
}

NodePair::NodePair() :
		_type(NodeType::kInvalid), _index(0) {
}

NodePair::NodePair(NodeType type, uint16 index) :
		_type(type), _index(index) {
}

void NodePair::readFromStream(Common::ReadStream *stream) {
	_type.readFromStream(stream);
	_index = stream->readUint16LE();
}

Common::String NodePair::describe() {
	return Common::String::format("(%s idx %d)", _type.getName(), _index);
}

XRCNode::XRCNode(XRCNode *parent, byte subType, uint16 index, const Common::String &name) :
		_parent(parent),
		_type(NodeType::kInvalid),
		_subType(subType),
		_index(index),
		_name(name) {
}

XRCNode::~XRCNode() {
	// Delete the children nodes
	Common::Array<XRCNode *>::iterator i = _children.begin();
	while (i != _children.end()) {
		delete *i;
		i++;
	}
}

XRCNode *XRCNode::read(Common::ReadStream *stream, XRCNode *parent) {
	// Read the resource type
	NodeType type;
	type.readFromStream(stream);

	byte subType = stream->readByte();
	uint16 index = stream->readUint16LE();
	Common::String name = readString(stream);

	// Create a new node
	XRCNode *node;
	switch (type.get()) {
	case NodeType::kCamera:
		node = new CameraXRCNode(parent, subType, index, name);
		break;
	case NodeType::kFloor:
		node = new FloorXRCNode(parent, subType, index, name);
		break;
	case NodeType::kFace:
		node = new FaceXRCNode(parent, subType, index, name);
		break;
	case NodeType::kCommand:
		node = new CommandXRCNode(parent, subType, index, name);
		break;
	default:
		node = new UnimplementedXRCNode(parent, type, subType, index, name);
		break;
	}

	// Read the data length
	uint32 dataLength = stream->readUint32LE();

	// Read the node type specific data using a memory stream
	if (dataLength > 0) {
		Common::SeekableReadStream *dataStream = stream->readStream(dataLength);

		node->readData(dataStream);

		if (isDataLeft(dataStream)) {
			warning("Not all XRC data was read. Type %s, subtype %d", node->getType().getName(), node->_subType);
		}

		delete dataStream;
	}

	node->readChildren(stream);

	return node;
}

void XRCNode::readChildren(Common::ReadStream *stream) {
	// Get the number of children
	uint16 numChildren = stream->readUint16LE();

	// Read more unknown data
	uint16 unknown3 = stream->readUint16LE();
	if (unknown3 != 0) {
		warning("Stark::XRCNode: \"%s\" has unknown3=0x%04X with unknown meaning", _name.c_str(), unknown3);
	}

	// Read the children nodes
	_children.reserve(numChildren);
	for (int i = 0; i < numChildren; i++) {
		XRCNode *child = XRCNode::read(stream, this);

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

NodePath XRCNode::readNodeReference(Common::ReadStream *stream) {
	NodePath path;

	uint32 size = stream->readUint32LE();
	for (uint i = 0; i < size; i++) {
		NodePair nodeId;
		nodeId.readFromStream(stream);
		path.push_back(nodeId);
	}

	return path;
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
	Common::String type(_type.getName());
	if (type.empty()) {
		type = Common::String::format("%d", _type.get());
	}

	// Build the node description
	Common::String description;
	for (uint i = 0; i < depth; i++) {
		description += "-";
	}
	description += Common::String::format(" %s - %s - (sub=%d, order=%d)", type.c_str(), _name.c_str(), _subType, _index);

	// Print the node description
	debug(description.c_str());

	// Print the node data
	printData();

	// Recursively print the children nodes
	for (uint i = 0; i < _children.size(); i++) {
		_children[i]->print(depth + 1);
	}
}

Common::String XRCNode::getArchive() {
	Common::String archive;

	switch (getType().get()) {
	case NodeType::kLevel:
		switch (_subType) {
		case 1:
			archive = Common::String::format("%s/%s.xarc", _name.c_str(), _name.c_str());
			break;
		case 2:
			archive = Common::String::format("%02x/%02x.xarc", _index, _index);
			break;
		default:
			error("Unknown level archive type %d", _subType);
		}
		break;
	case NodeType::kRoom:
		assert(_parent);
		archive = Common::String::format("%02x/%02x/%02x.xarc", _parent->_index, _index, _index);
		break;
	default:
		error("This type of node cannot load children %s", _type.getName());
	}

	return archive;
}

UnimplementedXRCNode::UnimplementedXRCNode(XRCNode *parent, NodeType type, byte subType, uint16 index, const Common::String &name) :
		XRCNode(parent, subType, index, name),
		_dataLength(0),
		_data(nullptr) {
	_type = type;
}

UnimplementedXRCNode::~UnimplementedXRCNode() {
	// Delete this node's data
	delete[] _data;
}

void UnimplementedXRCNode::readData(Common::SeekableReadStream *stream) {
	// Read the data
	_dataLength = stream->size();
	_data = new byte[_dataLength];
	uint32 bytesRead = stream->read(_data, _dataLength);

	// Verify the whole array could be read
	if (bytesRead != _dataLength) {
		error("Stark::XRCNode: data length mismatch (%d != %d)", bytesRead, _dataLength);
	}
}

void UnimplementedXRCNode::printData() {
	// Print the node data
	if (_data) {
		Common::hexdump(_data, _dataLength);
	}
}

CommandXRCNode::~CommandXRCNode() {
}

CommandXRCNode::CommandXRCNode(XRCNode *parent, byte subType, uint16 index, const Common::String &name) :
				XRCNode(parent, subType, index, name) {
	_type = NodeType::kCommand;
}

void CommandXRCNode::readData(Common::SeekableReadStream* stream) {
	uint32 count = stream->readUint32LE();
	for (uint i = 0; i < count; i++) {
		Argument argument;
		argument.type = stream->readUint32LE();

		switch (argument.type) {
		case Argument::kTypeInteger1:
		case Argument::kTypeInteger2:
			argument.intValue = stream->readUint32LE();
			break;

		case Argument::kTypeNodeReference:
			argument.referenceValue = readNodeReference(stream);
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

void CommandXRCNode::printData() {
	for (uint i = 0; i < _arguments.size(); i++) {
		switch (_arguments[i].type) {
		case Argument::kTypeInteger1:
		case Argument::kTypeInteger2:
			debug("%d: %d", i, _arguments[i].intValue);
			break;

		case Argument::kTypeNodeReference: {
			Common::String desc;

			NodePath path = _arguments[i].referenceValue;
			for (uint j = 0; j < path.size(); j++) {
				desc += path[j].describe() + " ";
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

CameraXRCNode::CameraXRCNode(XRCNode *parent, byte subType, uint16 index, const Common::String &name) :
		XRCNode(parent, subType, index, name),
		_fov(0),
		_f2(0) {
	_type = NodeType::kCamera;
}

void CameraXRCNode::readData(Common::SeekableReadStream* stream) {
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

FloorXRCNode::FloorXRCNode(XRCNode *parent, byte subType, uint16 index, const Common::String &name) :
		XRCNode(parent, subType, index, name),
		_facesCount(0) {
	_type = NodeType::kFloor;
}

FloorXRCNode::~FloorXRCNode() {
}

void FloorXRCNode::readData(Common::SeekableReadStream* stream) {
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

FaceXRCNode::FaceXRCNode(XRCNode *parent, byte subType, uint16 index, const Common::String &name) :
		XRCNode(parent, subType, index, name),
		_unk1(0),
		_unk2(0) {
	_type = NodeType::kFace;

	for (uint i = 0; i < ARRAYSIZE(_indices); i++) {
		_indices[i] = 0;
	}
}

FaceXRCNode::~FaceXRCNode() {
}

void FaceXRCNode::readData(Common::SeekableReadStream* stream) {
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
