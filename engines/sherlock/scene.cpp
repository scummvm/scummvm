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

#include "sherlock/scene.h"
#include "sherlock/sherlock.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/scalpel/scalpel_scene.h"
#include "sherlock/screen.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_scene.h"

namespace Sherlock {

static const int FS_TRANS[8] = {
	STOP_UP, STOP_UPRIGHT, STOP_RIGHT, STOP_DOWNRIGHT, STOP_DOWN,
	STOP_DOWNLEFT, STOP_LEFT, STOP_UPLEFT
};

/*----------------------------------------------------------------*/

BgFileHeader::BgFileHeader() {
	_numStructs = -1;
	_numImages = -1;
	_numcAnimations = -1;
	_descSize = -1;
	_seqSize = -1;

	// Serrated Scalpel
	_fill = -1;

	// Rose Tattoo
	_scrollSize = -1;
	_bytesWritten = -1;
	_fadeStyle = -1;
	Common::fill(&_palette[0], &_palette[PALETTE_SIZE], 0);
}

void BgFileHeader::load(Common::SeekableReadStream &s, bool isRoseTattoo) {
	_numStructs = s.readUint16LE();
	_numImages = s.readUint16LE();
	_numcAnimations = s.readUint16LE();
	_descSize = s.readUint16LE();
	_seqSize = s.readUint16LE();

	if (isRoseTattoo) {
		_scrollSize = s.readUint16LE();
		_bytesWritten = s.readUint32LE();
		_fadeStyle = s.readByte();
	} else {
		_fill = s.readUint16LE();

	}
}

/*----------------------------------------------------------------*/

void BgFileHeaderInfo::load(Common::SeekableReadStream &s) {
	_filesize = s.readUint32LE();
	_maxFrames = s.readByte();

	char buffer[9];
	s.read(buffer, 9);
	_filename = Common::String(buffer);
}

/*----------------------------------------------------------------*/

void Exit::load(Common::SeekableReadStream &s, bool isRoseTattoo) {
	if (isRoseTattoo) {
		char buffer[41];
		s.read(buffer, 41);
		_dest = Common::String(buffer);
	}

	left = s.readSint16LE();
	top = s.readSint16LE();
	setWidth(s.readUint16LE());
	setHeight(s.readUint16LE());

	_image = isRoseTattoo ? s.readByte() : 0;
	_scene = s.readSint16LE();

	if (!isRoseTattoo)
		_allow = s.readSint16LE();

	_people.x = s.readSint16LE();
	_people.y = s.readSint16LE();
	_peopleDir = s.readUint16LE();

	if (isRoseTattoo)
		_allow = s.readSint16LE();
}

/*----------------------------------------------------------------*/

void SceneEntry::load(Common::SeekableReadStream &s) {
	_startPosition.x = s.readSint16LE();
	_startPosition.y = s.readSint16LE();
	_startDir = s.readByte();
	_allow = s.readByte();
}

void SceneSound::load(Common::SeekableReadStream &s) {
	char buffer[9];
	s.read(buffer, 8);
	buffer[8] = '\0';

	_name = Common::String(buffer);
	_priority = s.readByte();
}

/*----------------------------------------------------------------*/

int ObjectArray::indexOf(const Object &obj) const {
	for (uint idx = 0; idx < size(); ++idx) {
		if (&(*this)[idx] == &obj)
			return idx;
	}

	return -1;
}

/*----------------------------------------------------------------*/

void ScaleZone::load(Common::SeekableReadStream &s) {
	left = s.readSint16LE();
	top = s.readSint16LE();
	setWidth(s.readUint16LE());
	setHeight(s.readUint16LE());

	_topNumber = s.readByte();
	_bottomNumber = s.readByte();
}

/*----------------------------------------------------------------*/

Scene *Scene::init(SherlockEngine *vm) {
	if (vm->getGameID() == GType_SerratedScalpel)
		return new Scalpel::ScalpelScene(vm);
	else
		return new Tattoo::TattooScene(vm);
}

Scene::Scene(SherlockEngine *vm): _vm(vm) {
	for (int idx = 0; idx < SCENES_COUNT; ++idx)
		Common::fill(&_sceneStats[idx][0], &_sceneStats[idx][65], false);
	_currentScene = -1;
	_goToScene = -1;
	_loadingSavedGame = false;
	_walkedInScene = false;
	_version = 0;
	_lzwMode = false;
	_invGraphicItems = 0;
	_cAnimFramePause = 0;
	_restoreFlag = false;
	_animating = 0;
	_doBgAnimDone = true;
	_tempFadeStyle = 0;
	_exitZone = -1;
}

Scene::~Scene() {
	freeScene();
}

void Scene::selectScene() {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	UserInterface &ui = *_vm->_ui;

	// Reset fields
	ui._windowOpen = ui._infoFlag = false;
	ui._menuMode = STD_MODE;

	// Free any previous scene
	freeScene();

	// Load the scene
	Common::String sceneFile = Common::String::format("res%02d", _goToScene);
	_rrmName = Common::String::format("res%02d.rrm", _goToScene);
	_currentScene = _goToScene;
	_goToScene = -1;

	loadScene(sceneFile);

	// If the fade style was changed from running a movie, then reset it
	if (_tempFadeStyle) {
		screen._fadeStyle = _tempFadeStyle;
		_tempFadeStyle = 0;
	}

	people._walkDest = Common::Point(people[AL]._position.x / FIXED_INT_MULTIPLIER,
		people[AL]._position.y / FIXED_INT_MULTIPLIER);

	_restoreFlag = true;
	events.clearEvents();

	// If there were any scripts waiting to be run, but were interrupt by a running
	// canimation (probably the last scene's exit canim), clear the _scriptMoreFlag
	if (talk._scriptMoreFlag == 3)
		talk._scriptMoreFlag = 0;
}

void Scene::freeScene() {
	if (_currentScene == -1)
		return;

	_vm->_talk->freeTalkVars();
	_vm->_inventory->freeInv();
	_vm->_music->freeSong();
	_vm->_sound->freeLoadedSounds();

	if (!_loadingSavedGame)
		saveSceneStatus();
	else
		_loadingSavedGame = false;

	_sequenceBuffer.clear();
	_descText.clear();
	_walkData.clear();
	_cAnim.clear();
	_bgShapes.clear();
	_zones.clear();
	_canimShapes.clear();

	for (uint idx = 0; idx < _images.size(); ++idx)
		delete _images[idx]._images;
	_images.clear();

	_currentScene = -1;
}

bool Scene::loadScene(const Common::String &filename) {
	Events &events = *_vm->_events;
	Map &map = *_vm->_map;
	Music &music = *_vm->_music;
	People &people = *_vm->_people;
	Resources &res = *_vm->_res;
	SaveManager &saves = *_vm->_saves;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	Talk &talk = *_vm->_talk;
	UserInterface &ui = *_vm->_ui;
	bool flag;

	_walkedInScene = false;

	// Reset the list of walkable areas
	_zones.clear();
	_zones.push_back(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));

