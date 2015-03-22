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

#include "sherlock/scene.h"
#include "sherlock/sherlock.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/decompress.h"

namespace Sherlock {

// Main Menu control locations
const int MENU_POINTS[12][4] = { 
	{ 13, 153, 72, 165 },
	{ 13, 169, 72, 181 },
	{ 13, 185, 72, 197 },
	{ 88, 153, 152, 165 },
	{ 88, 169, 152, 181 },
	{ 88, 185, 152, 197 },
	{ 165, 153, 232, 165 },
	{ 165, 169, 232, 181 },
	{ 165, 185, 233, 197 },
	{ 249, 153, 305, 165 },
	{ 249, 169, 305, 181 },
	{ 249, 185, 305, 197 } 
};

// Inventory control locations */
const int INVENTORY_POINTS[8][3] = { 
	{ 4, 50, 28 },
	{ 52, 99, 76 },
	{ 101, 140, 122 },
	{ 142, 187, 165 },
	{ 189, 219, 197 },
	{ 221, 251, 233 },
	{ 253, 283, 265 },
	{ 285, 315, 293 } 
};

/*----------------------------------------------------------------*/


void BgFileHeader::synchronize(Common::SeekableReadStream &s) {
	_numStructs = s.readUint16LE();
	_numImages = s.readUint16LE();
	_numcAnimations = s.readUint16LE();
	_descSize = s.readUint16LE();
	_seqSize = s.readUint16LE();
	_fill = s.readUint16LE();
}

/*----------------------------------------------------------------*/

void BgfileheaderInfo::synchronize(Common::SeekableReadStream &s) {
	_filesize = s.readUint32LE();
	_maxFrames = s.readByte();

	char buffer[9];
	s.read(buffer, 9);
	_filename = Common::String(buffer);
}

/*----------------------------------------------------------------*/

void Exit::synchronize(Common::SeekableReadStream &s) {
	int xp = s.readSint16LE();
	int yp = s.readSint16LE();
	int xSize = s.readSint16LE();
	int ySize = s.readSint16LE();
	_bounds = Common::Rect(xp, yp, xp + xSize, yp + ySize);

	_scene = s.readSint16LE();
	_allow = s.readSint16LE();
	_people.x = s.readSint16LE();
	_people.y = s.readSint16LE();
	_peopleDir = s.readUint16LE();
}

/*----------------------------------------------------------------*/

void SceneEntry::synchronize(Common::SeekableReadStream &s) {
	_startPosition.x = s.readSint16LE();
	_startPosition.y = s.readSint16LE();
	_startDir = s.readByte();
	_allow = s.readByte();
}

void SceneSound::synchronize(Common::SeekableReadStream &s) {
	char buffer[9];
	s.read(buffer, 8);
	buffer[8] = '\0';

	_name = Common::String(buffer);
	_priority = s.readByte();
}

/*----------------------------------------------------------------*/

Scene::Scene(SherlockEngine *vm): _vm(vm) {
	for (int idx = 0; idx < SCENES_COUNT; ++idx)
		Common::fill(&_stats[idx][0], &_stats[idx][9], false);
	_currentScene = -1;
	_goToScene = -1;
	_changes = false;
	_charPoint = _oldCharPoint = 0;
	_windowOpen = false;
	_infoFlag = false;
	_keyboardInput = 0;
	_walkedInScene = false;
	_ongoingCans = 0;
	_version = 0;
	_lzwMode = false;
	_invGraphicItems = 0;
	_hsavedPos = Common::Point(-1, -1);
	_hsavedFs = -1;
	_cAnimFramePause = 0;
	_menuMode = STD_MODE;
	_invMode = INVMODE_0;
	_restoreFlag = false;
	_invLookFlag = false;
	_lookHelp = false;
	_animating = 0;
	_doBgAnimDone = true;
	_oldLook = 0;
	_tempFadeStyle = 0;

	_controlPanel = new ImageFile("controls.vgs");
	_controls = nullptr; // new ImageFile("menu.all");
}

Scene::~Scene() {
	delete _controlPanel;
	delete _controls;
	freeScene();
}

/**
 * Handles loading the scene specified by _goToScene
 */
void Scene::selectScene() {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;

	// Reset fields
	_windowOpen = _infoFlag = false;
	_menuMode = STD_MODE;
	_keyboardInput = 0;
	_oldKey = _help = _oldHelp = 0;
	_oldTemp = _temp = 0;

	// Load the scene
	Common::String sceneFile = Common::String::format("res%02d", _goToScene);
	_rrmName = Common::String::format("res%02d.rrm", _goToScene);
	_currentScene = _goToScene;
	_goToScene = -1;

	loadScene(sceneFile);

	// If the fade style was changed from running amovie, then reset it
	if (_tempFadeStyle) {
		screen._fadeStyle = _tempFadeStyle;
		_tempFadeStyle = 0;
	}

	people._walkDest = Common::Point(people[AL]._position.x / 100,
		people[AL]._position.y / 100);

	_restoreFlag = true;
	events.clearEvents();

	// If there were any scripst waiting to be run, but were interrupt by a running
	// canimation (probably the last scene's exit canim), clear the _scriptMoreFlag
	if (_vm->_scriptMoreFlag == 3)
		_vm->_scriptMoreFlag = 0;
}

/**
 * Fres all the graphics and other dynamically allocated data for the scene
 */
void Scene::freeScene() {
	_vm->_talk->freeTalkVars();
	_vm->_inventory->freeInventory();
	_vm->_sound->freeSong();
	_vm->_sound->freeLoadedSounds();

	if (!_vm->_loadingSavedGame)
		saveSceneStatus();
	else
		_vm->_loadingSavedGame = false;

	_sequenceBuffer.clear();
	_descText.clear();
	_walkData.clear();
	_cAnim.clear();
	_bgShapes.clear();
	_roomBounds.clear();
	_canimShapes.clear();

	for (uint idx = 0; idx < _images.size(); ++idx)
		delete _images[idx]._images;
	_images.clear();
}

/**
 * Loads the data associated for a given scene. The .BGD file's format is:
 * BGHEADER: Holds an index for the rest of the file
 * STRUCTS:  The objects for the scene
 * IMAGES:   The graphic information for the structures
 *
 * The _misc field of the structures contains the number of the graphic image
 * that it should point to after loading; _misc is then set to 0.
 */
bool Scene::loadScene(const Common::String &filename) {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	bool flag;

	freeScene();
	_walkedInScene = false;
	_ongoingCans = 0;

	// Reset the list of walkable areas
	_roomBounds.clear();
	_roomBounds.push_back(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));

