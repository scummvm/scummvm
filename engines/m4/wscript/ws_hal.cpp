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

#include "m4/wscript/ws_hal.h"
#include "m4/core/errors.h"
#include "m4/globals.h"

namespace M4 {

void KillCCB(CCB *myCCB, bool restoreFlag) {
	error("TODO: KillCCB");
#ifdef TODO
	if (!myCCB) {
		error_show(FL, 'WSIC');
	}
	if (restoreFlag && (!(myCCB->flags & CCB_SKIP)) && (!(myCCB->flags & CCB_HIDE))) {
		if ((myCCB->flags & CCB_STREAM) && myCCB->maxArea) {
			vmng_AddRectToRectList(&deadRectList, myCCB->maxArea->x1, myCCB->maxArea->y1,
				myCCB->maxArea->x2, myCCB->maxArea->y2);
		} else {
			vmng_AddRectToRectList(&deadRectList, myCCB->currLocation->x1, myCCB->currLocation->y1,
				myCCB->currLocation->x2, myCCB->currLocation->y2);
		}
	}
	if (myCCB->flags & CCB_DISC_STREAM) {
		ws_CloseSSstream(myCCB);
	}
	if (myCCB->currLocation) {
		mem_free((void *)myCCB->currLocation);
	}
	if (myCCB->newLocation) {
		mem_free((void *)myCCB->newLocation);
	}
	if (myCCB->maxArea) {
		mem_free((void *)myCCB->maxArea);
	}
	if (myCCB->source) {
		mem_free((char *)myCCB->source);
	}
	mem_free((void *)myCCB);
#endif
}

} // End of namespace M4
