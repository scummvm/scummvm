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
#include "sherlock/screen.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/scalpel/scalpel_people.h"
#include "sherlock/scalpel/scalpel_scene.h"
#include "sherlock/scalpel/scalpel_screen.h"
#include "sherlock/scalpel/3do/scalpel_3do_screen.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_user_interface.h"

namespace Sherlock {

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

void BgFileHeaderInfo::load3DO(Common::SeekableReadStream &s) {
	_filesize = s.readUint32BE();
	_maxFrames = s.readByte();

	char buffer[9];
	s.read(buffer, 9);
	_filename = Common::String(buffer);
	s.skip(2); // only on 3DO!
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

	_newPosition.x = s.readSint16LE();
	_newPosition.y = s.readSint16LE();
	_newPosition._facing = s.readUint16LE();

	if (isRoseTattoo)
		_allow = s.readSint16LE();
}

void Exit::load3DO(Common::SeekableReadStream &s) {
	left = s.readSint16BE();
	top = s.readSint16BE();
	setWidth(s.readUint16BE());
	setHeight(s.readUint16BE());

	_image = 0;
	_scene = s.readSint16BE();

	_allow = s.readSint16BE();

	_newPosition.x = s.readSint16BE();
	_newPosition.y = s.readSint16BE();
	_newPosition._facing = s.readUint16BE();
	s.skip(2); // Filler
}

/*----------------------------------------------------------------*/

void SceneEntry::load(Common::SeekableReadStream &s) {
	_startPosition.x = s.readSint16LE();
	_startPosition.y = s.readSint16LE();
	_startDir = s.readByte();
	_allow = s.readByte();
}

void SceneEntry::load3DO(Common::SeekableReadStream &s) {
	_startPosition.x = s.readSint16BE();
	_startPosition.y = s.readSint16BE();
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

void SceneSound::load3DO(Common::SeekableReadStream &s) {
	load(s);
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

void WalkArray::load(Common::SeekableReadStream &s, bool isRoseTattoo) {
	_pointsCount = (int8)s.readByte();

	for (int idx = 0; idx < _pointsCount; ++idx) {
		int x = s.readSint16LE();
		int y = isRoseTattoo ? s.readSint16LE() : s.readByte();
		push_back(Common::Point(x, y));
	}
}

/*----------------------------------------------------------------*/

Scene *Scene::init(SherlockEngine *vm) {
	if (vm->getGameID() == GType_SerratedScalpel)
		return new Scalpel::ScalpelScene(vm);
	else
		return new Tattoo::TattooScene(vm);
}

Scene::Scene(SherlockEngine *vm): _vm(vm) {
	_sceneStats = new bool *[SCENES_COUNT];
	_sceneStats[0] = new bool[SCENES_COUNT * (MAX_BGSHAPES + 1)];
	Common::fill(&_sceneStats[0][0], &_sceneStats[0][SCENES_COUNT * (MAX_BGSHAPES + 1)], false);
	for (int idx = 1; idx < SCENES_COUNT; ++idx) {
		_sceneStats[idx] = _sceneStats[idx - 1] + (MAX_BGSHAPES + 1);
	}

	_currentScene = -1;
	_goToScene = -1;
	_walkedInScene = false;
	_version = 0;
	_compressed = false;
	_invGraphicItems = 0;
	_cAnimFramePause = 0;
	_restoreFlag = false;
	_animating = 0;
	_doBgAnimDone = true;
	_tempFadeStyle = 0;
	_doBgAnimDone = false;
}

Scene::~Scene() {
	freeScene();
	delete[] _sceneStats[0];
	delete[] _sceneStats;
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

	// Load the scene
	Common::String sceneFile = Common::String::format("res%02d", _goToScene);
	// _rrmName gets set during loadScene()
	// _rrmName is for ScalpelScene::startCAnim
	_currentScene = _goToScene;
	_goToScene = -1;

	loadScene(sceneFile);

	// If the fade style was changed from running a movie, then reset it
	if (_tempFadeStyle) {
		screen._fadeStyle = _tempFadeStyle;
		_tempFadeStyle = 0;
	}

	people[HOLMES]._walkDest = Common::Point(people[HOLMES]._position.x / FIXED_INT_MULTIPLIER,
		people[HOLMES]._position.y / FIXED_INT_MULTIPLIER);

	_restoreFlag = true;
	events.clearEvents();

	// If there were any scripts waiting to be run, but were interrupt by a running
	// canimation (probably the last scene's exit canim), clear the _scriptMoreFlag
	if (talk._scriptMoreFlag == 3)
		talk._scriptMoreFlag = 0;
}

void Scene::freeScene() {
	SaveManager &saves = *_vm->_saves;

	if (_currentScene == -1)
		return;

	_vm->_ui->clearWindow();
	_vm->_talk->freeTalkVars();
	_vm->_talk->clearSequences();
	_vm->_inventory->freeInv();
	_vm->_music->freeSong();
	_vm->_sound->freeLoadedSounds();

	if (!saves._justLoaded)
		saveSceneStatus();

	_sequenceBuffer.clear();
	_descText.clear();
	_walkPoints.clear();
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
	Music &music = *_vm->_music;
	People &people = *_vm->_people;
	Resources &res = *_vm->_res;
	SaveManager &saves = *_vm->_saves;
	Screen &screen = *_vm->_screen;
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

	//
	// Load the room resource file for the scene
	//

	if (!IS_3DO) {
		// PC version
		Common::String roomFilename = filename + ".rrm";
		_roomFilename = roomFilename;

		flag = _vm->_res->exists(roomFilename);
		if (flag) {
			Common::SeekableReadStream *rrmStream = _vm->_res->load(roomFilename);

			rrmStream->seek(39);
			if (IS_SERRATED_SCALPEL) {
				_version = rrmStream->readByte();
				_compressed = _version == 10;
			} else {
				_compressed = rrmStream->readByte() > 0;
			}

			// Go to header and read it in
			rrmStream->seek(rrmStream->readUint32LE());

			BgFileHeader bgHeader;
			bgHeader.load(*rrmStream, IS_ROSE_TATTOO);
			_invGraphicItems = bgHeader._numImages + 1;

			if (IS_ROSE_TATTOO) {
				// Resize the screen if necessary
				int fullWidth = SHERLOCK_SCREEN_WIDTH + bgHeader._scrollSize;
				if (screen._backBuffer1.width() != fullWidth) {
					screen._backBuffer1.create(fullWidth, SHERLOCK_SCREEN_HEIGHT);
					screen._backBuffer2.create(fullWidth, SHERLOCK_SCREEN_HEIGHT);
				}

				// Handle initializing the palette
				screen.initPaletteFade(bgHeader._bytesWritten);
				rrmStream->read(screen._cMap, PALETTE_SIZE);
				paletteLoaded();
				screen.translatePalette(screen._cMap);

				// Read in background
				if (_compressed) {
					res.decompress(*rrmStream, (byte *)screen._backBuffer1.getPixels(), fullWidth * SHERLOCK_SCREEN_HEIGHT);
				} else {
					rrmStream->read(screen._backBuffer1.getPixels(), fullWidth * SHERLOCK_SCREEN_HEIGHT);
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
				Common::SeekableReadStream *infoStream = !_compressed ? rrmStream : res.decompress(*rrmStream, bgHeader._numStructs * 625);

				_bgShapes.resize(bgHeader._numStructs);
				for (int idx = 0; idx < bgHeader._numStructs; ++idx)
					_bgShapes[idx].load(*infoStream, true);

				if (_compressed)
					delete infoStream;

				// Load description text
				_descText.resize(bgHeader._descSize);
				if (_compressed)
					res.decompress(*rrmStream, (byte *)&_descText[0], bgHeader._descSize);
				else
					rrmStream->read(&_descText[0], bgHeader._descSize);

				// Load sequences
				_sequenceBuffer.resize(bgHeader._seqSize);
				if (_compressed)
					res.decompress(*rrmStream, &_sequenceBuffer[0], bgHeader._seqSize);
				else
					rrmStream->read(&_sequenceBuffer[0], bgHeader._seqSize);
			} else if (!_compressed) {
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
				Common::SeekableReadStream *imageStream = _compressed ?
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
				Common::SeekableReadStream *cAnimStream = _compressed ?
					res.decompress(*rrmStream, animSize * bgHeader._numcAnimations) :
					rrmStream->readStream(animSize * bgHeader._numcAnimations);

				// Load cAnim offset table as well
				uint32 *cAnimOffsetTablePtr = new uint32[bgHeader._numcAnimations];
				uint32 *cAnimOffsetPtr = cAnimOffsetTablePtr;
				memset(cAnimOffsetTablePtr, 0, bgHeader._numcAnimations * sizeof(uint32));
 				if (IS_SERRATED_SCALPEL) {
					// Save current stream offset
					int32 curOffset = rrmStream->pos();
					rrmStream->seek(44); // Seek to cAnim-Offset-Table
					for (uint16 curCAnim = 0; curCAnim < bgHeader._numcAnimations; curCAnim++) {
						*cAnimOffsetPtr = rrmStream->readUint32LE();
						cAnimOffsetPtr++;
					}
					// Seek back to original stream offset
					rrmStream->seek(curOffset);
				}
				// TODO: load offset table for Rose Tattoo as well

				// Go to the start of the cAnimOffsetTable
				cAnimOffsetPtr = cAnimOffsetTablePtr;

				_cAnim.resize(bgHeader._numcAnimations);
				for (uint idx = 0; idx < _cAnim.size(); ++idx) {
					_cAnim[idx].load(*cAnimStream, IS_ROSE_TATTOO, *cAnimOffsetPtr);
					cAnimOffsetPtr++;
				}

				delete cAnimStream;
				delete[] cAnimOffsetTablePtr;
			}



			// Read in the room bounding areas
			int size = rrmStream->readUint16LE();
			Common::SeekableReadStream *boundsStream = !_compressed ? rrmStream :
				res.decompress(*rrmStream, size);

			_zones.resize(size / 10);
			for (uint idx = 0; idx < _zones.size(); ++idx) {
				_zones[idx].left = boundsStream->readSint16LE();
				_zones[idx].top = boundsStream->readSint16LE();
				_zones[idx].setWidth(boundsStream->readSint16LE() + 1);
				_zones[idx].setHeight(boundsStream->readSint16LE() + 1);
				boundsStream->skip(2);	// Skip unused scene number field
			}

			if (_compressed)
				delete boundsStream;

			// Ensure we've reached the path version byte
			if (rrmStream->readByte() != (IS_SERRATED_SCALPEL ? 254 : 251))
				error("Invalid scene path data");

			// Load the walk directory and walk data
			assert(_zones.size() < MAX_ZONES);


			for (uint idx1 = 0; idx1 < _zones.size(); ++idx1) {
				Common::fill(&_walkDirectory[idx1][0], &_walkDirectory[idx1][MAX_ZONES], 0);
				for (uint idx2 = 0; idx2 < _zones.size(); ++idx2)
					_walkDirectory[idx1][idx2] = rrmStream->readSint16LE();
			}

			// Read in the walk data
			size = rrmStream->readUint16LE();
			Common::SeekableReadStream *walkStream = !_compressed ? rrmStream->readStream(size) :
				res.decompress(*rrmStream, size);

			// Translate the file offsets of the walk directory to indexes in the loaded walk data
			for (uint idx1 = 0; idx1 < _zones.size(); ++idx1) {
				for (uint idx2 = 0; idx2 < _zones.size(); ++idx2) {
					int dataOffset = _walkDirectory[idx1][idx2];
					if (dataOffset == -1)
						continue;

					// Check to see if we've already loaded the walk set for the given data offset
					uint dataIndex = 0;
					while (dataIndex < _walkPoints.size() && _walkPoints[dataIndex]._fileOffset != dataOffset)
						++dataIndex;

					if (dataIndex == _walkPoints.size()) {
						// Walk data for that offset hasn't been loaded yet, so load it now
						_walkPoints.push_back(WalkArray());

						walkStream->seek(dataOffset);
						_walkPoints[_walkPoints.size() - 1]._fileOffset = dataOffset;
						_walkPoints[_walkPoints.size() - 1].load(*walkStream, IS_ROSE_TATTOO);
						dataIndex = _walkPoints.size() - 1;
					}

					_walkDirectory[idx1][idx2] = dataIndex;
				}
			}

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
				Common::SeekableReadStream *bgStream = !_compressed ? rrmStream :
					res.decompress(*rrmStream, SHERLOCK_SCREEN_WIDTH * SHERLOCK_SCENE_HEIGHT);

				bgStream->read(screen._backBuffer1.getPixels(), SHERLOCK_SCREEN_WIDTH * SHERLOCK_SCENE_HEIGHT);

				if (_compressed)
					delete bgStream;
			}

			// Backup the image and set the palette
			screen._backBuffer2.SHblitFrom(screen._backBuffer1);
			screen.setPalette(screen._cMap);

			delete rrmStream;
		}

	} else {
		// === 3DO version ===
		_roomFilename = "rooms/" + filename + ".rrm";
		flag = _vm->_res->exists(_roomFilename);
		if (!flag)
			error("loadScene: 3DO room data file not found");

		Common::SeekableReadStream *roomStream = _vm->_res->load(_roomFilename);
		uint32 roomStreamSize = roomStream->size();

		// there should be at least all bytes of the header data
		if (roomStreamSize < 128)
			error("loadScene: 3DO room data file is too small");

		// Read 3DO header
		roomStream->skip(4); // UINT32: offset graphic data?
		uint16 header3DO_numStructs = roomStream->readUint16BE();
		uint16 header3DO_numImages = roomStream->readUint16BE();
		uint16 header3DO_numAnimations = roomStream->readUint16BE();
		roomStream->skip(6);

		uint32 header3DO_bgInfo_offset        = roomStream->readUint32BE() + 0x80;
		uint32 header3DO_bgInfo_size          = roomStream->readUint32BE();
		uint32 header3DO_bgShapes_offset      = roomStream->readUint32BE() + 0x80;
		uint32 header3DO_bgShapes_size        = roomStream->readUint32BE();
		uint32 header3DO_descriptions_offset  = roomStream->readUint32BE() + 0x80;
		uint32 header3DO_descriptions_size    = roomStream->readUint32BE();
		uint32 header3DO_sequence_offset      = roomStream->readUint32BE() + 0x80;
		uint32 header3DO_sequence_size        = roomStream->readUint32BE();
		uint32 header3DO_cAnim_offset         = roomStream->readUint32BE() + 0x80;
		uint32 header3DO_cAnim_size           = roomStream->readUint32BE();
		uint32 header3DO_roomBounding_offset  = roomStream->readUint32BE() + 0x80;
		uint32 header3DO_roomBounding_size    = roomStream->readUint32BE();
		uint32 header3DO_walkDirectory_offset = roomStream->readUint32BE() + 0x80;
		uint32 header3DO_walkDirectory_size   = roomStream->readUint32BE();
		uint32 header3DO_walkData_offset      = roomStream->readUint32BE() + 0x80;
		uint32 header3DO_walkData_size        = roomStream->readUint32BE();
		uint32 header3DO_exits_offset         = roomStream->readUint32BE() + 0x80;
		uint32 header3DO_exits_size           = roomStream->readUint32BE();
		uint32 header3DO_entranceData_offset  = roomStream->readUint32BE() + 0x80;
		uint32 header3DO_entranceData_size    = roomStream->readUint32BE();
		uint32 header3DO_soundList_offset     = roomStream->readUint32BE() + 0x80;
		uint32 header3DO_soundList_size       = roomStream->readUint32BE();
		//uint32 header3DO_unknown_offset       = roomStream->readUint32BE() + 0x80;
		//uint32 header3DO_unknown_size         = roomStream->readUint32BE();
		roomStream->skip(8); // Skip over unknown offset+size
		uint32 header3DO_bgGraphicData_offset = roomStream->readUint32BE() + 0x80;
		uint32 header3DO_bgGraphicData_size   = roomStream->readUint32BE();

		// Calculate amount of entries
		int32 header3DO_soundList_count       = header3DO_soundList_size / 9;

		_invGraphicItems = header3DO_numImages + 1;

		// Verify all offsets
		if (header3DO_bgInfo_offset >= roomStreamSize)
			error("loadScene: 3DO bgInfo offset points outside of room file");
		if (header3DO_bgInfo_size > (roomStreamSize - header3DO_bgInfo_offset))
			error("loadScene: 3DO bgInfo size goes beyond room file");
		if (header3DO_bgShapes_offset >= roomStreamSize)
			error("loadScene: 3DO bgShapes offset points outside of room file");
		if (header3DO_bgShapes_size > (roomStreamSize - header3DO_bgShapes_offset))
			error("loadScene: 3DO bgShapes size goes beyond room file");
		if (header3DO_descriptions_offset >= roomStreamSize)
			error("loadScene: 3DO descriptions offset points outside of room file");
		if (header3DO_descriptions_size > (roomStreamSize - header3DO_descriptions_offset))
			error("loadScene: 3DO descriptions size goes beyond room file");
		if (header3DO_sequence_offset >= roomStreamSize)
			error("loadScene: 3DO sequence offset points outside of room file");
		if (header3DO_sequence_size > (roomStreamSize - header3DO_sequence_offset))
			error("loadScene: 3DO sequence size goes beyond room file");
		if (header3DO_cAnim_offset >= roomStreamSize)
			error("loadScene: 3DO cAnim offset points outside of room file");
		if (header3DO_cAnim_size > (roomStreamSize - header3DO_cAnim_offset))
			error("loadScene: 3DO cAnim size goes beyond room file");
		if (header3DO_roomBounding_offset >= roomStreamSize)
			error("loadScene: 3DO roomBounding offset points outside of room file");
		if (header3DO_roomBounding_size > (roomStreamSize - header3DO_roomBounding_offset))
			error("loadScene: 3DO roomBounding size goes beyond room file");
		if (header3DO_walkDirectory_offset >= roomStreamSize)
			error("loadScene: 3DO walkDirectory offset points outside of room file");
		if (header3DO_walkDirectory_size > (roomStreamSize - header3DO_walkDirectory_offset))
			error("loadScene: 3DO walkDirectory size goes beyond room file");
		if (header3DO_walkData_offset >= roomStreamSize)
			error("loadScene: 3DO walkData offset points outside of room file");
		if (header3DO_walkData_size > (roomStreamSize - header3DO_walkData_offset))
			error("loadScene: 3DO walkData size goes beyond room file");
		if (header3DO_exits_offset >= roomStreamSize)
			error("loadScene: 3DO exits offset points outside of room file");
		if (header3DO_exits_size > (roomStreamSize - header3DO_exits_offset))
			error("loadScene: 3DO exits size goes beyond room file");
		if (header3DO_entranceData_offset >= roomStreamSize)
			error("loadScene: 3DO entranceData offset points outside of room file");
		if (header3DO_entranceData_size > (roomStreamSize - header3DO_entranceData_offset))
			error("loadScene: 3DO entranceData size goes beyond room file");
		if (header3DO_soundList_offset >= roomStreamSize)
			error("loadScene: 3DO soundList offset points outside of room file");
		if (header3DO_soundList_size > (roomStreamSize - header3DO_soundList_offset))
			error("loadScene: 3DO soundList size goes beyond room file");
		if (header3DO_bgGraphicData_offset >= roomStreamSize)
			error("loadScene: 3DO bgGraphicData offset points outside of room file");
		if (header3DO_bgGraphicData_size > (roomStreamSize - header3DO_bgGraphicData_offset))
			error("loadScene: 3DO bgGraphicData size goes beyond room file");

		// === BGINFO === read in the shapes header info
		Common::Array<BgFileHeaderInfo> bgInfo;

		uint32 expected3DO_bgInfo_size = header3DO_numStructs * 16;
		if (expected3DO_bgInfo_size != header3DO_bgInfo_size) // Security check
			error("loadScene: 3DO bgInfo size mismatch");

		roomStream->seek(header3DO_bgInfo_offset);
		bgInfo.resize(header3DO_numStructs);
		for (uint idx = 0; idx < bgInfo.size(); ++idx)
			bgInfo[idx].load3DO(*roomStream);

		// === BGSHAPES === read in the shapes info
		uint32 expected3DO_bgShapes_size = header3DO_numStructs * 588;
		if (expected3DO_bgShapes_size != header3DO_bgShapes_size) // Security check
			error("loadScene: 3DO bgShapes size mismatch");

		roomStream->seek(header3DO_bgShapes_offset);
		_bgShapes.resize(header3DO_numStructs);
		for (int idx = 0; idx < header3DO_numStructs; ++idx)
			_bgShapes[idx].load3DO(*roomStream);

		// === DESCRIPTION === read description text
		if (header3DO_descriptions_size) {
			roomStream->seek(header3DO_descriptions_offset);
			_descText.resize(header3DO_descriptions_size);
			roomStream->read(&_descText[0], header3DO_descriptions_size);
		}

		// === SEQUENCE === read sequence buffer
		if (header3DO_sequence_size) {
			roomStream->seek(header3DO_sequence_offset);
			_sequenceBuffer.resize(header3DO_sequence_size);
			roomStream->read(&_sequenceBuffer[0], header3DO_sequence_size);
		}

		// === IMAGES === set up the list of images used by the scene
		roomStream->seek(header3DO_bgGraphicData_offset);
		_images.resize(header3DO_numImages + 1);
		for (int idx = 0; idx < header3DO_numImages; ++idx) {
			_images[idx + 1]._filesize = bgInfo[idx]._filesize;
			_images[idx + 1]._maxFrames = bgInfo[idx]._maxFrames;

			// Read image data into memory
			Common::SeekableReadStream *imageStream = roomStream->readStream(bgInfo[idx]._filesize);

			// Load image data into an ImageFile array as room file data
			// which is basically a fixed header, followed by a raw cel header, followed by regular cel data
			_images[idx + 1]._images = new ImageFile3DO(*imageStream, true);

			delete imageStream;
		}

		// === BGSHAPES === Set up the bgShapes
		for (int idx = 0; idx < header3DO_numStructs; ++idx) {
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

		// === CANIM === read cAnim list
		_cAnim.clear();
		if (header3DO_numAnimations) {
			roomStream->seek(header3DO_cAnim_offset);
			Common::SeekableReadStream *cAnimStream = roomStream->readStream(header3DO_cAnim_size);

			uint32 *cAnimOffsetTablePtr = new uint32[header3DO_numAnimations];
			uint32 *cAnimOffsetPtr = cAnimOffsetTablePtr;
			uint32 cAnimOffset = 0;
			memset(cAnimOffsetTablePtr, 0, header3DO_numAnimations * sizeof(uint32));

			// Seek to end of graphics data and load cAnim offset table from there
			roomStream->seek(header3DO_bgGraphicData_offset + header3DO_bgGraphicData_size);
			for (uint16 curCAnim = 0; curCAnim < header3DO_numAnimations; curCAnim++) {
				cAnimOffset = roomStream->readUint32BE();
				if (cAnimOffset >= roomStreamSize)
					error("loadScene: 3DO cAnim entry offset points outside of room file");

				*cAnimOffsetPtr = cAnimOffset;
				cAnimOffsetPtr++;
			}

			// Go to the start of the cAnimOffsetTable
			cAnimOffsetPtr = cAnimOffsetTablePtr;

			_cAnim.resize(header3DO_numAnimations);
			for (uint idx = 0; idx < _cAnim.size(); ++idx) {
				_cAnim[idx].load3DO(*cAnimStream, *cAnimOffsetPtr);
				cAnimOffsetPtr++;
			}

			delete cAnimStream;
			delete[] cAnimOffsetTablePtr;
		}

		// === BOUNDING AREAS === Read in the room bounding areas
		int roomBoundingCount = header3DO_roomBounding_size / 12;
		uint32 expected3DO_roomBounding_size = roomBoundingCount * 12;
		if (expected3DO_roomBounding_size != header3DO_roomBounding_size)
			error("loadScene: 3DO roomBounding size mismatch");

		roomStream->seek(header3DO_roomBounding_offset);
		_zones.resize(roomBoundingCount);
		for (uint idx = 0; idx < _zones.size(); ++idx) {
			_zones[idx].left = roomStream->readSint16BE();
			_zones[idx].top = roomStream->readSint16BE();
			_zones[idx].setWidth(roomStream->readSint16BE() + 1);
			_zones[idx].setHeight(roomStream->readSint16BE() + 1);
			roomStream->skip(4); // skip UINT32
		}

		// === WALK DIRECTORY === Load the walk directory
		uint32 expected3DO_walkDirectory_size = _zones.size() * _zones.size() * 2;
		if (expected3DO_walkDirectory_size != header3DO_walkDirectory_size)
			error("loadScene: 3DO walkDirectory size mismatch");

		roomStream->seek(header3DO_walkDirectory_offset);
		assert(_zones.size() < MAX_ZONES);
		for (uint idx1 = 0; idx1 < _zones.size(); ++idx1) {
			for (uint idx2 = 0; idx2 < _zones.size(); ++idx2)
				_walkDirectory[idx1][idx2] = roomStream->readSint16BE();
		}

		// === WALK DATA === Read in the walk data
		roomStream->seek(header3DO_walkData_offset);

		// Read in the walk data
		Common::SeekableReadStream *walkStream = !_compressed ? roomStream->readStream(header3DO_walkData_size) :
			res.decompress(*roomStream, header3DO_walkData_size);

		// Translate the file offsets of the walk directory to indexes in the loaded walk data
		for (uint idx1 = 0; idx1 < _zones.size(); ++idx1) {
			for (uint idx2 = 0; idx2 < _zones.size(); ++idx2) {
				int dataOffset = _walkDirectory[idx1][idx2];
				if (dataOffset == -1)
					continue;

				// Check to see if we've already loaded the walk set for the given data offset
				uint dataIndex = 0;
				while (dataIndex < _walkPoints.size() && _walkPoints[dataIndex]._fileOffset != dataOffset)
					++dataIndex;

				if (dataIndex == _walkPoints.size()) {
					// Walk data for that offset hasn't been loaded yet, so load it now
					_walkPoints.push_back(WalkArray());

					walkStream->seek(dataOffset);
					_walkPoints[_walkPoints.size() - 1]._fileOffset = dataOffset;
					_walkPoints[_walkPoints.size() - 1].load(*walkStream, IS_ROSE_TATTOO);
					dataIndex = _walkPoints.size() - 1;
				}

				_walkDirectory[idx1][idx2] = dataIndex;
			}
		}

		delete walkStream;

		// === EXITS === Read in the exits
		roomStream->seek(header3DO_exits_offset);

		int exitsCount = header3DO_exits_size / 20;

		_exits.resize(exitsCount);
		for (int idx = 0; idx < exitsCount; ++idx)
			_exits[idx].load3DO(*roomStream);

		// === ENTRANCE === Read in the entrance
		if (header3DO_entranceData_size != 8)
			error("loadScene: 3DO entranceData size mismatch");

		roomStream->seek(header3DO_entranceData_offset);
		_entrance.load3DO(*roomStream);

		// === SOUND LIST === Initialize sound list
		roomStream->seek(header3DO_soundList_offset);
		_sounds.resize(header3DO_soundList_count);
		for (int idx = 0; idx < header3DO_soundList_count; ++idx)
			_sounds[idx].load3DO(*roomStream);

		delete roomStream;

		// === BACKGROUND PICTURE ===
		// load from file rooms\[filename].bg
		// it's uncompressed 15-bit RGB555 data

		Common::String roomBackgroundFilename = "rooms/" + filename + ".bg";
		flag = _vm->_res->exists(roomBackgroundFilename);
		if (!flag)
			error("loadScene: 3DO room background file not found (%s)", roomBackgroundFilename.c_str());

		Common::File roomBackgroundStream;
		if (!roomBackgroundStream.open(roomBackgroundFilename))
			error("Could not open file - %s", roomBackgroundFilename.c_str());

		int totalPixelCount = SHERLOCK_SCREEN_WIDTH * SHERLOCK_SCENE_HEIGHT;
		uint16 *roomBackgroundDataPtr = NULL;
		uint16 *pixelSourcePtr = NULL;
		uint16 *pixelDestPtr = (uint16 *)screen._backBuffer1.getPixels();
		uint16  curPixel = 0;
		uint32  roomBackgroundStreamSize = roomBackgroundStream.size();
		uint32  expectedBackgroundSize   = totalPixelCount * 2;

		// Verify file size of background file
		if (expectedBackgroundSize != roomBackgroundStreamSize)
			error("loadScene: 3DO room background file not expected size");

		roomBackgroundDataPtr = new uint16[totalPixelCount];
		roomBackgroundStream.read(roomBackgroundDataPtr, roomBackgroundStreamSize);
		roomBackgroundStream.close();

		// Convert data from RGB555 to RGB565
		pixelSourcePtr = roomBackgroundDataPtr;
		for (int pixels = 0; pixels < totalPixelCount; pixels++) {
			curPixel = READ_BE_UINT16(pixelSourcePtr++);

			byte curPixelRed   = (curPixel >> 10) & 0x1F;
			byte curPixelGreen = (curPixel >> 5) & 0x1F;
			byte curPixelBlue  = curPixel & 0x1F;
			*pixelDestPtr = ((curPixelRed << 11) | (curPixelGreen << 6) | (curPixelBlue));
			pixelDestPtr++;
		}

		delete[] roomBackgroundDataPtr;

#if 0
		// code to show the background
		screen.SHblitFrom(screen._backBuffer1);
		_vm->_events->wait(10000);
#endif

		// Backup the image
		screen._backBuffer2.SHblitFrom(screen._backBuffer1);
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

	events.clearEvents();
	return flag;
}

void Scene::loadSceneSounds() {
	Sound &sound = *_vm->_sound;

	for (uint idx = 0; idx < _sounds.size(); ++idx)
		sound.loadSound(_sounds[idx]._name, _sounds[idx]._priority);
}

void Scene::checkSceneStatus() {
	if (_sceneStats[_currentScene][MAX_BGSHAPES]) {
		for (int idx = 0; idx < MAX_BGSHAPES; ++idx) {
			bool flag = _sceneStats[_currentScene][idx];

			if (idx < (int)_bgShapes.size()) {
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
	int count = MIN((int)_bgShapes.size(), MAX_BGSHAPES);
	for (int idx = 0; idx < count; ++idx) {
		Object &obj = _bgShapes[idx];
		_sceneStats[_currentScene][idx] = obj._type == HIDDEN || obj._type == REMOVE
			|| obj._type == HIDE_SHAPE || obj._type == INVALID;
	}

	// Flag scene as having been visited
	_sceneStats[_currentScene][MAX_BGSHAPES] = true;
}

void Scene::checkSceneFlags(bool flag) {
	SpriteType mode = flag ? HIDE_SHAPE : HIDDEN;

	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];

		if (o._requiredFlag[0] || o._requiredFlag[1]) {
			bool objectFlag = true;
			if (o._requiredFlag[0] != 0)
				objectFlag = _vm->readFlags(o._requiredFlag[0]);
			if (o._requiredFlag[1] != 0)
				objectFlag &= _vm->readFlags(o._requiredFlag[1]);

			if (!objectFlag) {
				// Kill object
				if (o._type != HIDDEN && o._type != INVALID) {
					if (o._images == nullptr || o._images->size() == 0)
						// No shape to erase, so flag as hidden
						o._type = HIDDEN;
					else
						// Flag it as needing to be hidden after first erasing it
						o._type = mode;
				}
			} else if (IS_ROSE_TATTOO || o._requiredFlag[0] > 0) {
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
	Point32 &hSavedPos = people._savedPos;
	int &hSavedFacing = people._savedPos._facing;

	if (hSavedPos.x < 1) {
		// No exit information from last scene-check entrance info
		if (_entrance._startPosition.x < 1) {
			// No entrance info either, so use defaults
			if (IS_SERRATED_SCALPEL) {
				hSavedPos = Point32(160 * FIXED_INT_MULTIPLIER, 100 * FIXED_INT_MULTIPLIER);
				hSavedFacing = 4;
			} else {
				hSavedPos = people[HOLMES]._position;
				hSavedFacing = people[HOLMES]._sequenceNumber;
			}
		} else {
			// setup entrance info
			hSavedPos.x = _entrance._startPosition.x * FIXED_INT_MULTIPLIER;
			hSavedPos.y = _entrance._startPosition.y * FIXED_INT_MULTIPLIER;
			if (IS_SERRATED_SCALPEL) {
				hSavedPos.x /= 100;
				hSavedPos.y /= 100;
			}

			hSavedFacing = _entrance._startDir;
		}
	} else {
		// Exit information exists, translate it to real sequence info
		// Note: If a savegame was just loaded, then the data is already correct.
		// Otherwise, this is a linked scene or entrance info, and must be translated
		if (hSavedFacing < 8 && !saves._justLoaded) {
			if (IS_ROSE_TATTOO)
				hSavedFacing = Tattoo::FS_TRANS[hSavedFacing];
			else
				hSavedFacing = Scalpel::FS_TRANS[hSavedFacing];

			hSavedPos.x *= FIXED_INT_MULTIPLIER;
			hSavedPos.y *= FIXED_INT_MULTIPLIER;
		}
	}

	int cAnimNum = -1;

	if (!saves._justLoaded) {
		if (hSavedFacing < 101) {
			// Standard info, so set it
			people[HOLMES]._position = hSavedPos;
			people[HOLMES]._sequenceNumber = hSavedFacing;
		} else {
			// It's canimation information
			cAnimNum = hSavedFacing - 101;
		}
	}

	// Reset positioning for next load
	hSavedPos = Common::Point(-1, -1);
	hSavedFacing = -1;

	if (cAnimNum != -1) {
		// Prevent Holmes from being drawn
		people[HOLMES]._position = Common::Point(0, 0);
	}

	// If the scene is capable of scrolling, set the current scroll so that whoever has control
	// of the scroll code is in the middle of the screen
	if (screen._backBuffer1.width() > SHERLOCK_SCREEN_WIDTH)
		people[people._walkControl].centerScreenOnPerson();

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
					Common::Point(people[HOLMES]._position.x / FIXED_INT_MULTIPLIER,
					people[HOLMES]._position.y / FIXED_INT_MULTIPLIER))) {
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
	if (screen._fadeStyle) {
		if (!IS_3DO) {
			// do pixel-transition for PC
			screen.randomTransition();
		} else {
			// fade in for 3DO
			screen.clear();
			static_cast<Scalpel::Scalpel3DOScreen *>(_vm->_screen)->fadeIntoScreen3DO(3);
		}
	} else {
		screen.slamArea(screen._currentScroll.x, screen._currentScroll.y, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
	}
	screen.update();

	// Start any initial animation for the scene
	if (cAnimNum != -1) {
		CAnim &c = _cAnim[cAnimNum];
		PositionFacing pt = c._goto[0];

		c._goto[0].x = c._goto[0].y = -1;
		people[HOLMES]._position = Common::Point(0, 0);

		startCAnim(cAnimNum, 1);
		c._goto[0] = pt;
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

void Scene::synchronize(Serializer &s) {
	if (s.isSaving())
		saveSceneStatus();

	if (s.isSaving()) {
		s.syncAsSint16LE(_currentScene);
	} else {
		s.syncAsSint16LE(_goToScene);
	}

	for (int sceneNum = 1; sceneNum < SCENES_COUNT; ++sceneNum) {
		for (int flag = 0; flag <= MAX_BGSHAPES; ++flag) {
			s.syncAsByte(_sceneStats[sceneNum][flag]);
		}
	}
}

void Scene::checkBgShapes() {
	People &people = *_vm->_people;
	Person &holmes = people[HOLMES];
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
