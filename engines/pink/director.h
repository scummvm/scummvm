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

#ifndef PINK_DIRECTOR_H
#define PINK_DIRECTOR_H

#include "common/array.h"
#include "common/rect.h"
#include "common/system.h"

namespace Pink {

class Actor;
class ActionCEL;
class ActionSound;
class CelDecoder;

class Director {
public:
	Director(OSystem *system);
	Actor *getActorByPoint(Common::Point point);

	void draw();
	void update();

	void addSprite(ActionCEL *sprite);
	void removeSprite(ActionCEL *sprite);
	void setPallette(const byte *pallete);

	void addSound(ActionSound* sound);
	void removeSound(ActionSound* sound);

	void clear();

	bool showBounds;

private:
	void drawSprite(ActionCEL *sprite);
	OSystem *_system;
	Common::Array<ActionCEL*> _sprites;
	Common::Array<ActionSound*> _sounds;
};

} // End of namespace Pink


#endif
