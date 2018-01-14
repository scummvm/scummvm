/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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

#ifndef BLADERUNNER_OBSTACLES_H
#define BLADERUNNER_OBSTACLES_H

#include "bladerunner/vector.h"

namespace BladeRunner {

class BladeRunnerEngine;

class Obstacles {
	static const int kPolygonCount = 50;
	static const int kVertexCount = 160;

	struct Polygon {
		bool    isPresent;
		int     verticeCount;
		float   left;
		float   bottom;
		float   right;
		float   top;
		Vector2 vertices[kVertexCount];
		int     vertexType[kVertexCount];
	};

	BladeRunnerEngine *_vm;

	Polygon  _polygons[kPolygonCount];
	Polygon  _polygonsBackup[kPolygonCount];
	Vector2 *_vertices;
	int      _verticeCount;
	int      _count;
	bool     _backup;

public:
	Obstacles(BladeRunnerEngine *vm);
	~Obstacles();

	void clear();
	void add(float x0, float z0, float x1, float z1);
	bool find(const Vector3 &from, const Vector3 &to, Vector3 *next) const;
	void backup();
	void restore();
};

} // End of namespace BladeRunner

#endif
