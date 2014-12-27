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

#include "engines/stark/xrcreader.h"

#include "engines/stark/resources/camera.h"
#include "engines/stark/resources/command.h"
#include "engines/stark/resources/floor.h"
#include "engines/stark/resources/floorface.h"
#include "engines/stark/resources/resource.h"

namespace Stark {

XRCReadStream::XRCReadStream(Common::SeekableReadStream *parentStream, DisposeAfterUse::Flag disposeParentStream) :
		SeekableSubReadStream(parentStream, 0, parentStream->size(), disposeParentStream) {
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

ResourceReference XRCReadStream::readResourceReference() {
	ResourceReference path;

	uint32 pathSize = readUint32LE();
	for (uint i = 0; i < pathSize; i++) {
		ResourcePair resourceId;
		resourceId.readFromStream(this);
		path.push_back(resourceId);
	}

	return path;
}

Math::Vector3d XRCReadStream::readVector3() {
	Math::Vector3d v;
	v.readFromStream(this);
	return v;
}

Math::Vector4d XRCReadStream::readVector4() {
	Math::Vector4d v;
	v.readFromStream(this);
	return v;
}

float XRCReadStream::readFloat() {
	float f;
	read(&f, sizeof(float));
	return f;
}

bool XRCReadStream::isDataLeft() {
	return pos() < size();
}

Resource *XRCReader::importTree(Common::SeekableReadStream *stream) {
	XRCReadStream *xrcStream = new XRCReadStream(stream);
	return importResource(xrcStream, nullptr);
}

Resource *XRCReader::importResource(XRCReadStream *stream, Resource *parent) {
	Resource *resource = createResource(stream, parent);
	importResourceData(stream, resource);
	importResourceChildren(stream, resource);
	return resource;
}

Resource *XRCReader::createResource(XRCReadStream *stream, Resource *parent) {
	// Read the resource type and subtype
	ResourceType type;
	type.readFromStream(stream);
	byte subType = stream->readByte();

	// Read the resource properties
	uint16 index = stream->readUint16LE();
	Common::String name = stream->readString();

	// Create a new resource
	Resource *resource;
	switch (type.get()) {
	case ResourceType::kCamera:
		resource = new Camera(parent, subType, index, name);
		break;
	case ResourceType::kFloor:
		resource = new Floor(parent, subType, index, name);
		break;
	case ResourceType::kFloorFace:
		resource = new FloorFace(parent, subType, index, name);
		break;
	case ResourceType::kCommand:
		resource = new Command(parent, subType, index, name);
		break;
	default:
		resource = new UnimplementedResource(parent, type, subType, index, name);
		break;
	}

	return resource;
}

void XRCReader::importResourceData(XRCReadStream *stream, Resource *resource) {
	// Read the data length
	uint32 dataLength = stream->readUint32LE();

	// Read the resource type specific data using a memory stream
	if (dataLength > 0) {
		XRCReadStream *xrcDataStream = new XRCReadStream(stream->readStream(dataLength));

		resource->readData(xrcDataStream);

		if (xrcDataStream->isDataLeft()) {
			warning("Not all XRC data was read. Type %s, name %s",
					resource->getType().getName(), resource->getName().c_str());
		}

		delete xrcDataStream;
	}
}

void XRCReader::importResourceChildren(XRCReadStream *stream, Resource *resource) {
	// Get the number of children
	uint16 numChildren = stream->readUint16LE();

	// Read more unknown data
	uint16 unknown3 = stream->readUint16LE();
	if (unknown3 != 0) {
		warning("Stark::XRCReader: \"%s\" has unknown3=0x%04X with unknown meaning", resource->getName().c_str(), unknown3);
	}

	// Read the children resources
	for (int i = 0; i < numChildren; i++) {
		Resource *child = importResource(stream, resource);

		// Add child to parent
		resource->addChild(child);
	}
}

} // End of namespace Stark
