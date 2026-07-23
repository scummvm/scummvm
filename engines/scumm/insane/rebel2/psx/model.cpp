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
#include "common/util.h"

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

bool loadRA2PSXTextures(const Common::Array<byte> &data,
		Common::Array<RA2PSXTexture> &textures) {
	const uint initialCount = textures.size();
	uint32 offset = 0;
	while (offset + 20 <= data.size()) {
		Common::String name;
		for (uint i = 0; i < 8 && data[offset + i]; ++i) {
			if (data[offset + i] < 0x20 || data[offset + i] >= 0x7f)
				return textures.size() > initialCount;
			name += (char)data[offset + i];
		}
		if (name.empty())
			break;

		const uint16 recordSize = READ_LE_UINT16(data.data() + offset + 12);
		const uint16 widthField = READ_LE_UINT16(data.data() + offset + 16);
		const uint16 heightField = READ_LE_UINT16(data.data() + offset + 18);
		const uint16 width = (widthField & 0xff) ? widthField & 0xff : 256;
		const uint16 height = (heightField & 0xff) ? heightField & 0xff : 256;
		const bool eightBit = (widthField & 0x100) != 0;
		const uint32 paletteColors = eightBit ? 256 : 16;
		const uint32 pixelCount = (uint32)width * height;
		const uint32 pixelBytes = eightBit ? pixelCount : (pixelCount + 1) / 2;
		const uint32 paletteOffset = offset + 20;
		const uint32 pixelsOffset = paletteOffset + paletteColors * 2;
		if (recordSize < 20 || offset + recordSize > data.size() ||
				pixelsOffset + pixelBytes > offset + recordSize)
			break;

		RA2PSXTexture texture;
		texture.name = name;
		texture.width = width;
		texture.height = height;
		texture.pixels.resize(pixelCount);
		for (uint32 i = 0; i < pixelCount; ++i) {
			const byte packed = data[pixelsOffset + (eightBit ? i : i / 2)];
			const byte paletteIndex = eightBit ? packed : ((i & 1) ? packed >> 4 : packed & 0xf);
			const uint16 value = READ_LE_UINT16(data.data() + paletteOffset + paletteIndex * 2);
			if (!value) {
				texture.pixels[i] = 0;
				continue;
			}

			const uint32 r = ((value & 0x1f) << 3) | ((value & 0x1f) >> 2);
			const uint32 g = (((value >> 5) & 0x1f) << 3) | (((value >> 5) & 0x1f) >> 2);
			const uint32 b = (((value >> 10) & 0x1f) << 3) | (((value >> 10) & 0x1f) >> 2);
			texture.pixels[i] = 0x01000000 | ((value & 0x8000) ? 0x02000000 : 0) |
					(r << 16) | (g << 8) | b;
		}
		textures.push_back(texture);
		offset += recordSize;
	}
	return textures.size() > initialCount;
}

RA2PSXModel::RA2PSXModel() : _radius(1.0f) {
}

