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
#include "engines/stark/xrcreader.h"
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

void XRCNode::addChild(XRCNode *child) {
	_children.push_back(child);
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

void UnimplementedXRCNode::readData(XRCReadStream *stream) {
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

void CommandXRCNode::readData(XRCReadStream *stream) {
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
			argument.referenceValue = stream->readNodeReference();
			break;
		case Argument::kTypeString:
			argument.stringValue = stream->readString();
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

void CameraXRCNode::readData(XRCReadStream *stream) {
	_position = stream->readVector3();
	_lookAt = stream->readVector3();
	_fov = stream->readFloat();
	_f2 = stream->readFloat();
	_v3 = stream->readVector4();
	_v4 = stream->readVector3();
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

void FloorXRCNode::readData(XRCReadStream *stream) {
	_facesCount = stream->readUint32LE();
	uint32 positionsCount = stream->readUint32LE();

	for (uint i = 0; i < positionsCount; i++) {
		Math::Vector3d v = stream->readVector3();
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

void FaceXRCNode::readData(XRCReadStream *stream) {
	for (uint i = 0; i < ARRAYSIZE(_indices); i++) {
		_indices[i] = stream->readSint16LE();
	}

	_unk1 = stream->readFloat();

	for (uint i = 0; i < ARRAYSIZE(_indices); i++) {
		stream->readSint16LE(); // Skipped in the original
	}

	_unk2 = stream->readFloat();
}

void FaceXRCNode::printData() {
	debug("indices: %d %d %d, unk1 %f, unk2 %f", _indices[0], _indices[1], _indices[2], _unk1, _unk2);
}

} // End of namespace Stark