	_descText.clear();
	_comments = "";
	_bgShapes.clear();
	_cAnim.clear();
	_sequenceBuffer.clear();

	// Check if it's a scene we need to keep trakc track of how many times we've visited
	for (int idx = (int)_sceneTripCounters.size() - 1; idx >= 0; --idx) {
		if (_sceneTripCounters[idx]._sceneNumber == _currentScene) {
			if (--_sceneTripCounters[idx]._numTimes == 0) {
				_vm->setFlags(_sceneTripCounters[idx]._flag);
				_sceneTripCounters.remove_at(idx);
			}
		}
	}

	if (IS_ROSE_TATTOO) {
		// Set the NPC paths for the scene
		setNPCPath(0);

		// Handle loading music for the scene
		if (sound._midiDrvLoaded) {
			if (talk._scriptMoreFlag != 1 && talk._scriptMoreFlag != 3)
				sound._nextSongName = Common::String::format("res%02d", _currentScene);

			// If it's a new song, then start it up
			if (sound._currentSongName.compareToIgnoreCase(sound._nextSongName)) {
				if (music.loadSong(sound._nextSongName)) {
					sound.setMIDIVolume(sound._musicVolume);
					if (music._musicOn)
						music.startSong();
				}
			}
		}
	}

	//
	// Load the room resource file for the scene
	//

