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

#ifndef SHERLOCK_TATTOO_SCENE_H
#define SHERLOCK_TATTOO_SCENE_H

#include "common/scummsys.h"
#include "sherlock/scene.h"

namespace Sherlock {

namespace Tattoo {

enum {
	STARTING_INTRO_SCENE = 91
};

class TattooScene : public Scene {
private:
	int _arrowZone;
	int _maskCounter;
	Common::Point _maskOffset;
	bool _labTableScene;
private:
	void doBgAnimCheckCursor();

	void doBgAnimEraseBackground();

	/**
	 * Update the background objects and canimations as part of doBgAnim
	 */
	void doBgAnimUpdateBgObjectsAndAnim();

	void doBgAnimDrawSprites();

	/**
	 * Returns the scale value for the passed co-ordinates. This is taken from the scene's
	 * scale zones, interpolating inbetween the top and bottom values of the zones as needed
	 */
	int getScaleVal(const Common::Point &pt);
protected:
	/**
	 * Loads the data associated for a given scene. The room resource file's format is:
	 * BGHEADER: Holds an index for the rest of the file
	 * STRUCTS:  The objects for the scene
	 * IMAGES:   The graphic information for the structures
	 *
	 * The _misc field of the structures contains the number of the graphic image
	 * that it should point to after loading; _misc is then set to 0.
	 */
	virtual bool loadScene(const Common::String &filename);

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
	ImageFile *_mask, *_mask1;
	CAnimStream _activeCAnim;
public:
	TattooScene(SherlockEngine *vm);

	/**
	 * Draw all objects and characters.
	 */
	virtual void doBgAnim();

	/**
	 * Update the screen back buffer with all of the scene objects which need
	 * to be drawn
	 */
	virtual void updateBackground();

};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
