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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_PUZZLE_H
#define ASYLUM_PUZZLE_H

#include "asylum/eventhandler.h"
#include "asylum/shared.h"

#include "common/rect.h"

namespace Asylum {

class AsylumEngine;
class Screen;
class Sound;
class Video;
class Cursor;
class GraphicResource;
struct GraphicQueueItem;

class Puzzle : public EventHandler {
public:
	Puzzle(AsylumEngine *engine);
	virtual ~Puzzle();

	virtual void reset() {}

	//////////////////////////////////////////////////////////////////////////
	// TODO remove
	void init();
	virtual void open() = 0;
	virtual void close() = 0;
	bool isActive() { return _active; }
	//////////////////////////////////////////////////////////////////////////

protected:
	AsylumEngine *_vm;



	//////////////////////////////////////////////////////////////////////////
	// TODO remove
	Cursor *_cursor;
	GraphicResource *_bgResource;

	bool    _leftClickUp;
	bool    _leftClickDown;
	bool    _rightClickDown;
	bool    _active;

	virtual void update() {};
	void playSound(ResourceId resourceId, bool loop = false);
	//////////////////////////////////////////////////////////////////////////
};

} // End of namespace Asylum

#endif // ASYLUM_PUZZLE_H
