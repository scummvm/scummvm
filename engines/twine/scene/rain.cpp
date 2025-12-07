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

#include "twine/scene/rain.h"
#include "twine/renderer/renderer.h"
#include "twine/scene/grid.h"
#include "twine/twine.h"

namespace TwinE {

#define RAIN_VX 200
#define RAIN_VY 2500
#define RAIN_WEIGHT 30
#define RAIN_STOP 0
#define RAIN_DELTA_X 128
#define RAIN_DELTA_Y 256
#define RAIN_DELTA_Z 128

void Rain::InitOneRain(T_RAIN *pt) {
	IVec3 cameraPos = _engine->_renderer->getCameraPosition();

	int32 rndy = _engine->getRandomNumber(cameraPos.y + 10000);

	pt->YRain = cameraPos.y + rndy;

	rndy = rndy / 2 + 15000;

	pt->XRain = cameraPos.x - rndy + _engine->getRandomNumber(30000);
	pt->ZRain = cameraPos.z - rndy + _engine->getRandomNumber(30000);
	pt->Timer = 0;
}

void Rain::InitRain() {
	for (int32 i = 0; i < MAX_RAIN; i++) {
		InitOneRain(&TabRain[i]);
	}

	LastTimer = 0;
}

void Rain::GereRain() {
	int32 temp = _engine->timerRef;
	DeltaRain = LastTimer ? (temp - LastTimer) * 10 : 0;
	LastTimer = temp;

	for (int32 i = 0; i < MAX_RAIN; i++) {
		if (!TabRain[i].Timer) {
			TabRain[i].XRain += DeltaRain / 2;
			TabRain[i].ZRain += DeltaRain / 2;
			TabRain[i].YRain -= DeltaRain;
		}
	}
}

void Rain::ClearImpactRain() {
	for (int32 i = 0; i < MAX_RAIN; i++) {
		if (TabRain[i].Timer) {
			InitOneRain(&TabRain[i]);
		}
	}
}

void Rain::AffRain() {
	int32 lFactorX = _engine->_renderer->getLFactorX();
	int32 lFactorY = _engine->_renderer->getLFactorY();
	IVec3 cameraRot = _engine->_renderer->getCameraRotation();
	int32 cameraZr = cameraRot.z;

	// ClipZFar approximation
	int32 clipZFar = 14000; // Default value from CREDITS.CPP
	int32 startZFog = 5000;    // Default value from CREDITS.CPP

	for (int32 i = 0; i < MAX_RAIN; i++) {
		if (TabRain[i].Timer) {
			int32 dt = LastTimer - TabRain[i].Timer;

			int32 c = TabRain[i].XRain >> 16;
			int32 x = (int16)(TabRain[i].XRain & 0xFFFF);
			int32 y = TabRain[i].YRain;
			int32 z = TabRain[i].ZRain;

			int32 xp, yp;
			yp = (RAIN_VY - RAIN_WEIGHT * dt) * dt / 256;
			if (yp < 0) {
				yp = 0;
				xp = RAIN_VX * RAIN_VY / RAIN_WEIGHT / 256;
			} else {
				xp = RAIN_VX * dt / 256;
				yp = (yp * lFactorY) / z;
			}

			xp = (xp * lFactorX) / z;

			int32 x0 = x - xp;
			int32 x1 = x + xp;

			// int32 y0 = y - yp;
			// int32 y1 = y;

			// z = ruleThree32(0, 65535, clipZFar, z);

			// Draw splash
			_engine->_workVideoBuffer.drawLine(x, y, x0, y - yp, c);
			_engine->_workVideoBuffer.drawLine(x, y, x1, y - yp, c);

			if (dt && !yp) {
				InitOneRain(&TabRain[i]);
			}
		} else {
			if (TabRain[i].YRain <= RAIN_STOP) {
				InitOneRain(&TabRain[i]);
				continue;
			}

			IVec3 p1 = _engine->_renderer->longWorldRot(TabRain[i].XRain - RAIN_DELTA_X, TabRain[i].YRain + RAIN_DELTA_Y, TabRain[i].ZRain - RAIN_DELTA_Z);
			IVec3 proj1 = _engine->_renderer->projectPoint(p1);

			int32 xp = proj1.x;
			int32 yp = proj1.y;
			int32 z0 = ruleThree32(0, 65535, clipZFar, cameraZr - p1.z);

			IVec3 p2 = _engine->_renderer->longWorldRot(TabRain[i].XRain, TabRain[i].YRain, TabRain[i].ZRain);
			IVec3 proj2 = _engine->_renderer->projectPoint(p2);

			int32 Z0 = cameraZr - p2.z;
			// int32 z1 = ruleThree32(0, 65535, clipZFar, Z0);

			int32 c = boundRuleThree(16 * 3 + 10, 16 * 3 + 3, clipZFar - startZFog, Z0);

			// Draw rain drop
			_engine->_workVideoBuffer.drawLine(xp, yp, proj2.x, proj2.y, c);

			// Check collision with ground
			int32 groundHeight = 0;
			IVec3 pos(TabRain[i].XRain, TabRain[i].YRain, TabRain[i].ZRain);
			_engine->_grid->getBlockBufferGround(pos, groundHeight);

			if (TabRain[i].YRain <= groundHeight) {
				// Splash
				TabRain[i].XRain = ((xp & 0xFFFF) | (c << 16));
				TabRain[i].YRain = yp;
				TabRain[i].ZRain = z0;
				TabRain[i].Timer = LastTimer;
			}
		}
	}
}

Rain::Rain(TwinEEngine *engine) : _engine(engine) {
}

} // namespace TwinE
