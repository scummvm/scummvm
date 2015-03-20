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
#include "sherlock/decompress.h"

namespace Sherlock {

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
	_position.x = s.readSint16LE();
	_position.y = s.readSint16LE();
	_size.x = s.readSint16LE();
	_size.y = s.readSint16LE();
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
	_goToRoom = -1;
	_changes = false;
	_oldCharPoint = 0;
	_windowOpen = _infoFlag = false;
	_menuMode = _keyboardInput = 0;
	_walkedInScene = false;
	_ongoingCans = 0;
	_version = 0;
	_lzwMode = false;
	_invGraphicItems = 0;

	_controlPanel = new ImageFile("controls.vgs");
	_controls = nullptr; // new ImageFile("menu.all");
}

Scene::~Scene() {
	delete _controlPanel;
	delete _controls;
	clear();
}

/**
 * Takes care of clearing any scene data
 */
void Scene::clear() {
}

void Scene::selectScene() {
	// Reset fields
	_windowOpen = _infoFlag = false;
	_menuMode = _keyboardInput = 0;
	_oldKey = _help = _oldHelp = 0;
	_oldTemp = _temp = 0;

	// Load the scene
	Common::String sceneFile = Common::String::format("res%02d", _goToRoom);
	Common::String roomName = Common::String::format("res%02d.rrm", _goToRoom);
	_currentScene = _goToRoom;
	_goToRoom = -1;

	loadScene(sceneFile);
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
void Scene::loadScene(const Common::String &filename) {
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	bool flag;

	_walkedInScene = false;
	_ongoingCans = 0;

	// Reset the list of walkable areas
	_roomBounds.clear();
	_roomBounds.push_back(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));

	clear();
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

		// Set up inv list
		_inv.resize(bgHeader._numImages + 1);
		for (int idx = 0; idx < bgHeader._numImages; ++idx) {
			_inv[idx + 1]._filesize = bgInfo[idx]._filesize;
			_inv[idx + 1]._maxFrames = bgInfo[idx]._maxFrames;

			// Read in the image data
			Common::SeekableReadStream *imageStream = !_lzwMode ? rrmStream :
				decompressLZ(*rrmStream, bgInfo[idx]._filesize);

			_inv[idx + 1]._images = new ImageFile(*imageStream);

			if (_lzwMode)
				delete imageStream;
		}

		// Set up the bgShapes
		for (int idx = 0; idx < bgHeader._numStructs; ++idx) {
			_bgShapes[idx]._examine = Common::String(&_descText[_bgShapes[idx]._descOffset]);
			_bgShapes[idx]._sequences = &_sequenceBuffer[_bgShapes[idx]._sequenceOffset];
			_bgShapes[idx]._misc = 0;
			_bgShapes[idx]._seqCounter = 0;
			_bgShapes[idx]._seqcounter2 = 0;
			_bgShapes[idx]._seqStack = 0;
			_bgShapes[idx]._frameNumber = -1;
			_bgShapes[idx]._position = Common::Point(0, 0);
			_bgShapes[idx]._oldSize = Common::Point(1, 1);

			_bgShapes[idx]._images = _inv[_bgShapes[idx]._misc]._images;
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
		if (sound._sfxEnabled) {
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

	// TODO
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
	int mode = mode ? HIDE_SHAPE : HIDDEN;

	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		// TODO: read_flags calls
	}
}

/**
 * Checks scene objects against the player's inventory items. If there are any
 * matching names, it means the given item has already been picked up, and should
 * be hidden in the scene.
 */
void Scene::checkInventory() {

}

} // End of namespace Sherlock
