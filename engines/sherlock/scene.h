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
#include "sherlock/objects.h"
#include "sherlock/resources.h"

namespace Sherlock {

#define SCENES_COUNT 63
#define MAX_ZONES    40
#define INFO_LINE   140
#define CONTROLS_Y  138
#define CONTROLS_Y1 151

enum MenuMode {
	STD_MODE		=  0,
	LOOK_MODE		=  1,
	MOVE_MODE		=  2,
	TALK_MODE		=  3,
	PICKUP_MODE		=  4,
	OPEN_MODE		=  5,
	CLOSE_MODE		=  6,
	INV_MODE		=  7,
	USE_MODE		=  8,
	GIVE_MODE		=  9,
	JOURNAL_MODE	= 10,
	FILES_MODE		= 11,
	SETUP_MODE		= 12
};

enum InvMode {
	INVMODE_0	= 0,
	INVMODE_1	= 1,
	INVMODE_2	= 2,
	INVMODE_3	= 3,
	INVMODE_255 = 255
};

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

class Scene {
private:
	SherlockEngine *_vm;
	Common::String _rrmName;
	int _cAnimFramePause;
	Common::String _cAnimStr;
	MenuMode _menuMode;
	InvMode _invMode;
	bool _lookScriptFlag;
	int _selector;
	bool _invLookFlag;
	bool _lookHelp;

	bool loadScene(const Common::String &filename);

	void checkSceneStatus();

	void checkInventory();

	void transitionToScene();

	void updateBackground();

	void checkBgShapes(ImageFrame *frame, const Common::Point &pt);
public:
	int _currentScene;
	int _goToRoom;
	bool _changes;
	bool _stats[SCENES_COUNT][9];
	bool _savedStats[SCENES_COUNT][9];
	Common::Point _bigPos;
	Common::Point _overPos;
	int _charPoint, _oldCharPoint;
	ImageFile *_controls;
	ImageFile *_controlPanel;
	bool _windowOpen, _infoFlag;
	int _keyboardInput;
	int _oldKey, _help, _oldHelp;
	int _oldTemp, _temp;
	bool _walkedInScene;
	int _ongoingCans;
	int _version;
	bool _lzwMode;
	int _invGraphicItems;
	Common::String _comments;
	Common::Array<char> _descText;
	Common::Array<Common::Rect> _roomBounds;
	Common::Array<Object> _bgShapes;
	Common::Array<CAnim> _cAnim;
	Common::Array<byte> _sequenceBuffer;
	Common::Array<InvGraphicType> _inv;
	int _walkDirectory[MAX_ZONES][MAX_ZONES];
	Common::Array<byte> _walkData;
	Common::Array<Exit> _exits;
	SceneEntry _entrance;
	Common::Array<SceneSound> _sounds;
	Common::Point _hsavedPos;
	int _hsavedFs;
	Common::Array<Object> _canimShapes;
	bool _restoreFlag;
	int _animating;
	bool _doBgAnimDone;
public:
	Scene(SherlockEngine *vm);
	~Scene();

	void clear();

	void selectScene();

	void checkSceneFlags(bool mode);

	Exit *checkForExit(const Common::Rect &r);

	void printObjDesc(const Common::String &str, bool firstTime);

	int startCAnim(int cAnimNum, int playRate);

	int toggleObject(const Common::String &name);

	void doBgAnim();

	void clearInfo();
};

} // End of namespace Sherlock

#endif
