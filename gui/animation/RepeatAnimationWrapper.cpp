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

// Based on code by omergilad.

#include "gui/animation/RepeatAnimationWrapper.h"

namespace GUI {

void RepeatAnimationWrapper::update(Drawable* drawable, long currentTime) {
	// Update wrapped animation
	_animation->update(drawable, currentTime);

	// If the animation is finished, increase the repeat count and restart it if needed
	if (_animation->isFinished()) {
		++_repeatCount;
		if (_timesToRepeat > 0 && _repeatCount >= _timesToRepeat) {
			finishAnimation();
		} else {
			_animation->start(currentTime);
		}
	}
}

void RepeatAnimationWrapper::start(long currentTime) {
	Animation::start(currentTime);
	_repeatCount = 0;

	// Start wrapped animation
	_animation->start(currentTime);
}

} // End of namespace GUI
