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
#include "bladerunner/screen_effects.h"
#include "bladerunner/set_effects.h"
#include "bladerunner/slice_animations.h"

#include "common/memstream.h"
#include "common/rect.h"
#include "common/util.h"

namespace BladeRunner {

SliceRenderer::SliceRenderer(BladeRunnerEngine *vm) {
	_vm = vm;
	_pixelFormat = screenPixelFormat();

	for (int i = 0; i < 942; i++) { // yes, its going just to 942 and not 997
		_animationsShadowEnabled[i] = true;
	}
	_animation = -1;
	_frame     = -1;
	_facing    = 0.0f;
	_scale     = 0.0f;

	_screenEffects = nullptr;
	_view          = nullptr;
	_lights        = nullptr;
	_setEffects    = nullptr;
	_sliceFramePtr = nullptr;

	_frameBottomZ      = 0.0f;
	_frameSliceHeight  = 0.0f;
	_framePaletteIndex = 0;
	_frameSliceCount   = 0;
	_startSlice        = 0.0f;
	_endSlice          = 0.0f;
	_m13               = 0;
	_m23               = 0;

	_shadowPolygonDefault[ 0] = Vector3( 16.0f,  96.0f, 0.0f);
	_shadowPolygonDefault[ 1] = Vector3( 16.0f, 160.0f, 0.0f);
	_shadowPolygonDefault[ 2] = Vector3( 64.0f, 192.0f, 0.0f);
	_shadowPolygonDefault[ 3] = Vector3( 80.0f, 240.0f, 0.0f);
	_shadowPolygonDefault[ 4] = Vector3(160.0f, 240.0f, 0.0f);
	_shadowPolygonDefault[ 5] = Vector3(192.0f, 192.0f, 0.0f);
	_shadowPolygonDefault[ 6] = Vector3(240.0f, 160.0f, 0.0f);
	_shadowPolygonDefault[ 7] = Vector3(240.0f,  96.0f, 0.0f);
	_shadowPolygonDefault[ 8] = Vector3(192.0f,  64.0f, 0.0f);
	_shadowPolygonDefault[ 9] = Vector3(160.0f,  16.0f, 0.0f);
	_shadowPolygonDefault[10] = Vector3( 96.0f,  16.0f, 0.0f);
	_shadowPolygonDefault[11] = Vector3( 64.0f,  64.0f, 0.0f);

	for (int i = 0; i < 12; ++i) {
		_shadowPolygonCurrent[i] = Vector3(0.0f, 0.0f, 0.0f);
	}
}

SliceRenderer::~SliceRenderer() {
}

void SliceRenderer::setScreenEffects(ScreenEffects *screenEffects) {
	_screenEffects = screenEffects;
}

void SliceRenderer::setView(View *view) {
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

	Vector3 viewPos = _view->_sliceViewMatrix * _position;
	float dir = atan2(viewPos.x, viewPos.z) + _facing;
	float s = sin(dir);
	float c = cos(dir);

	Matrix3x2 mRotation(c, -s, 0.0f,
	                    s,  c, 0.0f);

	Matrix3x2 mView(_view->_sliceViewMatrix(0,0), _view->_sliceViewMatrix(0,1), 0.0f,
	                _view->_sliceViewMatrix(2,0), _view->_sliceViewMatrix(2,1), 0.0f);

	return mView * mRotation;
}

void SliceRenderer::calculateBoundingRect() {
	assert(_sliceFramePtr);

	_screenRectangle.left   = 0;
	_screenRectangle.right  = 0;
	_screenRectangle.top    = 0;
	_screenRectangle.bottom = 0;

	Matrix4x3 viewMatrix = _view->_sliceViewMatrix;

	Vector3 frameBottom = Vector3(0.0f, 0.0f, _frameBottomZ);
	Vector3 frameTop    = Vector3(0.0f, 0.0f, _frameBottomZ + _frameSliceCount * _frameSliceHeight);

	Vector3 bottom = viewMatrix * (_position + frameBottom);
	Vector3 top    = viewMatrix * (_position + frameTop);

	top = bottom + _scale * (top - bottom);

	if (bottom.z <= 0.0f || top.z <= 0.0f) {
		return;
	}

	Vector4 startScreenVector(
	           _view->_viewportPosition.x + (top.x / top.z) * _view->_viewportPosition.z,
	           _view->_viewportPosition.y + (top.y / top.z) * _view->_viewportPosition.z,
	           1.0f / top.z,
	           _frameSliceCount * (1.0f / top.z));

	Vector4 endScreenVector(
	           _view->_viewportPosition.x + (bottom.x / bottom.z) * _view->_viewportPosition.z,
	           _view->_viewportPosition.y + (bottom.y / bottom.z) * _view->_viewportPosition.z,
	           1.0f / bottom.z,
	           0.0f);

	Vector4 delta = endScreenVector - startScreenVector;

	if (delta.y == 0.0f) {
		return;
	}

	/*
	 * Calculate min and max Y
	 */

	float screenTop    =   0.0f;
	float screenBottom = 479.0f;

	if (startScreenVector.y < screenTop) {
		if (endScreenVector.y < screenTop) {
			return;
		}
		float f = (screenTop - startScreenVector.y) / delta.y;
		startScreenVector = startScreenVector + f * delta;
	} else if (startScreenVector.y > screenBottom) {
		if (endScreenVector.y >= screenBottom) {
			return;
		}
		float f = (screenBottom - startScreenVector.y) / delta.y;
		startScreenVector = startScreenVector + f * delta;
	}

	if (endScreenVector.y < screenTop) {
		float f = (screenTop - endScreenVector.y) / delta.y;
		endScreenVector = endScreenVector + f * delta;
	} else if (endScreenVector.y > screenBottom) {
		float f = (screenBottom - endScreenVector.y) / delta.y;
		endScreenVector = endScreenVector + f * delta;
	}

	_screenRectangle.top    = (int)MIN(startScreenVector.y, endScreenVector.y);
	_screenRectangle.bottom = (int)MAX(startScreenVector.y, endScreenVector.y) + 1;

	/*
	 * Calculate min and max X
	 */

	Matrix3x2 facingRotation = calculateFacingRotationMatrix();

	Matrix3x2 mProjection(_view->_viewportPosition.z / bottom.z,  0.0f, 0.0f,
	                                                       0.0f, 25.5f, 0.0f);

	Matrix3x2 mOffset(1.0f, 0.0f, _framePos.x,
	                  0.0f, 1.0f, _framePos.y);

	Matrix3x2 mScale(_frameScale.x,          0.0f, 0.0f,
	                          0.0f, _frameScale.y, 0.0f);

	_mvpMatrix = mProjection * (facingRotation * (mOffset * mScale));

	Matrix3x2 mStart(
		1.0f, 0.0f, startScreenVector.x,
		0.0f, 1.0f, 25.5f / startScreenVector.z
	);

	Matrix3x2 mEnd(
		1.0f, 0.0f, endScreenVector.x,
		0.0f, 1.0f, 25.5f / endScreenVector.z
	);

	Matrix3x2 mStartMVP = mStart * _mvpMatrix;
	Matrix3x2 mEndMVP   = mEnd   * _mvpMatrix;

	float minX =  640.0f;
	float maxX =    0.0f;

	for (float i = 0.0f; i <= 255.0f; i += 255.0f) {
		for (float j = 0.0f; j <= 255.0f; j += 255.0f) {
			Vector2 v1 = mStartMVP * Vector2(i, j);
			minX = MIN(minX, v1.x);
			maxX = MAX(maxX, v1.x);

			Vector2 v2 = mEndMVP * Vector2(i, j);
			minX = MIN(minX, v2.x);
			maxX = MAX(maxX, v2.x);
		}
	}

	_screenRectangle.left  = CLIP((int)minX,     0, 640);
	_screenRectangle.right = CLIP((int)maxX + 1, 0, 640);

	_startScreenVector.x = startScreenVector.x;
	_startScreenVector.y = startScreenVector.y;
	_startScreenVector.z = startScreenVector.z;
	_endScreenVector.x   = endScreenVector.x;
	_endScreenVector.y   = endScreenVector.y;
	_endScreenVector.z   = endScreenVector.z;
	_startSlice          = startScreenVector.w;
	_endSlice            = endScreenVector.w;
}

void SliceRenderer::loadFrame(int animation, int frame) {
	_animation = animation;
	_frame = frame;
	_sliceFramePtr = _vm->_sliceAnimations->getFramePtr(_animation, _frame);

	Common::MemoryReadStream stream((byte *)_sliceFramePtr, _vm->_sliceAnimations->_animations[_animation].frameSize);

	_frameScale.x      = stream.readFloatLE();
	_frameScale.y      = stream.readFloatLE();
	_frameSliceHeight  = stream.readFloatLE();
	_framePos.x        = stream.readFloatLE();
	_framePos.y        = stream.readFloatLE();
	_frameBottomZ      = stream.readFloatLE();
	_framePaletteIndex = stream.readUint32LE();
	_frameSliceCount   = stream.readUint32LE();
}

struct SliceLineIterator {
	Matrix3x2 _sliceMatrix;
	int _startY;
	int _endY;
	int _currentY;

