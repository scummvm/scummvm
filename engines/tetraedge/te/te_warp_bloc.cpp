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
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/tetraedge.h"

namespace Tetraedge {

TeWarpBloc::TeWarpBloc() : _cubeFace(FaceInvalid), _textureDataFileOffset(0) {
	_mesh.reset(TeMesh::makeInstance());
}

TeWarpBloc::~TeWarpBloc() {
}

void TeWarpBloc::color(const TeColor &col) {
	color(0, col);
	color(1, col);
	color(2, col);
	color(3, col);
}

void TeWarpBloc::color(uint num, const TeColor &col) {
	_mesh->setColor(num, col);
}

void TeWarpBloc::create(CubeFace face, uint x, uint y, const TeVector2s32 &offset) {
	_cubeFace = face;
	_offset = offset;

	_mesh->setConf(4, 4, TeMesh::MeshMode_TriangleStrip, 0, 0);

	float x1 = offset._x * (1000.0f / x) - 500.0f;
	float x2 = 1000.0f / x + x1;
	float y1 = offset._y * (1000.0f / y) - 500.0f;
	float y2 = 1000.0f / y + y1;

	switch (face) {
	case Face0:
		_mesh->setVertex(0, TeVector3f32(-x1, 500, -y1));
		_mesh->setVertex(1, TeVector3f32(-x2, 500, -y1));
		_mesh->setVertex(2, TeVector3f32(-x2, 500, -y2));
		_mesh->setVertex(3, TeVector3f32(-x1, 500, -y2));
		break;
	case Face1:
		_mesh->setVertex(0, TeVector3f32(-x1, -500, y1));
		_mesh->setVertex(1, TeVector3f32(-x2, -500, y1));
		_mesh->setVertex(2, TeVector3f32(-x2, -500, y2));
		_mesh->setVertex(3, TeVector3f32(-x1, -500, y2));
		break;
	case Face2:
		_mesh->setVertex(0, TeVector3f32(-x1, y1, 500));
		_mesh->setVertex(1, TeVector3f32(-x2, y1, 500));
		_mesh->setVertex(2, TeVector3f32(-x2, y2, 500));
		_mesh->setVertex(3, TeVector3f32(-x1, y2, 500));
		break;
	case Face3:
		_mesh->setVertex(0, TeVector3f32(x1, y1, -500));
		_mesh->setVertex(1, TeVector3f32(x2, y1, -500));
		_mesh->setVertex(2, TeVector3f32(x2, y2, -500));
		_mesh->setVertex(3, TeVector3f32(x1, y2, -500));
		break;
	case Face4:
		_mesh->setVertex(0, TeVector3f32(500, y1, x1));
		_mesh->setVertex(1, TeVector3f32(500, y1, x2));
		_mesh->setVertex(2, TeVector3f32(500, y2, x2));
		_mesh->setVertex(3, TeVector3f32(500, y2, x1));
		break;
	case Face5:
		_mesh->setVertex(0, TeVector3f32(-500, y1, -x1));
		_mesh->setVertex(1, TeVector3f32(-500, y1, -x2));
		_mesh->setVertex(2, TeVector3f32(-500, y2, -x2));
		_mesh->setVertex(3, TeVector3f32(-500, y2, -x1));
		break;
	default:
		break;
	}

	_mesh->setTextureUV(0, TeVector2f32(0, 0));
	_mesh->setTextureUV(1, TeVector2f32(1, 0));
	_mesh->setTextureUV(2, TeVector2f32(1, 1));
	_mesh->setTextureUV(3, TeVector2f32(0, 1));
	_mesh->setNormal(0, TeVector3f32(0, 0, 1));
	_mesh->setNormal(1, TeVector3f32(0, 0, 1));
	_mesh->setNormal(2, TeVector3f32(0, 0, 1));
	_mesh->setNormal(3, TeVector3f32(0, 0, 1));
	_mesh->setIndex(0, 0);
	_mesh->setIndex(1, 1);
	_mesh->setIndex(2, 3);
	_mesh->setIndex(3, 2);
	_mesh->setColor(TeColor(255, 255, 255, 255));
}

void TeWarpBloc::create() {
	_mesh->materials().clear();
}

void TeWarpBloc::index(uint offset, uint val) {
	assert(offset < 4);
	_mesh->setIndex(offset, val);
}

bool TeWarpBloc::isLoaded() const {
	return _mesh->materials().size() > 0 && _mesh->material(0)->_texture;
}

void TeWarpBloc::loadTexture(Common::SeekableReadStream &file, const Common::String &type) {
	if (isLoaded())
		return;

	if (!file.seek(_textureDataFileOffset))
		error("TeWarpBloc::LoadTexture: seek error");

	TeImage img;
	img.load(file, type);

	TeIntrusivePtr<Te3DTexture> tex = Te3DTexture::makeInstance();
	tex->load(img);
	_mesh->defaultMaterial(tex);
}

void TeWarpBloc::render() {
	_mesh->draw();
}

void TeWarpBloc::texture(uint idx, float x, float y) {
	assert(idx < 4);
	_mesh->setTextureUV(idx, TeVector2f32(x, y));
}

void TeWarpBloc::unloadTexture() {
	if (!isLoaded())
		return;
	_mesh->material(0)->_texture.release();
}

void TeWarpBloc::vertex(uint idx, float x, float y, float z) {
	assert(idx < 4);
	_mesh->setVertex(idx, TeVector3f32(x, y, z));
}

TeVector3f32 TeWarpBloc::vertex(uint idx) const {
	assert(idx < 4);
	return _mesh->vertex(idx);
}

} // end namespace Tetraedge
