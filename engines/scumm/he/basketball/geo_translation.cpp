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

#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"

#include "scumm/he/basketball/geo_translations.h"

namespace Scumm {

int LogicHEBasketball::u32_userInitScreenTranslations() {
	// Find the angle between the left and bottom baseline in the court image...
	_courtAngle = atan(TRANSLATED_MAX_Y / (double)TRANSLATED_MAX_START_X);

	// The relationship between the location in the game world and the location in pixels
	// from the bottom of the court can be described by the parametric equation:
	// 
	// y = [(x-c)^(1/2) - c^(1/2)] / a^(1/2)
	//
	// Here, x is worldPoint.y and y is pixelsFromBottom. This is where we calculate the constant
	// coefficients a, b, and c.
	_yTranslationA = (float)((((MAX_WORLD_Y / 2.0) * TRANSLATED_MAX_Y) - (MAX_WORLD_Y * (float)TRANSLATED_MID_Y)) / ((TRANSLATED_MID_Y * TRANSLATED_MID_Y * (float)TRANSLATED_MAX_Y) - (TRANSLATED_MAX_Y * TRANSLATED_MAX_Y * (float)TRANSLATED_MID_Y)));
	assert(_yTranslationA != 0);

	_yTranslationB = (MAX_WORLD_Y / (float)TRANSLATED_MAX_Y) - (_yTranslationA * TRANSLATED_MAX_Y);
	_yTranslationC = (_yTranslationB * _yTranslationB) / (4.0 * _yTranslationA);
	assert(_yTranslationC != 0);

	// The relationship between the location in pixels from the bottom of the court and
	// the location in world points can be described by the parametric equation:
	// 
	// y = ax^2 + bx +c
	// 
	// Here, x is pixelsFromBottom and y is worldPoint.y. This is where we calculate the constant
	// coefficients a, b, and c.
	_yRevTranslationA = (((MAX_WORLD_Y / (float)2) * TRANSLATED_MAX_Y) - (MAX_WORLD_Y * (float)TRANSLATED_MID_Y)) / ((TRANSLATED_MID_Y * TRANSLATED_MID_Y * (float)TRANSLATED_MAX_Y) - (TRANSLATED_MAX_Y * TRANSLATED_MAX_Y * (float)TRANSLATED_MID_Y));
	_yRevTranslationB = (MAX_WORLD_Y / (float)TRANSLATED_MAX_Y) - (_yRevTranslationA * TRANSLATED_MAX_Y);
	_yRevTranslationC = 0;

	// As you move up the screen, the number of world points per screen pixel increases
	// parametrically. Vice versa for moving down the screen. It may be desirable to
	// have a top and bottom cutoff point. So there will be a point above the court where
	// the point to pixel ratio stops increasing and a point below the court where the point
	// to pixel ratio stops decreasing. Here, the corresponding world points are calculated.
	_topScalingPointCutoff = _vm->_basketball->u32FloatToInt(
		_yRevTranslationA * TOP_SCALING_PIXEL_CUTOFF * TOP_SCALING_PIXEL_CUTOFF +
		_yRevTranslationB * TOP_SCALING_PIXEL_CUTOFF +
		_yRevTranslationC);

	_bottomScalingPointCutoff = _vm->_basketball->u32FloatToInt(
		_yRevTranslationA * BOTTOM_SCALING_PIXEL_CUTOFF * BOTTOM_SCALING_PIXEL_CUTOFF +
		_yRevTranslationB * BOTTOM_SCALING_PIXEL_CUTOFF +
		_yRevTranslationC);

	assert(_topScalingPointCutoff >= MAX_WORLD_Y);
	assert(_bottomScalingPointCutoff <= 0);

	return 1;
}

static U32FltPoint2D worldToScreenTranslation(const U32FltPoint3D &worldPoint, LogicHEBasketball *logic) {
	U32FltPoint2D screenPoint; // The point on the screen that corresponds to worldPoint
	float courtWidth;          // The width of the court in pixels at the at the current y location
	float xOffset;             // The number of pixels from the left side of the screen to the court at the current y location

	float pixelsFromBottom;
	float zToYOffset; // Given the current world z coordinate, how many pixels in the y direction should the object be moved
	float a, c;       // The constant coefficients for the parametric equation which describes the relation between world y coordinates and screen y coordinates
	float slope;      // The derivative of the above mentioned equation

	assert(MAX_WORLD_X != 0);
	assert(MAX_WORLD_Y != 0);
	assert(TRANSLATED_MAX_START_X != 0);

	// Let's find y...

	a = logic->_yTranslationA;
	c = logic->_yTranslationC;

	// Normally, the game world coordinates compress as you move further up the screen.
	// This adds to the illusion of depth perception. Sometimes it may be desirable to stop
	// this compression after a certain point on the screen. This 'if block' handles that
	// case...
	if (worldPoint.y < logic->_bottomScalingPointCutoff) {
		slope = 1 / (2 * sqrt(a * logic->_bottomScalingPointCutoff + a * c));
		pixelsFromBottom = slope * (worldPoint.y - logic->_bottomScalingPointCutoff) + BOTTOM_SCALING_PIXEL_CUTOFF;
	} else if (worldPoint.y < logic->_topScalingPointCutoff) {
		slope = 1 / (2 * sqrt(a * worldPoint.y + a * c));
		pixelsFromBottom = (sqrt(worldPoint.y + c) - sqrt(c)) / sqrt(a);
	} else {
		slope = 1 / (2 * sqrt(a * logic->_topScalingPointCutoff + a * c));
		pixelsFromBottom = slope * (worldPoint.y - logic->_topScalingPointCutoff) + TOP_SCALING_PIXEL_CUTOFF;
	}

	screenPoint.y = BB_SCREEN_SIZE_Y - COURT_Y_OFFSET - pixelsFromBottom;

	// Let's find x...

	if (pixelsFromBottom < BOTTOM_SCALING_PIXEL_CUTOFF) {
		courtWidth = TRANSLATED_NEAR_MAX_X - (2.0 * (BOTTOM_SCALING_PIXEL_CUTOFF / tan(logic->_courtAngle)));
		xOffset = (tan((BBALL_M_PI / 2.0) - logic->_courtAngle) * BOTTOM_SCALING_PIXEL_CUTOFF) + COURT_X_OFFSET;
	} else if (pixelsFromBottom < TOP_SCALING_PIXEL_CUTOFF) {
		courtWidth = (TRANSLATED_NEAR_MAX_X - (2.0 * (pixelsFromBottom / tan(logic->_courtAngle))));
		xOffset = (tan((BBALL_M_PI / 2.0) - logic->_courtAngle) * pixelsFromBottom) + COURT_X_OFFSET;
	} else {
		// Find the width of the court in pixels at the current y coordinate...
		courtWidth = TRANSLATED_NEAR_MAX_X - (2.0 * (TOP_SCALING_PIXEL_CUTOFF / tan(logic->_courtAngle)));

		// Find the number of pixels beetween the left side of the screen and the beginning of the court at the current y coordinate...
		xOffset = tan(((BBALL_M_PI / 2.0) - logic->_courtAngle) * TOP_SCALING_PIXEL_CUTOFF) + COURT_X_OFFSET;
	}

	// Find the screen x based on the world x and y...
	screenPoint.x = (worldPoint.x * courtWidth / MAX_WORLD_X) + xOffset;

	// Now factor in world z to the screen y coordinate...
	zToYOffset = (courtWidth / MAX_WORLD_X) * worldPoint.z;
	screenPoint.y -= zToYOffset;

	return screenPoint;
}

int LogicHEBasketball::u32_userWorldToScreenTranslation(const U32FltPoint3D &worldPoint) {
	U32FltPoint2D screenPoint = worldToScreenTranslation(worldPoint, this);

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32FloatToInt(screenPoint.x));
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, _vm->_basketball->u32FloatToInt(screenPoint.y));

	return 1;
}

