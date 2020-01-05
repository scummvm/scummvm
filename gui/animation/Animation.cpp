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

#include "gui/animation/Animation.h"

namespace GUI {

Animation::Animation()
		: _startTime(0), _duration(0), _finished(false), _finishOnEnd(true) {
}

Animation::~Animation() {
}

void Animation::start(long currentTime) {
	_finished = false;
	_startTime = currentTime;
}

void Animation::setDuration(long duration) {
	_duration = duration;
}

void Animation::update(Drawable *drawable, long currentTime) {
	float interpolation;

	if (currentTime < _startTime) {
		// If the start time is in the future, nothing changes - the interpolated value is 0
		interpolation = 0;
	} else if (currentTime > _startTime + _duration) {
		// If the animation is finished, the interpolated value is 1 and the animation is marked as finished
		interpolation = 1;
		finishAnimation();
	} else {
		// Calculate the interpolated value
		interpolation = (currentTime - _startTime) / (float) (_duration);
	}

	// Activate the interpolator if present
	if (_interpolator.get() != nullptr) {
		interpolation = _interpolator->interpolate(interpolation);
	}

	updateInternal(drawable, interpolation);
}

void Animation::finishAnimation() {
	if (_finishOnEnd) {
		_finished = true;
	}
}

void Animation::updateInternal(Drawable *drawable, float interpolation) {
	// Default implementation
}

bool Animation::isFinished() const {
	return _finished;
}

bool Animation::isFinishOnEnd() const {
	return _finishOnEnd;
}

void Animation::setFinishOnEnd(bool finishOnEnd) {
	_finishOnEnd = finishOnEnd;
}

InterpolatorPtr Animation::getInterpolator() const {
	return _interpolator;
}

void Animation::setInterpolator(InterpolatorPtr interpolator) {
	_interpolator = interpolator;
}

} // End of namespace GUI
