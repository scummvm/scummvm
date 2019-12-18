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

#ifndef ULTIMA6_CORE_FPS_COUNTER_H
#define ULTIMA6_CORE_FPS_COUNTER_H

#include "ultima/ultima6/gui/gui_widget.h"
#include "ultima/ultima6/fonts/font.h"

namespace Ultima {
namespace Ultima6 {

class Game;

class FpsCounter: public GUI_Widget {
protected:

	Game *game;
	Font *font;

	char fps_string[7]; // "000.00\0"

public:
	FpsCounter(Game *g);
	~FpsCounter();

	void setFps(float fps);

	virtual void Display(bool full_redraw);

	void update() {
		update_display = true;
	}
};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
