/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"

#include "engines/grim/keyframe.h"
#include "engines/grim/textsplit.h"

namespace Grim {

KeyframeAnim::KeyframeAnim(const char *filename, const char *data, int len) :
		Resource(filename) {

	if (len >= 4 && READ_BE_UINT32(data) == MKID_BE('FYEK'))
		loadBinary(data, len);
	else {
		TextSplitter ts(data, len);
		loadText(ts);
	}
}

void KeyframeAnim::loadBinary(const char *data, int len) {
	// First four bytes are the FYEK Keyframe identifier code
	// Next 36 bytes are the filename
	if (gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL) {
		char filebuf[37];

		memcpy(filebuf, data + 4, 36);
		filebuf[36] = 0;
		printf("Loading Keyframe '%s'.", filebuf);
	}
	// Next four bytes are the flags
	_flags = READ_LE_UINT32(data + 40);
	// Next four bytes are a duplicate of _numJoints (?)
	// Next four bytes are the type
	_type = READ_LE_UINT32(data + 48);
	// Next four bytes are the frames per second
	_fps = get_float(data + 52);
	// Next four bytes are the number of frames
	_numFrames = READ_LE_UINT32(data + 56);
	// Next four bytes are the number of joints
	_numJoints = READ_LE_UINT32(data + 60);
	// Next four bytes are unknown (?)
	// Next four bytes are the number of markers
	_numMarkers = READ_LE_UINT32(data + 68);
	_markers = new Marker[_numMarkers];
	for (int i = 0; i < _numMarkers; i++) {
		_markers[i].frame = get_float(data + 72 + 4 * i);
		_markers[i].val = READ_LE_UINT32(data + 104 + 4 * i);
	}

	_nodes = new KeyframeNode *[_numJoints];
	for (int i = 0; i < _numJoints; i++)
		_nodes[i] = NULL;
	const char *dataEnd = data + len;
	// The first 136 bytes are for the header, this was originally
	// listed as 180 bytes since the first operation is usually a
	// "null" key, however ma_card_hold.key showed that this is
	// not always the case so we should not skip this operation
	data += 136;
	while (data < dataEnd) {
		int nodeNum;
		// The first 32 bytes (of a keyframe) are the name handle
		// The next four bytes are the node number identifier
		nodeNum = READ_LE_UINT32(data + 32);

		// Because of the issue above ma_card_hold.key used to crash
		// at this part without checking to make sure nodeNum is a
		// valid number, we'll leave this in just in case something
		// else is still wrong but it should now load correctly in
		// all cases
		if (nodeNum >= _numJoints) {
			if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL) {
				warning("A node number was greater than the maximum number of nodes (%d/%d)", nodeNum, _numJoints);
			}
			return;
		}
		_nodes[nodeNum] = new KeyframeNode;
		_nodes[nodeNum]->loadBinary(data);
	}
}

void KeyframeAnim::loadText(TextSplitter &ts) {
	ts.expectString("section: header");
	ts.scanString("flags %x", 1, &_flags);
	ts.scanString("type %x", 1, &_type);
	ts.scanString("frames %d", 1, &_numFrames);
	ts.scanString("fps %f", 1, &_fps);
	ts.scanString("joints %d", 1, &_numJoints);

	if (strcasecmp(ts.currentLine(), "section: markers") == 0) {
		ts.nextLine();
		ts.scanString("markers %d", 1, &_numMarkers);
		_markers = new Marker[_numMarkers];
		for (int i = 0; i < _numMarkers; i++)
			ts.scanString("%f %d", 2, &_markers[i].frame, &_markers[i].val);
	} else {
		_numMarkers = 0;
		_markers = NULL;
	}

	ts.expectString("section: keyframe nodes");
	int numNodes;
	ts.scanString("nodes %d", 1, &numNodes);
	_nodes = new KeyframeNode *[_numJoints];
	for (int i = 0; i < _numJoints; i++)
		_nodes[i] = NULL;
	for (int i = 0; i < numNodes; i++) {
		int which;
		ts.scanString("node %d", 1, &which);
		_nodes[which] = new KeyframeNode;
		_nodes[which]->loadText(ts);
	}
}

KeyframeAnim::~KeyframeAnim() {
	for (int i = 0; i < _numJoints; i++)
		delete _nodes[i];
	delete[] _markers;
}

