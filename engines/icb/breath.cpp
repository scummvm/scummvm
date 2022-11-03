/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/debug.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_objects.h"

#include "engines/icb/icb.h"
#include "engines/icb/breath.h"

namespace ICB {

mcodeFunctionReturnCodes fn_breath(int32 &result, int32 *params) { return (MS->fn_breath(result, params)); }

// fn_breath(on/off,x,y,z);
mcodeFunctionReturnCodes _game_session::fn_breath(int32 &, int32 *params) {
	M->breath.on = (uint8)params[0];

	M->breath.Init();

	// location is not used at the moment
	M->breath.position.vx = (int16)params[1];
	M->breath.position.vy = (int16)params[2];
	M->breath.position.vz = (int16)params[3];

	return IR_CONT;
}

// init the breath, so we get a stream of breath....
Breath::Breath() {
	on = BREATH_OFF; // off as default...
}

// smoke

// var              /dt

// y (height)       +rand
// z (out)          +rand
// c (colour)       -const
// s (size)         +const
//

#define SMOKE_DY (6)
#define SMOKE_DZ (2)
#define SMOKE_DC (-2)
#define SMOKE_DS (4)

#define SMOKE_IY (0)
#define SMOKE_IZ (0)
#define SMOKE_IC (32)
#define SMOKE_IS (4)

#define BREATH_DY (-(g_icb->getRandomSource()->getRandomNumber(2 - 1)))
#define BREATH_DZ (g_icb->getRandomSource()->getRandomNumber(4 - 1))
#define BREATH_DC (-4)
#define BREATH_DS (2)

#define BREATH_IY (0)
#define BREATH_IZ (0)
#define BREATH_IC (32)
#define BREATH_IS (2)

#define BREATH_WAIT_SECONDS 2

#define BREATH_WAIT (12 * BREATH_WAIT_SECONDS * BREATH_DC + BREATH_IC) // between breaths

void Breath::Init() {
	breathEnd = (int16)(BREATH_WAIT);

	int32 i;
	for (i = 0; i < MAX_BREATH; i++) {
		breathStarted[i] = 0;

		if (on == BREATH_ON) {
			breathColour[i] = (int16)(breathEnd - (BREATH_DC * i));
		} else { // smoke...
			breathColour[i] = (int16)(breathEnd - (SMOKE_DC * i));
		}
	}

	// reset cycles count (smoke uses this to turn off)
	allStarted = 0;
}

// update the breath every cycle
void Breath::Update() {
	if (on) {
		int32 i;
		int32 numberStarted;

		numberStarted = 0;

		// update each particle
		for (i = 0; i < MAX_BREATH; i++) {
			// update
			if (on == BREATH_ON) {
				breathZ[i] = (int8)(breathZ[i] + BREATH_DZ);
				breathY[i] = (int8)(breathY[i] + BREATH_DY);
				breathSize[i] += BREATH_DS;
				breathColour[i] += BREATH_DC;
			} else { // smoking
				breathZ[i] += SMOKE_DZ;
				breathY[i] += SMOKE_DY;
				breathSize[i] += SMOKE_DS;
				breathColour[i] += SMOKE_DC;
			}

			// turn off

			// if we have all started and we are ending and we are smoking then reset...
			if ((breathColour[i] <= breathEnd) && (allStarted) && (on == BREATH_SMOKE)) {
				breathStarted[i] = 1;
			}
			// otherwise reset it
			else if (breathColour[i] <= breathEnd) {
				// set values
				if (on == BREATH_ON) {
					breathZ[i] = BREATH_IZ;
					breathY[i] = BREATH_IY;
					breathSize[i] = BREATH_IS;
					breathColour[i] = BREATH_IC;
				} else { // smoke
					breathZ[i] = SMOKE_IZ;
					breathY[i] = SMOKE_IY;
					breathSize[i] = SMOKE_IS;
					breathColour[i] = SMOKE_IC;
				}

				// we have definitely started now!
				breathStarted[i] = 1;
			}

			// for testing if all on (for smoke)

			// to see if all have started
			if (breathStarted[i])
				numberStarted++;
		}

		if (on == BREATH_SMOKE) {
			// if all have stopped and we are smoking then time to turn off
			if ((numberStarted == MAX_BREATH) && (allStarted)) {
				on = BREATH_OFF;
			}
			// if all started set it
			else if (numberStarted == MAX_BREATH) {
				allStarted = 1;

				// reset this field (when they are all set again we can turn off)
				for (i = 0; i < MAX_BREATH; i++)
					breathStarted[i] = 0;
			}
		}
	}
}

} // End of namespace ICB
