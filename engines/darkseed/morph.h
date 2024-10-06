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
#ifndef DARKSEED_MORPH_H
#define DARKSEED_MORPH_H

#include "common/rect.h"
#include "graphics/managed_surface.h"

namespace Darkseed {

enum class MorphDirection : uint8 {
	Forward,
	Backward
};

class Morph {
private:
	Common::Rect _area;
	Graphics::ManagedSurface _src;
	Graphics::ManagedSurface _dest;
	MorphDirection _direction = MorphDirection::Forward;
	int16 _stepCount = 0;
public:
	explicit Morph(const Common::Rect &area);

	void loadSrcFromScreen();
	void loadDestFromScreen();

	void start(MorphDirection direction);

	bool morphStep();
private:
	void draw(int16 drawIdx);
};

} // End of namespace Darkseed

#endif // DARKSEED_MORPH_H
