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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef PARALLACTION_WALK_H
#define PARALLACTION_WALK_H

#include "common/ptr.h"
#include "common/list.h"

#include "parallaction/objects.h"


namespace Parallaction {

struct Character;

class PathBuilder {

protected:
	Character *_ch;

public:
	PathBuilder(Character *ch) : _ch(ch) { }
	virtual ~PathBuilder() { }

	virtual void buildPath(uint16 x, uint16 y) = 0;
};


class PathBuilder_NS : public PathBuilder {

	PointList	_subPath;

	void correctPathPoint(Common::Point &to);
	uint32 buildSubPath(const Common::Point& pos, const Common::Point& stop);
	uint16 walkFunc1(const Common::Point &to, Common::Point& node);

public:
	PathBuilder_NS(Character *ch);
	void buildPath(uint16 x, uint16 y);
};


class PathBuilder_BR : public PathBuilder {

	bool directPathExists(const Common::Point &from, const Common::Point &to);

public:
	PathBuilder_BR(Character *ch);
	void buildPath(uint16 x, uint16 y);
};

class PathWalker {
protected:
	Character	*_ch;
public:
	PathWalker(Character *ch) : _ch(ch) { }
	virtual ~PathWalker() { }
	virtual void walk() = 0;
};

class PathWalker_NS : public PathWalker {


	void finalizeWalk();
	void clipMove(Common::Point& pos, const Common::Point& to);
	void checkDoor(const Common::Point &foot);

public:
	PathWalker_NS(Character *ch) : PathWalker(ch) { }
	void walk();
};


class PathWalker_BR : public PathWalker {


	int			_walkDelay;
	int			_fieldC;
	Common::Point _startFoot;
	bool		_first;
	int			_step;

	int			_dirFrame;

	void finalizeWalk();

public:
	PathWalker_BR(Character *ch);
	void walk();
};

}

#endif
