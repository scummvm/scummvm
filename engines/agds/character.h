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

#ifndef AGDS_CHARACTER_H
#define AGDS_CHARACTER_H

#include "common/scummsys.h"
#include "common/rect.h"

namespace Common	{ class SeekableReadStream; }
namespace Graphics	{ struct Surface; }
namespace Video		{ class FlicDecoder; }

namespace AGDS {

class AGDSEngine;
class Object;

class Character {
	int _counter;

public:
	Character(): _counter(-1) {
	}

	bool load(Common::SeekableReadStream* stream) {
		return true;
	}

	void enable(bool enabled = true) {
	}

	void animate(int frames) {
		_counter = frames;
	}

	int getPhase() const {
		return _counter;
	}

	void paint(AGDSEngine & engine, Graphics::Surface & backbuffer) {
		if (_counter >= 0)
			--_counter;
	}

};


} // End of namespace AGDS

#endif /* AGDS_ANIMATION_H */
