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

#ifndef ULTIMA6_VIEWS_MD_SKY_STRIP_WIDGET_H
#define ULTIMA6_VIEWS_MD_SKY_STRIP_WIDGET_H

#include "ultima/ultima6/gui/gui_widget.h"
#include "ultima/ultima6/files/u6_shape.h"

namespace Ultima {
namespace Ultima6 {

class GameClock;
class Configuration;
class Player;

class MDSkyStripWidget : public GUI_Widget {

protected:
	Configuration *config;
	GameClock *clock;
	U6Shape strip1, strip2;
	Player *player;

public:
	MDSkyStripWidget(Configuration *cfg, GameClock *c, Player *p);
	~MDSkyStripWidget();

	void init(sint16 x, sint16 y);
	void Display(bool full_redraw);

private:
	void display_surface();
};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
