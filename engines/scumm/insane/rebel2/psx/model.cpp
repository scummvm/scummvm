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
 */

#include "common/endian.h"
#include "common/system.h"

#include "scumm/insane/rebel2/psx/psx.h"

#include <math.h>

namespace Scumm {

static bool readU16(const Common::Array<byte> &data, uint32 offset, uint16 &value) {
	if (offset + 2 > data.size())
		return false;
	value = READ_LE_UINT16(data.data() + offset);
	return true;
}

static bool readU32(const Common::Array<byte> &data, uint32 offset, uint32 &value) {
	if (offset + 4 > data.size())
		return false;
	value = READ_LE_UINT32(data.data() + offset);
	return true;
}

RA2PSXModel::RA2PSXModel() : _radius(1.0f) {
}

bool RA2PSXModel::load(const Common::Array<byte> &data) {
	_vertices.clear();
	_faces.clear();
	_radius = 1.0f;
	if (!parseModel(data, 0, 0) || _vertices.empty() || _faces.empty())
		return false;

	double radiusSquared = 1.0;
	for (uint i = 0; i < _vertices.size(); ++i) {
		const RA2PSXVertex &vertex = _vertices[i];
		const double lengthSquared = (double)vertex.x * vertex.x +
				(double)vertex.y * vertex.y + (double)vertex.z * vertex.z;
		if (lengthSquared > radiusSquared)
			radiusSquared = lengthSquared;
	}
	_radius = (float)sqrt(radiusSquared);
	return true;
}

bool RA2PSXModel::parseModel(const Common::Array<byte> &data, uint32 modelOffset, int depth) {
	if (depth > 8 || modelOffset + 8 > data.size())
		return false;

	uint32 nodeOffset;
	uint32 childOffset;
	if (!readU32(data, modelOffset, nodeOffset) || !readU32(data, modelOffset + 4, childOffset))
		return false;

	uint32 nodeCount = 0;
	while (nodeOffset) {
		if (++nodeCount > 1024 || nodeOffset + 8 > data.size())
			return false;
		uint32 nextOffset;
		uint32 objectOffset;
		if (!readU32(data, nodeOffset, nextOffset) || !readU32(data, nodeOffset + 4, objectOffset) ||
				!parseObject(data, objectOffset))
			return false;
		nodeOffset = nextOffset;
	}

	return !childOffset || parseModel(data, childOffset, depth + 1);
}

bool RA2PSXModel::parseObject(const Common::Array<byte> &data, uint32 objectOffset) {
	if (objectOffset + 48 > data.size())
		return false;

	uint16 vertexCount;
	uint32 vertexOffset;
	uint32 primitiveOffset;
	if (!readU16(data, objectOffset + 2, vertexCount) || !readU32(data, objectOffset + 4, vertexOffset) ||
			!readU32(data, objectOffset + 8, primitiveOffset) || !vertexCount ||
			vertexOffset + (uint32)vertexCount * 8 > data.size())
		return false;

	const uint32 firstVertex = _vertices.size();
	if (firstVertex + vertexCount > 0xffff)
		return false;
	for (uint32 index = 0; index < vertexCount; ++index) {
		const byte *source = data.data() + vertexOffset + index * 8;
		RA2PSXVertex vertex;
		vertex.x = (int16)READ_LE_UINT16(source);
		vertex.y = (int16)READ_LE_UINT16(source + 2);
		vertex.z = (int16)READ_LE_UINT16(source + 4);
		_vertices.push_back(vertex);
	}

	Common::HashMap<uint16, bool> seen;
	for (uint32 listIndex = 0; listIndex < 8; ++listIndex) {
		uint32 listOffset;
		if (!readU32(data, objectOffset + 12 + listIndex * 4, listOffset) || listOffset >= data.size())
			return false;

		bool terminated = false;
		for (uint32 count = 0; count < 65536; ++count) {
			uint16 rawIndex;
			if (!readU16(data, listOffset, rawIndex))
				return false;
			listOffset += 2;
			if (rawIndex == 0xffff) {
				terminated = true;
				break;
			}
			if (seen.contains(rawIndex))
				continue;
			seen[rawIndex] = true;

			const uint32 record = primitiveOffset + (uint32)rawIndex * 32;
			if (record + 32 > data.size())
				return false;
			const uint16 mode = READ_LE_UINT16(data.data() + record + 12) & 0x1f;
			RA2PSXFace face = {};
			face.vertexCount = (mode & 0x10) ? 4 : 3;
			const uint16 offsets[4] = {
				READ_LE_UINT16(data.data() + record),
				READ_LE_UINT16(data.data() + record + 2),
				READ_LE_UINT16(data.data() + record + 4),
				READ_LE_UINT16(data.data() + record + 24)
			};
			for (uint vertexIndex = 0; vertexIndex < face.vertexCount; ++vertexIndex) {
				if ((offsets[vertexIndex] & 7) || offsets[vertexIndex] / 8 >= vertexCount)
					return false;
				face.vertex[vertexIndex] = firstVertex + offsets[vertexIndex] / 8;
			}
			face.r = data[record + 16];
			face.g = data[record + 17];
			face.b = data[record + 18];
			if (!(face.r | face.g | face.b))
				face.r = face.g = face.b = 0x7f;
			_faces.push_back(face);
		}
		if (!terminated)
			return false;
	}
	return true;
}

#ifdef USE_TINYGL

RA2PSXTinyGLRenderer::RA2PSXTinyGLRenderer() : _context(nullptr), _width(0), _height(0) {
}

RA2PSXTinyGLRenderer::~RA2PSXTinyGLRenderer() {
	if (_context)
		TinyGL::destroyContext(_context);
}

bool RA2PSXTinyGLRenderer::init(int width, int height) {
	if (_context)
		return _width == width && _height == height;
	_context = TinyGL::createContext(width, height, g_system->getScreenFormat(), 256, false, false);
	if (!_context)
		return false;
	_width = width;
	_height = height;
	TinyGL::setContext(_context);
	tglViewport(0, 0, width, height);
	tglClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	tglEnable(TGL_DEPTH_TEST);
	tglDepthFunc(TGL_LESS);
	tglDisable(TGL_LIGHTING);
	tglDisable(TGL_TEXTURE_2D);
	tglDisable(TGL_CULL_FACE);
	return true;
}

void RA2PSXTinyGLRenderer::beginFrame(const Graphics::Surface &background) {
	if (!_context)
		return;
	TinyGL::setContext(_context);

	Graphics::Surface surface;
	TinyGL::getSurfaceRef(surface);
	surface.fillRect(Common::Rect(surface.w, surface.h), 0);
	const int width = MIN<int>(background.w, surface.w);
	const int height = MIN<int>(background.h, surface.h);
	const int sourceX = (background.w - width) / 2;
	const int sourceY = (background.h - height) / 2;
	const int destX = (surface.w - width) / 2;
	const int destY = (surface.h - height) / 2;
	surface.copyRectToSurface(background, destX, destY,
			Common::Rect(sourceX, sourceY, sourceX + width, sourceY + height));
	tglClear(TGL_DEPTH_BUFFER_BIT);

	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	tglOrthof(0.0f, (float)_width, (float)_height, 0.0f, -1024.0f, 1024.0f);
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();
}

void RA2PSXTinyGLRenderer::renderModel(const RA2PSXModel &model, float x, float y, float size,
		float pitch, float yaw, float roll, bool depthTest) {
	if (!_context || model.vertices().empty())
		return;
	TinyGL::setContext(_context);
	if (!depthTest)
		tglDisable(TGL_DEPTH_TEST);
	tglMatrixMode(TGL_MODELVIEW);
	tglPushMatrix();
	tglTranslatef(x, y, 0.0f);
	tglRotatef(roll, 0.0f, 0.0f, 1.0f);
	tglRotatef(pitch, 1.0f, 0.0f, 0.0f);
	tglRotatef(yaw, 0.0f, 1.0f, 0.0f);
	const float scale = size / model.radius();
	tglScalef(scale, -scale, scale);

	const Common::Array<RA2PSXVertex> &vertices = model.vertices();
	const Common::Array<RA2PSXFace> &faces = model.faces();
	for (uint faceIndex = 0; faceIndex < faces.size(); ++faceIndex) {
		const RA2PSXFace &face = faces[faceIndex];
		tglColor3ub(face.r, face.g, face.b);
		tglBegin(face.vertexCount == 4 ? TGL_QUADS : TGL_TRIANGLES);
		for (uint vertexIndex = 0; vertexIndex < face.vertexCount; ++vertexIndex) {
			const RA2PSXVertex &vertex = vertices[face.vertex[vertexIndex]];
			tglVertex3f((float)vertex.x, (float)vertex.y, (float)vertex.z);
		}
		tglEnd();
	}
	tglPopMatrix();
	if (!depthTest)
		tglEnable(TGL_DEPTH_TEST);
}

void RA2PSXTinyGLRenderer::finishFrame(Graphics::Surface &surface) {
	if (!_context)
		return;
	TinyGL::setContext(_context);
	TinyGL::presentBuffer();
	TinyGL::getSurfaceRef(surface);
}

#endif

} // End of namespace Scumm