	Common::String rrmFile = filename + ".rrm";
	flag = _vm->_res->exists(rrmFile);
	if (flag) {
		Common::SeekableReadStream *rrmStream = _vm->_res->load(rrmFile);

		rrmStream->seek(39);
		if (IS_SERRATED_SCALPEL) {
			_version = rrmStream->readByte();
			_lzwMode = _version == 10;
		} else {
			_lzwMode = rrmStream->readByte() > 0;
		}

		// Go to header and read it in
		rrmStream->seek(rrmStream->readUint32LE());

		BgFileHeader bgHeader;
		bgHeader.load(*rrmStream, IS_ROSE_TATTOO);
		_invGraphicItems = bgHeader._numImages + 1;

		if (IS_ROSE_TATTOO) {
			screen.initPaletteFade(bgHeader._bytesWritten);
			rrmStream->read(screen._cMap, PALETTE_SIZE);
			screen.translatePalette(screen._cMap);
			screen.setupBGArea(screen._cMap);

			ui.initScrollVars();

			// Read in background
			if (_lzwMode) {
				res.decompress(*rrmStream, (byte *)screen._backBuffer1.getPixels(), SHERLOCK_SCREEN_WIDTH * SHERLOCK_SCREEN_HEIGHT);
			} else {
				rrmStream->read(screen._backBuffer1.getPixels(), SHERLOCK_SCREEN_WIDTH * SHERLOCK_SCREEN_HEIGHT);
			}
		} 

		// Read in the shapes header info
		Common::Array<BgFileHeaderInfo> bgInfo;
		bgInfo.resize(bgHeader._numStructs);

		for (uint idx = 0; idx < bgInfo.size(); ++idx)
			bgInfo[idx].load(*rrmStream);

		// Read information
		if (IS_ROSE_TATTOO) {
			// Load shapes
			Common::SeekableReadStream *infoStream = !_lzwMode ? rrmStream : res.decompress(*rrmStream, bgHeader._numStructs * 625);

			_bgShapes.resize(bgHeader._numStructs);
			for (int idx = 0; idx < bgHeader._numStructs; ++idx)
				_bgShapes[idx].load(*infoStream, _vm->getGameID() == GType_RoseTattoo);

			if (_lzwMode)
				delete infoStream;

			// Load description text
			_descText.resize(bgHeader._descSize);
			if (_lzwMode)
				res.decompress(*rrmStream, (byte *)&_descText[0], bgHeader._descSize);
			else
				rrmStream->read(&_descText[0], bgHeader._descSize);

			// Load sequences
			_sequenceBuffer.resize(bgHeader._seqSize);
			if (_lzwMode)
				res.decompress(*rrmStream, &_sequenceBuffer[0], bgHeader._seqSize);
			else
				rrmStream->read(&_sequenceBuffer[0], bgHeader._seqSize);
		} else if (!_lzwMode) {
			// Serrated Scalpel uncompressed info
			_bgShapes.resize(bgHeader._numStructs);
			for (int idx = 0; idx < bgHeader._numStructs; ++idx)
				_bgShapes[idx].load(*rrmStream, false);

			if (bgHeader._descSize) {
				_descText.resize(bgHeader._descSize);
				rrmStream->read(&_descText[0], bgHeader._descSize);
			}

			if (bgHeader._seqSize) {
				_sequenceBuffer.resize(bgHeader._seqSize);
				rrmStream->read(&_sequenceBuffer[0], bgHeader._seqSize);
			}
		} else {
			// Serrated Scalpel compressed info
			Common::SeekableReadStream *infoStream;

			// Read shapes
			infoStream = Resources::decompressLZ(*rrmStream, bgHeader._numStructs * 569);

			_bgShapes.resize(bgHeader._numStructs);
			for (int idx = 0; idx < bgHeader._numStructs; ++idx)
				_bgShapes[idx].load(*infoStream, false);

			delete infoStream;

			// Read description texts
			if (bgHeader._descSize) {
				infoStream = Resources::decompressLZ(*rrmStream, bgHeader._descSize);

				_descText.resize(bgHeader._descSize);
				infoStream->read(&_descText[0], bgHeader._descSize);

				delete infoStream;
			}

			// Read sequences
			if (bgHeader._seqSize) {
				infoStream = Resources::decompressLZ(*rrmStream, bgHeader._seqSize);

				_sequenceBuffer.resize(bgHeader._seqSize);
				infoStream->read(&_sequenceBuffer[0], bgHeader._seqSize);

				delete infoStream;
			}
		}

		// Set up the list of images used by the scene
		_images.resize(bgHeader._numImages + 1);
		for (int idx = 0; idx < bgHeader._numImages; ++idx) {
			_images[idx + 1]._filesize = bgInfo[idx]._filesize;
			_images[idx + 1]._maxFrames = bgInfo[idx]._maxFrames;

			// Read in the image data
			Common::SeekableReadStream *imageStream = _lzwMode ?
				res.decompress(*rrmStream, bgInfo[idx]._filesize) :
				rrmStream->readStream(bgInfo[idx]._filesize);

			_images[idx + 1]._images = new ImageFile(*imageStream);

			delete imageStream;
		}

		// Set up the bgShapes
		for (int idx = 0; idx < bgHeader._numStructs; ++idx) {
			_bgShapes[idx]._images = _images[_bgShapes[idx]._misc]._images;
			_bgShapes[idx]._imageFrame = !_bgShapes[idx]._images ? (ImageFrame *)nullptr :
				&(*_bgShapes[idx]._images)[0];

			_bgShapes[idx]._examine = Common::String(&_descText[_bgShapes[idx]._descOffset]);
			_bgShapes[idx]._sequences = &_sequenceBuffer[_bgShapes[idx]._sequenceOffset];
			_bgShapes[idx]._misc = 0;
			_bgShapes[idx]._seqCounter = 0;
			_bgShapes[idx]._seqCounter2 = 0;
			_bgShapes[idx]._seqStack = 0;
			_bgShapes[idx]._frameNumber = -1;
			_bgShapes[idx]._oldPosition = Common::Point(0, 0);
			_bgShapes[idx]._oldSize = Common::Point(1, 1);
		}

		// Load in cAnim list
		_cAnim.clear();
		if (bgHeader._numcAnimations) {
			int animSize = IS_SERRATED_SCALPEL ? 65 : 47;
			Common::SeekableReadStream *canimStream = _lzwMode ?
				res.decompress(*rrmStream, animSize * bgHeader._numcAnimations) :
				rrmStream->readStream(animSize * bgHeader._numcAnimations);

			_cAnim.resize(bgHeader._numcAnimations);
			for (uint idx = 0; idx < _cAnim.size(); ++idx)
				_cAnim[idx].load(*canimStream, IS_ROSE_TATTOO);

			delete canimStream;
		}

		// Read in the room bounding areas
		int size = rrmStream->readUint16LE();
		Common::SeekableReadStream *boundsStream = !_lzwMode ? rrmStream :
			res.decompress(*rrmStream, size);

		_zones.resize(size / 10);
		for (uint idx = 0; idx < _zones.size(); ++idx) {
			_zones[idx].left = boundsStream->readSint16LE();
			_zones[idx].top = boundsStream->readSint16LE();
			_zones[idx].setWidth(boundsStream->readSint16LE() + 1);
			_zones[idx].setHeight(boundsStream->readSint16LE() + 1);
			boundsStream->skip(2);	// Skip unused scene number field
		}

		if (_lzwMode)
			delete boundsStream;

		// Ensure we've reached the path version byte
		if (rrmStream->readByte() != (IS_SERRATED_SCALPEL ? 254 : 251))
			error("Invalid scene path data");

		// Load the walk directory
		assert(_zones.size() < MAX_ZONES);
		for (uint idx1 = 0; idx1 < _zones.size(); ++idx1) {
			for (uint idx2 = 0; idx2 < _zones.size(); ++idx2)
				_walkDirectory[idx1][idx2] = rrmStream->readSint16LE();
		}

		// Read in the walk data
		size = rrmStream->readUint16LE();
		Common::SeekableReadStream *walkStream = !_lzwMode ? rrmStream :
			res.decompress(*rrmStream, size);

		_walkData.resize(size);
		walkStream->read(&_walkData[0], size);

		if (_lzwMode)
			delete walkStream;

		if (IS_ROSE_TATTOO) {
			// Read in the entrance
			_entrance.load(*rrmStream);

			// Load scale zones
			_scaleZones.resize(rrmStream->readByte());
			for (uint idx = 0; idx < _scaleZones.size(); ++idx)
				_scaleZones[idx].load(*rrmStream);
		}

		// Read in the exits
		_exitZone = -1;
		int numExits = rrmStream->readByte();
		_exits.resize(numExits);

		for (int idx = 0; idx < numExits; ++idx)
			_exits[idx].load(*rrmStream, IS_ROSE_TATTOO);

		if (IS_SERRATED_SCALPEL)
			// Read in the entrance
			_entrance.load(*rrmStream);

		// Initialize sound list
		int numSounds = rrmStream->readByte();
		_sounds.resize(numSounds);

		for (int idx = 0; idx < numSounds; ++idx)
			_sounds[idx].load(*rrmStream);

		loadSceneSounds();

		if (IS_ROSE_TATTOO) {
			// Load the object sound list
			char buffer[27];
			
			_objSoundList.resize(rrmStream->readUint16LE());
			for (uint idx = 0; idx < _objSoundList.size(); ++idx) {
				rrmStream->read(buffer, 27);
				_objSoundList[idx] = Common::String(buffer);
			}
		} else {
			// Read in palette
			rrmStream->read(screen._cMap, PALETTE_SIZE);
			screen.translatePalette(screen._cMap);
			Common::copy(screen._cMap, screen._cMap + PALETTE_SIZE, screen._sMap);

			// Read in the background
			Common::SeekableReadStream *bgStream = !_lzwMode ? rrmStream :
				res.decompress(*rrmStream, SHERLOCK_SCREEN_WIDTH * SHERLOCK_SCENE_HEIGHT);

			bgStream->read(screen._backBuffer1.getPixels(), SHERLOCK_SCREEN_WIDTH * SHERLOCK_SCENE_HEIGHT);

			if (_lzwMode)
				delete bgStream;
		}

		// Backup the image and set the palette
		screen._backBuffer2.blitFrom(screen._backBuffer1);
		screen.setPalette(screen._cMap);

		delete rrmStream;
	}

	// Handle drawing any on-screen interface
	ui.drawInterface();

	checkSceneStatus();

	if (!saves._justLoaded) {
		for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
			if (_bgShapes[idx]._type == HIDDEN && _bgShapes[idx]._aType == TALK_EVERY)
				_bgShapes[idx].toggleHidden();
		}

		// Check for TURNON objects
		for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
			if (_bgShapes[idx]._type == HIDDEN && (_bgShapes[idx]._flags & TURNON_OBJ))
				_bgShapes[idx].toggleHidden();
		}

		// Check for TURNOFF objects
		for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
			if (_bgShapes[idx]._type != HIDDEN && (_bgShapes[idx]._flags & TURNOFF_OBJ) &&
					_bgShapes[idx]._type != INVALID)
				_bgShapes[idx].toggleHidden();
			if (_bgShapes[idx]._type == HIDE_SHAPE)
				// Hiding isn't needed, since objects aren't drawn yet
				_bgShapes[idx]._type = HIDDEN;
		}
	}

	checkSceneFlags(false);
	checkInventory();

	// Handle starting any music for the scene
	if (IS_SERRATED_SCALPEL && music._musicOn && music.loadSong(_currentScene))
		music.startSong();

	// Load walking images if not already loaded
	people.loadWalk();

	// Transition to the scene and setup entrance co-ordinates and animations
	transitionToScene();

	// Player has not yet walked in this scene
	_walkedInScene = false;
	saves._justLoaded = false;

	if (!_vm->isDemo()) {
		// Reset the previous map location and position on overhead map
		map._oldCharPoint = _currentScene;

		if (IS_SERRATED_SCALPEL) {
			map._overPos.x = (map[_currentScene].x - 6) * FIXED_INT_MULTIPLIER;
			map._overPos.y = (map[_currentScene].y + 9) * FIXED_INT_MULTIPLIER;

		}
	}

	events.clearEvents();
	return flag;
}