int LogicHEBasketball::u32_userScreenToWorldTranslation(const U32FltPoint2D &screenPoint) {
	U32FltPoint2D worldPoint; // The point in the game world that corresponds to screenPoint
	float courtWidth;         // The width of the court in pixels at the at the current y location
	float xOffset;            // The number of pixels from the left side of the screen to the court at the current y location

	float pixelsFromBottom;
	float a, b, c; // The constant coefficients for the parametric equation which describes the relation between screen y coordinates and world y coordinates
	float slope;   // The derivative of the above mentioned equation

	assert(TRANSLATED_MAX_START_X != 0);
	assert(TRANSLATED_NEAR_MAX_X != 0);
	assert(TRANSLATED_FAR_MAX_X != 0);
	assert(TRANSLATED_MAX_Y != 0);

	// Let's find y...

	a = _yRevTranslationA;
	b = _yRevTranslationB;
	c = _yRevTranslationC;

	pixelsFromBottom = BB_SCREEN_SIZE_Y - COURT_Y_OFFSET - screenPoint.y;

	// Normally, the game world coordinates compress as you move further up the screen.
	// This adds to the illusion of depth perception. Sometimes it may be desirable to stop
	// this compression after a certain point on the screen. This 'if block' handles that
	// case...
	if (pixelsFromBottom < BOTTOM_SCALING_PIXEL_CUTOFF) {
		slope = (2 * a * BOTTOM_SCALING_PIXEL_CUTOFF + b);
		worldPoint.y = slope * (pixelsFromBottom - BOTTOM_SCALING_PIXEL_CUTOFF) + _bottomScalingPointCutoff;
	} else if (pixelsFromBottom < TOP_SCALING_PIXEL_CUTOFF) {
		worldPoint.y = a * pixelsFromBottom * pixelsFromBottom + b * pixelsFromBottom + c;
	} else {
		slope = (2 * a * TOP_SCALING_PIXEL_CUTOFF + b);
		worldPoint.y = slope * (pixelsFromBottom - TOP_SCALING_PIXEL_CUTOFF) + _topScalingPointCutoff;
	}

	// -Let's find x...

	if (pixelsFromBottom < BOTTOM_SCALING_PIXEL_CUTOFF) {
		courtWidth = TRANSLATED_NEAR_MAX_X - (2.0 * (BOTTOM_SCALING_PIXEL_CUTOFF / tan(_courtAngle)));
		xOffset = (tan((BBALL_M_PI / 2.0) - _courtAngle) * BOTTOM_SCALING_PIXEL_CUTOFF) + COURT_X_OFFSET;
	} else if (pixelsFromBottom < TOP_SCALING_PIXEL_CUTOFF) {
		// Find the width of the court in pixels at the current y coordinate...
		courtWidth = TRANSLATED_NEAR_MAX_X - (2.0 * (pixelsFromBottom / tan(_courtAngle)));

		// Find the number of pixels beetween the left side of the screen and the beginning of the court at the current y coordinate...
		xOffset = (tan((BBALL_M_PI / 2.0) - _courtAngle) * pixelsFromBottom) + COURT_X_OFFSET;
	} else {
		// Find the width of the court in pixels at the current y coordinate...
		courtWidth = TRANSLATED_NEAR_MAX_X - (2.0 * (TOP_SCALING_PIXEL_CUTOFF / tan(_courtAngle)));

		// Find the number of pixels beetween the left side of the screen and the beginning of the court at the current y coordinate...
		xOffset = (tan((BBALL_M_PI / 2.0) - _courtAngle) * TOP_SCALING_PIXEL_CUTOFF) + COURT_X_OFFSET;
	}

	// Find the world x based on the screen x and y...
	assert(courtWidth != 0);
	worldPoint.x = (screenPoint.x - xOffset) * MAX_WORLD_X / courtWidth;

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32FloatToInt(worldPoint.x));
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, _vm->_basketball->u32FloatToInt(worldPoint.y));

	return 1;
}

