/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm.h"
#include "intern.h"
#include "actor.h"
#include "charset.h"

void Scumm::setCameraAtEx(int at) {
	if (!(_features & GF_NEW_CAMERA)) {
		camera._mode = CM_NORMAL;
		camera._cur.x = at;
		setCameraAt(at, 0);
		camera._movingToActor = false;
	}
}

void Scumm::setCameraAt(int pos_x, int pos_y) {
	if (camera._mode != CM_FOLLOW_ACTOR || abs(pos_x - camera._cur.x) > (_screenWidth / 2)) {
		camera._cur.x = pos_x;
	}
	camera._dest.x = pos_x;

	if (camera._cur.x < VAR(VAR_CAMERA_MIN_X))
		camera._cur.x = (short) VAR(VAR_CAMERA_MIN_X);

	if (camera._cur.x > VAR(VAR_CAMERA_MAX_X))
		camera._cur.x = (short) VAR(VAR_CAMERA_MAX_X);

	if (VAR_SCROLL_SCRIPT != 0xFF && VAR(VAR_SCROLL_SCRIPT)) {
		if (_version <= 2)
			VAR(VAR_CAMERA_POS_X) = camera._cur.x / 8;
		else
			VAR(VAR_CAMERA_POS_X) = camera._cur.x;
		runScript(VAR(VAR_SCROLL_SCRIPT), 0, 0, 0);
	}

	// If the camera moved and text is visible, remove it
	if (camera._cur.x != camera._last.x && _charset->_hasMask)
		stopTalk();
}

void Scumm_v7::setCameraAt(int pos_x, int pos_y) {
	ScummVM::Point old;

	old = camera._cur;

	camera._cur.x = pos_x;
	camera._cur.y = pos_y;

	clampCameraPos(&camera._cur);

	camera._dest = camera._cur;
	VAR(VAR_CAMERA_DEST_X) = camera._dest.x;
	VAR(VAR_CAMERA_DEST_Y) = camera._dest.y;

	assert(camera._cur.x >= (_screenWidth / 2) && camera._cur.y >= (_screenHeight / 2));

	if ((camera._cur.x != old.x || camera._cur.y != old.y)
			&& VAR(VAR_SCROLL_SCRIPT)) {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x;
		VAR(VAR_CAMERA_POS_Y) = camera._cur.y;
		runScript(VAR(VAR_SCROLL_SCRIPT), 0, 0, 0);
	}
}

void Scumm::setCameraFollows(Actor *a) {

	int t, i;

	camera._mode = CM_FOLLOW_ACTOR;
	camera._follows = a->number;

	if (!a->isInCurrentRoom()) {
		startScene(a->getRoom(), 0, 0);
		camera._mode = CM_FOLLOW_ACTOR;
		camera._cur.x = a->x;
		setCameraAt(camera._cur.x, 0);
	}

	t = (a->x >> 3);

	if (t - _screenStartStrip < camera._leftTrigger || t - _screenStartStrip > camera._rightTrigger)
		setCameraAt(a->x, 0);

	for (i = 1; i < _numActors; i++) {
		if (_actors[i].isInCurrentRoom())
			_actors[i].needRedraw = true;
	}
	runInventoryScript(0);
}

void Scumm_v7::setCameraFollows(Actor *a) {

	byte oldfollow = camera._follows;
	int ax, ay;

	camera._follows = a->number;
	VAR(VAR_CAMERA_FOLLOWED_ACTOR) = a->number;

	if (!a->isInCurrentRoom()) {
		startScene(a->getRoom(), 0, 0);
	}

	ax = abs(a->x - camera._cur.x);
	ay = abs(a->y - camera._cur.y);

	if (ax > VAR(VAR_CAMERA_THRESHOLD_X) || ay > VAR(VAR_CAMERA_THRESHOLD_Y) || ax > (_screenWidth / 2) || ay > (_screenHeight / 2)) {
		setCameraAt(a->x, a->y);
	}

	if (a->number != oldfollow)
		runInventoryScript(0);
}


void Scumm::clampCameraPos(ScummVM::Point *pt) {
	if (pt->x < VAR(VAR_CAMERA_MIN_X))
		pt->x = (short) VAR(VAR_CAMERA_MIN_X);

	if (pt->x > VAR(VAR_CAMERA_MAX_X))
		pt->x = (short) VAR(VAR_CAMERA_MAX_X);

	if (pt->y < VAR(VAR_CAMERA_MIN_Y))
		pt->y = (short) VAR(VAR_CAMERA_MIN_Y);

	if (pt->y > VAR(VAR_CAMERA_MAX_Y))
		pt->y = (short) VAR(VAR_CAMERA_MAX_Y);
}

