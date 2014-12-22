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

#ifndef STARK_XRC_H
#define STARK_XRC_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "common/stream.h"

#include "math/vector3d.h"
#include "math/vector4d.h"

namespace Stark {

class NodeType {
public:
	enum Type {
		kInvalid                = 0,
		kRoot                   = 1,
		kLevel                  = 2,
		kRoom                   = 3,
		kLayer                  = 4,
		kCamera                 = 5,
		kFloor                  = 6,
		kFace                   = 7,
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

	NodeType();
	NodeType(Type type);

	void readFromStream(Common::ReadStream *stream);

	Type get();
	const char *getName();
	bool is(Type type);

private:
	Type _type;
};

class NodePair {
public:
	NodePair();
	NodePair(NodeType type, uint16 index);

	void readFromStream(Common::ReadStream *stream);
	Common::String describe();

private:
	NodeType _type;
	uint16 _index;
};

typedef Common::Array<NodePair> NodePath;

class XRCNode {
public:
	virtual ~XRCNode();

	static XRCNode *read(Common::ReadStream *stream);

	Common::String getName() const { return _name; }
	NodeType getType() const { return _type; }
	Common::Array<XRCNode *> getChildren() const { return _children; }

	/**
	 * Get the archive file name containing the data for this node.
	 * Only Levels and Rooms have archives.
	 */
	Common::String getArchive();

	void print(uint depth = 0);

protected:
	XRCNode();

	void readCommon(Common::ReadStream *stream);
	virtual void readData(Common::ReadStream *stream) = 0;
	void readChildren(Common::ReadStream *stream);

	static Common::String readString(Common::ReadStream *stream);
	static NodePath readNodeReference(Common::ReadStream *stream);
	static Math::Vector3d readVector3(Common::ReadStream *stream);
	static Math::Vector4d readVector4(Common::ReadStream *stream);
	static float readFloat(Common::ReadStream *stream);
	static bool isDataLeft(Common::SeekableReadStream *stream);

	virtual void printData() = 0;

	NodeType _type;
	byte _subType;
	uint16 _nodeIndex;	// Node order inside the parent node
	Common::String _name;

	uint32 _dataLength;

	uint16 _unknown3;

	XRCNode *_parent;
	Common::Array<XRCNode *> _children;
};

class UnimplementedXRCNode : public XRCNode {
public:
	virtual ~UnimplementedXRCNode();

	const byte *getData() const { return _data; }

protected:
	UnimplementedXRCNode();

	void readData(Common::ReadStream *stream) override;
	void printData() override;

	byte *_data;

	friend class XRCNode;
};

class ScriptXRCNode : public XRCNode {
public:
	virtual ~ScriptXRCNode();

	struct Argument {
		enum Type {
			kTypeInteger1 = 1,
			kTypeInteger2 = 2,
			kTypeNodeReference = 3,
			kTypeString = 4
		};

		uint32 type;
		uint32 intValue;
		Common::String stringValue;
		NodePath referenceValue;
	};

protected:
	ScriptXRCNode();

	void readData(Common::ReadStream *stream) override;
	void printData() override;

	Common::Array<Argument> _arguments;

	friend class XRCNode;
};

class CameraXRCNode : public XRCNode {
public:
	virtual ~CameraXRCNode();

protected:
	CameraXRCNode();

	void readData(Common::ReadStream *stream) override;
	void printData() override;

	Math::Vector3d _position;
	Math::Vector3d _lookAt;
	float _fov;
	float _f2;
	Math::Vector4d _v3;
	Math::Vector3d _v4;

	friend class XRCNode;
};

class FloorXRCNode : public XRCNode {
public:
	virtual ~FloorXRCNode();

protected:
	FloorXRCNode();

	void readData(Common::ReadStream *stream) override;
	void printData() override;

	uint32 _facesCount;
	Common::Array<Math::Vector3d> _positions;

	friend class XRCNode;
};

class FaceXRCNode : public XRCNode {
public:
	virtual ~FaceXRCNode();

protected:
	FaceXRCNode();

	void readData(Common::ReadStream *stream) override;
	void printData() override;

	int16 _indices[3];

	float _unk1;
	float _unk2;

	friend class XRCNode;
};

} // End of namespace Stark

#endif // STARK_XRC_H
