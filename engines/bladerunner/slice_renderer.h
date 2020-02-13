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

#ifndef BLADERUNNER_SLICE_RENDERER_H
#define BLADERUNNER_SLICE_RENDERER_H

#include "bladerunner/color.h"
#include "bladerunner/vector.h"
#include "bladerunner/view.h"
#include "bladerunner/matrix.h"

#include "common/rect.h"

#include "graphics/surface.h"

namespace Common {
class MemoryReadStream;
}

namespace BladeRunner {

class ScreenEffects;
class BladeRunnerEngine;
class Lights;
class SetEffects;

class SliceRenderer {
	BladeRunnerEngine *_vm;

	int       _animation;
	int       _frame;
	Vector3   _position;
	float     _facing;
	float     _scale;

	ScreenEffects *_screenEffects;
	View          *_view;
	Lights        *_lights;
	SetEffects    *_setEffects;

	void *_sliceFramePtr;

	// Animation frame data
	Vector2 _frameScale;
	float   _frameBottomZ;
	Vector2 _framePos;
	float   _frameSliceHeight;
	uint32  _framePaletteIndex;
	uint32  _frameSliceCount;

	Matrix3x2    _mvpMatrix;
	Vector3      _startScreenVector;
	Vector3      _endScreenVector;
	float        _startSlice;
	float        _endSlice;
	Common::Rect _screenRectangle;

	int _m11lookup[256];
	int _m12lookup[256];
	int _m13;
	int _m21lookup[256];
	int _m22lookup[256];
	int _m23;

	bool _animationsShadowEnabled[997];

	Vector3 _shadowPolygonDefault[12];
	Vector3 _shadowPolygonCurrent[12];

	Color _setEffectColor;
	Color _lightsColor;

	Graphics::PixelFormat _pixelFormat;

public:
	SliceRenderer(BladeRunnerEngine *vm);
	~SliceRenderer();

	void setScreenEffects(ScreenEffects *aesc);
	void setView(View *view);
	void setLights(Lights *lights);
	void setSetEffects(SetEffects *setEffects);

	void setupFrameInWorld(int animationId, int animationFrame, Vector3 position, float facing, float scale = 1.0f);
	void getScreenRectangle(Common::Rect *screenRectangle, int animationId, int animationFrame, Vector3 position, float facing, float scale);
	void drawInWorld(int animationId, int animationFrame, Vector3 position, float facing, float scale, Graphics::Surface &surface, uint16 *zbuffer);
	void drawOnScreen(int animationId, int animationFrame, int screenX, int screenY, float facing, float scale, Graphics::Surface &surface);

	void preload(int animationId);

	void disableShadows(int *animationsIdsList, int listSize);

private:
	void calculateBoundingRect();
	Matrix3x2 calculateFacingRotationMatrix();
	void loadFrame(int animation, int frame);

	void drawSlice(int slice, bool advanced, int y, Graphics::Surface &surface, uint16 *zbufferLine);
	void drawShadowInWorld(int transparency, Graphics::Surface &surface, uint16 *zbuffer);
	void drawShadowPolygon(int transparency, Graphics::Surface &surface, uint16 *zbuffer);
};

class SliceRendererLights {
	Lights *_lights;
	Color   _cacheColor[20];
	float   _cacheCounter[20];
	float   _cacheStart[20];
	int     _cacheRecalculation;

public:
	Color   _finalColor;

public:
	SliceRendererLights(Lights *lights);

	void calculateColorBase(Vector3 position1, Vector3 position2, float height);
	void calculateColorSlice(Vector3 position);
};

} // End of namespace BladeRunner

#endif
