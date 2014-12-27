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

#include "engines/stark/xrc.h"

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

NodePath XRCReadStream::readNodeReference() {
	NodePath path;

	uint32 pathSize = readUint32LE();
	for (uint i = 0; i < pathSize; i++) {
		NodePair nodeId;
		nodeId.readFromStream(this);
		path.push_back(nodeId);
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

XRCNode *XRCReader::readTree(Common::SeekableReadStream *stream) {
	XRCReadStream *xrcStream = new XRCReadStream(stream);
	return readNode(xrcStream, nullptr);
}

XRCNode *XRCReader::readNode(XRCReadStream *stream, XRCNode *parent) {
	XRCNode *node = buildNode(stream, parent);
	readNodeData(stream, node);
	readNodeChildren(stream, node);
	return node;
}

XRCNode *XRCReader::buildNode(XRCReadStream *stream, XRCNode *parent) {
	// Read the resource type and subtype
	NodeType type;
	type.readFromStream(stream);
	byte subType = stream->readByte();

	// Read the resource properties
	uint16 index = stream->readUint16LE();
	Common::String name = stream->readString();

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

	return node;
}

void XRCReader::readNodeData(XRCReadStream *stream, XRCNode *node) {
	// Read the data length
	uint32 dataLength = stream->readUint32LE();

	// Read the node type specific data using a memory stream
	if (dataLength > 0) {
		XRCReadStream *xrcDataStream = new XRCReadStream(stream->readStream(dataLength));

		node->readData(xrcDataStream);

		if (xrcDataStream->isDataLeft()) {
			warning("Not all XRC data was read. Type %s, name %s",
					node->getType().getName(), node->getName().c_str());
		}

		delete xrcDataStream;
	}
}

void XRCReader::readNodeChildren(XRCReadStream *stream, XRCNode *node) {
	// Get the number of children
	uint16 numChildren = stream->readUint16LE();

	// Read more unknown data
	uint16 unknown3 = stream->readUint16LE();
	if (unknown3 != 0) {
		warning("Stark::XRCNode: \"%s\" has unknown3=0x%04X with unknown meaning", node->getName().c_str(), unknown3);
	}

	// Read the children nodes
	for (int i = 0; i < numChildren; i++) {
		XRCNode *child = readNode(stream, node);

		// Add child to parent
		node->addChild(child);
	}
}

} // End of namespace Stark