void KeyframeAnim::animate(Model::HierNode *nodes, float time, int priority1, int priority2) const {
	float frame = time * _fps;

	if (frame > _numFrames)
		frame = _numFrames;

	for (int i = 0; i < _numJoints; i++) {
		if (_nodes[i])
			_nodes[i]->animate(nodes[i], frame, ((_type & nodes[i]._type) != 0 ? priority2 : priority1));
	}
}

void KeyframeAnim::KeyframeEntry::loadBinary(const char *&data) {
	_frame = get_float(data);
	_flags = READ_LE_UINT32(data + 4);
	_pos = Graphics::get_vector3d(data + 8);
	_pitch = get_float(data + 20);
	_yaw = get_float(data + 24);
	_roll = get_float(data + 28);
	_dpos = Graphics::get_vector3d(data + 32);
	_dpitch = get_float(data + 44);
	_dyaw = get_float(data + 48);
	_droll = get_float(data + 52);
	data += 56;
}

void KeyframeAnim::KeyframeNode::loadBinary(const char *&data) {
	// If the name handle is entirely null (like ma_rest.key)
	// then we shouldn't try to set the name
	if (READ_LE_UINT32(data) == 0)
		memcpy(_meshName, "(null)", 32);
	else
		memcpy(_meshName, data, 32);
	_numEntries = READ_LE_UINT32(data + 36);
	data += 44;
	_entries = new KeyframeEntry[_numEntries];
	for (int i = 0; i < _numEntries; i++)
		_entries[i].loadBinary(data);
}

void KeyframeAnim::KeyframeNode::loadText(TextSplitter &ts) {
	ts.scanString("mesh name %s", 1, _meshName);
	ts.scanString("entries %d", 1, &_numEntries);
	_entries = new KeyframeEntry[_numEntries];
	for (int i = 0; i < _numEntries; i++) {
		int which;
		unsigned flags;
		float frame, x, y, z, p, yaw, r, dx, dy, dz, dp, dyaw, dr;
		ts.scanString(" %d: %f %x %f %f %f %f %f %f", 9, &which, &frame, &flags, &x, &y, &z, &p, &yaw, &r);
		ts.scanString(" %f %f %f %f %f %f", 6, &dx, &dy, &dz, &dp, &dyaw, &dr);
		_entries[which]._frame = frame;
		_entries[which]._flags = (int)flags;
		_entries[which]._pos = Graphics::Vector3d(x, y, z);
		_entries[which]._dpos = Graphics::Vector3d(dx, dy, dz);
		_entries[which]._pitch = p;
		_entries[which]._yaw = yaw;
		_entries[which]._roll = r;
		_entries[which]._dpitch = dp;
		_entries[which]._dyaw = dyaw;
		_entries[which]._droll = dr;
	}
}

KeyframeAnim::KeyframeNode::~KeyframeNode() {
	delete[] _entries;
}

void KeyframeAnim::KeyframeNode::animate(Model::HierNode &node, float frame, int priority) const {
	if (_numEntries == 0)
		return;
	if (priority < node._priority)
		return;

	// Do a binary search for the nearest previous frame
	// Loop invariant: entries_[low].frame_ <= frame < entries_[high].frame_
	int low = 0, high = _numEntries;
	while (high > low + 1) {
		int mid = (low + high) / 2;
		if (_entries[mid]._frame <= frame)
			low = mid;
		else
			high = mid;
	}

	float dt = frame - _entries[low]._frame;
	Graphics::Vector3d pos = _entries[low]._pos + dt * _entries[low]._dpos;
	float pitch = _entries[low]._pitch + dt * _entries[low]._dpitch;
	float yaw = _entries[low]._yaw + dt * _entries[low]._dyaw;
	float roll = _entries[low]._roll + dt * _entries[low]._droll;
	if (pitch > 180)
		pitch -= 360;
	if (yaw > 180)
		yaw -= 360;
	if (roll > 180)
		roll -= 360;

	if (priority > node._priority) {
		node._priority = priority;
		node._totalWeight = 1;
		node._animPos = pos;
		node._animPitch = pitch;
		node._animYaw = yaw;
		node._animRoll = roll;
	} else { // priority == node._priority
		node._totalWeight++;
		node._animPos += pos;
		node._animPitch += pitch;
		node._animYaw += yaw;
		node._animRoll += roll;
	}

	// node
}

} // end of namespace Grim
