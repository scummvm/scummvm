// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef KEYFRAME_H
#define KEYFRAME_H

#include "vector3d.h"
#include "resource.h"
#include "model.h"

class TextSplitter;

class KeyframeAnim : public Resource {
public:
	KeyframeAnim(const char *filename, const char *data, int len);
	~KeyframeAnim();

	void loadBinary(const char *data, int len);
	void loadText(TextSplitter &ts);
	void animate(Model::HierNode *nodes, float time, int priority1 = 1, int priority2 = 5) const;

	float length() const { return _numFrames / _fps; }

private:
	int _flags, _type, _numFrames, _numJoints;
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
		Vector3d _pos, _dpos;
		float _pitch, _yaw, _roll, _dpitch, _dyaw, _droll;
	};

	struct KeyframeNode {
		void loadBinary(const char *&data);
		void loadText(TextSplitter &ts);
		~KeyframeNode();

		void animate(Model::HierNode &node, float frame, int priority) const;

		char _meshName[32];
		int _numEntries;
		KeyframeEntry *_entries;
	};

	KeyframeNode **_nodes;
};

#endif