	float _currentZ;
	float _stepZ;
	float _currentSlice;
	float _stepSlice;

	float _currentX;
	float _stepX;

	float _field_38;

	void setup(float endScreenX,   float endScreenY,   float endScreenZ,
	           float startScreenX, float startScreenY, float startScreenZ,
	           float endSlice,     float startSlice,
	           Matrix3x2 m);
	float line() const;
	void advance();
};

void SliceLineIterator::setup(
		float endScreenX,   float endScreenY,   float endScreenZ,
		float startScreenX, float startScreenY, float startScreenZ,
		float endSlice,     float startSlice,
		Matrix3x2 m) {
	_startY   = (int)startScreenY;
	_endY     = (int)endScreenY;
	_currentY = _startY;

	float size = endScreenY - startScreenY;

	if (size <= 0.0f || startScreenZ <= 0.0f) {
		_currentY = _endY + 1;
	}

	_currentZ  = startScreenZ;
	_stepZ     = (endScreenZ - startScreenZ) / size;

	_stepSlice     = (endSlice - startSlice) / size;
	_currentSlice  = startSlice - (startScreenY - floor(startScreenY) - 1.0f) * _stepSlice;

	_currentX = startScreenX;
	_stepX    = (endScreenX - startScreenX) / size;

	_field_38 = (25.5f / size) * (1.0f / endScreenZ - 1.0f / startScreenZ);

	float offsetX =         _currentX;
	float offsetZ = 25.5f / _currentZ;

	Matrix3x2 mTranslate = Matrix3x2(1.0f, 0.0f, offsetX,
	                                 0.0f, 1.0f, offsetZ);

	Matrix3x2 mScale = Matrix3x2(65536.0f,  0.0f, 0.0f,  // x position is using fixed-point precisson with 16 bits
	                                 0.0f, 64.0f, 0.0f); // z position is using fixed-point precisson with 6 bits

	_sliceMatrix = mScale * (mTranslate * m);
}

float SliceLineIterator::line() const {
	float var_0 = 0.0f;

	if (_currentZ != 0.0f)
		var_0 = _currentSlice / _currentZ;

	if (var_0 < 0.0)
		var_0 = 0.0f;

	return var_0;
}

void SliceLineIterator::advance() {
	_currentZ     += _stepZ;
	_currentSlice += _stepSlice;
	_currentX     += _stepX;
	_currentY     += 1;

	_sliceMatrix._m[0][2] += _stepX * 65536.0f;
	_sliceMatrix._m[1][2] += _field_38 * 64.0f;
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

	setupFrameInWorld(animationId, animationFrame, position, facing, scale);

	assert(_sliceFramePtr);

	if (_screenRectangle.isEmpty()) {
		return;
	}

	SliceLineIterator sliceLineIterator;
	sliceLineIterator.setup(
		_endScreenVector.x,   _endScreenVector.y,   _endScreenVector.z,
		_startScreenVector.x, _startScreenVector.y, _startScreenVector.z,
		_endSlice,            _startSlice,
		_mvpMatrix
	);

	SliceRendererLights sliceRendererLights = SliceRendererLights(_lights);

	_lights->setupFrame(_view->_frame);
	_setEffects->setupFrame(_view->_frame);

	float sliceLine = sliceLineIterator.line();

	sliceRendererLights.calculateColorBase(
		Vector3(_position.x, _position.y, _position.z + _frameBottomZ + sliceLine * _frameSliceHeight),
		Vector3(_position.x, _position.y, _position.z + _frameBottomZ),
		sliceLineIterator._endY - sliceLineIterator._startY);

	float setEffectsColorCoeficient;
	Color setEffectColor;
	_setEffects->calculateColor(
		_view->_cameraPosition,
		Vector3(_position.x, _position.y, _position.z + _frameBottomZ + sliceLine * _frameSliceHeight),
		&setEffectsColorCoeficient,
		&setEffectColor);

	_lightsColor.r = setEffectsColorCoeficient * sliceRendererLights._finalColor.r * 65536.0f;
	_lightsColor.g = setEffectsColorCoeficient * sliceRendererLights._finalColor.g * 65536.0f;
	_lightsColor.b = setEffectsColorCoeficient * sliceRendererLights._finalColor.b * 65536.0f;

	_setEffectColor.r = setEffectColor.r * 31.0f * 65536.0f;
	_setEffectColor.g = setEffectColor.g * 31.0f * 65536.0f;
	_setEffectColor.b = setEffectColor.b * 31.0f * 65536.0f;

	setupLookupTable(_m12lookup, sliceLineIterator._sliceMatrix(0, 1));
	setupLookupTable(_m11lookup, sliceLineIterator._sliceMatrix(0, 0));
	_m13 = sliceLineIterator._sliceMatrix(0, 2);
	setupLookupTable(_m21lookup, sliceLineIterator._sliceMatrix(1, 0));
	setupLookupTable(_m22lookup, sliceLineIterator._sliceMatrix(1, 1));
	_m23 = sliceLineIterator._sliceMatrix(1, 2);

	if (_animationsShadowEnabled[_animation]) {
		float coeficientShadow;
		Color colorShadow;
		_setEffects->calculateColor(
				_view->_cameraPosition,
				_position,
				&coeficientShadow,
				&colorShadow);

		int transparency = 32.0f * sqrt(setEffectColor.r * setEffectColor.r + setEffectColor.g * setEffectColor.g + setEffectColor.b * setEffectColor.b);

		drawShadowInWorld(transparency, surface, zbuffer);
	}

	int frameY = sliceLineIterator._startY;

	uint16 *zBufferLinePtr = zbuffer + 640 * frameY;

	while (sliceLineIterator._currentY <= sliceLineIterator._endY) {
		_m13 = sliceLineIterator._sliceMatrix(0, 2);
		_m23 = sliceLineIterator._sliceMatrix(1, 2);
		sliceLine = sliceLineIterator.line();

		sliceRendererLights.calculateColorSlice(Vector3(_position.x, _position.y, _position.z + _frameBottomZ + sliceLine * _frameSliceHeight));

		if (sliceLineIterator._currentY & 1) {
			_setEffects->calculateColor(
				_view->_cameraPosition,
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

		if (frameY >= 0 && frameY < surface.h) {
			drawSlice((int)sliceLine, true, frameY, surface, zBufferLinePtr);
		}

		sliceLineIterator.advance();
		frameY += 1;
		zBufferLinePtr += 640;
	}
}

void SliceRenderer::drawOnScreen(int animationId, int animationFrame, int screenX, int screenY, float facing, float scale, Graphics::Surface &surface) {
	if (scale == 0.0f) {
		return;
	}
	_position.x = 0;
	_position.y = 0;
	_position.z = 0;
	_facing = facing;

	loadFrame(animationId, animationFrame);

	float frameHeight = _frameSliceHeight * _frameSliceCount;
	float frameSize = sqrt(_frameScale.x * 255.0f * _frameScale.x * 255.0f + _frameScale.y * 255.0f * _frameScale.y * 255.0f);
	float size = scale / MAX(frameSize, frameHeight);

	float s = sin(_facing);
	float c = cos(_facing);

	Matrix3x2 mRotation(c, -s, 0.0f,
	                    s,  c, 0.0f);

	Matrix3x2 mFrame(_frameScale.x,           0.0f, _framePos.x,
	                           0.0f, _frameScale.y, _framePos.y);

	Matrix3x2 mScale(size,  0.0f, 0.0f,
	                 0.0f, 25.5f, 0.0f);

	Matrix3x2 mTranslate(1.0f, 0.0f, screenX,
	                     0.0f, 1.0f, 32768.0f);

	Matrix3x2 mScaleFixed(65536.0f,  0.0f, 0.0f,  // x position is using fixed-point precisson with 16 bits
	                          0.0f, 64.0f, 0.0f); // z position is using fixed-point precisson with 6 bits

	Matrix3x2 m = mScaleFixed * (mTranslate * (mScale * (mRotation * mFrame)));

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

	uint16 lineZbuffer[640];

	while (currentSlice < _frameSliceCount) {
		if (currentY >= 0 && currentY < surface.h) {
			memset(lineZbuffer, 0xFF, 640 * 2);
			drawSlice(currentSlice, false, currentY, surface, lineZbuffer);
			currentSlice += sliceStep;
			currentY--;
		}
	}
}

void SliceRenderer::drawSlice(int slice, bool advanced, int y, Graphics::Surface &surface, uint16 *zbufferLine) {
	if (slice < 0 || (uint32)slice >= _frameSliceCount) {
		return;
	}

	SliceAnimations::Palette &palette = _vm->_sliceAnimations->getPalette(_framePaletteIndex);

	byte *p = (byte *)_sliceFramePtr + 0x20 + 4 * slice;

	uint32 polyOffset = READ_LE_UINT32(p);

	p = (byte *)_sliceFramePtr + polyOffset;

	uint32 polyCount = READ_LE_UINT32(p);
	p += 4;

	while (polyCount--) {
		uint32 vertexCount = READ_LE_UINT32(p);
		p += 4;

		if (vertexCount == 0)
			continue;

		uint32 lastVertex = vertexCount - 1;
		int lastVertexX = MAX((_m11lookup[p[3 * lastVertex]] + _m12lookup[p[3 * lastVertex + 1]] + _m13) / 65536, 0);

		int previousVertexX = lastVertexX;

		while (vertexCount--) {
			int vertexX = CLIP((_m11lookup[p[0]] + _m12lookup[p[1]] + _m13) / 65536, 0, 640);

			if (vertexX > previousVertexX) {
				int vertexZ = (_m21lookup[p[0]] + _m22lookup[p[1]] + _m23) / 64;

				if (vertexZ >= 0 && vertexZ < 65536) {
					uint32 outColor = palette.value[p[2]];
					if (advanced) {
						Color256 aescColor = { 0, 0, 0 };
						_screenEffects->getColor(&aescColor, vertexX, y, vertexZ);

						Color256 color = palette.color[p[2]];
						color.r = ((int)(_setEffectColor.r + _lightsColor.r * color.r) / 65536) + aescColor.r;
						color.g = ((int)(_setEffectColor.g + _lightsColor.g * color.g) / 65536) + aescColor.g;
						color.b = ((int)(_setEffectColor.b + _lightsColor.b * color.b) / 65536) + aescColor.b;

						int bladeToScummVmConstant = 256 / 32;
						outColor = _pixelFormat.RGBToColor(CLIP(color.r * bladeToScummVmConstant, 0, 255), CLIP(color.g * bladeToScummVmConstant, 0, 255), CLIP(color.b * bladeToScummVmConstant, 0, 255));
					}

					for (int x = previousVertexX; x != vertexX; ++x) {
						if (vertexZ < zbufferLine[x]) {
							zbufferLine[x] = (uint16)vertexZ;

							void *dstPtr = surface.getBasePtr(CLIP(x, 0, surface.w - 1), CLIP(y, 0, surface.h - 1));
							drawPixel(surface, dstPtr, outColor);
						}
					}
				}
			}
			p += 3;
			previousVertexX = vertexX;
		}
	}
}

void SliceRenderer::drawShadowInWorld(int transparency, Graphics::Surface &surface, uint16 *zbuffer) {
	Matrix4x3 mOffset(
		1.0f, 0.0f, 0.0f, _framePos.x,
		0.0f, 1.0f, 0.0f, _framePos.y,
		0.0f, 0.0f, 1.0f, 0.0f);

	Matrix4x3 mTransition(
		1.0f, 0.0f, 0.0f, _position.x,
		0.0f, 1.0f, 0.0f, _position.y,
		0.0f, 0.0f, 1.0f, _position.z);

	Matrix4x3 mRotation(
		cosf(_facing), -sinf(_facing), 0.0f, 0.0f,
		sinf(_facing),  cosf(_facing), 0.0f, 0.0f,
		          0.0f,          0.0f, 1.0f, 0.0f);

	Matrix4x3 mScale(
		_frameScale.x,          0.0f,              0.0f, 0.0f,
		         0.0f, _frameScale.y,              0.0f, 0.0f,
		         0.0f,          0.0f, _frameSliceHeight, 0.0f);

	Matrix4x3 m = _view->_sliceViewMatrix * (mTransition * (mRotation * (mOffset * mScale)));

	for (int i = 0; i < 12; ++i) {
		Vector3 t = m * _shadowPolygonDefault[i];
		if (t.z > 0.0f) {
			_shadowPolygonCurrent[i] = Vector3(
				_view->_viewportPosition.x + t.x / t.z * _view->_viewportPosition.z,
				_view->_viewportPosition.y + t.y / t.z * _view->_viewportPosition.z,
				t.z * 25.5f
			);
		} else {
			_shadowPolygonCurrent[i] = Vector3(0.0f, 0.0f, 0.0f);
		}
	}

	drawShadowPolygon(transparency, surface, zbuffer);
}

void SliceRenderer::drawShadowPolygon(int transparency, Graphics::Surface &surface, uint16 *zbuffer) {
	// this simplified polygon drawing algo is in the game

	int yMax = 0;
	int yMin = 480;
	uint16 zMin = 65535;

	int polygonLeft[480] = {};
	int polygonRight[480] = {};

	int iNext = 11;
	for (int i = 0; i < 12; ++i) {
		int xCurrent = _shadowPolygonCurrent[i].x;
		int yCurrent = _shadowPolygonCurrent[i].y;
		int xNext = _shadowPolygonCurrent[iNext].x;
		int yNext = _shadowPolygonCurrent[iNext].y;

		if (yCurrent < yMin) {
			yMin = yCurrent;
		}
		if (yCurrent > yMax) {
			yMax = yCurrent;
		}
		if (_shadowPolygonCurrent[i].z < zMin) {
			zMin = _shadowPolygonCurrent[i].z;
		}

		int xDelta = abs(xNext - xCurrent);
		int yDelta = abs(yNext - yCurrent);

		int xDirection = -1;
		if (xCurrent < xNext) {
			xDirection = 1;
		}

		int xCounter = 0;

		int x = xCurrent;
		int y = yCurrent;

		if (yCurrent > yNext) {
			while (y >= yNext) {
				if (y >= 0 && y < 480) {
					polygonLeft[y] = x;
				}
				xCounter += xDelta;
				while (xCounter >= yDelta) {
					x += xDirection;
					xCounter -= yDelta;
				}
				--y;
			}
		} else if (yCurrent < yNext) {
			while (y <= yNext) {
				if (y >= 0 && y < 480) {
					polygonRight[y] = x;
				}
				xCounter += xDelta;
				while (xCounter >= yDelta) {
					x += xDirection;
					xCounter -= yDelta;
				}
				++y;
			}
		}
		iNext = (iNext + 1) % 12;
	}

	yMax = CLIP(yMax, 0, 480);
	yMin = CLIP(yMin, 0, 480);

	int ditheringFactor[] = {
		0,  8,  2, 10,
		12, 4, 14,  6,
		3, 11,  1,  9,
		15, 7, 13,  5
	};

	for (int y = yMin; y < yMax; ++y) {
		int xMin = CLIP(polygonLeft[y], 0, 640);
		int xMax = CLIP(polygonRight[y], 0, 640);

		for (int x = MIN(xMin, xMax); x < MAX(xMin, xMax); ++x) {
			uint16 z = zbuffer[x + y * 640];
			void *pixel = surface.getBasePtr(CLIP(x, 0, surface.w - 1), CLIP(y, 0, surface.h - 1));

			if (z >= zMin) {
				int index = (x & 3) + ((y & 3) << 2);
				if (transparency - ditheringFactor[index] <= 0) {
					uint8 r, g, b;
					surface.format.colorToRGB(READ_UINT32(pixel), r, g, b);
					r *= 0.75f;
					g *= 0.75f;
					b *= 0.75f;

					drawPixel(surface, pixel, surface.format.RGBToColor(r, g, b));
				}
			}
		}
	}
}

void SliceRenderer::preload(int animationId) {
	int frameCount = _vm->_sliceAnimations->getFrameCount(animationId);
	for (int i = 0; i < frameCount; ++i) {
		_vm->_sliceAnimations->getFramePtr(animationId, i);
	}
}

void SliceRenderer::disableShadows(int animationsIdsList[], int listSize) {
	for (int i = 0; i < listSize; ++i) {
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

	_cacheRecalculation = 0.0f;
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