int LogicHEBasketball::u32_userGetCourtDimensions() {
	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32FloatToInt(MAX_WORLD_X));
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, _vm->_basketball->u32FloatToInt(MAX_WORLD_Y));
	writeScummVar(_vm1->VAR_U32_USER_VAR_C, _vm->_basketball->u32FloatToInt(BASKET_X));
	writeScummVar(_vm1->VAR_U32_USER_VAR_D, _vm->_basketball->u32FloatToInt(BASKET_Y));
	writeScummVar(_vm1->VAR_U32_USER_VAR_E, _vm->_basketball->u32FloatToInt(BASKET_Z));

	return 1;
}

int LogicHEBasketball::u32_userComputePointsForPixels(int pixels, int screenYPos) {
	float points;
	float pixelsFromBottom;
	float courtWidth;
	float courtAngle;

	courtAngle = atan(TRANSLATED_MAX_Y / (double)TRANSLATED_MAX_START_X);
	pixelsFromBottom = BB_SCREEN_SIZE_Y - COURT_Y_OFFSET - screenYPos;

	if (pixelsFromBottom < 0) {
		courtWidth = TRANSLATED_NEAR_MAX_X;
// FIXME: Remove duplicated condition branch?
#if 0
	} else if (pixelsFromBottom < TRANSLATED_MAX_Y) {
		// Find the width of the court in pixels at the current y coordinate...
		courtWidth = TRANSLATED_NEAR_MAX_X - (2.0 * (pixelsFromBottom / tan(courtAngle)));
#endif
	} else {
		// Find the width of the court in pixels at the current y coordinate...
		courtWidth = TRANSLATED_NEAR_MAX_X - (2.0 * (pixelsFromBottom / tan(courtAngle)));
	}

	points = (MAX_WORLD_X / courtWidth) * pixels;

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32FloatToInt(points));

	return 1;
}

} // End of namespace Scumm
