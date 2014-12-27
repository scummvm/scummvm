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

#ifndef STARK_XRC_READER_H
#define STARK_XRC_READER_H

#include "common/array.h"
#include "common/str.h"
#include "common/substream.h"
#include "common/types.h"

#include "math/vector3d.h"
#include "math/vector4d.h"

namespace Stark {

class XRCNode;
class NodePair;
typedef Common::Array<NodePair> NodePath;

/**
 * A read stream with helper functions to read usual XRC data types
 */
class XRCReadStream : public Common::SeekableSubReadStream {
public:
	XRCReadStream(Common::SeekableReadStream *parentStream, DisposeAfterUse::Flag disposeParentStream = DisposeAfterUse::YES);
	virtual ~XRCReadStream();

	Common::String readString();
	NodePath readNodeReference();
	Math::Vector3d readVector3();
	Math::Vector4d readVector4();
	float readFloat();
	bool isDataLeft();
};

/**
 * An XRC stream parser, used to build resource trees.
 */
class XRCReader {
public:
	/**
	 * Build an XRC tree from a stream
	 */
	static XRCNode *readTree(Common::SeekableReadStream *stream);

protected:
	static XRCNode *readNode(XRCReadStream *stream, XRCNode *parent);
	static XRCNode *buildNode(XRCReadStream *stream, XRCNode *parent);
	static void readNodeChildren(XRCReadStream *stream, XRCNode *node);
	static void readNodeData(XRCReadStream* stream, XRCNode* node);
};

} // End of namespace Stark

#endif // STARK_XRC_READER_H
