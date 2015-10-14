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

#ifndef SHERLOCK_SCENE_H
#define SHERLOCK_SCENE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "sherlock/objects.h"
#include "sherlock/resources.h"
#include "sherlock/screen.h"

namespace Sherlock {

#define MAX_ZONES	40
#define INFO_LINE	140

class SherlockEngine;

struct BgFileHeader {
	int _numStructs;
	int _numImages;
	int _numcAnimations;
	int _descSize;
	int _seqSize;

	// Serrated Scalpel
	int _fill;

	// Rose Tattoo
	int _scrollSize;
	int _bytesWritten;				// Size of the main body of the RRM
	int _fadeStyle;					// Fade style
	byte _palette[PALETTE_SIZE];	// Palette


	BgFileHeader();

	/**
	 * Load the data for the object
	 */
	void load(Common::SeekableReadStream &s, bool isRoseTattoo);
};

struct BgFileHeaderInfo {
	int _filesize;				// How long images are
	int _maxFrames;				// How many unique frames in object
	Common::String _filename;	// Filename of object

	/**
	 * Load the data for the object
	 */
	void load(Common::SeekableReadStream &s);
	void load3DO(Common::SeekableReadStream &s);
};

class Exit: public Common::Rect {
public:
	int _scene;
	int _allow;
	PositionFacing _newPosition;

	Common::String _dest;
	int _image;					// Arrow image to use

	/**
	 * Load the data for the object
	 */
	void load(Common::SeekableReadStream &s, bool isRoseTattoo);
	void load3DO(Common::SeekableReadStream &s);
};

struct SceneEntry {
	Common::Point _startPosition;
	int _startDir;
	int _allow;

	/**
	 * Load the data for the object
	 */
	void load(Common::SeekableReadStream &s);
	void load3DO(Common::SeekableReadStream &s);
};

struct SceneSound {
	Common::String _name;
	int _priority;

	/**
	 * Load the data for the object
	 */
	void load(Common::SeekableReadStream &s);
	void load3DO(Common::SeekableReadStream &s);
};

class ObjectArray : public Common::Array<Object> {
public:
	/**
	 * Retuurn the index of the passed object in the array
	 */
	int indexOf(const Object &obj) const;
};

class ScaleZone: public Common::Rect {
public:
	int _topNumber;		// Numerator of scale size at the top of the zone
	int _bottomNumber;	// Numerator of scale size at the bottom of the zone

	void load(Common::SeekableReadStream &s);
};

class WalkArray : public Common::Array < Common::Point > {
public:
	int _pointsCount;
	int _fileOffset;

	WalkArray() : _pointsCount(0), _fileOffset(-1) {}

	/**
	 * Load data for the walk array entry
	 */
	void load(Common::SeekableReadStream &s, bool isRoseTattoo);
};

class Scene {
private:
	/**
	 * Loads sounds for the scene
	 */
	void loadSceneSounds();

	/**
	 * Set objects to their current persistent state. This includes things such as
	 * opening or moving them
	 */
	void checkSceneStatus();

	/**
	 * Checks scene objects against the player's inventory items. If there are any
	 * matching names, it means the given item has already been picked up, and should
	 * be hidden in the scene.
	 */
	void checkInventory();

	/**
	 * Set up any entrance co-ordinates or entrance canimations, and then transition
	 * in the scene
	 */
	void transitionToScene();

	/**
	 * Restores objects to the correct status. This ensures that things like being opened or moved
	 * will remain the same on future visits to the scene
	 */
	void saveSceneStatus();
protected:
	SherlockEngine *_vm;
	Common::String _roomFilename;

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
	virtual void drawAllShapes() = 0;

	/**
	 * Called by loadScene when the palette is loaded for Rose Tattoo
	 */
	virtual void paletteLoaded() {}

	Scene(SherlockEngine *vm);
public:
	int _currentScene;
	int _goToScene;
	bool **_sceneStats;
	bool _walkedInScene;
	int _version;
	bool _compressed;
	int _invGraphicItems;
	Common::String _comments;
	Common::Array<char> _descText;
	Common::Array<Common::Rect> _zones;
	ObjectArray _bgShapes;
	Common::Array<CAnim> _cAnim;
	Common::Array<byte> _sequenceBuffer;
	Common::Array<SceneImage> _images;
	int _walkDirectory[MAX_ZONES][MAX_ZONES];
	Common::Array<WalkArray> _walkPoints;
	Common::Array<Exit> _exits;
	SceneEntry _entrance;
	Common::Array<SceneSound> _sounds;
	Common::Array<Object *> _canimShapes;
	Common::Array<ScaleZone> _scaleZones;
	Common::StringArray _objSoundList;
	bool _restoreFlag;
	int _animating;
	bool _doBgAnimDone;
	int _tempFadeStyle;
	int _cAnimFramePause;
public:
	static Scene *init(SherlockEngine *vm);
	virtual ~Scene();

	/**
	 * Handles loading the scene specified by _goToScene
	 */
	void selectScene();

	/**
	 * Check the scene's objects against the game flags. If false is passed,
	 * it means the scene has just been loaded. A value of true means that the scene
	 * is in use (ie. not just loaded)
	 */
	void checkSceneFlags(bool mode);

	/**
	 * Check whether the passed area intersects with one of the scene's exits
	 */
	Exit *checkForExit(const Common::Rect &r);

	/**
	 * Scans through the object list to find one with a matching name, and will
	 * call toggleHidden with all matches found. Returns the numer of matches found
	 */
	int toggleObject(const Common::String &name);

	/**
	 * Checks to see if the given position in the scene belongs to a given zone type.
	 * If it is, the zone is activated and used just like a TAKL zone or aFLAG_SET zone.
	 */
	int checkForZones(const Common::Point &pt, int zoneType);

	/**
	 * Check which zone the the given position is located in.
	 */
	int whichZone(const Common::Point &pt);

	/**
	 * Fres all the graphics and other dynamically allocated data for the scene
	 */
	virtual void freeScene();

	/**
	 * Returns the index of the closest zone to a given point.
	 */
	virtual int closestZone(const Common::Point &pt) = 0;

	/**
	 * Attempts to find a background shape within the passed bounds. If found,
	 * it will return the shape number, or -1 on failure.
	 */
	virtual int findBgShape(const Common::Point &pt) = 0;

	/**
	 * Synchronize the data for a savegame
	 */
	virtual void synchronize(Serializer &s);
public:
	/**
	 * Draw all objects and characters.
	 */
	virtual void doBgAnim() = 0;

	/**
	 * Update the screen back buffer with all of the scene objects which need
	 * to be drawn
	 */
	virtual void updateBackground();

	/**
	 * Attempt to start a canimation sequence. It will load the requisite graphics, and
	 * then copy the canim object into the _canimShapes array to start the animation.
	 *
	 * @param cAnimNum		The canim object within the current scene
	 * @param playRate		Play rate. 0 is invalid; 1=normal speed, 2=1/2 speed, etc.
	 *		A negative playRate can also be specified to play the animation in reverse
	 */
	virtual int startCAnim(int cAnimNum, int playRate = 1) = 0;
};

} // End of namespace Sherlock

#endif
