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

#include "engines/stark/resources/resource.h"
#include "engines/stark/xrcreader.h"
#include "engines/stark/debug.h"

#include "common/debug-channels.h"
#include "common/streamdebug.h"
#include "common/util.h"

namespace Stark {

ResourceType::ResourceType(Type type) {
	_type = type;
}

ResourceType::ResourceType() :
		ResourceType::ResourceType(kInvalid) {
}

const char *ResourceType::getName() {
	static const struct {
		ResourceType::Type type;
		const char *name;
	} typeNames[] = {
			{ ResourceType::kInvalid,                "Invalid"                },
			{ ResourceType::kRoot,                   "Root"                   },
			{ ResourceType::kLevel,                  "Level"                  },
			{ ResourceType::kLocation,               "Location"               },
			{ ResourceType::kLayer,                  "Layer"                  },
			{ ResourceType::kCamera,                 "Camera"                 },
			{ ResourceType::kFloor,                  "Floor"                  },
			{ ResourceType::kFloorFace,              "FloorFace"              },
			{ ResourceType::kItem,                   "Item"                   },
			{ ResourceType::kScript,                 "Script"                 },
			{ ResourceType::kAnimHier,               "AnimHier"               },
			{ ResourceType::kAnim,                   "Anim"                   },
			{ ResourceType::kDirection,              "Direction"              },
			{ ResourceType::kImage,                  "Image"                  },
			{ ResourceType::kAnimScript,             "AnimScript"             },
			{ ResourceType::kAnimScriptItem,         "AnimScriptItem"         },
			{ ResourceType::kSoundItem,              "SoundItem"              },
			{ ResourceType::kFloorField,             "FloorField"             },
			{ ResourceType::kBookmark,               "Bookmark"               },
			{ ResourceType::kKnowledgeSet,           "KnowledgeSet"           },
			{ ResourceType::kKnowledge,              "Knowledge"              },
			{ ResourceType::kCommand,                "Command"                },
			{ ResourceType::kPATTable,               "PATTable"               },
			{ ResourceType::kContainer,              "Container"              },
			{ ResourceType::kDialog,                 "Dialog"                 },
			{ ResourceType::kSpeech,                 "Speech"                 },
			{ ResourceType::kLight,                  "Light"                  },
			{ ResourceType::kCursor,                 "Cursor"                 },
			{ ResourceType::kBoneMesh,               "BoneMesh"               },
			{ ResourceType::kScroll,                 "Scroll"                 },
			{ ResourceType::kFMV,                    "FMV"                    },
			{ ResourceType::kLipSynch,               "LipSynch"               },
			{ ResourceType::kAnimScriptBonesTrigger, "AnimScriptBonesTrigger" },
			{ ResourceType::kString,                 "String"                 },
			{ ResourceType::kTextureSet,             "TextureSet"             }
	};

	for (uint i = 0; i < ARRAYSIZE(typeNames); i++) {
		if (typeNames[i].type == _type) {
			return typeNames[i].name;
		}
	}

	return nullptr;
}

ResourceType::Type ResourceType::get() {
	return _type;
}

Resource::Resource(Resource *parent, byte subType, uint16 index, const Common::String &name) :
		_parent(parent),
		_type(ResourceType::kInvalid),
		_subType(subType),
		_index(index),
		_name(name) {
}

Resource::~Resource() {
	// Delete the children resources
	Common::Array<Resource *>::iterator i = _children.begin();
	while (i != _children.end()) {
		delete *i;
		i++;
	}
}

void Resource::readData(XRCReadStream *stream) {
}

void Resource::onPostRead() {
}

void Resource::onAllLoaded() {
	Common::Array<Resource *>::iterator i = _children.begin();
	while (i != _children.end()) {
		(*i)->onAllLoaded();
		i++;
	}
}

void Resource::onEnterLocation() {
	Common::Array<Resource *>::iterator i = _children.begin();
	while (i != _children.end()) {
		(*i)->onEnterLocation();
		i++;
	}
}

void Resource::onGameLoop(uint msecs) {
	Common::Array<Resource *>::iterator i = _children.begin();
	while (i != _children.end()) {
		(*i)->onGameLoop(msecs);
		i++;
	}
}

void Resource::onExitLocation() {
	Common::Array<Resource *>::iterator i = _children.begin();
	while (i != _children.end()) {
		(*i)->onExitLocation();
		i++;
	}
}

void Resource::onPreDestroy() {
	Common::Array<Resource *>::iterator i = _children.begin();
	while (i != _children.end()) {
		(*i)->onPreDestroy();
		i++;
	}
}

void Resource::print(uint depth) {
	// Display value for the resource type
	Common::String type(_type.getName());
	if (type.empty()) {
		type = Common::String::format("%d", _type.get());
	}

	// Build the resource description
	Common::String description;
	for (uint i = 0; i < depth; i++) {
		description += "-";
	}
	description += Common::String::format(" %s - %s - (sub=%d, order=%d)", type.c_str(), _name.c_str(), _subType, _index);

	// Print the resource description
	debug(description.c_str());

	// Print the resource data
	printData();

	// Recursively print the children resources
	for (uint i = 0; i < _children.size(); i++) {
		_children[i]->print(depth + 1);
	}
}

void Resource::addChild(Resource *child) {
	_children.push_back(child);
}

Resource *Resource::findChild(ResourceType type, int subType, bool mustBeUnique) {
	Resource *child = nullptr;

	for (uint i = 0; i < _children.size(); i++) {
		if (_children[i]->getType() == type
				&& (_children[i]->getSubType() == subType || subType == -1)) {
			// Found a matching child
			if (!child) {
				child = _children[i];
			} else if (mustBeUnique) {
				error("Several children resources matching criteria type = %s, subtype = %d", type.getName(), subType);
			}
		}
	}

	return child;
}

Resource *Resource::findChildWithIndex(ResourceType type, int subType, uint16 index) {
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

Common::Array<Resource *> Resource::listChildren(ResourceType type, int subType) {
	Common::Array<Resource *> list;

	for (uint i = 0; i < _children.size(); i++) {
		if (_children[i]->getType() == type
				&& (_children[i]->getSubType() == subType || subType == -1)) {
			// Found a matching child
			list.push_back(_children[i]);
		}
	}

	return list;
}

UnimplementedResource::UnimplementedResource(Resource *parent, ResourceType type, byte subType, uint16 index, const Common::String &name) :
		Resource(parent, subType, index, name),
		_dataLength(0),
		_data(nullptr) {
	_type = type;
}

UnimplementedResource::~UnimplementedResource() {
	// Delete this resource's data
	delete[] _data;
}

void UnimplementedResource::readData(XRCReadStream *stream) {
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

} // End of namespace Stark
