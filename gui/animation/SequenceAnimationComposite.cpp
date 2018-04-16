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

#include "gui/animation/SequenceAnimationComposite.h"

namespace GUI {

void SequenceAnimationComposite::start(long currentTime) {
	Animation::start(currentTime);

	// The first animation in the sequence should a start time equal to this sequence
	if (_sequence.size() >= 1)
		_sequence[0]->start(currentTime);

	// Set the index to 0
	_index = 0;
}

void SequenceAnimationComposite::addAnimation(AnimationPtr animation) {
	_sequence.push_back(animation);
}

void SequenceAnimationComposite::update(Drawable *drawable, long currentTime) {
	uint16 sequenceSize = _sequence.size();

	// Check index bounds
	if (_index >= sequenceSize)
		return;

	// Get the current animation in the sequence
	AnimationPtr anim = _sequence[_index];

	// Update the drawable
	anim->update(drawable, currentTime);

	// Check if the current animation is finished
	if (anim->isFinished()) {
		// Increase the index - move to the next animation
		++_index;

		if (_index >= sequenceSize) {
			// Finished the sequence
			finishAnimation();
		} else {
			// Set the start time for the next animation
			_sequence[_index]->start(currentTime);
		}
	}
}

} // End of namespace GUI