bool RA2PSXModel::load(const Common::Array<byte> &data) {
	_vertices.clear();
	_faces.clear();
	_textures.clear();
	_radius = 1.0f;

	uint32 textureOffset;
	if (!readU32(data, 16, textureOffset) || textureOffset + 4 > data.size())
		return false;
	const uint32 textureCount = READ_LE_UINT32(data.data() + textureOffset);
	if (textureCount > 256 || textureCount > (data.size() - textureOffset - 4) / 8)
		return false;
	for (uint32 i = 0; i < textureCount; ++i) {
		const uint32 nameOffset = textureOffset + 4 + i * 8;
		RA2PSXTexture texture;
		for (uint j = 0; j < 8 && data[nameOffset + j]; ++j) {
			if (data[nameOffset + j] < 0x20 || data[nameOffset + j] >= 0x7f)
				return false;
			texture.name += (char)data[nameOffset + j];
		}
		if (texture.name.empty())
			return false;
		texture.width = 0;
		texture.height = 0;
		_textures.push_back(texture);
	}

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

bool RA2PSXModel::loadTextures(const Common::Array<byte> &data) {
	Common::Array<RA2PSXTexture> decoded;
	if (!loadRA2PSXTextures(data, decoded))
		return false;

	for (uint i = 0; i < _textures.size(); ++i) {
		for (uint j = 0; j < decoded.size(); ++j) {
			if (_textures[i].name.equalsIgnoreCase(decoded[j].name)) {
				_textures[i] = decoded[j];
				break;
			}
		}
		if (_textures[i].pixels.empty())
			return false;
	}
	return true;
}

const RA2PSXTexture *RA2PSXModel::texture(int index) const {
	if (index < 0 || (uint)index >= _textures.size() || !_textures[index].width ||
			!_textures[index].height || _textures[index].pixels.empty())
		return nullptr;
	return &_textures[index];
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
	const uint32 normalOffset = vertexOffset + (uint32)vertexCount * 8;
	if (normalOffset > primitiveOffset)
		return false;

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
			const byte mode = data[record + 12];
			RA2PSXFace face = {};
			face.mode = mode;
			face.texture = -1;
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
			if (!(mode & 0x44)) {
				const uint16 normalOffsets[4] = {
					READ_LE_UINT16(data.data() + record + 6),
					READ_LE_UINT16(data.data() + record + 8),
					READ_LE_UINT16(data.data() + record + 10),
					READ_LE_UINT16(data.data() + record + 26)
				};
				for (uint vertexIndex = 0; vertexIndex < face.vertexCount; ++vertexIndex) {
					const uint normalIndex = (mode & 1) ? vertexIndex : 0;
					const uint16 sourceOffset = normalOffsets[normalIndex];
					if ((sourceOffset & 7) || normalOffset + sourceOffset + 8 > primitiveOffset)
						return false;
					const byte *source = data.data() + normalOffset + sourceOffset;
					face.normalX[vertexIndex] = (int16)READ_LE_UINT16(source);
					face.normalY[vertexIndex] = (int16)READ_LE_UINT16(source + 2);
					face.normalZ[vertexIndex] = (int16)READ_LE_UINT16(source + 4);
				}
			}
			static const byte uvOffsets[4] = { 14, 20, 22, 28 };
			for (uint vertexIndex = 0; vertexIndex < face.vertexCount; ++vertexIndex) {
				face.u[vertexIndex] = data[record + uvOffsets[vertexIndex]];
				face.v[vertexIndex] = data[record + uvOffsets[vertexIndex] + 1];
				face.r[vertexIndex] = data[record + 16];
				face.g[vertexIndex] = data[record + 17];
				face.b[vertexIndex] = data[record + 18];
			}
			if (mode & 0x40) {
				face.r[1] = data[record + 6];
				face.g[1] = data[record + 7];
				face.b[1] = data[record + 8];
				face.r[2] = data[record + 9];
				face.g[2] = data[record + 10];
				face.b[2] = data[record + 11];
				if (face.vertexCount == 4) {
					face.r[3] = data[record + 26];
					face.g[3] = data[record + 27];
					face.b[3] = data[record + 28];
				}
			}
			if ((mode & 2) && data[record + 19] >= 0x80) {
				const uint texture = data[record + 19] - 0x80;
				if (texture >= _textures.size())
					return false;
				face.texture = texture;
			}
			_faces.push_back(face);
		}
		if (!terminated)
			return false;
	}
	return true;
}

#ifdef USE_TINYGL

RA2PSXTinyGLRenderer::RA2PSXTinyGLRenderer() : _context(nullptr), _activeTexture(nullptr),
		_textureEnabled(false), _blendEnabled(false), _width(0), _height(0) {
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
	tglDisable(TGL_ALPHA_TEST);
	tglDisable(TGL_BLEND);
	tglDisable(TGL_CULL_FACE);
	tglAlphaFunc(TGL_GREATER, 0.0f);
	tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);
	tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_MODULATE);
	return true;
}

