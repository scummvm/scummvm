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

#include "engines/stark/resources/object.h"

#include "common/debug-channels.h"
#include "common/streamdebug.h"
#include "common/util.h"

#include "engines/stark/debug.h"
#include "engines/stark/formats/xrc.h"

namespace Stark {
namespace Resources {

Type::Type(ResourceType type) {
	_type = type;
}

Type::Type() {
	_type = kInvalid;
}

const char *Type::getName() const {
	static const struct {
		Type::ResourceType type;
		const char *name;
	} typeNames[] = {
			{ Type::kInvalid,                "Invalid"                },
			{ Type::kRoot,                   "Root"                   },
			{ Type::kLevel,                  "Level"                  },
			{ Type::kLocation,               "Location"               },
			{ Type::kLayer,                  "Layer"                  },
			{ Type::kCamera,                 "Camera"                 },
			{ Type::kFloor,                  "Floor"                  },
			{ Type::kFloorFace,              "FloorFace"              },
			{ Type::kItem,                   "Item"                   },
			{ Type::kScript,                 "Script"                 },
			{ Type::kAnimHierarchy,          "AnimHierarchy"          },
			{ Type::kAnim,                   "Anim"                   },
			{ Type::kDirection,              "Direction"              },
			{ Type::kImage,                  "Image"                  },
			{ Type::kAnimScript,             "AnimScript"             },
			{ Type::kAnimScriptItem,         "AnimScriptItem"         },
			{ Type::kSoundItem,              "SoundItem"              },
			{ Type::kPath,                   "Path"                   },
			{ Type::kFloorField,             "FloorField"             },
			{ Type::kBookmark,               "Bookmark"               },
			{ Type::kKnowledgeSet,           "KnowledgeSet"           },
			{ Type::kKnowledge,              "Knowledge"              },
			{ Type::kCommand,                "Command"                },
			{ Type::kPATTable,               "PATTable"               },
			{ Type::kContainer,              "Container"              },
			{ Type::kDialog,                 "Dialog"                 },
			{ Type::kSpeech,                 "Speech"                 },
			{ Type::kLight,                  "Light"                  },
			{ Type::kCursor,                 "Cursor"                 },
			{ Type::kBonesMesh,              "BonesMesh"              },
			{ Type::kScroll,                 "Scroll"                 },
			{ Type::kFMV,                    "FMV"                    },
			{ Type::kLipSync,                "LipSynch"               },
			{ Type::kAnimSoundTrigger,       "AnimSoundTrigger"       },
			{ Type::kString,                 "String"                 },
			{ Type::kTextureSet,             "TextureSet"             }
	};

	for (uint i = 0; i < ARRAYSIZE(typeNames); i++) {
		if (typeNames[i].type == _type) {
			return typeNames[i].name;
		}
	}

	return nullptr;
}

Type::ResourceType Type::get() const {
	return _type;
}

Object::Object(Object *parent, byte subType, uint16 index, const Common::String &name) :
		_parent(parent),
		_type(Type::kInvalid),
		_subType(subType),
		_index(index),
		_name(name) {
}

Object::~Object() {
	// Delete the children resources
	Common::Array<Object *>::iterator i = _children.begin();
	while (i != _children.end()) {
		delete *i;
		i++;
	}
}

void Object::readData(Formats::XRCReadStream *stream) {
}

void Object::printData() {
}

void Object::saveLoad(ResourceSerializer *serializer) {
}

void Object::saveLoadCurrent(ResourceSerializer *serializer) {
}

void Object::onPostRead() {
}

void Object::onAllLoaded() {
	Common::Array<Object *>::iterator i = _children.begin();
	while (i != _children.end()) {
		(*i)->onAllLoaded();
		i++;
	}
}

void Object::onEnterLocation() {
	Common::Array<Object *>::iterator i = _children.begin();
	while (i != _children.end()) {
		(*i)->onEnterLocation();
		i++;
	}
}

void Object::onGameLoop() {
	Common::Array<Object *>::iterator i = _children.begin();
	while (i != _children.end()) {
		(*i)->onGameLoop();
		i++;
	}
}

void Object::onEnginePause(bool pause) {
	Common::Array<Object *>::iterator i = _children.begin();
	while (i != _children.end()) {
		(*i)->onEnginePause(pause);
		i++;
	}
}

void Object::onExitLocation() {
	Common::Array<Object *>::iterator i = _children.begin();
	while (i != _children.end()) {
		(*i)->onExitLocation();
		i++;
	}
}

void Object::onPreDestroy() {
	Common::Array<Object *>::iterator i = _children.begin();
	while (i != _children.end()) {
		(*i)->onPreDestroy();
		i++;
	}
}

void Object::print(uint depth) {
	printDescription(depth);
	printData();

	// Recursively print the children resources
	for (uint i = 0; i < _children.size(); i++) {
		_children[i]->print(depth + 1);
	}
}

void Object::printDescription(uint depth) const {
	// Build the resource type string
	Common::String type(_type.getName());
	if (type.empty()) {
		type = Common::String::format("%d", _type.get());
	}

	// Build the resource description
	Common::String description = Common::String::format("%s - %s - (sub=%d, index=%d)", type.c_str(), _name.c_str(), _subType, _index);
	printWithDepth(depth, description);
}

void Object::printWithDepth(uint depth, const Common::String &string) const {
	Common::String prefix;
	for (uint i = 0; i < depth; i++) {
		prefix += "-";
	}

	debug("%s %s", prefix.c_str(), string.c_str());
}

Object *Object::findChildWithIndex(Type type, uint16 index, int subType) const {
	for (uint i = 0; i < _children.size(); i++) {
		if (_children[i]->getType() == type
				&& (_children[i]->getSubType() == subType || subType == -1)
				&& _children[i]->getIndex() == index) {
			// Found a matching child
			return _children[i];
		}
	}

	return nullptr;
}

Object *Object::findChildWithOrder(Type type, uint16 order, int subType) const {
	uint16 count = 0;
	for (uint i = 0; i < _children.size(); i++) {
		if (_children[i]->getType() == type
		    && (_children[i]->getSubType() == subType || subType == -1)) {
			if (count == order) {
				// Found a matching child
				return _children[i];
			}
			count++;
		}
	}

	return nullptr;
}

Object *Object::findChildWithName(Type type, const Common::String &name, int subType) const {
	for (uint i = 0; i < _children.size(); ++i) {
		if (_children[i]->getType() == type
				&& (_children[i]->getSubType() == subType || subType == -1)
				&& _children[i]->getName() == name) {
			// Found a matching child
			return _children[i];
		}
	}

	return nullptr;
}

template<>
Object *Object::cast<Object>(Object *resource) {
	// No type check when asking for the abstract resource
	return resource;
}

template<>
Common::Array<Object *> Object::listChildren<Object>(int subType) const {
	assert(subType == -1);

	Common::Array<Object *> list;

	for (uint i = 0; i < _children.size(); i++) {
		list.push_back(_children[i]);
	}

	return list;
}

template<>
Object *Object::findParent() {
	return _parent;
}

void Object::addChild(Object *child) {
	_children.push_back(child);
}

UnimplementedResource::UnimplementedResource(Object *parent, Type type, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_dataLength(0),
		_data(nullptr) {
	_type = type;
}

UnimplementedResource::~UnimplementedResource() {
	// Delete this resource's data
	delete[] _data;
}

void UnimplementedResource::readData(Formats::XRCReadStream *stream) {
	// Read the data
	_dataLength = stream->size();
	_data = new byte[_dataLength];
	uint32 bytesRead = stream->read(_data, _dataLength);

	// Verify the whole array could be read
	if (bytesRead != _dataLength) {
		error("Stark::UnimplementedResource: data length mismatch (%d != %d)", bytesRead, _dataLength);
	}
}

void UnimplementedResource::printData() {
	// Print the resource data
	if (_data) {
		Common::hexdump(_data, _dataLength);
	}
}

} // End of namespace Resources
} // End of namespace Stark
