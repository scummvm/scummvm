/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_KEYFRAME_H
#define GRIM_KEYFRAME_H

#include "engines/grim/object.h"

namespace Grim {

class KeyframeAnim : public Object {
public:
	KeyframeAnim(const Common::String &filename, const char *data, int len);
	~KeyframeAnim();

	void loadBinary(const char *data, int len);
	void loadText(TextSplitter &ts);
	void animate(Model::HierNode *nodes, int num, float time, int priority1 = 1, int priority2 = 5, float fade = 1) const;

	float getLength() const { return _numFrames / _fps; }
	const Common::String &getFilename() const { return _fname; }

private:
	Common::String _fname;
	unsigned int _flags, _type;
	int _numFrames, _numJoints;
	float _fps;
	int _numMarkers;

	struct Marker {
		float frame;
		int val;
	};
	Marker *_markers;

	struct KeyframeEntry {
		void loadBinary(const char *&data);

		float _frame;
		int _flags;
		Graphics::Vector3d _pos, _dpos;
		float _pitch, _yaw, _roll, _dpitch, _dyaw, _droll;
	};

	struct KeyframeNode {
		void loadBinary(const char *&data);
		void loadText(TextSplitter &ts);
		~KeyframeNode();

		void animate(Model::HierNode &node, float frame, int priority, float fade) const;

		char _meshName[32];
		int _numEntries;
		KeyframeEntry *_entries;
	};

	KeyframeNode **_nodes;
};

} // end of namespace Grim

#endif