	_descText.clear();
	_comments = "";
	_bgShapes.clear();
	_cAnim.clear();
	_sequenceBuffer.clear();

	//
	// Load background shapes from <filename>.rrm
	//

	Common::String rrmFile = filename + ".rrm";
	flag = _vm->_res->exists(rrmFile);
	if (flag) {
		Common::SeekableReadStream *rrmStream = _vm->_res->load(rrmFile);

		rrmStream->seek(39);
		_version = rrmStream->readByte();
		_lzwMode = _version == 10;

		// Go to header and read it in
		rrmStream->seek(rrmStream->readUint32LE());
		BgFileHeader bgHeader;
		bgHeader.synchronize(*rrmStream);
		_invGraphicItems = bgHeader._numImages + 1;

		// Read in the shapes header info
		Common::Array<BgfileheaderInfo> bgInfo;
		bgInfo.resize(bgHeader._numStructs);

		for (uint idx = 0; idx < bgInfo.size(); ++idx)
			bgInfo[idx].synchronize(*rrmStream);

		// Read information
		Common::SeekableReadStream *infoStream = !_lzwMode ? rrmStream :
			decompressLZ(*rrmStream, bgHeader._numImages * 569 + 
				bgHeader._descSize + bgHeader._seqSize);

		_bgShapes.resize(bgHeader._numStructs + 1);
		for (int idx = 0; idx < bgHeader._numStructs; ++idx)
			_bgShapes[idx].synchronize(*infoStream);

		if (bgHeader._descSize) {
			_descText.resize(bgHeader._descSize);
			infoStream->read(&_descText[0], bgHeader._descSize);
		}

		if (bgHeader._seqSize) {
			_sequenceBuffer.resize(bgHeader._seqSize);
			infoStream->read(&_sequenceBuffer[0], bgHeader._seqSize);
		}			

		if (_lzwMode)
			delete infoStream;

		// Set up the list of images used by the scene
		_images.resize(bgHeader._numImages + 1);
		for (int idx = 0; idx < bgHeader._numImages; ++idx) {
			_images[idx + 1]._filesize = bgInfo[idx]._filesize;
			_images[idx + 1]._maxFrames = bgInfo[idx]._maxFrames;

			// Read in the image data
			Common::SeekableReadStream *imageStream = !_lzwMode ? rrmStream :
				decompressLZ(*rrmStream, bgInfo[idx]._filesize);

			_images[idx + 1]._images = new ImageFile(*imageStream);

			if (_lzwMode)
				delete imageStream;
		}

		// Set up the bgShapes
		for (int idx = 0; idx < bgHeader._numStructs; ++idx) {
			_bgShapes[idx]._examine = Common::String(&_descText[_bgShapes[idx]._descOffset]);
			_bgShapes[idx]._sequences = &_sequenceBuffer[_bgShapes[idx]._sequenceOffset];
			_bgShapes[idx]._misc = 0;
			_bgShapes[idx]._seqCounter = 0;
			_bgShapes[idx]._seqCounter2 = 0;
			_bgShapes[idx]._seqStack = 0;
			_bgShapes[idx]._frameNumber = -1;
			_bgShapes[idx]._position = Common::Point(0, 0);
			_bgShapes[idx]._oldSize = Common::Point(1, 1);

			_bgShapes[idx]._images = _images[_bgShapes[idx]._misc]._images;
			_bgShapes[idx]._imageFrame = !_bgShapes[idx]._images ? (ImageFrame *)nullptr :
				&(*_bgShapes[idx]._images)[0];
		}

		// Set up end of list
		_bgShapes[bgHeader._numStructs]._sequences = &_sequenceBuffer[0] + bgHeader._seqSize;
		_bgShapes[bgHeader._numStructs]._examine = nullptr;

		// Load in cAnim list
		Common::SeekableReadStream *canimStream = !_lzwMode ? rrmStream :
			decompressLZ(*rrmStream, 65 * bgHeader._numcAnimations);

		_cAnim.resize(bgHeader._numcAnimations);
		for (uint idx = 0; idx < _cAnim.size(); ++idx)
			_cAnim[idx].synchronize(*canimStream);

		if (_lzwMode)
			delete canimStream;
		
		// Read in the room bounding areas
		int size = rrmStream->readUint16LE();
		Common::SeekableReadStream *boundsStream = !_lzwMode ? rrmStream :
			decompressLZ(*rrmStream, size);

		_roomBounds.resize(size / 10);
		for (uint idx = 0; idx < _roomBounds.size(); ++idx) {
			_roomBounds[idx].left = boundsStream->readSint16LE();
			_roomBounds[idx].top = boundsStream->readSint16LE();
			_roomBounds[idx].setWidth(boundsStream->readSint16LE());
			_roomBounds[idx].setHeight(boundsStream->readSint16LE());
			boundsStream->skip(2);	// Skip unused scene number field
		}

		if (_lzwMode)
			delete boundsStream;

		// Back at version byte, so skip over it
		rrmStream->skip(1);

		// Load the walk directory
		for (int idx1 = 0; idx1 < MAX_ZONES; ++idx1) {
			for (int idx2 = 0; idx2 < MAX_ZONES; ++idx2)
				_walkDirectory[idx1][idx2] = rrmStream->readSint16LE();
		}

		// Read in the walk data
		size = rrmStream->readUint16LE();
		Common::SeekableReadStream *walkStream = !_lzwMode ? rrmStream :
			decompressLZ(*rrmStream, size);

		_walkData.resize(size);
		walkStream->read(&_walkData[0], size);

		if (_lzwMode)
			delete walkStream;

		// Read in the exits
		int numExits = rrmStream->readByte();
		_exits.resize(numExits);

		for (int idx = 0; idx < numExits; ++idx)
			_exits[idx].synchronize(*rrmStream);

		// Read in the entrance
		_entrance.synchronize(*rrmStream);

		// Initialize sound list
		int numSounds = rrmStream->readByte();
		_sounds.resize(numSounds);

		for (int idx = 0; idx < numSounds; ++idx)
			_sounds[idx].synchronize(*rrmStream);

		// If sound is turned on, load the sounds into memory
		if (sound._soundOn) {
			for (int idx = 0; idx < numSounds; ++idx) {
				sound.loadSound(_sounds[idx]._name, _sounds[idx]._priority);
				_sounds[idx]._name = "";
			}
		}

		// Read in palette
		rrmStream->read(screen._cMap, PALETTE_SIZE);
		for (int idx = 0; idx < PALETTE_SIZE; ++idx)
			screen._cMap[idx] = VGA_COLOR_TRANS(screen._cMap[idx]);
		
		Common::copy(screen._cMap, screen._cMap + PALETTE_SIZE, screen._sMap);

		// Read in the background
		Common::SeekableReadStream *bgStream = !_lzwMode ? rrmStream :
			decompressLZ(*rrmStream, SHERLOCK_SCREEN_WIDTH * SHERLOCK_SCENE_HEIGHT);

		bgStream->read(screen._backBuffer.getPixels(), SHERLOCK_SCREEN_WIDTH * SHERLOCK_SCENE_HEIGHT);

		if (_lzwMode)
			delete bgStream;

		// Set the palette
		screen._backBuffer2.blitFrom(screen._backBuffer);
		screen.setPalette(screen._cMap);

		delete rrmStream;
	}

