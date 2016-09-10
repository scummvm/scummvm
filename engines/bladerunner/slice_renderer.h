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

#include "bladerunner/vector.h"
#include "bladerunner/view.h"
#include "bladerunner/matrix.h"

#include "graphics/surface.h"

namespace Common {
	class MemoryReadStream;
}

namespace BladeRunner {

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

	View      _view;
	Lights     *_lights;
	SetEffects *_setEffects;

	void       *_sliceFramePtr;

	// Animation frame data
	Vector2 _frameFront;
	float   _frameBottomZ;
	Vector2 _framePos;
	float   _frameSliceHeight;
	uint32  _framePaletteIndex;
	uint32  _frameSliceCount;

	Matrix3x2 _field_109E;
	Vector3   _field_10B6;
	Vector3   _field_10C2;
	float     _field_10CE;
	float     _field_10D2;
	int       _minX;
	int       _maxX;
	int       _minY;
	int       _maxY;

	int _t1[256];
	int _t2[256];
	int _c3;
	int _t4[256];
	int _t5[256];
	int _c6;

	bool _animationsShadowEnabled[997];

	Matrix3x2 calculateFacingRotationMatrix();
	void drawSlice(int slice, uint16 *frameLinePtr, uint16 *zbufLinePtr);

public:
	SliceRenderer(BladeRunnerEngine *vm);
	~SliceRenderer();

	void setView(const View &view);
	void setLights(Lights *lights);
	void setSetEffects(SetEffects *setEffects);

	void setupFrame(int animation, int frame, Vector3 position, float facing, float scale = 1.0f);
	void calculateBoundingRect();

	void drawFrame(Graphics::Surface &surface, uint16 *zbuffer);

	void preload(int animationId);

	void disableShadows(int* animationsIdsList, int listSize);
};

} // End of namespace BladeRunner

#endif