void Scene::loadSceneSounds() {
	Sound &sound = *_vm->_sound;

	for (uint idx = 0; idx < _sounds.size(); ++idx)
		sound.loadSound(_sounds[idx]._name, _sounds[idx]._priority);
}

void Scene::checkSceneStatus() {
	if (_sceneStats[_currentScene][64]) {
		for (uint idx = 0; idx < 64; ++idx) {
			bool flag = _sceneStats[_currentScene][idx];

			if (idx < _bgShapes.size()) {
				Object &obj = _bgShapes[idx];

				if (flag) {
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
		}
	}
}

void Scene::saveSceneStatus() {
	// Flag any objects for the scene that have been altered
	int count = MIN((int)_bgShapes.size(), 64);
	for (int idx = 0; idx < count; ++idx) {
		Object &obj = _bgShapes[idx];
		_sceneStats[_currentScene][idx] = obj._type == HIDDEN || obj._type == REMOVE
			|| obj._type == HIDE_SHAPE || obj._type == INVALID;
	}

	// Flag scene as having been visited
	_sceneStats[_currentScene][64] = true;
}

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
			} else if (_bgShapes[idx]._requiredFlag > 0) {
				// Restore object
				if (o._images == nullptr || o._images->size() == 0)
					o._type = NO_SHAPE;
				else
					o._type = ACTIVE_BG_SHAPE;
			}
		}
	}

	// Check inventory for items to remove based on flag changes
	for (int idx = 0; idx < _vm->_inventory->_holdings; ++idx) {
		InventoryItem &ii = (*_vm->_inventory)[idx];
		if (ii._requiredFlag && !_vm->readFlags(ii._requiredFlag)) {
			// Kill object: move it after the active holdings
			InventoryItem tempItem = (*_vm->_inventory)[idx];
			_vm->_inventory->insert_at(_vm->_inventory->_holdings, tempItem);
			_vm->_inventory->remove_at(idx);
			_vm->_inventory->_holdings--;
		}
	}

	// Check inactive inventory items for ones to reactivate based on flag changes
	for (uint idx = _vm->_inventory->_holdings; idx < _vm->_inventory->size(); ++idx) {
		InventoryItem &ii = (*_vm->_inventory)[idx];
		if (ii._requiredFlag && _vm->readFlags(ii._requiredFlag)) {
			// Restore object: move it after the active holdings
			InventoryItem tempItem = (*_vm->_inventory)[idx];
			_vm->_inventory->remove_at(idx);
			_vm->_inventory->insert_at(_vm->_inventory->_holdings, tempItem);
			_vm->_inventory->_holdings++;
		}
	}
}