	// Clear user interface area and draw controls
	screen._backBuffer2.fillRect(0, INFO_LINE, SHERLOCK_SCREEN_WIDTH, INFO_LINE + 10, INFO_BLACK);
	screen._backBuffer.transBlitFrom((*_controlPanel)[0], Common::Point(0, CONTROLS_Y));
	screen._backBuffer2.transBlitFrom((*_controlPanel)[0], Common::Point(0, CONTROLS_Y));

	_changes = false;
	checkSceneStatus();

	if (!_vm->_justLoaded) {
		for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
			if (_bgShapes[idx]._type == HIDDEN && _bgShapes[idx]._aType == TALK_EVERY)
				_bgShapes[idx].toggleHidden();
		}

		// Check for TURNON objects
		for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
			if (_bgShapes[idx]._type == HIDDEN && (_bgShapes[idx]._flags & 0x20))
				_bgShapes[idx].toggleHidden();
		}

		// Check for TURNOFF objects
		for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
			if (_bgShapes[idx]._type != HIDDEN && (_bgShapes[idx]._flags & 0x40) &&
					_bgShapes[idx]._type != INVALID)
				_bgShapes[idx].toggleHidden();
		}
	}

	checkSceneFlags(false);
	checkInventory();

	// Handle starting any music for the scene
	if (sound._musicOn && sound.loadSong(_currentScene)) {
		if (sound._music)
			sound.startSong();
	}

	// Load walking images if not already loaded
	people.loadWalk();

	// Transition to the scene and setup entrance co-ordinates and animations
	transitionToScene();

	// Player has not yet walked in this scene
	_walkedInScene = false;

	// Reset the position on the overland map
	_vm->_oldCharPoint = _currentScene;
	_vm->_over.x = _vm->_map[_currentScene].x * 100 - 600;
	_vm->_over.y = _vm->_map[_currentScene].y * 100 + 900;

	events.clearEvents();
	return flag;
}

/**
 * Set objects to their current persistent state. This includes things such as
 * opening or moving them
 */
void Scene::checkSceneStatus() {
	if (_stats[_currentScene][8]) {
		for (int idx = 0; idx < 8; ++idx) {
			int val = _stats[_currentScene][idx];

			for (int bit = 0; bit < 8; ++bit) {
				uint objNumber = idx * 8 + bit;
				if (objNumber < _bgShapes.size()) {
					Object &obj = _bgShapes[objNumber];

					if (val & 1) {
						// No shape to erase, so flag as hidden
						obj._type = HIDDEN;
					} else if (obj._images == nullptr || obj._images->size() == 0) {
						// No shape
						obj._type = NO_SHAPE;
					} else {
						obj._type = ACTIVE_BG_SHAPE;
					}
				} else {
					// Finished checks
					return;
				}

				val >>= 1;
			}
		}
	}
}

/**
 * Check the scene's objects against the game flags. If false is passed,
 * it means the scene has just been loaded. A value of true means that the scene
 * is in use (ie. not just loaded)
 */
void Scene::checkSceneFlags(bool flag) {
	SpriteType mode = flag ? HIDE_SHAPE : HIDDEN;

	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		
		if (o._requiredFlag) {
			if (!_vm->readFlags(_bgShapes[idx]._requiredFlag)) {
				// Kill object
				if (o._type != HIDDEN && o._type != INVALID) {
					if (o._images == nullptr || o._images->size() == 0)
						// No shape to erase, so flag as hidden
						o._type = HIDDEN;
					else
						// Flag it as needing to be hidden after first erasing it
						o._type = mode;
				}
			} else if (_bgShapes[idx]._requiredFlag) {
				// Restore object
				if (o._images == nullptr || o._images->size() == 0)
					o._type = NO_SHAPE;
				else
					o._type = ACTIVE_BG_SHAPE;
			}
		}
	}

	// Check inventory
	for (int idx = 0; idx < _vm->_inventory->_holdings; ++idx) {
		InventoryItem &ii = (*_vm->_inventory)[idx];
		if (ii._requiredFlag && !_vm->readFlags(ii._requiredFlag)) {
			// Kill object: move it after the active holdings
			InventoryItem tempItem = (*_vm->_inventory)[idx];
			_vm->_inventory->insert_at(_vm->_inventory->_holdings, tempItem);
			_vm->_inventory->remove_at(idx);
			_vm->_inventory->_holdings--;
			break;
		}
	}

	for (uint idx = _vm->_inventory->_holdings; idx < _vm->_inventory->size(); ++idx) {
		InventoryItem &ii = (*_vm->_inventory)[idx];
		if (ii._requiredFlag && _vm->readFlags(ii._requiredFlag)) {
			// Restore object: move it after the active holdings
			InventoryItem tempItem = (*_vm->_inventory)[idx];
			_vm->_inventory->remove_at(idx);
			_vm->_inventory->insert_at(_vm->_inventory->_holdings, tempItem);
			_vm->_inventory->_holdings++;
			break;
		}
	}
}

/**
 * Checks scene objects against the player's inventory items. If there are any
 * matching names, it means the given item has already been picked up, and should
 * be hidden in the scene.
 */
