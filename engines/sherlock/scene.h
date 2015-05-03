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
#include "common/array.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "sherlock/objects.h"
#include "sherlock/resources.h"

namespace Sherlock {

#define SCENES_COUNT 63
#define MAX_ZONES    40
#define INFO_LINE   140

class SherlockEngine;

struct BgFileHeader {
	int _numStructs;
	int _numImages;
	int _numcAnimations;
	int _descSize;
	int _seqSize;
	int _fill;

	void synchronize(Common::SeekableReadStream &s);
};

struct BgfileheaderInfo {
	int _filesize;				// How long images are
	int _maxFrames;				// How many unique frames in object
	Common::String _filename;	// Filename of object

	void synchronize(Common::SeekableReadStream &s);
};

struct Exit {
	Common::Rect _bounds;

	int _scene;
	int _allow;
	Common::Point _people;
	int _peopleDir;

	void synchronize(Common::SeekableReadStream &s);
};

struct SceneEntry {
	Common::Point _startPosition;
	int _startDir;
	int _allow;

	void synchronize(Common::SeekableReadStream &s);
};

struct SceneSound {
	Common::String _name;
	int _priority;

	void synchronize(Common::SeekableReadStream &s);
};

class ObjectArray: public Common::Array<Object> {
public:
	int indexOf(const Object &obj) const;
};

class Scene {
private:
	SherlockEngine *_vm;
	Common::String _rrmName;
	int _selector;
	bool _lookHelp;
	bool _loadingSavedGame;

	bool loadScene(const Common::String &filename);

	void checkSceneStatus();

	void checkInventory();

	void transitionToScene();

	void checkBgShapes(ImageFrame *frame, const Common::Point &pt);

	void saveSceneStatus();
public:
	int _currentScene;
	int _goToScene;
	bool _changes;
	bool _sceneStats[SCENES_COUNT][65];
	bool _savedStats[SCENES_COUNT][9];
	int _keyboardInput;
	int _oldKey, _help, _oldHelp;
	int _oldTemp, _temp;
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
	SceneEntry _entrance;
	Common::Array<SceneSound> _sounds;
	ObjectArray _canimShapes;
	bool _restoreFlag;
	int _animating;
	bool _doBgAnimDone;
	int _tempFadeStyle;
	int _cAnimFramePause;
	bool _invLookFlag;
public:
	Scene(SherlockEngine *vm);
	~Scene();

	void selectScene();

	void freeScene();

	void checkSceneFlags(bool mode);

	Exit *checkForExit(const Common::Rect &r);

	int startCAnim(int cAnimNum, int playRate);

	int toggleObject(const Common::String &name);

	void doBgAnim();

	void clearInfo();

	int findBgShape(const Common::Rect &r);

	int checkForZones(const Common::Point &pt, int zoneType);

	int whichZone(const Common::Point &pt);

	int closestZone(const Common::Point &pt);

	void updateBackground();

	void synchronize(Common::Serializer &s);
};

} // End of namespace Sherlock

#endif
