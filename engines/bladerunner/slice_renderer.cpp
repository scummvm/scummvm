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

#include "common/memstream.h"
#include "common/rect.h"
#include "common/util.h"

namespace BladeRunner {

SliceRenderer::SliceRenderer(BladeRunnerEngine *vm) {
	_vm = vm;
	_pixelFormat = createRGB555();
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

void SliceRenderer::setLights(Lights *lights) {
	_lights = lights;
}

void SliceRenderer::setSetEffects(SetEffects *setEffects) {
	_setEffects = setEffects;
}

void SliceRenderer::setupFrameInWorld(int animationId, int animationFrame, Vector3 position, float facing, float scale) {
	_position = position;
	_facing = facing;
	_scale = scale;

	loadFrame(animationId, animationFrame);

	calculateBoundingRect();
}

void SliceRenderer::getScreenRectangle(Common::Rect *screenRectangle, int animationId, int animationFrame, Vector3 position, float facing, float scale) {
	assert(screenRectangle);
	setupFrameInWorld(animationId, animationFrame, position, facing, scale);
	*screenRectangle = _screenRectangle;
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

	_screenRectangle.left   = 0;
	_screenRectangle.right  = 0;
	_screenRectangle.top    = 0;
	_screenRectangle.bottom = 0;

	Matrix4x3 viewMatrix = _view._sliceViewMatrix;

	Vector3 frameBottom = Vector3(0.0f, 0.0f, _frameBottomZ);
	Vector3 frameTop    = Vector3(0.0f, 0.0f, _frameBottomZ + _frameSliceCount * _frameSliceHeight);

	Vector3 bottom = viewMatrix * (_position + frameBottom);
	Vector3 top    = viewMatrix * (_position + frameTop);

	top = bottom + _scale * (top - bottom);

	if (bottom.z < 0.0f || top.z < 0.0f)
		return;

	Matrix3x2 facingRotation = calculateFacingRotationMatrix();

	Matrix3x2 m_projection(_view._viewportDistance / bottom.z,  0.0f, 0.0f,
	                                                     0.0f, 25.5f, 0.0f);

	Matrix3x2 m_frame(_frameScale.x,          0.0f, _framePos.x,
	                           0.0f, _frameScale.y, _framePos.y);

	_modelMatrix = m_projection * (facingRotation * m_frame);

	Vector4 startScreenVector(
	           _view._viewportHalfWidth   + top.x / top.z * _view._viewportDistance,
	           _view._viewportHalfHeight  + top.y / top.z * _view._viewportDistance,
	           1.0f / top.z,
	           _frameSliceCount * (1.0f / top.z));

	Vector4 endScreenVector(
	           _view._viewportHalfWidth   + bottom.x / bottom.z * _view._viewportDistance,
	           _view._viewportHalfHeight  + bottom.y / bottom.z * _view._viewportDistance,
	           1.0f / bottom.z,
	           0.0f);

	_startScreenVector.x = startScreenVector.x;
	_startScreenVector.y = startScreenVector.y;
	_startScreenVector.z = startScreenVector.z;
	_endScreenVector.x   = endScreenVector.x;
	_endScreenVector.y   = endScreenVector.y;
	_endScreenVector.z   = endScreenVector.z;
	_startSlice          = startScreenVector.w;
	_endSlice            = endScreenVector.w;

	Vector4 delta = endScreenVector - startScreenVector;

	if (delta.y == 0.0f) {
		return;
	}

	/*
	 * Calculate min and max Y
	 */

	float screenMinY =   0.0f;
	float screenMaxY = 479.0f;

	if (startScreenVector.y < screenMinY) {
		if (endScreenVector.y < screenMinY)
			return;

		float f = (screenMinY - startScreenVector.y) / delta.y;
		startScreenVector = startScreenVector + f * delta;
	} else if (startScreenVector.y > screenMaxY) {
		if (endScreenVector.y >= screenMaxY)
			return;

		float f = (screenMaxY - startScreenVector.y) / delta.y;
		startScreenVector = startScreenVector + f * delta;
	}

	if (endScreenVector.y < screenMinY) {
		float f = (screenMinY - endScreenVector.y) / delta.y;
		endScreenVector = endScreenVector + f * delta;
	} else if (endScreenVector.y > screenMaxY) {
		float f = (screenMaxY - endScreenVector.y) / delta.y;
		endScreenVector = endScreenVector + f * delta;
	}

	int bbox_min_y = (int)MIN(startScreenVector.y, endScreenVector.y);
	int bbox_max_y = (int)MAX(startScreenVector.y, endScreenVector.y) + 1;

	/*
	 * Calculate min and max X
	 */

	Matrix3x2 mB6 = _modelMatrix + Vector2(startScreenVector.x, 25.5f / startScreenVector.z);
	Matrix3x2 mC2 = _modelMatrix + Vector2(endScreenVector.x,   25.5f / endScreenVector.z);

	float min_x =  640.0f;
	float max_x =    0.0f;

	for (float i = 0.0f; i <= 256.0f; i += 255.0f) {
		for (float j = 0.0f; j <= 256.0f; j += 255.0f) {
			Vector2 v1 = mB6 * Vector2(i, j);

			min_x = MIN(min_x, v1.x);
			max_x = MAX(max_x, v1.x);

			Vector2 v2 = mC2 * Vector2(i, j);

			min_x = MIN(min_x, v2.x);
			max_x = MAX(max_x, v2.x);
		}
	}

	int bbox_min_x = CLIP((int)min_x,     0, 640);
	int bbox_max_x = CLIP((int)max_x + 1, 0, 640);

	_startScreenVector.x = startScreenVector.x;
	_startScreenVector.y = startScreenVector.y;
	_startScreenVector.z = startScreenVector.z;
	_endScreenVector.x   = endScreenVector.x;
	_endScreenVector.y   = endScreenVector.y;
	_endScreenVector.z   = endScreenVector.z;
	_startSlice          = startScreenVector.w;
	_endSlice            = endScreenVector.w;

	_screenRectangle.left   = bbox_min_x;
	_screenRectangle.right  = bbox_max_x;
	_screenRectangle.top    = bbox_min_y;
	_screenRectangle.bottom = bbox_max_y;
}

void SliceRenderer::loadFrame(int animation, int frame) {
	_animation = animation;
	_frame = frame;
	_sliceFramePtr = _vm->_sliceAnimations->getFramePtr(_animation, _frame);

	Common::MemoryReadStream stream((byte*)_sliceFramePtr, _vm->_sliceAnimations->_animations[_animation].frameSize);

	_frameScale.x = stream.readFloatLE();
	_frameScale.y = stream.readFloatLE();
	_frameSliceHeight = stream.readFloatLE();
	_framePos.x = stream.readFloatLE();
	_framePos.y = stream.readFloatLE();
	_frameBottomZ = stream.readFloatLE();
	_framePaletteIndex = stream.readUint32LE();
	_frameSliceCount = stream.readUint32LE();
}

struct SliceLineIterator {
	int _sliceMatrix[2][3];
	int _startY;
	int _endY;

	float _currentZ;
	float _stepZ;
	float _currentSlice;
	float _stepSlice;
	float _currentX;
	float _stepX;
	int   _field_38;
	int   _currentY;

	void setup(float endScreenX,   float endScreenY,   float endScreenZ,
	           float startScreenX, float startScreenY, float startScreenZ,
	           float endSlice,     float startSlice,
	           Matrix3x2 m);
	float line();
	void advance();
};

void SliceLineIterator::setup(
		float endScreenX,   float endScreenY,   float endScreenZ,
		float startScreenX, float startScreenY, float startScreenZ,
		float endSlice,     float startSlice,
		Matrix3x2 m) {
	_startY = (int)startScreenY;
	_endY   = (int)endScreenY;

	float size = endScreenY - startScreenY;

	if (size <= 0.0f || startScreenZ <= 0.0f)
		_currentY = _endY + 1;

	_currentZ  = startScreenZ;
	_stepZ     = (endScreenZ - startScreenZ) / size;

	_stepSlice     = (endSlice - startSlice) / size;
	_currentSlice  = startSlice - (startScreenY - floor(startScreenY) - 1.0f) * _stepSlice;

	_currentX = startScreenX;
	_stepX    = (endScreenX - startScreenX) / size;

	_field_38 = (int)((25.5f / size) * (1.0f / endScreenZ - 1.0f / startScreenZ) * 64.0);
	_currentY = _startY;

	float offsetX =         _currentX;
	float offsetZ = 25.5f / _currentZ;

	Matrix3x2 translate_matrix = Matrix3x2(1.0f, 0.0f, offsetX,
	                                       0.0f, 1.0f, offsetZ);

	Matrix3x2 scale_matrix = Matrix3x2(65536.0f,  0.0f, 0.0f,
	                                       0.0f, 64.0f, 0.0f);

	m = scale_matrix * (translate_matrix * m);

	for (int r = 0; r != 2; ++r)
		for (int c = 0; c != 3; ++c)
			_sliceMatrix[r][c] = m(r, c);
}

float SliceLineIterator::line() {
	float var_0 = 0.0f;

	if (_currentZ != 0.0f)
		var_0 = _currentSlice / _currentZ;

	if (var_0 < 0.0)
		var_0 = 0.0f;

	return var_0;
}

void SliceLineIterator::advance() {
	_currentZ          += _stepZ;
	_currentSlice      += _stepSlice;
	_currentX          += _stepX;
	_currentY          += 1;
	_sliceMatrix[0][2] += (int)(65536.0f * _stepX);
	_sliceMatrix[1][2] += _field_38;
}

static void setupLookupTable(int t[256], int inc) {
	int v = 0;
	for (int i = 0; i != 256; ++i) {
		t[i] = v;
		v += inc;
	}
}

void SliceRenderer::drawInWorld(int animationId, int animationFrame, Vector3 position, float facing, float scale, Graphics::Surface &surface, uint16 *zbuffer) {
	assert(_lights);
	assert(_setEffects);
	//assert(_view);

	_vm->_sliceRenderer->setupFrameInWorld(animationId, animationFrame, position, facing);
	assert(_sliceFramePtr);

	SliceLineIterator sliceLineIterator;
	sliceLineIterator.setup(
		_endScreenVector.x,   _endScreenVector.y,   _endScreenVector.z,
		_startScreenVector.x, _startScreenVector.y, _startScreenVector.z,
		_endSlice,            _startSlice,
		_modelMatrix
	);

	SliceRendererLights sliceRendererLights = SliceRendererLights(_lights);

	_lights->setupFrame(_view._frame);
	_setEffects->setupFrame(_view._frame);

	float sliceLine = sliceLineIterator.line();

	sliceRendererLights.calculateColorBase(
		Vector3(_position.x, _position.y, _position.z + _frameBottomZ + sliceLine * _frameSliceHeight),
		Vector3(_position.x, _position.y, _position.z + _frameBottomZ),
		sliceLineIterator._endY - sliceLineIterator._startY);

	float setEffectsColorCoeficient;
	Color setEffectColor;
	_setEffects->calculateColor(
		_view._cameraPosition,
		Vector3(_position.x, _position.y, _position.z + _frameBottomZ + sliceLine * _frameSliceHeight),
		&setEffectsColorCoeficient,
		&setEffectColor);

	_lightsColor.r = setEffectsColorCoeficient * sliceRendererLights._finalColor.r * 65536.0f;
	_lightsColor.g = setEffectsColorCoeficient * sliceRendererLights._finalColor.g * 65536.0f;
	_lightsColor.b = setEffectsColorCoeficient * sliceRendererLights._finalColor.b * 65536.0f;

	_setEffectColor.r = setEffectColor.r * 31.0f * 65536.0f;
	_setEffectColor.g = setEffectColor.g * 31.0f * 65536.0f;
	_setEffectColor.b = setEffectColor.b * 31.0f * 65536.0f;

	setupLookupTable(_m11lookup, sliceLineIterator._sliceMatrix[0][0]);
	setupLookupTable(_m12lookup, sliceLineIterator._sliceMatrix[0][1]);
	_m13 = sliceLineIterator._sliceMatrix[0][2];
	setupLookupTable(_m21lookup, sliceLineIterator._sliceMatrix[1][0]);
	setupLookupTable(_m22lookup, sliceLineIterator._sliceMatrix[1][1]);
	_m23 = sliceLineIterator._sliceMatrix[1][2];


	if(_animationsShadowEnabled[_animation]) {
		//TODO: draw shadows
	}

	int frameY = sliceLineIterator._startY;

	uint16 *frameLinePtr  = (uint16*)surface.getPixels() + 640 * frameY;
	uint16 *zBufferLinePtr = zbuffer + 640 * frameY;

	while (sliceLineIterator._currentY <= sliceLineIterator._endY) {
		sliceLine = sliceLineIterator.line();

		sliceRendererLights.calculateColorSlice(Vector3(_position.x, _position.y, _position.z + _frameBottomZ + sliceLine * _frameSliceHeight));

		if (sliceLineIterator._currentY & 1) {
			_setEffects->calculateColor(
				_view._cameraPosition,
				Vector3(_position.x, _position.y, _position.z + _frameBottomZ + sliceLine * _frameSliceHeight),
				&setEffectsColorCoeficient,
				&setEffectColor);
		}

		_lightsColor.r = setEffectsColorCoeficient * sliceRendererLights._finalColor.r * 65536.0f;
		_lightsColor.g = setEffectsColorCoeficient * sliceRendererLights._finalColor.g * 65536.0f;
		_lightsColor.b = setEffectsColorCoeficient * sliceRendererLights._finalColor.b * 65536.0f;

		_setEffectColor.r = setEffectColor.r * 31.0f * 65536.0f;
		_setEffectColor.g = setEffectColor.g * 31.0f * 65536.0f;
		_setEffectColor.b = setEffectColor.b * 31.0f * 65536.0f;

		if (frameY >= 0 && frameY < 480) {
			drawSlice((int)sliceLine, true, frameLinePtr, zBufferLinePtr);
		}

		sliceLineIterator.advance();
		frameY += 1;
		frameLinePtr += 640;
		zBufferLinePtr += 640;
	}
}

void SliceRenderer::drawOnScreen(int animationId, int animationFrame, int screenX, int screenY, float facing, float scale, Graphics::Surface &surface, uint16 *zbuffer) {
	if (scale == 0.0f) {
		return;
	}
	_position.x = 0;
	_position.y = 0;
	_position.z = 0;
	_facing = facing;

	loadFrame(animationId, animationFrame);

	float frameHeight = _frameSliceHeight * _frameSliceCount;
	float frameSize = sqrtf(_frameScale.x * 255.0f * _frameScale.x * 255.0f + _frameScale.y * 255.0f * _frameScale.y * 255.0f);
	float size = scale / MAX(frameSize, frameHeight);

	float s = sinf(_facing);
	float c = cosf(_facing);

	Matrix3x2 m_rotation(c, -s, 0.0f,
	                     s,  c, 0.0f);

	Matrix3x2 m_frame(_frameScale.x, 0.0f, _framePos.x,
	                  0.0f, _frameScale.y, _framePos.y);

	Matrix3x2 m_scale_size_25_5(size,  0.0f, 0.0f,
	                            0.0f, 25.5f, 0.0f);

	Matrix3x2 m_translate_x_32k(1.0f, 0.0f, screenX,
	                            0.0f, 1.0f, 32768.0f);

	Matrix3x2 m_scale_64k_64(65536.0f,  0.0f, 0.0f,
	                             0.0f, 64.0f, 0.0f);

	Matrix3x2 m = m_scale_64k_64 * (m_translate_x_32k * (m_scale_size_25_5 * (m_rotation * m_frame)));

	setupLookupTable(_m11lookup, m(0, 0));
	setupLookupTable(_m12lookup, m(0, 1));
	_m13 = m(0, 2);
	setupLookupTable(_m21lookup, m(1, 0));
	setupLookupTable(_m22lookup, m(1, 1));
	_m23 = m(1, 2);

	int frameY = screenY + (size / 2.0f * frameHeight);
	int currentY = frameY;

	float currentSlice = 0;
	float sliceStep = 1.0f / size / _frameSliceHeight;

	uint16 *frameLinePtr = (uint16*)surface.getPixels() + 640 * frameY;
	uint16 lineZbuffer[640];

	while (currentSlice < _frameSliceCount) {
		if (currentY >= 0 && currentY < 480) {
			memset(lineZbuffer, 0xFF, 640 * 2);
			drawSlice(currentSlice, false, frameLinePtr, lineZbuffer);
			currentSlice += sliceStep;
			currentY--;
			frameLinePtr -= 640;
		}
	}
}

void SliceRenderer::drawSlice(int slice, bool advanced, uint16 *frameLinePtr, uint16 *zbufLinePtr) {
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
		int lastVertexX = MAX((_m11lookup[p[3 * lastVertex]] + _m12lookup[p[3 * lastVertex + 1]] + _m13) >> 16, 0);

		int previousVertexX = lastVertexX;

		while (vertexCount--) {
			int vertexX = CLIP((_m11lookup[p[0]] + _m12lookup[p[1]] + _m13) >> 16, 0, 640);

			if (vertexX > previousVertexX) {
				int vertexZ = (_m21lookup[p[0]] + _m22lookup[p[1]] + _m23) >> 6;

				if (vertexZ >= 0 && vertexZ < 65536) {
					int color555 = palette.color555[p[2]];
					if (advanced) {
						Color256 color = palette.color[p[2]];

						color.r = (int)(_setEffectColor.r + _lightsColor.r * color.r) >> 16;
						color.g = (int)(_setEffectColor.g + _lightsColor.g * color.g) >> 16;
						color.b = (int)(_setEffectColor.b + _lightsColor.b * color.b) >> 16;

						int bladeToScummVmConstant = 256 / 32;

						color555 = _pixelFormat.RGBToColor(CLIP(color.r * bladeToScummVmConstant, 0, 255), CLIP(color.g * bladeToScummVmConstant, 0, 255), CLIP(color.b * bladeToScummVmConstant, 0, 255));
					}
					for (int x = previousVertexX; x != vertexX; ++x) {
						if (vertexZ < zbufLinePtr[x]) {
							frameLinePtr[x] = color555;
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

void SliceRenderer::disableShadows(int animationsIdsList[], int listSize) {
	int i;
	for (i = 0; i < listSize; i++) {
		_animationsShadowEnabled[animationsIdsList[i]] = false;
	}
}

SliceRendererLights::SliceRendererLights(Lights *lights) {
	_finalColor.r = 0.0f;
	_finalColor.g = 0.0f;
	_finalColor.b = 0.0f;

	_lights = lights;

	for (int i = 0; i < 20; i++) {
		_cacheColor[i].r = 0.0f;
		_cacheColor[i].g = 0.0f;
		_cacheColor[i].b = 0.0f;
	}
}

void SliceRendererLights::calculateColorBase(Vector3 position1, Vector3 position2, float height) {
	_finalColor.r = 0.0f;
	_finalColor.g = 0.0f;
	_finalColor.b = 0.0f;
	_cacheRecalculation = 0;
	if (_lights) {
		for (uint i = 0; i < _lights->_lights.size(); i++) {
			Light *light = _lights->_lights[i];
			if (i < 20) {
				float cacheCoeficient = light->calculate(position1, position2/*, height*/);
				_cacheStart[i] = cacheCoeficient;
				_cacheCounter[i] = cacheCoeficient;

				Color color;
				light->calculateColor(&color, position1);
				_cacheColor[i] = color;
				_finalColor.r += color.r;
				_finalColor.g += color.g;
				_finalColor.b += color.b;
			} else {
				Color color;
				light->calculateColor(&color, position1);
				_finalColor.r += color.r;
				_finalColor.g += color.g;
				_finalColor.b += color.b;
			}
		}

		_finalColor.r += _lights->_ambientLightColor.r;
		_finalColor.g += _lights->_ambientLightColor.g;
		_finalColor.b += _lights->_ambientLightColor.b;
	}
}

void SliceRendererLights::calculateColorSlice(Vector3 position) {
	_finalColor.r = 0.0f;
	_finalColor.g = 0.0f;
	_finalColor.b = 0.0f;

	if (_lights) {
		for (uint i = 0; i < _lights->_lights.size(); i++) {
			Light *light = _lights->_lights[i];
			if (i < 20) {
				_cacheCounter[i] -= 1.0f;
				if (_cacheCounter[i] <= 0.0f) {
					do {
						_cacheCounter[i] = _cacheCounter[i] + _cacheStart[i];
					} while (_cacheCounter[i] <= 0.0f);
					light->calculateColor(&_cacheColor[i], position);
					_cacheRecalculation++;
				}
				_finalColor.r += _cacheColor[i].r;
				_finalColor.g += _cacheColor[i].g;
				_finalColor.b += _cacheColor[i].b;
			} else {
				Color color;
				light->calculateColor(&color, position);
				_cacheRecalculation++;
				_finalColor.r += color.r;
				_finalColor.g += color.g;
				_finalColor.b += color.b;
			}
		}
		_finalColor.r += _lights->_ambientLightColor.r;
		_finalColor.g += _lights->_ambientLightColor.g;
		_finalColor.b += _lights->_ambientLightColor.b;
	}
}

} // End of namespace BladeRunner
