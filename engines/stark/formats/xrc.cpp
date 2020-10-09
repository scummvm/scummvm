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

#include "engines/stark/formats/xrc.h"

#include "engines/stark/formats/xarc.h"
#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/animhierarchy.h"
#include "engines/stark/resources/animscript.h"
#include "engines/stark/resources/animsoundtrigger.h"
#include "engines/stark/resources/bonesmesh.h"
#include "engines/stark/resources/bookmark.h"
#include "engines/stark/resources/camera.h"
#include "engines/stark/resources/container.h"
#include "engines/stark/resources/command.h"
#include "engines/stark/resources/dialog.h"
#include "engines/stark/resources/direction.h"
#include "engines/stark/resources/fmv.h"
#include "engines/stark/resources/image.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/floor.h"
#include "engines/stark/resources/floorface.h"
#include "engines/stark/resources/floorfield.h"
#include "engines/stark/resources/knowledge.h"
#include "engines/stark/resources/knowledgeset.h"
#include "engines/stark/resources/layer.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/resources/light.h"
#include "engines/stark/resources/lipsync.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/path.h"
#include "engines/stark/resources/pattable.h"
#include "engines/stark/resources/root.h"
#include "engines/stark/resources/script.h"
#include "engines/stark/resources/scroll.h"
#include "engines/stark/resources/speech.h"
#include "engines/stark/resources/sound.h"
#include "engines/stark/resources/string.h"
#include "engines/stark/resources/textureset.h"
#include "engines/stark/resourcereference.h"

