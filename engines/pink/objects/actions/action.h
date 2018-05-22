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

#ifndef PINK_ACTION_H
#define PINK_ACTION_H

#include "pink/objects/object.h"

namespace Pink {

class Actor;
class Director;

class Action : public NamedObject {
public:
	virtual void deserialize(Archive &archive);
	virtual void start(bool unk) {};
	virtual void end() {};
	virtual void update() {};
	virtual void toConsole() {};

	virtual bool initPalette(Director *director) { return 0; }

	Actor *getActor() { return _actor; }

	virtual void pause() {};
	virtual void unpause() {};

protected:
	Actor *_actor;
};

} // End of namespace Pink

#endif
