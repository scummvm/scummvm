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

#include "bladerunner/slice_renderer.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/lights.h"
#include "bladerunner/set_effects.h"
#include "bladerunner/slice_animations.h"

#include "common/debug.h"
#include "common/memstream.h"
#include "common/rect.h"
#include "common/util.h"

namespace BladeRunner {

#if 0
void dump(const char *str, Vector3 v) {
	debug("%s %g %g %g", str, v.x, v.y, v.z);
}

void dump(const char *str, Matrix3x2 m) {
	debug("%s", str);
	debug("%11.6g %11.6g %11.6g", m(0, 0), m(0, 1), m(0, 2));
	debug("%11.6g %11.6g %11.6g", m(1, 0), m(1, 1), m(1, 2));
}

void dump(const char *str, Matrix4x3 m) {
	debug("%s", str);
	debug("%11.6g %11.6g %11.6g %11.6g", m(0, 0), m(0, 1), m(0, 2), m(0, 3));
	debug("%11.6g %11.6g %11.6g %11.6g", m(1, 0), m(1, 1), m(1, 2), m(1, 3));
	debug("%11.6g %11.6g %11.6g %11.6g", m(2, 0), m(2, 1), m(2, 2), m(2, 3));
}
#endif

SliceRenderer::SliceRenderer(BladeRunnerEngine* vm) {
	_vm = vm;
	int i;

	for (i = 0; i < 942; i++) { // yes, its going just to 942 and not 997
		_animationsShadowEnabled[i] = true;
	}
}

SliceRenderer::~SliceRenderer() {
}

void SliceRenderer::setView(const View &view) {
	_view = view;
}

void SliceRenderer::setLights(Lights* lights){
	_lights = lights;
}

void SliceRenderer::setSetEffects(SetEffects* setEffects){
	_setEffects = setEffects;
}

void SliceRenderer::setupFrame(int animation, int frame, Vector3 position, float facing, float scale) {
	_animation = animation;
	_frame = frame;
	_position = position;
	_facing = facing;
	_scale = scale;

	_sliceFramePtr = _vm->_sliceAnimations->getFramePtr(_animation, _frame);

	Common::MemoryReadStream stream((byte*)_sliceFramePtr, _vm->_sliceAnimations->_animations[_animation].frameSize);

	_frameFront.x      = stream.readFloatLE();
	_frameFront.y      = stream.readFloatLE();
	_frameSliceHeight  = stream.readFloatLE();
	_framePos.x        = stream.readFloatLE();
	_framePos.y        = stream.readFloatLE();
	_frameBottomZ      = stream.readFloatLE();
	_framePaletteIndex = stream.readUint32LE();
	_frameSliceCount   = stream.readUint32LE();

	calculateBoundingRect();
}

Matrix3x2 SliceRenderer::calculateFacingRotationMatrix() {
	assert(_sliceFramePtr);

	Matrix4x3 viewMatrix = _view._sliceViewMatrix;
	Vector3 viewPos = viewMatrix * _position;
	float dir = atan2f(viewPos.x, viewPos.z) + _facing;
	float s = sinf(dir);
	float c = cosf(dir);

	Matrix3x2 rotation( c, -s, 0.0f,
	                    s,  c, 0.0f);

	Matrix3x2 viewRotation(viewMatrix(0,0), viewMatrix(0,1), 0.0f,
	                       viewMatrix(2,0), viewMatrix(2,1), 0.0f);

	return viewRotation * rotation;
}

void SliceRenderer::calculateBoundingRect() {
	assert(_sliceFramePtr);

	_minX = 0.0f;
	_maxX = 0.0f;
	_minY = 0.0f;
	_maxY = 0.0f;

	Matrix4x3 viewMatrix = _view._sliceViewMatrix;

	Vector3 frameBottom = Vector3(0.0f, 0.0f, _frameBottomZ);
	Vector3 frameTop    = Vector3(0.0f, 0.0f, _frameBottomZ + _frameSliceCount * _frameSliceHeight);

	Vector3 bottom = viewMatrix * (_position + frameBottom);
	Vector3 top    = viewMatrix * (_position + frameTop);

	top = bottom + _scale * (top - bottom);

	if (bottom.z < 0.0f || top.z < 0.0f)
		return;

	Matrix3x2 facingRotation = calculateFacingRotationMatrix();

	Matrix3x2 m4(_view._viewportDistance / bottom.z,  0.0f, 0.0f,
	                                           0.0f, 25.5f, 0.0f);

	Matrix3x2 m2(_frameFront.x,          0.0f, _framePos.x,
	                      0.0f, _frameFront.y, _framePos.y);

	_field_109E = m4 * (facingRotation * m2);

	Vector4 B6(_view._viewportHalfWidth   + top.x / top.z * _view._viewportDistance,
	           _view._viewportHalfHeight  + top.y / top.z * _view._viewportDistance,
	           1.0f / top.z,
	           _frameSliceCount * (1.0f / top.z));

	Vector4 C2(_view._viewportHalfWidth   + bottom.x / bottom.z * _view._viewportDistance,
	           _view._viewportHalfHeight  + bottom.y / bottom.z * _view._viewportDistance,
	           1.0f / bottom.z,
	           0.0f);

	_field_10B6.x = B6.x;
	_field_10B6.y = B6.y;
	_field_10B6.z = B6.z;
	_field_10C2.x = C2.x;
	_field_10C2.y = C2.y;
	_field_10C2.z = C2.z;
	_field_10CE   = B6.w;
	_field_10D2   = C2.w;

	Vector4 delta = C2 - B6;

	if (delta.y == 0.0f)
		return;

	/*
	 * Calculate min and max Y
	 */

	float screenMinY =   0.0f;
	float screenMaxY = 479.0f;

	if (B6.y < screenMinY) {
		if (C2.y < screenMinY)
			return;

		float f = (screenMinY - B6.y) / delta.y;
		B6 = B6 + f * delta;
	} else if (B6.y > screenMaxY) {
		if (C2.y >= screenMaxY)
			return;

		float f = (screenMaxY - B6.y) / delta.y;
		B6 = B6 + f * delta;
	}

	if (C2.y < screenMinY) {
		float f = (screenMinY - C2.y) / delta.y;
		C2 = C2 + f * delta;
	} else if (C2.y > screenMaxY) {
		float f = (screenMaxY - C2.y) / delta.y;
		C2 = C2 + f * delta;
	}

	int bbox_min_y = (int)MIN(B6.y, C2.y);
	int bbox_max_y = (int)MAX(B6.y, C2.y) + 1;

	/*
	 * Calculate min and max X
	 */

	Matrix3x2 mB6 = _field_109E + Vector2(B6.x, 25.5f / B6.z);
	Matrix3x2 mC2 = _field_109E + Vector2(C2.x, 25.5f / C2.z);

	float min_x =  640.0f;
	float max_x =    0.0f;

	for (float i = 0.0f; i <= 256.0f; i += 255.0f)
	{
		for (float j = 0.0f; j <= 256.0f; j += 255.0f)
		{
			Vector2 v1 = mB6 * Vector2(i, j);

			min_x = MIN(min_x, v1.x);
			max_x = MAX(max_x, v1.x);

			Vector2 v2 = mC2 * Vector2(i, j);

			min_x = MIN(min_x, v2.x);
			max_x = MAX(max_x, v2.x);
		}
	}

	int bbox_min_x = MIN(MAX((int)min_x,     0), 640);
	int bbox_max_x = MIN(MAX((int)max_x + 1, 0), 640);

	_field_10B6.x = B6.x;
	_field_10B6.y = B6.y;
	_field_10B6.z = B6.z;
	_field_10C2.x = C2.x;
	_field_10C2.y = C2.y;
	_field_10C2.z = C2.z;
	_field_10CE   = B6.w;
	_field_10D2   = C2.w;

	_minX = bbox_min_x;
	_minY = bbox_min_y;
	_maxX = bbox_max_x;
	_maxY = bbox_max_y;
}

struct SliceLineIterator {
	int _field_00[2][3];
	int _field_18;
	int _field_1C;