void Scene::checkInventory() {
	for (uint shapeIdx = 0; shapeIdx < _bgShapes.size(); ++shapeIdx) {
		for (uint invIdx = 0; invIdx < _vm->_inventory->size(); ++invIdx) {
			if (scumm_stricmp(_bgShapes[shapeIdx]._name.c_str(),
				(*_vm->_inventory)[invIdx]._name.c_str()) == 0) {
				_bgShapes[shapeIdx]._type = INVALID;
				break;
			}
		}
	}
}

/**
 * Set up any entrance co-ordinates or entrance canimations, and then transition
 * in the scene
 */
void Scene::transitionToScene() {
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;

	const int FS_TRANS[8] = {
		STOP_UP, STOP_UPRIGHT, STOP_RIGHT, STOP_DOWNRIGHT, STOP_DOWN,
		STOP_DOWNLEFT, STOP_LEFT, STOP_UPLEFT
	};

	if (_hsavedPos.x < 1) {
		// No exit information from last scene-check entrance info
		if (_entrance._startPosition.x < 1) {
			// No entrance info either, so use defaults
			_hsavedPos = Common::Point(16000, 10000);
			_hsavedFs = 4;
		} else {
			// setup entrance info
			_hsavedPos = _entrance._startPosition;
			_hsavedFs = _entrance._startDir;
		}
	} else {
		// Exit information exists, translate it to real sequence info
		// Note: If a savegame was just loaded, then the data is already correct.
		// Otherwise, this is a linked scene or entrance info, and must be translated
		if (_hsavedFs < 8 && !_vm->_justLoaded) {
			_hsavedFs = FS_TRANS[_hsavedFs];
			_hsavedPos.x *= 100;
			_hsavedPos.y *= 100;
		}
	}

	int cAnimNum = -1;

	if (_hsavedFs < 101) {
		// Standard info, so set it
		people[PLAYER]._position = _hsavedPos;
		people[PLAYER]._sequenceNumber = _hsavedFs;
	} else {
		// It's canimation information
		cAnimNum = _hsavedFs - 101;

		// Prevent Holmes from being drawn
		people[PLAYER]._position = Common::Point(0, 0);
	}

	for (uint objIdx = 0; objIdx < _bgShapes.size(); ++objIdx) {
		Object &obj = _bgShapes[objIdx];

		if (obj._aType > 1 && obj._type != INVALID && obj._type != HIDDEN) {
			Common::Point topLeft = obj._position;
			Common::Point bottomRight;

			if (obj._type != NO_SHAPE) {
				topLeft += obj._imageFrame->_offset;
				bottomRight.x = topLeft.x + obj._imageFrame->_frame.w;
				bottomRight.y = topLeft.y + obj._imageFrame->_frame.h;			
			} else {
				bottomRight = topLeft + obj._noShapeSize;
			}

			if (Common::Rect(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y).contains(
				Common::Point(people[PLAYER]._position.x / 100, people[PLAYER]._position.y / 100))) {
				// Current point is already inside box - impact occurred on
				// a previous call. So simply do nothing except talk until the
				// player is clear of the box
				switch (obj._aType) {
				case FLAG_SET:
					for (int useNum = 0; useNum < 4; ++useNum) {
						if (obj._use[useNum]._useFlag) {
							if (!_vm->readFlags(obj._use[useNum]._useFlag))
								_vm->setFlags(obj._use[useNum]._useFlag);
						}

						if (!talk._talkToAbort) {
							for (int nameIdx = 0; nameIdx < 4; ++nameIdx) {
								toggleObject(obj._use[useNum]._names[nameIdx]);
							}
						}
					}

					obj._type = HIDDEN;
					break;

				default:
					break;
				}
			}
		}
	}

	updateBackground();

	if (screen._fadeStyle)
		screen.randomTransition();
	else
		screen.blitFrom(screen._backBuffer);

	if (cAnimNum != -1) {
		CAnim &c = _cAnim[cAnimNum];
		Common::Point pt = c._position;

		c._position = Common::Point(-1, -1);
		people[AL]._position = Common::Point(0, 0);

		startCAnim(cAnimNum, 1);
		c._position = pt;
	}
}

/**
 * Scans through the object list to find one with a matching name, and will
 * call toggleHidden with all matches found. Returns the numer of matches found
 */
int Scene::toggleObject(const Common::String &name) {
	int count = 0;

	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		if (scumm_stricmp(name.c_str(), _bgShapes[idx]._name.c_str()) == 0) {
			++count;
			_bgShapes[idx].toggleHidden();
		}
	}

	return count;
}

/**
 * Update the screen back buffer with all of the scene objects which need
 * to be drawn
 */