namespace Stark {
namespace Formats {

XRCReadStream::XRCReadStream(const Common::String &archiveName,
		Common::SeekableReadStream *parentStream, DisposeAfterUse::Flag disposeParentStream) :
		SeekableSubReadStream(parentStream, 0, parentStream->size(), disposeParentStream),
		_archiveName(archiveName) {
}

XRCReadStream::~XRCReadStream() {
}

Common::String XRCReadStream::readString() {
	// Read the string length
	uint16 length = readUint16LE();

	// Read the string
	char *data = new char[length];
	read(data, length);
	Common::String string(data, length);
	delete[] data;

	return string;
}

Resources::Type XRCReadStream::readResourceType() {
	byte rawType;
	rawType = readByte();
	return Resources::Type((Resources::Type::ResourceType) (rawType));
}

ResourceReference XRCReadStream::readResourceReference() {
	ResourceReference reference;
	reference.loadFromStream(this);

	return reference;
}

Math::Vector3d XRCReadStream::readVector3() {
	Math::Vector3d v;
	v.readFromStream(this);
	return v;
}

Common::Rect XRCReadStream::readRect() {
	Common::Rect r;
	r.left = readSint32LE();
	r.top = readSint32LE();
	r.right = readSint32LE();
	r.bottom = readSint32LE();
	return r;
}

Common::Point XRCReadStream::readPoint() {
	uint32 x = readUint32LE();
	uint32 y = readUint32LE();

	return Common::Point(x, y);
}

bool XRCReadStream::readBool() {
	uint32 b = readUint32LE();
	return b != 0;
}

bool XRCReadStream::isDataLeft() {
	return pos() < size();
}

Common::String XRCReadStream::getArchiveName() const {
	return _archiveName;
}

Resources::Object *XRCReader::importTree(XARCArchive *archive) {
	// Find the XRC file
	Common::ArchiveMemberList members;
	archive->listMatchingMembers(members, "*.xrc");
	if (members.size() == 0) {
		error("No resource tree in archive '%s'", archive->getFilename().c_str());
	}
	if (members.size() > 1) {
		error("Too many resource scripts in archive '%s'", archive->getFilename().c_str());
	}

	// Open the XRC file
	Common::SeekableReadStream *stream = archive->createReadStreamForMember(members.front()->getName());
	XRCReadStream *xrcStream = new XRCReadStream(archive->getFilename(), stream);

	// Import the resource tree
	Resources::Object *root = importResource(xrcStream, nullptr);

	delete xrcStream;

	return root;
}

Resources::Object *XRCReader::importResource(XRCReadStream *stream, Resources::Object *parent) {
	Resources::Object *resource = createResource(stream, parent);
	importResourceData(stream, resource);
	importResourceChildren(stream, resource);

	// Resource lifecycle update
	resource->onPostRead();

	return resource;
}

Resources::Object *XRCReader::createResource(XRCReadStream *stream, Resources::Object *parent) {
	// Read the resource type and subtype
	Resources::Type type = stream->readResourceType();
	byte subType = stream->readByte();

	// Read the resource properties
	uint16 index = stream->readUint16LE();
	Common::String name = stream->readString();

	// Create a new resource
	Resources::Object *resource;
	switch (type.get()) {
	case Resources::Type::kRoot:
		resource = new Resources::Root(parent, subType, index, name);
		break;
	case Resources::Type::kLevel:
		resource = new Resources::Level(parent, subType, index, name);
		break;
	case Resources::Type::kLocation:
		resource = new Resources::Location(parent, subType, index, name);
		break;
	case Resources::Type::kLayer:
		resource = Resources::Layer::construct(parent, subType, index, name);
		break;
	case Resources::Type::kCamera:
		resource = new Resources::Camera(parent, subType, index, name);
		break;
	case Resources::Type::kFloor:
		resource = new Resources::Floor(parent, subType, index, name);
		break;
	case Resources::Type::kFloorFace:
		resource = new Resources::FloorFace(parent, subType, index, name);
		break;
	case Resources::Type::kItem:
		resource = Resources::Item::construct(parent, subType, index, name);
		break;
	case Resources::Type::kScript:
		resource = new Resources::Script(parent, subType, index, name);
		break;
	case Resources::Type::kAnimHierarchy:
		resource = new Resources::AnimHierarchy(parent, subType, index, name);
		break;
	case Resources::Type::kAnim:
		resource = Resources::Anim::construct(parent, subType, index, name);
		break;
	case Resources::Type::kDirection:
		resource = new Resources::Direction(parent, subType, index, name);
		break;
	case Resources::Type::kImage:
		resource = Resources::Image::construct(parent, subType, index, name);
		break;
	case Resources::Type::kAnimScript:
		resource = new Resources::AnimScript(parent, subType, index, name);
		break;
	case Resources::Type::kAnimScriptItem:
		resource = new Resources::AnimScriptItem(parent, subType, index, name);
		break;
	case Resources::Type::kSoundItem:
		resource = new Resources::Sound(parent, subType, index, name);
		break;
	case Resources::Type::kPath:
		resource = Resources::Path::construct(parent, subType, index, name);
		break;
	case Resources::Type::kFloorField:
		resource = new Resources::FloorField(parent, subType, index, name);
		break;
	case Resources::Type::kBookmark:
		resource = new Resources::Bookmark(parent, subType, index, name);
		break;
	case Resources::Type::kKnowledgeSet:
		resource = new Resources::KnowledgeSet(parent, subType, index, name);
		break;
	case Resources::Type::kKnowledge:
		resource = new Resources::Knowledge(parent, subType, index, name);
		break;
	case Resources::Type::kCommand:
		resource = new Resources::Command(parent, subType, index, name);
		break;
	case Resources::Type::kPATTable:
		resource = new Resources::PATTable(parent, subType, index, name);
		break;
	case Resources::Type::kContainer:
		resource = new Resources::Container(parent, subType, index, name);
		break;
	case Resources::Type::kDialog:
		resource = new Resources::Dialog(parent, subType, index, name);
		break;
	case Resources::Type::kSpeech:
		resource = new Resources::Speech(parent, subType, index, name);
		break;
	case Resources::Type::kLight:
		resource = new Resources::Light(parent, subType, index, name);
		break;
	case Resources::Type::kBonesMesh:
		resource = new Resources::BonesMesh(parent, subType, index, name);
		break;
	case Resources::Type::kScroll:
		resource = new Resources::Scroll(parent, subType, index, name);
		break;
	case Resources::Type::kFMV:
		resource = new Resources::FMV(parent, subType, index, name);
		break;
	case Resources::Type::kLipSync:
		resource = new Resources::LipSync(parent, subType, index, name);
        break;
	case Resources::Type::kAnimSoundTrigger:
		resource = new Resources::AnimSoundTrigger(parent, subType, index, name);
		break;
	case Resources::Type::kString:
		resource = new Resources::String(parent, subType, index, name);
        break;
	case Resources::Type::kTextureSet:
		resource = new Resources::TextureSet(parent, subType, index, name);
		break;
	default:
		resource = new Resources::UnimplementedResource(parent, type, subType, index, name);
		break;
	}

	return resource;
}

void XRCReader::importResourceData(XRCReadStream *stream, Resources::Object *resource) {
	// Read the data length
	uint32 dataLength = stream->readUint32LE();

	// Read the resource type specific data using a memory stream
	if (dataLength > 0) {
		XRCReadStream *xrcDataStream = new XRCReadStream(stream->getArchiveName(), stream->readStream(dataLength));

		resource->readData(xrcDataStream);

		if (xrcDataStream->isDataLeft()) {
			warning("Not all XRC data was read. Type %s, subtype %d, name %s",
				resource->getType().getName(), resource->getSubType(), resource->getName().c_str());
		}

		if (xrcDataStream->eos()) {
			warning("Too much XRC data was read. Type %s, subtype %d, name %s",
				resource->getType().getName(), resource->getSubType(), resource->getName().c_str());
		}

		delete xrcDataStream;
	}
}

void XRCReader::importResourceChildren(XRCReadStream *stream, Resources::Object *resource) {
	// Get the number of children
	uint16 numChildren = stream->readUint16LE();

	// Read more unknown data
	uint16 unknown3 = stream->readUint16LE();
	if (unknown3 != 0) {
		warning("Stark::XRCReader: \"%s\" has unknown3=0x%04X with unknown meaning", resource->getName().c_str(), unknown3);
	}

	// Read the children resources
	for (int i = 0; i < numChildren; i++) {
		Resources::Object *child = importResource(stream, resource);

		// Add child to parent
		resource->addChild(child);
	}
}

} // End of namespace Formats
} // End of namespace Stark