TGLuint RA2PSXTinyGLRenderer::getTextureId(const RA2PSXTexture &texture) {
	for (uint i = 0; i < _textureBindings.size(); ++i) {
		if (_textureBindings[i].texture == &texture)
			return _textureBindings[i].id;
	}

	Common::Array<byte> rgba;
	rgba.resize(texture.pixels.size() * 4);
	for (uint i = 0; i < texture.pixels.size(); ++i) {
		const uint32 pixel = texture.pixels[i];
		rgba[i * 4] = (pixel >> 16) & 0xff;
		rgba[i * 4 + 1] = (pixel >> 8) & 0xff;
		rgba[i * 4 + 2] = pixel & 0xff;
		rgba[i * 4 + 3] = !(pixel & 0x01000000) ? 0 :
				(pixel & 0x02000000) ? 0x80 : 0xff;
	}

	TextureBinding binding;
	binding.texture = &texture;
	tglGenTextures(1, &binding.id);
	tglBindTexture(TGL_TEXTURE_2D, binding.id);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_NEAREST);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_NEAREST);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_S, TGL_CLAMP_TO_EDGE);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_T, TGL_CLAMP_TO_EDGE);
	tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGBA, texture.width, texture.height, 0,
			TGL_RGBA, TGL_UNSIGNED_BYTE, rgba.data());
	_textureBindings.push_back(binding);
	return binding.id;
}

void RA2PSXTinyGLRenderer::setFaceState(const RA2PSXModel &model, const RA2PSXFace &face) {
	const RA2PSXTexture *texture = model.texture(face.texture);
	if (texture) {
		if (!_textureEnabled) {
			tglEnable(TGL_TEXTURE_2D);
			tglEnable(TGL_ALPHA_TEST);
			_textureEnabled = true;
		}
		if (_activeTexture != texture) {
			tglBindTexture(TGL_TEXTURE_2D, getTextureId(*texture));
			_activeTexture = texture;
		}
	} else if (_textureEnabled) {
		tglDisable(TGL_TEXTURE_2D);
		tglDisable(TGL_ALPHA_TEST);
		_textureEnabled = false;
	}

	const bool blend = (face.mode & 8) != 0;
	if (blend != _blendEnabled) {
		if (blend)
			tglEnable(TGL_BLEND);
		else
			tglDisable(TGL_BLEND);
		_blendEnabled = blend;
	}
}

static int getRA2PSXDepthCue(float depth, float focalLength) {
	if (depth <= 0.0f)
		return 4096;
	const int quotient = MIN(0x1ffff, (int)(focalLength * 65536.0f / depth));
	return CLIP<int>((0x04440000LL - 28416LL * quotient) >> 12, 0, 4096);
}