void Scene::updateBackground() {
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	Surface surface = screen._backBuffer.getSubArea(
		Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCENE_HEIGHT));
	Sprite &player = people[AL];

	// Update Holmes if he's turned on
	if (people._holmesOn)
		player.adjustSprite();

	// Flag the bg shapes which need to be redrawn
	checkBgShapes(player._imageFrame, Common::Point(player._position.x / 100,
		player._position.y / 100));

	// Draw all active shapes which are behind the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		if (_bgShapes[idx]._type == ACTIVE_BG_SHAPE && _bgShapes[idx]._misc == BEHIND)
			surface.transBlitFrom(_bgShapes[idx]._imageFrame->_frame,
				_bgShapes[idx]._position, _bgShapes[idx]._flags & 2);
	}

	// Draw all canimations which are behind the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		if (_canimShapes[idx]._type == ACTIVE_BG_SHAPE && _canimShapes[idx]._misc == BEHIND)
			surface.transBlitFrom(_canimShapes[idx]._imageFrame->_frame,
				_canimShapes[idx]._position, _canimShapes[idx]._flags & 2);
	}

	// Draw all active shapes which are normal and behind the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		if (_bgShapes[idx]._type == ACTIVE_BG_SHAPE && _bgShapes[idx]._misc == NORMAL_BEHIND)
			surface.transBlitFrom(_bgShapes[idx]._imageFrame->_frame,
			_bgShapes[idx]._position, _bgShapes[idx]._flags & 2);
	}

	// Draw all canimations which are normal and behind the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		if (_canimShapes[idx]._type == ACTIVE_BG_SHAPE && _canimShapes[idx]._misc == NORMAL_BEHIND)
			surface.transBlitFrom(_canimShapes[idx]._imageFrame->_frame,
			_canimShapes[idx]._position, _canimShapes[idx]._flags & 2);
	}

	// Draw the player if he's active
	if (player._type == CHARACTER && people.isHolmesActive()) {
		bool flipped = player._sequenceNumber == WALK_LEFT || player._sequenceNumber == STOP_LEFT ||
			player._sequenceNumber == WALK_UPLEFT || player._sequenceNumber == STOP_UPLEFT ||
			player._sequenceNumber == WALK_DOWNRIGHT || player._sequenceNumber == STOP_DOWNRIGHT;

		surface.transBlitFrom(player._imageFrame->_frame,
			Common::Point(player._position.x / 100, player._position.y / 100), flipped);
	}

	// Draw all static and active shapes that are NORMAL and are in front of the player
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		if ((_bgShapes[idx]._type == ACTIVE_BG_SHAPE || _bgShapes[idx]._type == STATIC_BG_SHAPE) &&
				_bgShapes[idx]._misc == NORMAL_FORWARD)
			surface.transBlitFrom(_bgShapes[idx]._imageFrame->_frame,
				_bgShapes[idx]._position, _bgShapes[idx]._flags & 2);
	}

	// Draw all static and active canimations that are NORMAL and are in front of the player
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		if ((_canimShapes[idx]._type == ACTIVE_BG_SHAPE || _canimShapes[idx]._type == STATIC_BG_SHAPE) &&
				_canimShapes[idx]._misc == NORMAL_FORWARD)
			surface.transBlitFrom(_canimShapes[idx]._imageFrame->_frame,
				_canimShapes[idx]._position, _canimShapes[idx]._flags & 2);
	}

	// Draw all static and active shapes that are FORWARD
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		_bgShapes[idx]._oldPosition = _bgShapes[idx]._position;
		_bgShapes[idx]._oldSize = Common::Point(_bgShapes[idx]._imageFrame->_frame.w, 
			_bgShapes[idx]._imageFrame->_frame.h);

		if ((_bgShapes[idx]._type == ACTIVE_BG_SHAPE || _bgShapes[idx]._type == STATIC_BG_SHAPE) &&
				_bgShapes[idx]._misc == FORWARD)
			surface.transBlitFrom(_bgShapes[idx]._imageFrame->_frame,
				_bgShapes[idx]._position, _bgShapes[idx]._flags & 2);
	}

	// Draw all static and active canimations that are forward
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		if ((_canimShapes[idx]._type == ACTIVE_BG_SHAPE || _canimShapes[idx]._type == STATIC_BG_SHAPE) &&
			_canimShapes[idx]._misc == FORWARD)
			surface.transBlitFrom(_canimShapes[idx]._imageFrame->_frame,
				_canimShapes[idx]._position, _canimShapes[idx]._flags & 2);
	}
}

Exit *Scene::checkForExit(const Common::Rect &r) {
	for (uint idx = 0; idx < _exits.size(); ++idx) {
		if (_exits[idx]._bounds.intersects(r))
			return &_exits[idx];
	}

	return nullptr;
}

/**
 * Checks all the background shapes. If a background shape is animating,
 * it will flag it as needing to be drawn. If a non-animating shape is
 * colliding with another shape, it will also flag it as needing drawing
 */
void Scene::checkBgShapes(ImageFrame *frame, const Common::Point &pt) {
	// Iterate through the shapes
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &obj = _bgShapes[idx];
		if (obj._type == STATIC_BG_SHAPE || obj._type == ACTIVE_BG_SHAPE) {
			if ((obj._flags & 5) == 1) {
				obj._misc = (pt.y < (obj._position.y + obj._imageFrame->_frame.h - 1)) ?
					NORMAL_FORWARD : NORMAL_BEHIND;
			} else if (!(obj._flags & 1)) {
				obj._misc = BEHIND;
			} else if (obj._flags & 4) {
				obj._misc = FORWARD;
			}
		}
	}

	// Iterate through the canimshapes
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		Object &obj = _canimShapes[idx];
		if (obj._type == STATIC_BG_SHAPE || obj._type == ACTIVE_BG_SHAPE) {
			if ((obj._flags & 5) == 1) {
				obj._misc = (pt.y < (obj._position.y + obj._imageFrame->_frame.h - 1)) ?
				NORMAL_FORWARD : NORMAL_BEHIND;
			}
			else if (!(obj._flags & 1)) {
				obj._misc = BEHIND;
			}
			else if (obj._flags & 4) {
				obj._misc = FORWARD;
			}
		}
	}
}

/**
 * Attempt to start a canimation sequence. It will load the requisite graphics, and
 * then copy the canim object into the _canimShapes array to start the animation.
 *
 * @param cAnimNum		The canim object within the current scene
 * @param playRate		Play rate. 0 is invalid; 1=normal speed, 2=1/2 speed, etc.
 *		A negative playRate can also be specified to play the animation in reverse
 */
