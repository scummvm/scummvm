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

#include "bladerunner/obstacles.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/savefile.h"

namespace BladeRunner {

Obstacles::Obstacles(BladeRunnerEngine *vm) {
	_vm = vm;
	_polygons       = new Polygon[kPolygonCount];
	_polygonsBackup = new Polygon[kPolygonCount];
	_vertices       = new Vector2[kVertexCount];
	clear();
}

Obstacles::~Obstacles() {
	delete[] _vertices;
	delete[] _polygonsBackup;
	delete[] _polygons;
}

void Obstacles::clear() {
	for (int i = 0; i < kPolygonCount; i++) {
		_polygons[i].isPresent = false;
		_polygons[i].verticeCount = 0;
		for (int j = 0; j < kPolygonVertexCount; j++) {
			_polygons[i].vertices[j].x = 0.0f;
			_polygons[i].vertices[j].y = 0.0f;
		}
	}
	_verticeCount = 0;
	_backup = false;
	_count = 0;
}

void Obstacles::add(float x0, float z0, float x1, float z1) {
}

bool Obstacles::find(const Vector3 &from, const Vector3 &to, Vector3 *next) const {
	//TODO
	*next = to;
	return true;
}

void Obstacles::backup() {
}

void Obstacles::restore() {}

void Obstacles::save(SaveFileWriteStream &f) {
	f.writeBool(_backup);
	f.writeInt(_count);
	for (int i = 0; i < _count; ++i) {
		Polygon &p = _polygonsBackup[i];
		f.writeBool(p.isPresent);
		f.writeInt(p.verticeCount);
		f.writeFloat(p.left);
		f.writeFloat(p.bottom);
		f.writeFloat(p.right);
		f.writeFloat(p.top);
		for (int j = 0; j < kPolygonVertexCount; ++j) {
			f.writeVector2(p.vertices[j]);
		}
		for (int j = 0; j < kPolygonVertexCount; ++j) {
			f.writeInt(p.vertexType[j]);
		}
	}
	for (int i = 0; i < kVertexCount; ++i) {
		f.writeVector2(_vertices[i]);
	}
	f.writeInt(_verticeCount);
}

void Obstacles::load(SaveFileReadStream &f) {
	for (int i = 0; i < kPolygonCount; ++i) {
		_polygons[i].isPresent = false;
		_polygons[i].verticeCount = 0;
		_polygonsBackup[i].isPresent = false;
		_polygonsBackup[i].verticeCount = 0;
	}

	_backup = f.readBool();
	_count = f.readInt();
	for (int i = 0; i < _count; ++i) {
		Polygon &p = _polygonsBackup[i];
		p.isPresent = f.readBool();
		p.verticeCount = f.readInt();
		p.left = f.readFloat();
		p.bottom = f.readFloat();
		p.right = f.readFloat();
		p.top = f.readFloat();
		for (int j = 0; j < kPolygonVertexCount; ++j) {
			p.vertices[j] = f.readVector2();
		}
		for (int j = 0; j < kPolygonVertexCount; ++j) {
			p.vertexType[j] = f.readInt();
		}
	}

	for (int i = 0; i < kPolygonCount; ++i) {
		_polygons[i] = _polygonsBackup[i];
	}

	for (int i = 0; i < kVertexCount; ++i) {
		_vertices[i] = f.readVector2();
	}
	_verticeCount = f.readInt();
}


} // End of namespace BladeRunner
