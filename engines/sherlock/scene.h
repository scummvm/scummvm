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
 */

#ifndef SHERLOCK_SCENE_H
#define SHERLOCK_SCENE_H

#include "common/scummsys.h"
#include "sherlock/sprite.h"

namespace Sherlock {

#define SCENES_COUNT 63

class SherlockEngine;

class Scene {
private:
	SherlockEngine *_vm;
public:
	bool _stats[SCENES_COUNT][9];
	bool _savedStats[SCENES_COUNT][9];
	int _goToRoom;
	Common::Point _bigPos;
	Common::Point _overPos;
	int _oldCharPoint;
	Sprite *_controlSprites;
	int _numExits;
public:
	Scene(SherlockEngine *vm);

	~Scene();
};

} // End of namespace Sherlock

#endif
