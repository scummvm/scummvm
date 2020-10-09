/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#ifndef GRIM_KEYFRAME_H
#define GRIM_KEYFRAME_H

#include "math/vector3d.h"

#include "engines/grim/object.h"

namespace Common {
class SeekableReadStream;
}

namespace Grim {

class ModelNode;
class TextSplitter;

class KeyframeAnim : public Object {
public:
	KeyframeAnim(const Common::String &filename, Common::SeekableReadStream *data);
	~KeyframeAnim();

	void loadBinary(Common::SeekableReadStream *data);
	void loadText(TextSplitter &ts);
	bool isNodeAnimated(ModelNode *nodes, int num, float time, bool tagged) const;
	void animate(ModelNode *nodes, int num, float time, float fade, bool tagged) const;
	int getMarker(float startTime, float stopTime) const;

	float getLength() const { return _numFrames / _fps; }
	const Common::String &getFilename() const { return _fname; }

private:
	Common::String _fname;
	unsigned int _flags;
	/**
	 * A bitfield ID which specifies which joints of the skeleton hierarchy this
	 * KeyFrameAnim can animate on. This is ANDed against the _type of the ModelNode
	 * to test whether this KeyFrameAnim can animate that ModelNode, or if it is to
	 * be ignored.
	 */
	unsigned int _type;
	int _numFrames, _numJoints;
	float _fps;
	int _numMarkers;

	struct Marker {
		float frame;
		int val;
	};
	Marker *_markers;

	struct KeyframeEntry {
		void loadBinary(Common::SeekableReadStream *data);

		float _frame;
		int _flags;
		Math::Vector3d _pos, _dpos;
		Math::Angle _pitch, _yaw, _roll, _dpitch, _dyaw, _droll;
	};

	struct KeyframeNode {
		void loadBinary(Common::SeekableReadStream *data, char *meshName);
		void loadText(TextSplitter &ts);
		~KeyframeNode();

		void animate(ModelNode &node, float frame, float fade, bool useDelta) const;

		char _meshName[32];
		int _numEntries;
		KeyframeEntry *_entries;
	};

	KeyframeNode **_nodes;
};

} // end of namespace Grim

#endif