	float _field_20;
	float _field_24;
	float _field_28;
	float _field_2C;
	float _field_30;
	float _field_34;
	int   _field_38;
	int   _field_3C;

	void setup(float arg_1,  float arg_2,  float arg_3,
	           float arg_4,  float arg_5,  float arg_6,
	           float arg_7,  float arg_8,
	           Matrix3x2 arg_9);
	float line();
	void advance();
};

void SliceLineIterator::setup(
	float arg_1,  float arg_2,  float arg_3,
	float arg_4,  float arg_5,  float arg_6,
	float arg_7,  float arg_8,
	Matrix3x2 arg_9)
{
	_field_18 = (int)arg_5;
	_field_1C = (int)arg_2;

	float var_3 = arg_2 - arg_5;

	if (var_3 <= 0.0f || arg_6 <= 0.0f)
		_field_3C = _field_1C + 1;

	_field_20 = arg_6;
	_field_24 = (arg_3 - arg_6) / var_3;
	_field_2C = (arg_7 - arg_8) / var_3;
	_field_28 = arg_8 - (arg_5 - floor(arg_5) - 1.0f) * _field_2C;

	_field_30 = arg_4;
	_field_34 = (arg_1 - arg_4) / var_3;
	_field_38 = (int)((25.5f / var_3) * (1.0f / arg_3 - 1.0f / arg_6) * 64.0);
	_field_3C = _field_18;

	float var_54 =         _field_30;
	float var_55 = 25.5f / _field_20;

	Matrix3x2 translate_matrix = Matrix3x2(1.0f, 0.0f, var_54,
	                                       0.0f, 1.0f, var_55);

	Matrix3x2 scale_matrix = Matrix3x2(65536.0f,  0.0f, 0.0f,
	                                       0.0f, 64.0f, 0.0f);

	arg_9 = scale_matrix * (translate_matrix * arg_9);

	for (int r = 0; r != 2; ++r)
		for (int c = 0; c != 3; ++c)
			_field_00[r][c] = arg_9(r, c);
}

float SliceLineIterator::line() {
	float var_0 = 0.0f;

	if (_field_20 != 0.0f)
		var_0 = _field_28 / _field_20;

	if (var_0 < 0.0)
		var_0 = 0.0f;

	return var_0;
}

void SliceLineIterator::advance() {
	_field_20 += _field_24;
	_field_28 += _field_2C;
	_field_30 += _field_34;
	_field_3C += 1;
	_field_00[0][2] += (int)(65536.0f * _field_34);
	_field_00[1][2] += _field_38;
}

static
void setupLookupTable(int t[256], int inc) {
	int v = 0;
	for (int i = 0; i != 256; ++i) {
		t[i] = v;
		v += inc;
	}
}

void SliceRenderer::drawFrame(Graphics::Surface &surface, uint16 *zbuffer) {
	assert(_sliceFramePtr);

	SliceLineIterator sliceLineIterator;
	sliceLineIterator.setup(
		_field_10C2.x, _field_10C2.y, _field_10C2.z,
		_field_10B6.x, _field_10B6.y, _field_10B6.z,
		_field_10D2, _field_10CE, // 2 floats
		_field_109E              // 3x2 matrix
		);

	setupLookupTable(_t1, sliceLineIterator._field_00[0][0]);
	setupLookupTable(_t2, sliceLineIterator._field_00[0][1]);
	setupLookupTable(_t4, sliceLineIterator._field_00[1][0]);
	setupLookupTable(_t5, sliceLineIterator._field_00[1][1]);

	int frameY = sliceLineIterator._field_18;

	uint16 *frameLinePtr  = (uint16*)surface.getPixels() + 640 * frameY;
	uint16 *zBufferLinePtr = zbuffer + 640 * frameY;

	while (sliceLineIterator._field_3C <= sliceLineIterator._field_1C) {
		_c3 = sliceLineIterator._field_00[0][2];
		_c6 = sliceLineIterator._field_00[1][2];

		float sliceLine = sliceLineIterator.line();

		if (frameY >= 0 && frameY < 480)
			drawSlice((int)sliceLine, frameLinePtr, zBufferLinePtr);

		sliceLineIterator.advance();
		frameY += 1;
		frameLinePtr += 640;
		zBufferLinePtr += 640;
	}
}

void SliceRenderer::drawSlice(int slice, uint16 *frameLinePtr, uint16 *zbufLinePtr) {
	if (slice < 0 || (uint32)slice >= _frameSliceCount)
		return;

	SlicePalette &palette = _vm->_sliceAnimations->getPalette(_framePaletteIndex);

	byte *p = (byte*)_sliceFramePtr + 0x20 + 4 * slice;

	uint32 polyOffset = READ_LE_UINT32(p);

	p = (byte*)_sliceFramePtr + polyOffset;

	uint32 polyCount = READ_LE_UINT32(p);
	p += 4;
	while (polyCount--) {
		uint32 vertexCount = READ_LE_UINT32(p);
		p += 4;

		if (vertexCount == 0)
			continue;

		uint32 lastVertex = vertexCount - 1;
		int lastVertexX = MAX((_t1[p[3*lastVertex]] + _t2[p[3*lastVertex+1]] + _c3) >> 16, 0);

		int previousVertexX = lastVertexX;

		while (vertexCount--) {
			int vertexX = CLIP((_t1[p[0]] + _t2[p[1]] + _c3) >> 16, 0, 640);

			if (vertexX > previousVertexX) {
				int vertexZ = (_t4[p[0]] + _t5[p[1]] + _c6) >> 6;

				if (vertexZ >= 0 && vertexZ < 65536) {
					for (int x = previousVertexX; x != vertexX; ++x) {
						// debug("\t%04x < %04x", vertexZ, zbufLinePtr[x]);
						if (vertexZ < zbufLinePtr[x]) {
							frameLinePtr[x] = palette[p[2]];
							zbufLinePtr[x] = (uint16)vertexZ;
						}
					}
				}
			}
			p += 3;
			previousVertexX = vertexX;
		}
	}
}

void SliceRenderer::preload(int animationId) {
	int i;
	int frameCount = _vm->_sliceAnimations->getFrameCount(animationId);
	for (i = 0; i < frameCount; i++)
		_vm->_sliceAnimations->getFramePtr(animationId, i);
}

void SliceRenderer::disableShadows(int* animationsIdsList, int listSize) {
	int i;
	for (i = 0; i < listSize; i++) {
		_animationsShadowEnabled[animationsIdsList[i]] = false;
	}
}

} // End of namespace BladeRunner
