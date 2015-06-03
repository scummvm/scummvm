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

#ifndef SHERLOCK_SCALPEL_SCENE_H
#define SHERLOCK_SCALPEL_SCENE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "sherlock/objects.h"
#include "sherlock/scene.h"
#include "sherlock/screen.h"

namespace Sherlock {

namespace Scalpel {

class ScalpelScene : public Scene {
private:
	void doBgAnimCheckCursor();
protected:
	/**
	 * Checks all the background shapes. If a background shape is animating,
	 * it will flag it as needing to be drawn. If a non-animating shape is
	 * colliding with another shape, it will also flag it as needing drawing
	 */
	virtual void checkBgShapes();

	/**
	 * Draw all the shapes, people and NPCs in the correct order
	 */
	virtual void drawAllShapes();
public:
	ScalpelScene(SherlockEngine *vm) : Scene(vm) {}

	/**
	 * Draw all objects and characters.
	 */
	virtual void doBgAnim();
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif
