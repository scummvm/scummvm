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

#ifndef ULTIMA0_GFX_MONSTER_H
#define ULTIMA0_GFX_MONSTER_H

#include "graphics/managed_surface.h"
#include "ultima/ultima0/data/data.h"

namespace Ultima {
namespace Ultima0 {
namespace Gfx {

class Monster {
private:
	// Drawing position
	static int _xPos, _yPos;

	/**
	 * Emulate the Apple ][ HPLOT function
	 */
	static void hPlot(Graphics::ManagedSurface *s, double x, double y, ...);

	/**
	 * Monster drawing functions
	 */
	typedef void(*DrawFn)(Graphics::ManagedSurface *s, double x, double y, double d);
	static void drawSkeleton(Graphics::ManagedSurface *s, double x,double y,double d);
	static void drawThief(Graphics::ManagedSurface *s, double x,double y,double d);
	static void drawRat(Graphics::ManagedSurface *s, double x,double y,double d);
	static void drawOrc(Graphics::ManagedSurface *s, double x,double y,double d);
	static void drawViper(Graphics::ManagedSurface *s, double x,double y,double d);
	static void drawCarrion(Graphics::ManagedSurface *s, double x,double y,double d);
	static void drawGremlin(Graphics::ManagedSurface *s, double x,double y,double d);
	static void drawMimic(Graphics::ManagedSurface *s, double x,double y,double d);
	static void drawDaemon(Graphics::ManagedSurface *s, double x,double y,double d);
	static void drawBalrog(Graphics::ManagedSurface *s, double x,double y,double d);
	static DrawFn DRAW_FUNCTIONS[];

public:
	static void draw(Graphics::ManagedSurface *s, int x, int y,
		int monster, double scale);
};

} // namespace Gfx
} // namespace Ultima0
} // namespace Ultima

#endif