void RA2PSXTinyGLRenderer::setFaceColor(const RA2PSXFace &face, uint vertexIndex,
		float normalX, float normalY, float normalZ, float depth) {
	int r = face.r[vertexIndex];
	int g = face.g[vertexIndex];
	int b = face.b[vertexIndex];
	if (face.texture >= 0 && (face.mode & 4)) {
		r = g = b = 0xff;
	} else if (!(face.mode & 0x40)) {
		static const int lightMatrix[3][3] = {
			{ -3640, 1820, 455 },
			{ 0, -4096, 0 },
			{ -2816, 0, 1024 }
		};
		static const int colorMatrix[3][3] = {
			{ 1648, 128, 2000 },
			{ 1648, 128, 2000 },
			{ 1648, 1024, 2000 }
		};
		const float normal[3] = { normalX, normalY, normalZ };
		int light[3];
		for (uint i = 0; i < 3; ++i) {
			float value = 0.0f;
			for (uint j = 0; j < 3; ++j)
				value += lightMatrix[i][j] * normal[j];
			light[i] = CLIP<int>((int)(value / 4096.0f), 0, 0x7fff);
		}

		int effect[3];
		for (uint i = 0; i < 3; ++i) {
			int value = 0;
			for (uint j = 0; j < 3; ++j)
				value += colorMatrix[i][j] * light[j];
			effect[i] = 110 + MAX(0, value / 4096) / 16;
		}
		const int depthScale = 4096 - getRA2PSXDepthCue(depth, _width * 2.0f);
		r = MIN(255, r * effect[0] / 256) * depthScale / 4096;
		g = MIN(255, g * effect[1] / 256) * depthScale / 4096;
		b = MIN(255, b * effect[2] / 256) * depthScale / 4096;
	}
	if (face.texture >= 0 && !(face.mode & 4)) {
		r = MIN(r * 2, 0xff);
		g = MIN(g * 2, 0xff);
		b = MIN(b * 2, 0xff);
	}
	tglColor4ub(r, g, b, (face.texture < 0 && (face.mode & 8)) ? 0x80 : 0xff);
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
	const float pitchAngle = pitch * 0.017453292519943295f;
	const float yawAngle = yaw * 0.017453292519943295f;
	const float rollAngle = roll * 0.017453292519943295f;
	const float pitchCosine = cosf(pitchAngle);
	const float pitchSine = sinf(pitchAngle);
	const float yawCosine = cosf(yawAngle);
	const float yawSine = sinf(yawAngle);
	const float rollCosine = cosf(rollAngle);
	const float rollSine = sinf(rollAngle);
	const float depth = model.radius() * (_width * 2.0f) / size;

	const Common::Array<RA2PSXVertex> &vertices = model.vertices();
	struct FacingVertex {
		float x;
		float y;
	};
	Common::Array<FacingVertex> facingVertices;
	facingVertices.resize(vertices.size());
	for (uint i = 0; i < vertices.size(); ++i) {
		const float modelX = vertices[i].x;
		const float modelY = -vertices[i].y;
		const float modelZ = vertices[i].z;
		const float yawX = yawCosine * modelX + yawSine * modelZ;
		const float yawZ = -yawSine * modelX + yawCosine * modelZ;
		const float pitchY = pitchCosine * modelY - pitchSine * yawZ;
		facingVertices[i].x = rollCosine * yawX - rollSine * pitchY;
		facingVertices[i].y = rollSine * yawX + rollCosine * pitchY;
	}

	const Common::Array<RA2PSXFace> &faces = model.faces();
	for (uint faceIndex = 0; faceIndex < faces.size(); ++faceIndex) {
		const RA2PSXFace &face = faces[faceIndex];
		if (!(face.mode & 0x60)) {
			const FacingVertex &v0 = facingVertices[face.vertex[0]];
			const FacingVertex &v1 = facingVertices[face.vertex[1]];
			const FacingVertex &v2 = facingVertices[face.vertex[2]];
			if (v0.x * (v1.y - v2.y) + v1.x * (v2.y - v0.y) +
					v2.x * (v0.y - v1.y) <= 0.0f)
				continue;
		}
		const RA2PSXTexture *texture = model.texture(face.texture);
		setFaceState(model, face);
		tglBegin(face.vertexCount == 4 ? TGL_QUADS : TGL_TRIANGLES);
		for (uint vertexIndex = 0; vertexIndex < face.vertexCount; ++vertexIndex) {
			const float normalX = face.normalX[vertexIndex];
			const float normalY = -face.normalY[vertexIndex];
			const float normalZ = face.normalZ[vertexIndex];
			const float yawX = yawCosine * normalX + yawSine * normalZ;
			const float yawZ = -yawSine * normalX + yawCosine * normalZ;
			const float pitchY = pitchCosine * normalY - pitchSine * yawZ;
			const float pitchZ = pitchSine * normalY + pitchCosine * yawZ;
			setFaceColor(face, vertexIndex,
					rollCosine * yawX - rollSine * pitchY,
					rollSine * yawX + rollCosine * pitchY, pitchZ, depth);
			if (texture)
				tglTexCoord2f((face.u[vertexIndex] + 0.5f) / texture->width,
						(face.v[vertexIndex] + 0.5f) / texture->height);
			const RA2PSXVertex &vertex = vertices[face.vertex[vertexIndex]];
			tglVertex3f((float)vertex.x, (float)vertex.y, (float)vertex.z);
		}
		tglEnd();
	}
	tglPopMatrix();
	if (!depthTest)
		tglEnable(TGL_DEPTH_TEST);
}

