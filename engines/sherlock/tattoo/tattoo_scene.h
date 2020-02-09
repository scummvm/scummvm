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
#include "sherlock/tattoo/widget_lab.h"

namespace Sherlock {

namespace Tattoo {

extern const int FS_TRANS[8];

enum {
	STARTING_GAME_SCENE = 1, WEARY_PUNT = 52, TRAIN_RIDE = 69, STARTING_INTRO_SCENE = 91, OVERHEAD_MAP2 = 90, OVERHEAD_MAP = 100
};

struct SceneTripEntry {
	int _flag;
	int _sceneNumber;
	int _numTimes;

	SceneTripEntry() : _flag(0), _sceneNumber(0), _numTimes(0) {}
	SceneTripEntry(int flag, int sceneNumber, int numTimes) : _flag(flag),
		_sceneNumber(sceneNumber), _numTimes(numTimes) {}
};

class TattooScene : public Scene {
private:
	WidgetLab _labWidget;

	void doBgAnimCheckCursor();

	/**
	 * Update the background objects and canimations as part of doBgAnim
	 */
	void doBgAnimUpdateBgObjectsAndAnim();

	void doBgAnimDrawSprites();

	/**
	 * Resets the NPC path information when entering a new scene.
	 * @remarks		The default talk file for the given NPC is set to WATS##A, where ## is
	 *		the scene number being entered
	 */
	void setNPCPath(int npc);
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
	bool loadScene(const Common::String &filename) override;

	/**
	 * Checks all the background shapes. If a background shape is animating,
	 * it will flag it as needing to be drawn. If a non-animating shape is
	 * colliding with another shape, it will also flag it as needing drawing
	 */
	void checkBgShapes() override;

	/**
	 * Draw all the shapes, people and NPCs in the correct order
	 */
	void drawAllShapes() override;

	/**
	 * Called by loadScene when the palette is loaded for Rose Tattoo
	 */
	void paletteLoaded() override;

	/**
	 * Synchronize the data for a savegame
	 */
	void synchronize(Serializer &s) override;

	/**
	 * Returns the index of the closest zone to a given point.
	 */
	int closestZone(const Common::Point &pt) override;
public:
	StreamingImageFile _activeCAnim;
	Common::Array<SceneTripEntry> _sceneTripCounters;
	bool _labTableScene;
public:
	TattooScene(SherlockEngine *vm);

	/**
	 * Returns the scale value for the passed co-ordinates. This is taken from the scene's
	 * scale zones, interpolating inbetween the top and bottom values of the zones as needed
	 */
	int getScaleVal(const Point32 &pt);

	/**
	 * Fres all the graphics and other dynamically allocated data for the scene
	 */
	void freeScene() override;

	/**
	 * Draw all objects and characters.
	 */
	void doBgAnim() override;

	/**
	 * Update the screen back buffer with all of the scene objects which need
	 * to be drawn
	 */
	void updateBackground() override;

	/**
	 * Attempt to start a canimation sequence. It will load the requisite graphics, and
	 * then copy the canim object into the _canimShapes array to start the animation.
	 *
	 * @param cAnimNum		The canim object within the current scene
	 * @param playRate		Play rate. 0 is invalid; 1=normal speed, 2=1/2 speed, etc.
	 *		A negative playRate can also be specified to play the animation in reverse
	 */
	int startCAnim(int cAnimNum, int playRate = 1) override;

	/**
	 * Attempts to find a background shape within the passed bounds. If found,
	 * it will return the shape number, or -1 on failure.
	 */
	int findBgShape(const Common::Point &pt) override;
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