int Scene::startCAnim(int cAnimNum, int playRate) {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Resources &res = *_vm->_res;
	Talk &talk = *_vm->_talk;
	Common::Point tpPos, walkPos;
	int tpDir, walkDir;
	int tFrames = 0;
	int gotoCode = -1;

	// Validation
	if (cAnimNum >= (int)_cAnim.size())
		// number out of bounds
		return -1;
	if (_canimShapes.size() >= 3 || playRate == 0)
		// Too many active animations, or invalid play rate
		return 0;

	CAnim &cAnim = _cAnim[cAnimNum];
	if (playRate < 0) {
		// Reverse direction
		walkPos = cAnim._teleportPos;
		walkDir = cAnim._teleportDir;
		tpPos = cAnim._goto;
		tpDir = cAnim._gotoDir;
	} else {
		// Forward direction
		walkPos = cAnim._goto;
		walkDir = cAnim._gotoDir;
		tpPos = cAnim._teleportPos;
		tpDir = cAnim._teleportDir;
	}

	events.setCursor(WAIT);
	_canimShapes.push_back(Object());
	Object &cObj = _canimShapes[_canimShapes.size() - 1];

	if (walkPos.x != -1) {
		// Holmes must walk to the walk point before the cAnimation is started
		if (people[AL]._position != walkPos)
			people.walkToCoords(walkPos, walkDir);
	}

	if (talk._talkToAbort)
		return 1;

	// Copy the canimation into the bgShapes type canimation structure so it can be played
	cObj._allow = cAnimNum + 1;				// Keep track of the parent structure
	cObj._name = _cAnim[cAnimNum]._name;	// Copy name

	// Remove any attempt to draw object frame
	if (cAnim._type == NO_SHAPE && cAnim._sequences[0] < 100)
		cAnim._sequences[0] = 0;

	cObj._sequences = cAnim._sequences;
	cObj._images = nullptr;
	cObj._position = cAnim._position;
	cObj._delta = Common::Point(0, 0);
	cObj._type = cAnim._type;
	cObj._flags = cAnim._flags;

	cObj._maxFrames = 0;
	cObj._frameNumber = -1;
	cObj._sequenceNumber = cAnimNum;
	cObj._oldPosition = Common::Point(0, 0);
	cObj._oldPosition = Common::Point(0, 0);
	cObj._goto = Common::Point(0, 0);
	cObj._status = 0;
	cObj._misc = 0;
	cObj._imageFrame = nullptr;

	if (cAnim._name.size() > 0 && cAnim._type != NO_SHAPE) {
		if (tpPos.x != -1)
			people[AL]._type = REMOVE;

		Common::String fname = cAnim._name + ".vgs";
		if (!res.isInCache(fname)) {
			// Set up RRM scene data
			Common::SeekableReadStream *rrmStream = res.load(_rrmName);
			rrmStream->seek(44 + cAnimNum * 4);
			rrmStream->seek(rrmStream->readUint32LE());

			// Load the canimation into the cache
			Common::SeekableReadStream *imgStream = !_lzwMode ? rrmStream :
				decompressLZ(*rrmStream, cAnim._size);
			res.addToCache(fname, *imgStream);

			if (_lzwMode)
				delete imgStream;

			delete rrmStream;
		}

		// Now load the resource as an image
		cObj._maxFrames = cObj._images->size();
		cObj._images = new ImageFile(fname);
		cObj._imageFrame = &(*cObj._images)[0];

		int frames = 0;
		if (playRate < 0) {
			// Reverse direction
			// Count number of frames
			while (cObj._sequences[frames] && frames < MAX_FRAME)
				++frames;
		}
		else {
			// Forward direction
			Object::_countCAnimFrames = true;

			while (cObj._type == ACTIVE_BG_SHAPE) {
				cObj.checkObject(_bgShapes[0]);
				++frames;

				if (frames >= 1000)
					error("CAnim has infinite loop sequence");
			}

			if (frames > 1)
				--frames;

			Object::_countCAnimFrames = false;

			cObj._type = cAnim._type;
			cObj._frameNumber = -1;
			cObj._position = cAnim._position;
			cObj._delta = Common::Point(0, 0);
		}

		// Return if animation has no frames in it
		if (frames == 0)
			return -2;

		++frames;
		int repeat = ABS(playRate);
		int dir;

		if (playRate < 0) {
			// Play in reverse
			dir = -2;
			cObj._frameNumber = frames - 3;
		} else {
			dir = 0;
		}

		tFrames = frames - 1;
		int pauseFrame = (_cAnimFramePause) ? frames - _cAnimFramePause : -1;

		while (--frames) {
			if (frames == pauseFrame)
				printObjDesc(_cAnimStr, true);

			doBgAnim();

			// Repeat same frame
			int temp = repeat;
			while (--temp > 0) {
				cObj._frameNumber--;
				doBgAnim();
			}

			cObj._frameNumber += dir;
		}

		people[AL]._type = CHARACTER;
	}

	// Teleport to ending coordinates if necessary
	if (tpPos.x != -1) {
		people[AL]._position = tpPos;	// Place the player
		people[AL]._sequenceNumber = tpDir;
		people.gotoStand(people[AL]);
	}

	if (playRate < 0)
		// Reverse direction - set to end sequence
		cObj._frameNumber = tFrames - 1;
	
	if (cObj._frameNumber <= 26)
		gotoCode = cObj._sequences[cObj._frameNumber + 3];

	// Set canim to REMOVE type and free memory
	cObj.checkObject(_bgShapes[0]);

	if (gotoCode > 0 && !talk._talkToAbort) {
		_goToScene = gotoCode;

		if (_goToScene < 97 && _vm->_map[_goToScene].x) {
			_overPos = _vm->_map[_goToScene];
		}
	}

	people.loadWalk();

	if (tpPos.x != -1 && !talk._talkToAbort) {
		// Teleport to ending coordinates
		people[AL]._position = tpPos;
		people[AL]._sequenceNumber = tpDir;

		people.gotoStand(people[AL]);
	}

	return 1;
}

/**
 * Print the description of an object
 */
void Scene::printObjDesc(const Common::String &str, bool firstTime) {
	/* TODO
	
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	int savedSelector;

	if (str.hasPrefix("_")) {
		_lookScriptFlag = true;
		events.setCursor(MAGNIFY);
		savedSelector = _selector;
		talk.talkTo(str.c_str() + 1);
		_lookScriptFlag = false;

		if (talk._talkToAbort) {
			events.setCursor(ARROW);
			return;
		}

		// Check if looking at an inventory object
		if (!_invLookFlag) {
			// See if this look was called by a right button click or not
			if (!_lookHelp) {
				// If it wasn't a right button click, then we need depress 
				// the look button before we close the window. So save a copy of the
				// menu area, and draw the controls onto it
				Surface tempSurface((*_controls)[0]._frame->w, (*_controls)[0]._frame->h);
				tempSurface.blitFrom(screen._backBuffer2, Common::Point(0, 0),
					Common::Rect(MENU_POINTS[0][0], MENU_POINTS[0][1],
					MENU_POINTS[0][0] + tempSurface.w, MENU_POINTS[0][1] + tempSurface.h));
				screen._backBuffer2.transBlitFrom((*_controls)[0]._frame,
					Common::Point(MENU_POINTS[0][0], MENU_POINTS[0][1]));
				
				banishWindow(1);
				events.setCursor(MAGNIFY);

			}
		}
	}

	// TODO
	*/
}

/**
 * Animate all objects and people.
 */
