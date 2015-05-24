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

#define SCENES_COUNT 63
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
};

class Exit: public Common::Rect {
public:
	int _scene;
	int _allow;
	Common::Point _people;
	int _peopleDir;

	Common::String _dest;
	int _image;					// Arrow image to use

	/**
	 * Load the data for the object
	 */
	void load(Common::SeekableReadStream &s, bool isRoseTattoo);
};

struct SceneEntry {
	Common::Point _startPosition;
	int _startDir;
	int _allow;

	/**
	 * Load the data for the object
	 */
	void load(Common::SeekableReadStream &s);
};

struct SceneSound {
	Common::String _name;
	int _priority;

	/**
	 * Load the data for the object
	 */
	void load(Common::SeekableReadStream &s);
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

struct SceneTripEntry {
	bool _flag;
	int _sceneNumber;
	int _numTimes;

	SceneTripEntry() : _flag(false), _sceneNumber(0), _numTimes(0) {}
	SceneTripEntry(bool flag, int sceneNumber, int numTimes) : _flag(flag),
		_sceneNumber(sceneNumber), _numTimes(numTimes) {}
};

class Scene {
private:
	SherlockEngine *_vm;
	Common::String _rrmName;
	bool _loadingSavedGame;

	/**
	 * Loads the data associated for a given scene. The .BGD file's format is:
	 * BGHEADER: Holds an index for the rest of the file
	 * STRUCTS:  The objects for the scene
	 * IMAGES:   The graphic information for the structures
	 *
	 * The _misc field of the structures contains the number of the graphic image
	 * that it should point to after loading; _misc is then set to 0.
	 */
	bool loadScene(const Common::String &filename);

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
	 * Checks all the background shapes. If a background shape is animating,
	 * it will flag it as needing to be drawn. If a non-animating shape is
	 * colliding with another shape, it will also flag it as needing drawing
	 */
	void checkBgShapes(ImageFrame *frame, const Common::Point &pt);

	/**
	 * Restores objects to the correct status. This ensures that things like being opened or moved
	 * will remain the same on future visits to the scene
	 */
	void saveSceneStatus();

public:
	int _currentScene;
	int _goToScene;
	bool _sceneStats[SCENES_COUNT][65];
	bool _savedStats[SCENES_COUNT][9];
	bool _walkedInScene;
	int _version;
	bool _lzwMode;
	int _invGraphicItems;
	Common::String _comments;
	Common::Array<char> _descText;
	Common::Array<Common::Rect> _zones;
	Common::Array<Object> _bgShapes;
	Common::Array<CAnim> _cAnim;
	Common::Array<byte> _sequenceBuffer;
	Common::Array<SceneImage> _images;
	int _walkDirectory[MAX_ZONES][MAX_ZONES];
	Common::Array<byte> _walkData;
	Common::Array<Exit> _exits;
	int _exitZone;
	SceneEntry _entrance;
	Common::Array<SceneSound> _sounds;
	ObjectArray _canimShapes;
	Common::Array<ScaleZone> _scaleZones;
	Common::StringArray _objSoundList;
	bool _restoreFlag;
	int _animating;
	bool _doBgAnimDone;
	int _tempFadeStyle;
	int _cAnimFramePause;
	Common::Array<SceneTripEntry> _sceneTripCounters;
public:
	Scene(SherlockEngine *vm);
	~Scene();

	/**
	 * Handles loading the scene specified by _goToScene
	 */
	void selectScene();

	/**
	 * Fres all the graphics and other dynamically allocated data for the scene
	 */
	void freeScene();

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
	 * Attempt to start a canimation sequence. It will load the requisite graphics, and
	 * then copy the canim object into the _canimShapes array to start the animation.
	 *
	 * @param cAnimNum		The canim object within the current scene
	 * @param playRate		Play rate. 0 is invalid; 1=normal speed, 2=1/2 speed, etc.
	 *		A negative playRate can also be specified to play the animation in reverse
	 */
	int startCAnim(int cAnimNum, int playRate);

	/**
	 * Scans through the object list to find one with a matching name, and will
	 * call toggleHidden with all matches found. Returns the numer of matches found
	 */
	int toggleObject(const Common::String &name);

	/**
	 * Animate all objects and people.
	 */
	void doBgAnim();

	/**
	 * Attempts to find a background shape within the passed bounds. If found,
	 * it will return the shape number, or -1 on failure.
	 */
	int findBgShape(const Common::Rect &r);

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
	 * Returns the index of the closest zone to a given point.
	 */
	int closestZone(const Common::Point &pt);

	/**
	 * Update the screen back buffer with all of the scene objects which need
	 * to be drawn
	 */
	void updateBackground();

	/**
	 * Synchronize the data for a savegame
	 */
	void synchronize(Common::Serializer &s);

	/**
	 * Resets the NPC path information when entering a new scene.
	 * @remarks		The default talk file for the given NPC is set to WATS##A, where ## is
	 *		the scene number being entered
	 */
	void setNPCPath(int npc);
};

} // End of namespace Sherlock

#endif
