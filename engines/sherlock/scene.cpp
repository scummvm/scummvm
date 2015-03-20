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

int _fSize;					// How long images are
int _maxFrames;				// How many unique frames in object
Common::String _filename;	// Filename of object

/*----------------------------------------------------------------*/

Scene::Scene(SherlockEngine *vm): _vm(vm) {
	for (int idx = 0; idx < SCENES_COUNT; ++idx)
		Common::fill(&_stats[idx][0], &_stats[idx][9], false);
	_goToRoom = -1;
	_oldCharPoint = 0;
	_numExits = 0;
	_windowOpen = _infoFlag = false;
	_menuMode = _keyboardInput = 0;
	_walkedInScene = false;
	_ongoingCans = 0;
	_version = 0;
	_lzwMode = false;
	_invGraphicItems = 0;

	_controls = nullptr; // new ImageFile("menu.all");
}

Scene::~Scene() {
	delete _controls;
	clear();
}

/**
 * Takes care of clearing any scene data
 */
void Scene::clear() {
	for (uint idx = 0; idx < _bgShapes.size(); ++idx)
		delete _bgShapes[idx]._images;
}

void Scene::selectScene() {
	// Reset fields
	_numExits = 0;
	_windowOpen = _infoFlag = false;
	_menuMode = _keyboardInput = 0;
	_oldKey = _help = _oldHelp = 0;
	_oldTemp = _temp = 0;

	// Load the scene
	Common::String sceneFile = Common::String::format("res%02d", _goToRoom);
	Common::String roomName = Common::String::format("res%02d.rrm", _goToRoom);
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

		// TODO

		delete rrmStream;
	}

}

} // End of namespace Sherlock
