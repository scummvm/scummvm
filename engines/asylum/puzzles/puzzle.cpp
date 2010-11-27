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

#include "asylum/puzzles/puzzle.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

BlowUpPuzzle::BlowUpPuzzle(AsylumEngine *engine): _vm(engine),
	_leftClickUp(false), _leftClickDown(false), _rightClickDown(false), _active(false) {

	// setup cursor & background
	_cursor     = new Cursor(_vm);
	_bgResource = new GraphicResource(_vm, getWorld()->graphicResourceIds[0]);
}

BlowUpPuzzle::~BlowUpPuzzle() {
	delete _cursor;
	delete _bgResource;

	// Zero passed pointers
	_vm = NULL;
}

} // end of namespace Asylum
