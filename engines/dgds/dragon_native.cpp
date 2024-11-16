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

#include "dgds/dragon_native.h"
#include "dgds/includes.h"
#include "dgds/globals.h"
#include "dgds/scene.h"

namespace Dgds {


/*static*/
void DragonNative::drawCountdown(FontManager::FontType fontType, int16 x, int16 y) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	int16 countdownEnd = engine->getGameGlobals()->getGlobal(0x22);
	int16 currentMins = engine->getClock().getMins();
	const DgdsFont *fnt = engine->getFontMan()->getFont(fontType);
	Common::String str = Common::String::format("%d", countdownEnd - currentMins);
	fnt->drawString(&engine->_compositionBuffer, str, x, y, SCREEN_WIDTH - x, 10);
}


// The first row of this array corresponds to the
// positions of buttons in game passcode
// RYP YWP YRPWRY PBW
static const uint16 DRAGON_PASSCODE[] = {
	1, 4, 3, 4, 0, 3, 4, 1, 3, 0, 1, 4, 3, 2, 0,
	4, 4, 2, 3, 4, 0, 0, 4, 3, 2, 1, 1, 2, 4, 0,
	4, 1, 3, 2, 0, 2, 1, 4, 3, 4, 1, 3, 2, 0, 1
};

static uint16 passcodeBlockNum = 0;
static uint16 passcodeVal1 = 0;
static uint16 passcodeVal2 = 0;
static uint16 passcodeVal3 = 0;
static uint16 passcodeVal4 = 0;

/*static*/
 void DragonNative::updatePasscodeGlobal() {
	GDSScene *gdsScene = DgdsEngine::getInstance()->getGDSScene();
	int16 globalval = gdsScene->getGlobal(0x20);

	if (globalval > 34)
		return;

	if (globalval >= 30) {
		// One of the keypad buttons
		if (DRAGON_PASSCODE[passcodeVal4 + passcodeBlockNum * 15] == globalval - 30) {
			debug(1, "sceneOpUpdatePasscodeGlobal CORRECT: variables %d %d %d %d block %d, curval %d",
				passcodeVal1, passcodeVal2, passcodeVal3, passcodeVal4, passcodeBlockNum, globalval);

			// Correct entry! Increment the expected button
			passcodeVal4++;
			if (passcodeVal4 < passcodeVal3) {
				globalval = 0;
			} else if (passcodeVal3 < 15) {
				globalval = 5;
			} else {
				// Finished!
				globalval = 6;
			}
		} else {
			// Mistake
			debug(1, "sceneOpUpdatePasscodeGlobal WRONG: variables %d %d %d %d block %d, curval %d",
				passcodeVal1, passcodeVal2, passcodeVal3, passcodeVal4, passcodeBlockNum, globalval);
			passcodeVal1 = 0;
			passcodeVal2 = 5;
			globalval = 7;
		}
	} else {
		if (globalval > 4 || globalval == 0)
			return;

		debug(1, "sceneOpUpdatePasscodeGlobal OTHER: variables %d %d %d %d block %d, curval %d",
				passcodeVal1, passcodeVal2, passcodeVal3, passcodeVal4, passcodeBlockNum, globalval);

		if (globalval < 4) {
			passcodeBlockNum = globalval - 1; // expect block globalval-1
			passcodeVal1 = 5;
			passcodeVal2 = 0;
			passcodeVal3 = 15;	// 15 buttons expected
			passcodeVal4 = 0;
			return;
		} else if (passcodeVal2 > passcodeVal1) {
			passcodeVal1++;
			globalval = DRAGON_PASSCODE[passcodeVal1 + passcodeBlockNum * 15] + 20;
		} else if (passcodeVal2 > 14) {
			passcodeVal1 = 0;
			passcodeVal3 = passcodeVal2;
			passcodeVal4 = 0;
			globalval = 8;
		} else {
			passcodeVal1 = 0;
			passcodeVal2 += 5;
			passcodeVal3 = passcodeVal1;
			passcodeVal4 = 0;
			globalval = 8;
		}
	}

	gdsScene->setGlobal(0x20, globalval);
}


} // end namespace Dgds