void Scumm::moveCamera() {
	int pos = camera._cur.x;
	int actorx, t;
	Actor *a = NULL;

	camera._cur.x &= 0xFFF8;

	if (camera._cur.x < VAR(VAR_CAMERA_MIN_X)) {
		if (VAR_CAMERA_FAST_X != 0xFF && VAR(VAR_CAMERA_FAST_X))
			camera._cur.x = (short) VAR(VAR_CAMERA_MIN_X);
		else
			camera._cur.x += 8;
		cameraMoved();
		return;
	}

	if (camera._cur.x > VAR(VAR_CAMERA_MAX_X)) {
		if (VAR_CAMERA_FAST_X != 0xFF && VAR(VAR_CAMERA_FAST_X))
			camera._cur.x = (short) VAR(VAR_CAMERA_MAX_X);
		else
			camera._cur.x -= 8;
		cameraMoved();
		return;
	}

	if (camera._mode == CM_FOLLOW_ACTOR) {
		a = derefActor(camera._follows, "moveCamera");

		actorx = a->x;
		t = (actorx >> 3) - _screenStartStrip;

		if (t < camera._leftTrigger || t > camera._rightTrigger) {
			if (VAR_CAMERA_FAST_X != 0xFF && VAR(VAR_CAMERA_FAST_X)) {
				if (t > 35)
					camera._dest.x = actorx + 80;
				if (t < 5)
					camera._dest.x = actorx - 80;
			} else
				camera._movingToActor = true;
		}
	}

	if (camera._movingToActor) {
		a = derefActor(camera._follows, "moveCamera(2)");
		camera._dest.x = a->x;
	}

	if (camera._dest.x < VAR(VAR_CAMERA_MIN_X))
		camera._dest.x = (short) VAR(VAR_CAMERA_MIN_X);

	if (camera._dest.x > VAR(VAR_CAMERA_MAX_X))
		camera._dest.x = (short) VAR(VAR_CAMERA_MAX_X);

	if (VAR_CAMERA_FAST_X != 0xFF && VAR(VAR_CAMERA_FAST_X)) {
		camera._cur.x = camera._dest.x;
	} else {
		if (camera._cur.x < camera._dest.x)
			camera._cur.x += 8;
		if (camera._cur.x > camera._dest.x)
			camera._cur.x -= 8;
	}

	/* a is set a bit above */
	if (camera._movingToActor && (camera._cur.x >> 3) == (a->x >> 3)) {
		camera._movingToActor = false;
	}

	cameraMoved();

	if (VAR_SCROLL_SCRIPT != 0xFF && VAR(VAR_SCROLL_SCRIPT) && pos != camera._cur.x) {
		if (_version <= 2)
			VAR(VAR_CAMERA_POS_X) = camera._cur.x / 8;
		else
			VAR(VAR_CAMERA_POS_X) = camera._cur.x;
		runScript(VAR(VAR_SCROLL_SCRIPT), 0, 0, 0);
	}
}