void Scene::doBgAnim() {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	Talk &talk = *_vm->_talk;
	Surface surface = screen._backBuffer.getSubArea(Common::Rect(0, 0,
		SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCENE_HEIGHT));
	int cursorId = events.getCursor();
	Common::Point mousePos = events.mousePos();

	talk._talkToAbort = false;
	
	// Animate the mouse cursor
	if (cursorId >= WAIT) {
		if (++cursorId > (WAIT + 2))
			cursorId = WAIT;

		events.setCursor((CursorId)cursorId);
	}

	// Check for setting magnifying glass cursor
	if (_menuMode == INV_MODE || _menuMode == USE_MODE || _menuMode == GIVE_MODE) {
		if (_invMode == INVMODE_1) {
			// Only show Magnifying glass cursor if it's not on the inventory command line
			if (mousePos.y < CONTROLS_Y || mousePos.y >(CONTROLS_Y1 + 13))
				events.setCursor(MAGNIFY);
			else
				events.setCursor(ARROW);
		} else {
			events.setCursor(ARROW);
		}
	}

	if (sound._diskSoundPlaying && !*sound._soundIsOn) {
		// Loaded sound just finished playing
		// TODO: This is horrible.. refactor into the Sound class
		delete[] sound._digiBuf;
		sound._diskSoundPlaying = false;
	}

	if (_restoreFlag) {
		if (people[AL]._type == CHARACTER)
			people[AL].checkSprite();

		for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
			if (_bgShapes[idx]._type == ACTIVE_BG_SHAPE)
				_bgShapes[idx].checkObject(_bgShapes[idx]);
		}

		if (people._portraitLoaded && people._portrait._type == ACTIVE_BG_SHAPE)
			people._portrait.checkObject(people._portrait);

		for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
			if (_canimShapes[idx]._type != INVALID && _canimShapes[idx]._type != REMOVE)
				_canimShapes[idx].checkObject(_bgShapes[0]);
		}

		if (_currentScene == 12 && _vm->getGameID() == GType_SerratedScalpel)
			((Scalpel::ScalpelEngine *)_vm)->eraseMirror12();

		// Restore the back buffer from the back buffer 2 in the changed area
		Common::Rect bounds(people[AL]._oldPosition.x, people[AL]._oldPosition.y,
			people[AL]._oldPosition.x + people[AL]._oldSize.x,
			people[AL]._oldPosition.y + people[AL]._oldSize.y);
		Common::Point pt(bounds.left, bounds.top);

		if (people[AL]._type == CHARACTER)
			screen.restoreBackground(bounds);
		else if (people[AL]._type == REMOVE)
			screen._backBuffer.blitFrom(screen._backBuffer2, pt, bounds);

		for (uint idx = 0; _bgShapes.size(); ++idx) {
			Object &o = _bgShapes[idx];
			if (o._type == ACTIVE_BG_SHAPE || o._type == HIDE_SHAPE || o._type == REMOVE)
				screen.restoreBackground(bounds);
		}

		if (people._portraitLoaded)
			screen.restoreBackground(Common::Rect(
				people._portrait._oldPosition.x, people._portrait._oldPosition.y,
				people._portrait._oldPosition.x + people._portrait._oldSize.x,
				people._portrait._oldPosition.y + people._portrait._oldSize.y
			));

		for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
			Object &o = _bgShapes[idx];
			if (o._type == NO_SHAPE && ((o._flags & 1) == 0)) {
				// Restore screen area
				screen._backBuffer.blitFrom(screen._backBuffer2, o._position,
					Common::Rect(o._position.x, o._position.y,
					o._position.x + o._noShapeSize.x, o._position.y + o._noShapeSize.y));

				o._oldPosition = o._position;
				o._oldSize = o._noShapeSize;
			}
		}

		for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
			Object &o = _canimShapes[idx];
			if (o._type == ACTIVE_BG_SHAPE || o._type == HIDE_SHAPE || o._type == REMOVE)
				screen.restoreBackground(Common::Rect(o._oldPosition.x, o._oldPosition.y,
					o._oldPosition.x + o._oldSize.x, o._oldPosition.y + o._oldSize.y));
		}
	}

	//
	// Update the background objects and canimations
	//

	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if (o._type == ACTIVE_BG_SHAPE || o._type == NO_SHAPE)
			o.adjustObject();
	}

	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		if (_canimShapes[idx]._type != INVALID)
			_canimShapes[idx].adjustObject();
	}

	if (people[AL]._type == CHARACTER && people._holmesOn)
		people[AL].adjustSprite();

	// Flag the bg shapes which need to be redrawn
	checkBgShapes(people[AL]._imageFrame, 
		Common::Point(people[AL]._position.x / 100, people[AL]._position.y / 100));

	if (_currentScene == 12 && _vm->getGameID() == GType_SerratedScalpel)
		((Scalpel::ScalpelEngine *)_vm)->doMirror12();

	// Draw all active shapes which are behind the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if (o._type == ACTIVE_BG_SHAPE && o._misc == BEHIND)
			screen._backBuffer.transBlitFrom(o._imageFrame->_frame, o._position, o._flags & 2);
	}
	
	// Draw all canimations which are behind the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		Object &o = _canimShapes[idx];
		if (o._type == ACTIVE_BG_SHAPE && o._misc == BEHIND) {
			screen._backBuffer.transBlitFrom(o._imageFrame->_frame, o._position, o._flags & 2);
		}
	}

	// Draw all active shapes which are HAPPEN and behind the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if (o._type == ACTIVE_BG_SHAPE && o._misc == NORMAL_BEHIND)
			screen._backBuffer.transBlitFrom(o._imageFrame->_frame, o._position, o._flags & 2);
	}

	// Draw all canimations which are NORMAL and behind the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		Object &o = _canimShapes[idx];
		if (o._type == ACTIVE_BG_SHAPE && o._misc == NORMAL_BEHIND) {
			screen._backBuffer.transBlitFrom(o._imageFrame->_frame, o._position, o._flags & 2);
		}
	}

	// Draw the person if not animating
	if (people[AL]._type == CHARACTER && people.isHolmesActive()) {
		// If Holmes is too far to the right, move him back so he's on-screen
		int xRight = SHERLOCK_SCREEN_WIDTH - 2 - people[AL]._imageFrame->_frame.w;
		int tempX = MIN(people[AL]._position.x / 100, xRight);

		bool flipped = people[AL]._frameNumber == WALK_LEFT || people[AL]._frameNumber == STOP_LEFT ||
			people[AL]._frameNumber == WALK_UPLEFT || people[AL]._frameNumber == STOP_UPLEFT ||
			people[AL]._frameNumber == WALK_DOWNRIGHT || people[AL]._frameNumber == STOP_DOWNRIGHT;
		screen._backBuffer.transBlitFrom(people[AL]._imageFrame->_frame,
			Common::Point(tempX, people[AL]._position.y / 100 - people[AL]._imageFrame->_frame.h), flipped);
	}

	// Draw all static and active shapes are NORMAL and are in front of the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if ((o._type == ACTIVE_BG_SHAPE || o._type == STATIC_BG_SHAPE) && o._misc == NORMAL_FORWARD)
			screen._backBuffer.transBlitFrom(o._imageFrame->_frame, o._position, o._flags & 2);
	}

	// Draw all static and active canimations that are NORMAL and are in front of the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		Object &o = _canimShapes[idx];
		if ((o._type == ACTIVE_BG_SHAPE || o._type == STATIC_BG_SHAPE) && o._misc == NORMAL_BEHIND) {
			screen._backBuffer.transBlitFrom(o._imageFrame->_frame, o._position, o._flags & 2);
		}
	}

	// Draw all static and active shapes that are in front of the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if ((o._type == ACTIVE_BG_SHAPE || o._type == STATIC_BG_SHAPE) && o._misc == FORWARD)
			screen._backBuffer.transBlitFrom(o._imageFrame->_frame, o._position, o._flags & 2);
	}

	// Draw any active portrait
	if (people._portraitLoaded && people._portrait._type == ACTIVE_BG_SHAPE)
		screen._backBuffer.transBlitFrom(people._portrait._imageFrame->_frame,
			people._portrait._position, people._portrait._flags & 2);

	// Draw all static and active canimations that are in front of the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		Object &o = _canimShapes[idx];
		if ((o._type == ACTIVE_BG_SHAPE || o._type == STATIC_BG_SHAPE) && o._misc == FORWARD) {
			screen._backBuffer.transBlitFrom(o._imageFrame->_frame, o._position, o._flags & 2);
		}
	}

	// Draw all NO_SHAPE shapes which have flag bit 0 clear
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if (o._type == NO_SHAPE && (o._flags & 1) == 0)
			screen._backBuffer.transBlitFrom(o._imageFrame->_frame, o._position, o._flags & 2);
	}

	// Bring the newly built picture to the screen
	if (_animating == 2) {
		_animating = 0;
		screen.slamRect(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCENE_HEIGHT));
	} else {
		if (people[AL]._type != INVALID && ((_goToScene == -1 || _ongoingCans == 0))) {
			if (people[AL]._type == REMOVE) {
				screen.slamRect(Common::Rect(
					people[AL]._oldPosition.x, people[AL]._oldPosition.y,
					people[AL]._oldPosition.x + people[AL]._oldSize.x,
					people[AL]._oldPosition.y + people[AL]._oldSize.y
				));
			} else {
				screen.flushImage(people[AL]._imageFrame,
					Common::Point(people[AL]._position.x / 100, people[AL]._position.y / 100),
					&people[AL]._oldPosition.x, &people[AL]._oldPosition.y,
					&people[AL]._oldSize.x, &people[AL]._oldSize.y);
			}
		}

		if (_currentScene == 12 && _vm->getGameID() == GType_SerratedScalpel)
			((Scalpel::ScalpelEngine *)_vm)->flushMirror12();

		for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
			Object &o = _bgShapes[idx];
			if ((o._type == ACTIVE_BG_SHAPE || o._type == REMOVE) && _goToScene == -1) {
				screen.flushImage(o._imageFrame, o._position,
					&o._oldPosition.x, &o._oldPosition.y, &o._oldSize.x, &o._oldSize.y);
			}
		}

		if (people._portraitLoaded) {
			if (people._portrait._type == REMOVE)
				screen.slamRect(Common::Rect(
					people._portrait._position.x, people._portrait._position.y,
					people._portrait._position.x + people._portrait._delta.x,
					people._portrait._position.y + people._portrait._delta.y
				));
			else
				screen.flushImage(people._portrait._imageFrame, people._portrait._position,
					&people._portrait._oldPosition.x, &people._portrait._oldPosition.y,
					&people._portrait._oldSize.x, &people._portrait._oldSize.y);
				
			if (people._portrait._type == REMOVE)
				people._portrait._type = INVALID;
		}

		if (_goToScene == -1) {
			for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
				Object &o = _bgShapes[idx];
				if (o._type == NO_SHAPE && (o._flags & 1) == 0) {
					screen.slamArea(o._position.x, o._position.y, o._oldSize.x, o._oldSize.y);
					screen.slamArea(o._oldPosition.x, o._oldPosition.y, o._oldSize.x, o._oldSize.y);
				} else if (o._type == HIDE_SHAPE) {
					screen.flushImage(o._imageFrame, o._position,
						&o._oldPosition.x, &o._oldPosition.y, &o._oldSize.x, &o._oldSize.y);
				}
			}
		}

		for (int idx = _canimShapes.size() - 1; idx >= 0; --idx) {
			Object &o = _canimShapes[idx];
			if (o._type == REMOVE) {
				if (_goToScene == -1)
					screen.slamArea(o._position.x, o._position.y, o._delta.x, o._delta.y);

				_canimShapes.remove_at(idx);
			} else if (o._type == ACTIVE_BG_SHAPE) {
				screen.flushImage(o._imageFrame, o._position,
					&o._oldPosition.x, &o._oldPosition.y, &o._oldSize.x, &o._oldSize.y);
			}
		}
	}

	_restoreFlag = true;

	events.wait(1);
	_doBgAnimDone = true;

	// Check if the method was called for calling a portrait, and a talk was
	// interrupting it. This talk file would not have been executed at the time, 
	// since we needed to finish the 'doBgAnim' to finish clearing the portrait
	if (people._clearingThePortrait && _vm->_scriptMoreFlag == 3) {
		// Reset the flags and call to talk
		people._clearingThePortrait = _vm->_scriptMoreFlag = 0;
		talk.talkTo(_vm->_scriptName);
	}
}

/**
 * Clears the info line of the screen
 */
void Scene::clearInfo() {
	if (_infoFlag) {
		_vm->_screen->fillRect(16, INFO_LINE, SHERLOCK_SCREEN_WIDTH - 200, INFO_LINE + 9,
			INFO_BLACK);
		_infoFlag = false;
		_oldLook = -1;
	}
}

void Scene::saveSceneStatus() {
	// TODO
}

} // End of namespace Sherlock
