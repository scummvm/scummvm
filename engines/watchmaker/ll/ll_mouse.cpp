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

#include "watchmaker/ll/ll_mouse.h"
#include "watchmaker/utils.h"
#include "watchmaker/message.h"
#include "watchmaker/game.h"
#include "watchmaker/schedule.h"
#include "watchmaker/classes/do_camera.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/renderer.h"

namespace Watchmaker {

int32 mPosy = 0, mPosx = 0, mMoveX = 0, mMoveY = 0, mMove = 0, mCounter = 0, mHotspotX = 0, mHotspotY = 0;
uint8 bLPressed = 0, bRPressed = 0, mHide = 1, bSkipped = 0;
uint8 bLPressedPrev = 0, bRPressedPrev = 0;

/* -----------------08/05/98 11.47-------------------
 *                  ProcessMouse
 * --------------------------------------------------*/
void ProcessMouse(WGame &game) {
	t3dF32 diffx, diffy;
	int32 fittedx, fittedy;

	if (mMoveX || mMoveY) {
		mHide = 0;
		mCounter = 0;
		game._messageSystem.removeEvent(EventClass::MC_MOUSE, ME_MOUSEUPDATE);
		_vm->_messageSystem.doEvent(EventClass::MC_MOUSE, ME_MOUSEUPDATE, MP_DEFAULT, (int16)mPosx, (int16)mPosy, 0, &mMoveX, &mMoveY, NULL);
	}
	mMoveX = mMoveY = 0;

	diffx = 0.0f;
	diffy = 0.0f;
	game._cameraMan->CamAngleX = 0.0f;
	game._cameraMan->CamAngleY = 0.0f;
	fittedx = 0;
	fittedy = 0;

	fittedx = game._renderer->rInvFitX(mPosx);
	if (fittedx < (SCREEN_RES_X / 2)) {
		if (fittedx < 50) {
			diffx = - ((50.f - (t3dF32)fittedx) / 50.f);
		}

		if (fittedx < 5)    diffx *= 1.4f;
	} else {
		if ((SCREEN_RES_X - fittedx) < 50) {
			diffx = ((50.f - (t3dF32)(SCREEN_RES_X - fittedx)) / 50.f);
		}

		if ((SCREEN_RES_X - fittedx) < 5) diffx *= 1.4f;
	}


	fittedy = game._renderer->rInvFitY(mPosy);
	if (fittedy < (SCREEN_RES_Y / 2)) {
		if (fittedy < 50) {
			diffy = - ((50.f - (t3dF32)fittedy) / 50.f);
		}

		if (fittedy < 5)    diffy *= 1.4f;
	} else {
		if ((SCREEN_RES_Y - fittedy) < 50) {
			diffy = ((50.f - (t3dF32)(SCREEN_RES_Y - fittedy)) / 50.f);
		}

		if ((SCREEN_RES_Y - fittedy) < 5) diffy *= 1.4f;
	}

	auto windowInfo = game._renderer->getScreenInfos();
	if (mPosx > (int32)windowInfo.width) mPosx = windowInfo.width - 1;
	else if (mPosx <= 0) mPosx = 1;
	if (mPosy > (int32)windowInfo.height) mPosy = windowInfo.height - 1;
	else if (mPosy <= 0) mPosy = 1;

	game._cameraMan->MoveHeadAngles(diffx, diffy);
}
/* -----------------19/10/98 15.18-------------------
 *      DInputMouseGetCoords
 * --------------------------------------------------*/
void HandleMouseChanges() {
	// Mouse movement will have been accumulated prior to calling this function.
	// Button flags may also have been changed, this function then applies the button changes.

	//warning("L: %d %d R: %d %d", bLPressed, bLPressedPrev, bRPressed, bRPressedPrev);
	// Button 0 pressed or released
	if (bLPressed != bLPressedPrev) {
		// se ha rilasciato e non ha mosso il mouse
		if ((!bLPressed) && (!bSkipped) && ((mMove < 10) || (!(InvStatus & INV_MODE2) && !bFirstPerson && !bT2DActive))) {
			_vm->_messageSystem.doEvent(EventClass::MC_MOUSE, ME_MLEFT, MP_DEFAULT, (int16)mPosx, (int16)mPosy, bRPressed, NULL, NULL, NULL);
			bSkipped = FALSE;
		} else if (bLPressed && (mMove >= 10) && (InvStatus & INV_MODE2) && (bSomeOneSpeak)) {
			bSkipTalk = TRUE;
			bSkipped = TRUE;
		} else if (!bLPressed)
			bSkipped = FALSE;
		mMove = 0;
	}
	// Button 1 pressed or released
	if (bRPressed != bRPressedPrev) {
		if ((!bRPressed) && ((mMove < 10) || (!bFirstPerson && !bT2DActive)))
			_vm->_messageSystem.doEvent(EventClass::MC_MOUSE, ME_MRIGHT, MP_DEFAULT, (int16)mPosx, (int16)mPosy, bLPressed, NULL, NULL, NULL);
		mMove = 0;
	}
	bLPressedPrev = bLPressed;
	bRPressedPrev = bRPressed;
}

} // End of namespace Watchmaker
