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

#include "common/textconsole.h"
#include "tetraedge/game/credits.h"

namespace Tetraedge {

Credits::Credits() {
}

void Credits::enter(bool flag) {
	error("TODO: Implement Credits::enter");
}

void Credits::leave() {
	error("TODO: Implement Credits::leave");
}

bool Credits::onAnimFinished() {
	leave();
	return false;
}

bool Credits::onBackgroundAnimFinished() {
	//TeLayout *buttonsLayout = _gui.layout("buttons");
	error("TODO: Implement Credits::onBackgroundAnimFinished");
}

bool Credits::onPadButtonUp(uint button) {
	// Original calls this function here but it seems unnecessary?
	//TeLayout *buttonsLayout = _gui.layout("buttons");
	if (button & 2) // TODO; which button is 2?
		leave();
	return false;
}

bool Credits::onQuitButton() {
	TeCurveAnim2<TeI3DObject2, TeVector3f32> *anim1 = _gui.layoutPositionLinearAnimation("scrollTextPositionAnim");
	anim1->stop();
	TeCurveAnim2<TeI3DObject2, TeVector3f32> *anim2 = _gui.layoutPositionLinearAnimation("scrollTextAnchorAnim");
	anim2->stop();
	leave();
	return true;
}


} // end namespace Tetraedge
