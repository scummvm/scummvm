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

#ifndef STARK_RESOURCES_RESOURCE_H
#define STARK_RESOURCES_RESOURCE_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/types.h"

namespace Stark {

class XRCReadStream;

class ResourceType {
public:
	enum Type {
		kInvalid                = 0,
		kRoot                   = 1,
		kLevel                  = 2,
		kRoom                   = 3,
		kLayer                  = 4,
		kCamera                 = 5,
		kFloor                  = 6,
		kFloorFace              = 7,
		kItem                   = 8,
		kScript                 = 9,
		kAnimHier               = 10,
		kAnim                   = 11,
		kDirection              = 12,
		kImage                  = 13,
		kAnimScript             = 14,
		kAnimScriptItem         = 15,
		kSoundItem              = 16,
		kFloorField             = 18,
		kBookmark               = 19,
		kKnowledgeSet           = 20,
		kKnowledge              = 21,
		kCommand                = 22,
		kPATTable               = 23,
		kContainer              = 26,
		kDialog                 = 27,
		kSpeech                 = 29,
		kLight                  = 30,
		kCursor                 = 31, // Not sure about this one
		kBoneMesh               = 32,
		kScroll                 = 33,
		kFMV                    = 34,
		kLipSynch               = 35,
		kAnimScriptBonesTrigger = 36,
		kString                 = 37,
		kTextureSet             = 38
	};

	ResourceType();
	ResourceType(Type type);

	void readFromStream(Common::ReadStream *stream);

	Type get();
	const char *getName();
	bool is(Type type);

private:
	Type _type;
};

class ResourcePair {
public:
	ResourcePair();
	ResourcePair(ResourceType type, uint16 index);

	void readFromStream(Common::ReadStream *stream);
	Common::String describe();

private:
	ResourceType _type;
	uint16 _index;
};

typedef Common::Array<ResourcePair> ResourceReference;

class Resource {
public:
	virtual ~Resource();

	ResourceType getType() const { return _type; }
	Common::String getName() const { return _name; }

	Common::Array<Resource *> getChildren() const { return _children; }
	void addChild(Resource *child);


	virtual void readData(XRCReadStream *stream) = 0;

	/**
	 * Get the archive file name containing the data for this resource.
	 * Only Levels and Rooms have archives.
	 */
	Common::String getArchive();

	void print(uint depth = 0);

protected:
	Resource(Resource *parent, byte subType, uint16 index, const Common::String &name);

	virtual void printData() = 0;

	ResourceType _type;
	byte _subType;
	uint16 _index;
	Common::String _name;

	Resource *_parent;
	Common::Array<Resource *> _children;
};

class UnimplementedResource : public Resource {
public:
	UnimplementedResource(Resource *parent, ResourceType type, byte subType, uint16 index, const Common::String &name);
	virtual ~UnimplementedResource();

protected:
	void readData(XRCReadStream *stream) override;
	void printData() override;

	uint32 _dataLength;
	byte *_data;
};

} // End of namespace Stark

#endif // STARK_RESOURCES_RESOURCE_H