void Scumm_v7::moveCamera() {
	ScummVM::Point old = camera._cur;
	Actor *a = NULL;

	if (camera._follows) {
		a = derefActor(camera._follows, "moveCamera");
		if (abs(camera._cur.x - a->x) > VAR(VAR_CAMERA_THRESHOLD_X) ||
				abs(camera._cur.y - a->y) > VAR(VAR_CAMERA_THRESHOLD_Y)) {
			camera._movingToActor = true;
			if (VAR(VAR_CAMERA_THRESHOLD_X) == 0)
				camera._cur.x = a->x;
			if (VAR(VAR_CAMERA_THRESHOLD_Y) == 0)
				camera._cur.y = a->y;
			clampCameraPos(&camera._cur);
		}
	} else {
		camera._movingToActor = false;
	}

	if (camera._movingToActor) {
		VAR(VAR_CAMERA_DEST_X) = camera._dest.x = a->x;
		VAR(VAR_CAMERA_DEST_Y) = camera._dest.y = a->y;
	}

	assert(camera._cur.x >= (_screenWidth / 2) && camera._cur.y >= (_screenHeight / 2));

	clampCameraPos(&camera._dest);

	if (camera._cur.x < camera._dest.x) {
		camera._cur.x += (short) VAR(VAR_CAMERA_SPEED_X);
		if (camera._cur.x > camera._dest.x)
			camera._cur.x = camera._dest.x;
	}

	if (camera._cur.x > camera._dest.x) {
		camera._cur.x -= (short) VAR(VAR_CAMERA_SPEED_X);
		if (camera._cur.x < camera._dest.x)
			camera._cur.x = camera._dest.x;
	}

	if (camera._cur.y < camera._dest.y) {
		camera._cur.y += (short) VAR(VAR_CAMERA_SPEED_Y);
		if (camera._cur.y > camera._dest.y)
			camera._cur.y = camera._dest.y;
	}

	if (camera._cur.y > camera._dest.y) {
		camera._cur.y -= (short) VAR(VAR_CAMERA_SPEED_Y);
		if (camera._cur.y < camera._dest.y)
			camera._cur.y = camera._dest.y;
	}

	if (camera._cur.x == camera._dest.x && camera._cur.y == camera._dest.y) {

		camera._movingToActor = false;
		camera._accel.x = camera._accel.y = 0;
		VAR(VAR_CAMERA_SPEED_X) = VAR(VAR_CAMERA_SPEED_Y) = 0;
	} else {

		camera._accel.x += (short) VAR(VAR_CAMERA_ACCEL_X);
		camera._accel.y += (short) VAR(VAR_CAMERA_ACCEL_Y);

		VAR(VAR_CAMERA_SPEED_X) += camera._accel.x / 100;
		VAR(VAR_CAMERA_SPEED_Y) += camera._accel.y / 100;

		if (VAR(VAR_CAMERA_SPEED_X) < 8)
			VAR(VAR_CAMERA_SPEED_X) = 8;

		if (VAR(VAR_CAMERA_SPEED_Y) < 8)
			VAR(VAR_CAMERA_SPEED_Y) = 8;

	}

	cameraMoved();

	if (camera._cur.x != old.x || camera._cur.y != old.y) {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x;
		VAR(VAR_CAMERA_POS_Y) = camera._cur.y;

		if (VAR(VAR_SCROLL_SCRIPT))
			runScript(VAR(VAR_SCROLL_SCRIPT), 0, 0, 0);
	}
}


void Scumm::cameraMoved() {
	if (_features & GF_NEW_CAMERA) {
		assert(camera._cur.x >= (_screenWidth / 2) && camera._cur.y >= (_screenHeight / 2));
	} else {
		if (camera._cur.x < (_screenWidth / 2)) {
			camera._cur.x = (_screenWidth / 2);
		} else if (camera._cur.x > _roomWidth - (_screenWidth / 2)) {
			camera._cur.x = _roomWidth - (_screenWidth / 2);
		}
	}

	_screenStartStrip = camera._cur.x / 8 - gdi._numStrips / 2;
	_screenEndStrip = _screenStartStrip + gdi._numStrips - 1;

	_screenTop = camera._cur.y - (_screenHeight / 2);
	if (_features & GF_NEW_CAMERA) {
		_screenLeft = camera._cur.x - (_screenWidth / 2);
	} else {
		_screenLeft = _screenStartStrip << 3;
	}

#ifdef V7_SMOOTH_SCROLLING_HACK
	virtscr[0].xstart = _screenLeft;
#else
	virtscr[0].xstart = _screenStartStrip << 3;
#endif

	if (_charset->_hasMask) {
		int dx = camera._cur.x - camera._last.x;
		int dy = camera._cur.y - camera._last.y;
		if (dx || dy) {
			gdi._mask.left -= dx;
			gdi._mask.right -= dx;
			gdi._mask.top -= dy;
			gdi._mask.bottom -= dy;
		}
	}
}

void Scumm::panCameraTo(int x, int y) {
	camera._dest.x = x;
	camera._mode = CM_PANNING;
	camera._movingToActor = false;
}

void Scumm_v7::panCameraTo(int x, int y) {
	VAR(VAR_CAMERA_FOLLOWED_ACTOR) = camera._follows = 0;
	VAR(VAR_CAMERA_DEST_X) = camera._dest.x = x;
	VAR(VAR_CAMERA_DEST_Y) = camera._dest.y = y;
}

void Scumm::actorFollowCamera(int act) {
	if (!(_features & GF_NEW_CAMERA)) {
		int old;

		/* mi1 compatibilty */
		if (act == 0) {
			camera._mode = CM_NORMAL;
			camera._follows = 0;
			camera._movingToActor = false;
			return;
		}

		old = camera._follows;
		setCameraFollows(derefActor(act, "actorFollowCamera"));
		if (camera._follows != old)
			runInventoryScript(0);

		camera._movingToActor = false;
	}
}

