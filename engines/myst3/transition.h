/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef TRANSITION_H_
#define TRANSITION_H_

#include "engines/myst3/myst3.h"

#include "engines/myst3/gfx.h"

namespace Myst3 {

class FrameLimiter;

class Transition {
public:
	Transition(Myst3Engine *vm);
	virtual ~Transition();

	void draw(TransitionType type);

private:
	void drawStep(Texture *targetTexture, Texture *sourceTexture, uint completion);
	int computeDuration();
	void playSound();

	Myst3Engine *_vm;
	FrameLimiter *_frameLimiter;
	TransitionType _type;

	Texture *_sourceScreenshot;

};

} // End of namespace Myst3

#endif // TRANSITION_H_