void Scene::checkInventory() {
	for (uint shapeIdx = 0; shapeIdx < _bgShapes.size(); ++shapeIdx) {
		for (int invIdx = 0; invIdx < _vm->_inventory->_holdings; ++invIdx) {
			if (_bgShapes[shapeIdx]._name.equalsIgnoreCase((*_vm->_inventory)[invIdx]._name)) {
				_bgShapes[shapeIdx]._type = INVALID;
				break;
			}
		}
	}
}

void Scene::transitionToScene() {
	People &people = *_vm->_people;
	SaveManager &saves = *_vm->_saves;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	Common::Point &hSavedPos = people._hSavedPos;
	int &hSavedFacing = people._hSavedFacing;

	if (hSavedPos.x < 1) {
		// No exit information from last scene-check entrance info
		if (_entrance._startPosition.x < 1) {
			// No entrance info either, so use defaults
			hSavedPos = Common::Point(16000, 10000);
			hSavedFacing = 4;
		} else {
			// setup entrance info
			hSavedPos = _entrance._startPosition;
			hSavedFacing = _entrance._startDir;
		}
	} else {
		// Exit information exists, translate it to real sequence info
		// Note: If a savegame was just loaded, then the data is already correct.
		// Otherwise, this is a linked scene or entrance info, and must be translated
		if (hSavedFacing < 8 && !saves._justLoaded) {
			hSavedFacing = FS_TRANS[hSavedFacing];
			hSavedPos.x *= FIXED_INT_MULTIPLIER;
			hSavedPos.y *= FIXED_INT_MULTIPLIER;
		}
	}

	int cAnimNum = -1;

	if (hSavedFacing < 101) {
		// Standard info, so set it
		people[PLAYER]._position = hSavedPos;
		people[PLAYER]._sequenceNumber = hSavedFacing;
	} else {
		// It's canimation information
		cAnimNum = hSavedFacing - 101;
	}

	// Reset positioning for next load
	hSavedPos = Common::Point(-1, -1);
	hSavedFacing = -1;

	if (cAnimNum != -1) {
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
					Common::Point(people[PLAYER]._position.x / FIXED_INT_MULTIPLIER, 
					people[PLAYER]._position.y / FIXED_INT_MULTIPLIER))) {
				// Current point is already inside box - impact occurred on
				// a previous call. So simply do nothing except talk until the
				// player is clear of the box
				switch (obj._aType) {
				case FLAG_SET:
					for (int useNum = 0; useNum < USE_COUNT; ++useNum) {
						if (obj._use[useNum]._useFlag) {
							if (!_vm->readFlags(obj._use[useNum]._useFlag))
								_vm->setFlags(obj._use[useNum]._useFlag);
						}

						if (!talk._talkToAbort) {
							for (int nameIdx = 0; nameIdx < NAMES_COUNT; ++nameIdx) {
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

	// Actually do the transition
	if (screen._fadeStyle)
		screen.randomTransition();
	else
		screen.blitFrom(screen._backBuffer1);
	screen.update();

	// Start any initial animation for the scene
	if (cAnimNum != -1) {
		CAnim &c = _cAnim[cAnimNum];
		Common::Point pt = c._goto;

		c._goto = Common::Point(-1, -1);
		people[AL]._position = Common::Point(0, 0);

		startCAnim(cAnimNum, 1);
		c._goto = pt;
	}
}

int Scene::toggleObject(const Common::String &name) {
	int count = 0;

	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		if (name.equalsIgnoreCase(_bgShapes[idx]._name)) {
			++count;
			_bgShapes[idx].toggleHidden();
		}
	}

	return count;
}

void Scene::updateBackground() {
	People &people = *_vm->_people;

	// Update Holmes if he's turned on
	for (int idx = 0; idx < MAX_CHARACTERS; ++idx) {
		if (people[idx]._type == CHARACTER)
			people[idx].adjustSprite();
	}

	// Flag the bg shapes which need to be redrawn
	checkBgShapes();

	// Draw the shapes for the scene
	drawAllShapes();
}

Exit *Scene::checkForExit(const Common::Rect &r) {
	for (uint idx = 0; idx < _exits.size(); ++idx) {
		if (_exits[idx].intersects(r))
			return &_exits[idx];
	}

	return nullptr;
}

int Scene::startCAnim(int cAnimNum, int playRate) {
	Events &events = *_vm->_events;
	Map &map = *_vm->_map;
	People &people = *_vm->_people;
	Resources &res = *_vm->_res;
	Talk &talk = *_vm->_talk;
	UserInterface &ui = *_vm->_ui;
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

	CursorId oldCursor = events.getCursor();
	events.setCursor(WAIT);

	if (walkPos.x != -1) {
		// Holmes must walk to the walk point before the cAnimation is started
		if (people[AL]._position != walkPos)
			people.walkToCoords(walkPos, walkDir);
	}

	if (talk._talkToAbort)
		return 1;

	// Add new anim shape entry for displaying the animation
	_canimShapes.push_back(Object());
	Object &cObj = _canimShapes[_canimShapes.size() - 1];

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
	cObj._oldSize = Common::Point(0, 0);
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
			Common::SeekableReadStream *imgStream = !_lzwMode ? rrmStream->readStream(cAnim._size) :
				Resources::decompressLZ(*rrmStream, cAnim._size);
			res.addToCache(fname, *imgStream);

			delete imgStream;
			delete rrmStream;
		}

		// Now load the resource as an image
		cObj._images = new ImageFile(fname);
		cObj._imageFrame = &(*cObj._images)[0];
		cObj._maxFrames = cObj._images->size();

		int frames = 0;
		if (playRate < 0) {
			// Reverse direction
			// Count number of frames
			while (cObj._sequences[frames] && frames < MAX_FRAME)
				++frames;
		} else {
			// Forward direction
			Object::_countCAnimFrames = true;

			while (cObj._type == ACTIVE_BG_SHAPE) {
				cObj.checkObject();
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
				ui.printObjectDesc();

			doBgAnim();

			// Repeat same frame
			int temp = repeat;
			while (--temp > 0) {
				cObj._frameNumber--;
				doBgAnim();

				if (_vm->shouldQuit())
					return 0;
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

	// Unless anim shape has already been freed, set it to REMOVE so doBgAnim can free it
	if (_canimShapes.indexOf(cObj) != -1)
		cObj.checkObject();

	if (gotoCode > 0 && !talk._talkToAbort) {
		_goToScene = gotoCode;

		if (_goToScene < 97 && map[_goToScene].x) {
			map._overPos = map[_goToScene];
		}
	}

	people.loadWalk();

	if (tpPos.x != -1 && !talk._talkToAbort) {
		// Teleport to ending coordinates
		people[AL]._position = tpPos;
		people[AL]._sequenceNumber = tpDir;

		people.gotoStand(people[AL]);
	}

	events.setCursor(oldCursor);

	return 1;
}

int Scene::findBgShape(const Common::Rect &r) {
	if (!_doBgAnimDone)
		// New frame hasn't been drawn yet
		return -1;

	for (int idx = (int)_bgShapes.size() - 1; idx >= 0; --idx) {
		Object &o = _bgShapes[idx];
		if (o._type != INVALID && o._type != NO_SHAPE && o._type != HIDDEN
			&& o._aType <= PERSON) {
			if (r.intersects(o.getNewBounds()))
				return idx;
		} else if (o._type == NO_SHAPE) {
			if (r.intersects(o.getNoShapeBounds()))
				return idx;
		}
	}

	return -1;
}

int Scene::checkForZones(const Common::Point &pt, int zoneType) {
	int matches = 0;

	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if ((o._aType == zoneType && o._type != INVALID) && o._type != HIDDEN) {
			Common::Rect r = o._type == NO_SHAPE ? o.getNoShapeBounds() : o.getNewBounds();

			if (r.contains(pt)) {
				++matches;
				o.setFlagsAndToggles();
				_vm->_talk->talkTo(o._use[0]._target);
			}
		}
	}

	return matches;
}

int Scene::whichZone(const Common::Point &pt) {
	for (uint idx = 0; idx < _zones.size(); ++idx) {
		if (_zones[idx].contains(pt))
			return idx;
	}

	return -1;
}

int Scene::closestZone(const Common::Point &pt) {
	int dist = 1000;
	int zone = -1;

	for (uint idx = 0; idx < _zones.size(); ++idx) {
		Common::Point zc((_zones[idx].left + _zones[idx].right) / 2,
			(_zones[idx].top + _zones[idx].bottom) / 2);
		int d = ABS(zc.x - pt.x) + ABS(zc.y - pt.y);

		if (d < dist) {
			// Found a closer zone
			dist = d;
			zone = idx;
		}
	}

	return zone;
}

void Scene::synchronize(Common::Serializer &s) {
	if (s.isSaving())
		saveSceneStatus();

	if (s.isSaving()) {
		s.syncAsSint16LE(_currentScene);
	} else {
		s.syncAsSint16LE(_goToScene);
		_loadingSavedGame = true;
	}

	for (int sceneNum = 0; sceneNum < SCENES_COUNT; ++sceneNum) {
		for (int flag = 0; flag < 65; ++flag) {
			s.syncAsByte(_sceneStats[sceneNum][flag]);
		}
	}
}

void Scene::setNPCPath(int npc) {
	People &people = *_vm->_people;
	Talk &talk = *_vm->_talk;
	
	people[npc].clearNPC();
	people[npc]._name = Common::String::format("WATS%.2dA", _currentScene);

	// If we're in the middle of a script that will continue once the scene is loaded,
	// return without calling the path script
	if (talk._scriptMoreFlag == 1 || talk._scriptMoreFlag == 3)
		return;

	// Turn off all the NPCs, since the talk script will turn them back on as needed
	for (uint idx = 0; idx < MAX_NPC; ++idx)
		people[idx + 1]._type = INVALID;

	// Call the path script for the scene
	Common::String pathFile = Common::String::format("PATH%.2dA", _currentScene);
	talk.talkTo(pathFile);
}

void Scene::checkBgShapes() {
	People &people = *_vm->_people;
	Person &holmes = people._player;
	Common::Point pt(holmes._position.x / FIXED_INT_MULTIPLIER, holmes._position.y / FIXED_INT_MULTIPLIER);

	// Iterate through the shapes
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &obj = _bgShapes[idx];
		if (obj._type == ACTIVE_BG_SHAPE || (IS_SERRATED_SCALPEL && obj._type == STATIC_BG_SHAPE)) {
			if ((obj._flags & 5) == 1) {
				obj._misc = (pt.y < (obj._position.y + obj.frameHeight() - 1)) ?
					NORMAL_FORWARD : NORMAL_BEHIND;
			} else if (!(obj._flags & OBJ_BEHIND)) {
				obj._misc = BEHIND;
			} else if (obj._flags & OBJ_FORWARD) {
				obj._misc = FORWARD;
			}
		}
	}
}

} // End of namespace Sherlock
