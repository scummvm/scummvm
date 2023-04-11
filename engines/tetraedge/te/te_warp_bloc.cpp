/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tetraedge/te/te_warp_bloc.h"

namespace Tetraedge {

TeWarpBloc::TeWarpBloc() : _colors(nullptr), _cubeFace(FaceInvalid) {
}

TeWarpBloc::~TeWarpBloc() {
	if (_colors)
		delete _colors;
}

void TeWarpBloc::color(const TeColor &col) {
	color(0, col);
	color(1, col);
	color(2, col);
	color(3, col);
}

void TeWarpBloc::color(uint num, const TeColor &col) {
	if (_colors == nullptr) {
		_colors = new TeColor[4];
	}
	assert(num < 4);
	_colors[num] = col;
}

void TeWarpBloc::create(CubeFace face, uint x, uint y, const TeVector2s32 &offset) {
	_cubeFace = face;
	_offset = offset;

	float y1 = offset._y * (1000.0f / y) - 500.0f;
	float y2 = y1 + 1000.0f / y;
	float x1 = offset._x * (1000.0f / x) - 500.0f;
	float x2 = x1 + 1000.0f / x;
	switch (face) {
	case Face0:
		_verticies[0] = TeVector3f32(-x1, 500, -y1);
		_verticies[1] = TeVector3f32(-x2, 500, -y1);
		_verticies[2] = TeVector3f32(-x2, 500, -y2);
		_verticies[3] = TeVector3f32(-x1, 500, -y2);
		break;
	case Face1:
		_verticies[0] = TeVector3f32(-x1, -500, y1);
		_verticies[1] = TeVector3f32(-x2, -500, y1);
		_verticies[2] = TeVector3f32(-x2, -500, y2);
		_verticies[3] = TeVector3f32(-x1, -500, y2);
		break;
	case Face2:
		_verticies[0] = TeVector3f32(-x1, y1, 500);
		_verticies[1] = TeVector3f32(-x2, y1, 500);
		_verticies[2] = TeVector3f32(-x2, y2, 500);
		_verticies[3] = TeVector3f32(-x1, y2, 500);
		break;
	case Face3:
		_verticies[0] = TeVector3f32(x1, y1, -500);
		_verticies[1] = TeVector3f32(x2, y1, -500);
		_verticies[2] = TeVector3f32(x2, y2, -500);
		_verticies[3] = TeVector3f32(x1, y2, -500);
		break;
	case Face4:
		_verticies[0] = TeVector3f32(500, y1, x1);
		_verticies[1] = TeVector3f32(500, y1, x2);
		_verticies[2] = TeVector3f32(500, y2, x2);
		_verticies[3] = TeVector3f32(500, y2, x1);
		break;
	case Face5:
		_verticies[0] = TeVector3f32(-500, y1, -x1);
		_verticies[1] = TeVector3f32(-500, y1, -x2);
		_verticies[2] = TeVector3f32(-500, y2, -x2);
		_verticies[3] = TeVector3f32(-500, y2, -x1);
		break;
	default:
		break;
	}

	_texCoords[0] = TeVector2f32(0, 0);
	_texCoords[1] = TeVector2f32(1, 0);
	_texCoords[2] = TeVector2f32(1, 1);
	_texCoords[3] = TeVector2f32(0, 1);
	_indexes[0] = 0;
	_indexes[1] = 1;
	_indexes[2] = 2;
	_indexes[3] = 3;
}

void TeWarpBloc::create() {
	_colors = nullptr;
	_texture.release();
}

void TeWarpBloc::index(uint offset, uint val) {
	assert(offset < 4);
	_indexes[offset] = val;
}

bool TeWarpBloc::isLoaded() const {
	return _texture.get() != nullptr;
}

void TeWarpBloc::loadTexture(Common::File &file, const Common::String &type) {
	if (_texture)
		return;

	if (!file.seek(_textureDataFileOffset))
		error("TeWarpBloc::LoadTexture: seek error");

	TeImage img;
	img.load(file, type);

	_texture = Te3DTexture::makeInstance();
	_texture->load(img);
}

void TeWarpBloc::render() {
	error("Implement TeWarpBloc::render");
}

void TeWarpBloc::texture(uint idx, float x, float y) {
	assert(idx < 4);
	_texCoords[idx].setX(x);
	_texCoords[idx].setY(y);
}

void TeWarpBloc::unloadTexture() {
	_texture.release();
}

void TeWarpBloc::vertex(uint idx, float x, float y, float z) {
	assert(idx < 4);
	_verticies[idx].x() = x;
	_verticies[idx].y() = y;
	_verticies[idx].z() = z;
}

const TeVector3f32 &TeWarpBloc::vertex(uint idx) const {
	assert(idx < 4);
	return _verticies[idx];
}

} // end namespace Tetraedge