void RA2PSXTinyGLRenderer::renderPerspectiveModel(const RA2PSXModel &model,
		float x, float y, float z, float directionX, float directionY, float directionZ,
		float roll, bool depthTest) {
	if (!_context || model.vertices().empty())
		return;

	const float directionLength = sqrtf(directionX * directionX + directionY * directionY +
			directionZ * directionZ);
	if (directionLength < 0.001f)
		return;
	const float forwardX = directionX / directionLength;
	const float forwardY = directionY / directionLength;
	const float forwardZ = directionZ / directionLength;

	float rightX = forwardZ;
	float rightY = 0.0f;
	float rightZ = -forwardX;
	const float rightLength = sqrtf(rightX * rightX + rightZ * rightZ);
	if (rightLength < 0.001f) {
		rightX = 1.0f;
		rightZ = 0.0f;
	} else {
		rightX /= rightLength;
		rightZ /= rightLength;
	}
	const float downX = forwardY * rightZ - forwardZ * rightY;
	const float downY = forwardZ * rightX - forwardX * rightZ;
	const float downZ = forwardX * rightY - forwardY * rightX;
	const float angle = roll * 0.017453292519943295f;
	const float cosine = cosf(angle);
	const float sine = sinf(angle);
	const float modelXx = rightX * cosine + downX * sine;
	const float modelXy = rightY * cosine + downY * sine;
	const float modelXz = rightZ * cosine + downZ * sine;
	const float modelYx = downX * cosine - rightX * sine;
	const float modelYy = downY * cosine - rightY * sine;
	const float modelYz = downZ * cosine - rightZ * sine;

	struct ProjectedVertex {
		float x;
		float y;
		float z;
		bool visible;
	};
	Common::Array<ProjectedVertex> projected;
	projected.resize(model.vertices().size());
	const float centerX = _width * 0.5f;
	const float centerY = _height * 0.5f;
	const float focalLength = _width * 2.0f;
	for (uint i = 0; i < model.vertices().size(); ++i) {
		const RA2PSXVertex &vertex = model.vertices()[i];
		const float worldX = x + modelXx * vertex.x + modelYx * vertex.y + forwardX * vertex.z;
		const float worldY = y + modelXy * vertex.x + modelYy * vertex.y + forwardY * vertex.z;
		const float worldZ = z + modelXz * vertex.x + modelYz * vertex.y + forwardZ * vertex.z;
		projected[i].z = worldZ;
		projected[i].visible = worldZ > 1.0f;
		if (projected[i].visible) {
			projected[i].x = centerX + worldX * focalLength / worldZ;
			projected[i].y = centerY + worldY * focalLength / worldZ;
		}
	}

	TinyGL::setContext(_context);
	if (!depthTest)
		tglDisable(TGL_DEPTH_TEST);
	const Common::Array<RA2PSXFace> &faces = model.faces();
	for (uint faceIndex = 0; faceIndex < faces.size(); ++faceIndex) {
		const RA2PSXFace &face = faces[faceIndex];
		bool visible = true;
		for (uint vertexIndex = 0; vertexIndex < face.vertexCount; ++vertexIndex)
			visible &= projected[face.vertex[vertexIndex]].visible;
		if (!visible)
			continue;
		if (!(face.mode & 0x60)) {
			const ProjectedVertex &v0 = projected[face.vertex[0]];
			const ProjectedVertex &v1 = projected[face.vertex[1]];
			const ProjectedVertex &v2 = projected[face.vertex[2]];
			if (v0.x * (v1.y - v2.y) + v1.x * (v2.y - v0.y) +
					v2.x * (v0.y - v1.y) <= 0.0f)
				continue;
		}

		const RA2PSXTexture *texture = model.texture(face.texture);
		setFaceState(model, face);
		const float faceDepth = projected[face.vertex[2]].z;
		tglBegin(face.vertexCount == 4 ? TGL_QUADS : TGL_TRIANGLES);
		for (uint vertexIndex = 0; vertexIndex < face.vertexCount; ++vertexIndex) {
			setFaceColor(face, vertexIndex,
					modelXx * face.normalX[vertexIndex] +
							modelYx * face.normalY[vertexIndex] +
							forwardX * face.normalZ[vertexIndex],
					modelXy * face.normalX[vertexIndex] +
							modelYy * face.normalY[vertexIndex] +
							forwardY * face.normalZ[vertexIndex],
					modelXz * face.normalX[vertexIndex] +
							modelYz * face.normalY[vertexIndex] +
							forwardZ * face.normalZ[vertexIndex], faceDepth);
			if (texture)
				tglTexCoord2f((face.u[vertexIndex] + 0.5f) / texture->width,
						(face.v[vertexIndex] + 0.5f) / texture->height);
			const ProjectedVertex &vertex = projected[face.vertex[vertexIndex]];
			tglVertex3f(vertex.x, vertex.y, 0.0f);
		}
		tglEnd();
	}
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
