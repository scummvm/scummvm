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

#include "hdb/hdb.h"
#include "hdb/ai.h"
#include "hdb/file-manager.h"
#include "hdb/gfx.h"
#include "hdb/map.h"

namespace HDB {

Map::Map() {

	if (g_hdb->isPPC()) {
		_screenXTiles = 9;
		_screenYTiles = 11;
		_screenTileWidth = 8;
		_screenTileHeight = 10;
	} else {
		_screenXTiles = 17;
		_screenYTiles = 16;
		_screenTileWidth = 16;
		_screenTileHeight = 16;
	}

	_mapLoaded = false;

	_animCycle = 0;

	_numForegrounds = _numGratings = 0;

	_mapExplosions = NULL;
	_mapExpBarrels = NULL;
	_mapLaserBeams = NULL;

	_background = NULL;
	_foreground = NULL;
	_iconList = NULL;

	_width = 0;
	_height = 0;
	_mapX = 0;
	_mapY = 0;
	_mapTileX = 0;
	_mapTileY = 0;
	_mapTileXOff = 0;
	_mapTileYOff = 0;
	_backgroundOffset = 0;
	_foregroundOffset = 0;
	_iconNum = 0;
	_iconListOffset = 0;
	_infoNum = 0;
}

Map::~Map() {
	delete[] _background;
	delete[] _foreground;
	delete[] _iconList;

	free(_mapExplosions);
	free(_mapExpBarrels);
	free(_mapLaserBeams);
}

void Map::save(Common::OutSaveFile *out) {

	int i;

	// Save Map Dimensions and Offsets
	out->writeSint32LE(_mapX);
	out->writeSint32LE(_mapY);
	out->writeSint32LE(_width);
	out->writeSint32LE(_height);
	out->writeSint32LE(_mapTileX);
	out->writeSint32LE(_mapTileY);
	out->writeSint32LE(_mapTileXOff);
	out->writeSint32LE(_mapTileYOff);

	// Save out all Level2 Gratings and AnimCycle

	out->writeSint32LE(_numGratings);
	for (i = 0; i < _numGratings; i++) {
		out->writeUint16LE(_gratings[i].x);
		out->writeUint16LE(_gratings[i].y);
		out->writeUint16LE(_gratings[i].tile);
	}

	out->writeSint32LE(_animCycle);

	// find out how many SLOW, MEDIUM & FAST bg tile anims there are and save them all out
	out->writeUint32LE(_listBGAnimSlow.size());
	for (i = 0; (uint)i < _listBGAnimSlow.size(); i++) {
		out->writeUint32LE(_listBGAnimSlow[i]);
	}

	out->writeUint32LE(_listBGAnimMedium.size());
	for (i = 0; (uint)i < _listBGAnimMedium.size(); i++) {
		out->writeUint32LE(_listBGAnimMedium[i]);
	}

	out->writeUint32LE(_listBGAnimFast.size());
	for (i = 0; (uint)i < _listBGAnimFast.size(); i++) {
		out->writeUint32LE(_listBGAnimFast[i]);
	}

	// find out how many SLOW, MEDIUM & FAST fg tile anims there are and save them all out

	out->writeUint32LE(_listFGAnimSlow.size());
	for (i = 0; (uint)i < _listFGAnimSlow.size(); i++) {
		out->writeUint32LE(_listFGAnimSlow[i]);
	}

	out->writeUint32LE(_listFGAnimMedium.size());
	for (i = 0; (uint)i < _listFGAnimMedium.size(); i++) {
		out->writeUint32LE(_listFGAnimMedium[i]);
	}

	out->writeUint32LE(_listFGAnimFast.size());
	for (i = 0; (uint)i < _listFGAnimFast.size(); i++) {
		out->writeUint32LE(_listFGAnimFast[i]);
	}

	// save map data
	for (i = 0; i < _width * _height; i++)
		out->writeSint32LE(_background[i]);
	for (i = 0; i < _width * _height; i++)
		out->writeSint32LE(_foreground[i]);
	for (i = 0; i < _width * _height; i++)
		out->writeByte(_mapExplosions[i]);
	for (i = 0; i < _width * _height; i++)
		out->writeByte(_mapExpBarrels[i]);
	for (i = 0; i < _width * _height; i++)
		out->writeByte(_mapLaserBeams[i]);
}

void Map::loadSaveFile(Common::InSaveFile *in) {
	restartSystem();

	// Load Map Dimensions and Offsets
	_mapX = in->readSint32LE();
	_mapY = in->readSint32LE();
	_width = in->readUint32LE();
	_height = in->readUint32LE();
	_mapTileX = in->readSint32LE();
	_mapTileY = in->readSint32LE();
	_mapTileXOff = in->readSint32LE();
	_mapTileYOff = in->readSint32LE();

	uint size = _width * _height;

	// Load All level2 gratings and animCycle

	_numGratings = in->readSint32LE();
	for (int i = 0; i < _numGratings; i++) {
		_gratings[i].x = in->readUint16LE();
		_gratings[i].y = in->readUint16LE();
		_gratings[i].tile = in->readUint16LE();
	}

	_animCycle = in->readSint32LE();

	// find out how many SLOW, MEDIUM & FAST bg tile anims there are and load them all out
	_listBGAnimSlow.resize(in->readUint32LE());
	for (int i = 0; (uint)i < _listBGAnimSlow.size(); i++) {
		_listBGAnimSlow[i] = in->readUint32LE();
	}

	_listBGAnimMedium.resize(in->readUint32LE());
	for (int i = 0; (uint)i < _listBGAnimMedium.size(); i++) {
		_listBGAnimMedium[i] = in->readUint32LE();
	}

	_listBGAnimFast.resize(in->readUint32LE());
	for (int i = 0; (uint)i < _listBGAnimFast.size(); i++) {
		_listBGAnimFast[i] = in->readUint32LE();
	}

	// find out how many SLOW, MEDIUM & FAST fg tile anims there are and load them all out

	_listFGAnimSlow.resize(in->readUint32LE());
	for (int i = 0; (uint)i < _listFGAnimSlow.size(); i++) {
		_listFGAnimSlow[i] = in->readUint32LE();
	}

	_listFGAnimMedium.resize(in->readUint32LE());
	for (int i = 0; (uint)i < _listFGAnimMedium.size(); i++) {
		_listFGAnimMedium[i] = in->readUint32LE();
	}

	_listFGAnimFast.resize(in->readUint32LE());
	for (uint i = 0; (uint)i < _listFGAnimFast.size(); i++) {
		_listFGAnimFast[i] = in->readUint32LE();
	}

	// load map data

	_background = new int16[size];
	for (uint i = 0; i < size; i++)
		_background[i] = in->readSint32LE();

	_foreground = new int16[size];
	for (uint i = 0; i < size; i++)
		_foreground[i] = in->readSint32LE();

	_mapExplosions = (byte *)malloc(size);
	for (uint i = 0; i < size; i++)
		_mapExplosions[i] = in->readByte();

	_mapExpBarrels = (byte *)malloc(size);
	for (uint i = 0; i < size; i++)
		_mapExpBarrels[i] = in->readByte();

	_mapLaserBeams = (byte *)malloc(size);
	for (uint i = 0; i < size; i++)
		_mapLaserBeams[i] = in->readByte();

	// load all the map's tiles (cache)
	loadTiles();
	_mapLoaded = true;
}

int Map::loadTiles() {

	int temp;
	int skyIndex = 0;

	// Load all tiles
	for (uint j = 0; j < _height; j++) {
		for (uint i = 0; i < _width; i++) {
			int tile = _background[j * _width + i];
			if ((temp = g_hdb->_gfx->isSky(tile)) && !skyIndex) {
				skyIndex = temp;
			}
			g_hdb->_gfx->getTile(tile);
			g_hdb->_gfx->getTile(_foreground[j * _width + i]);
		}
	}

	return skyIndex;
}

void Map::restartSystem() {
	_listBGAnimFast.clear();
	_listBGAnimMedium.clear();
	_listBGAnimSlow.clear();
	_listFGAnimFast.clear();
	_listFGAnimMedium.clear();
	_listFGAnimSlow.clear();

	delete[] _background;
	_background = NULL;
	delete[] _foreground;
	_foreground = NULL;
	delete[] _iconList;
	_iconList = NULL;

	_width = _height = 0;
	_animCycle = 0;

	free(_mapExplosions);
	free(_mapExpBarrels);
	free(_mapLaserBeams);

	_mapExplosions = NULL;
	_mapExpBarrels = NULL;
	_mapLaserBeams = NULL;

	// mark all in-memory tiles as being in memory, but able to be freed
	g_hdb->_gfx->markTileCacheFreeable();
	g_hdb->_gfx->markGfxCacheFreeable();

	_mapLoaded = false;
}

bool Map::loadMap(char *name) {
	Common::SeekableReadStream *mapStream = g_hdb->_fileMan->findFirstData(name, TYPE_BINARY);
	if (mapStream == NULL) {
		warning("The %s MPC entry can't be found", name);
		delete mapStream;
		return false;
	}

	load(mapStream);
	delete mapStream;

	return true;
}

bool Map::load(Common::SeekableReadStream *stream) {
	debug(5, "map stream size: %d(%x)", stream->size(), stream->size());

	// Load MSM data header
	stream->read(_name, 32);
	_width = stream->readUint16LE();
	_height = stream->readUint16LE();
	_backgroundOffset = stream->readUint32LE();
	_foregroundOffset = stream->readUint32LE();
	_iconNum = stream->readUint16LE();
	_infoNum = stream->readUint16LE();            // not used in the original
	_iconListOffset = stream->readUint32LE();
	_infoListOffset = stream->readUint32LE();     // not used in the original

	debug(5, "map: w: %d(%x), h: %d(%x) bg: %x fg: %x icon#: %d(%x) icon: %x info#: %d(%x) info: %x",
			_width, _width, _height, _height, _backgroundOffset, _foregroundOffset, _iconNum, _iconNum,
			_iconListOffset, _infoNum, _infoNum, _infoListOffset);

	uint size = _width * _height;
	// Reading Background
	_background = new int16[size];
	stream->seek(_backgroundOffset);
	for (uint i = 0; i < size; i++) {
		_background[i] = stream->readUint16LE();
	}
	if (gDebugLevel >= 5) {
		debug(5, "Background:");
		Common::hexdump((const byte *)_background, 512);
	}

	// Reading Foreground
	_foreground = new int16[size];
	stream->seek(_foregroundOffset);
	for (uint i = 0; i < size; i++) {
		_foreground[i] = stream->readUint16LE();
	}

	if (gDebugLevel >= 5) {
		debug(5, "Foreground:");
		Common::hexdump((const byte *)_foreground, 512);
	}

	// Reading Icon List
	_iconList = new MSMIcon[_iconNum];
	for (uint i = 0; i < _iconNum; i++) {
		_iconList[i].icon = stream->readUint16LE();
		_iconList[i].x = stream->readUint16LE();
		_iconList[i].y = stream->readUint16LE();

		stream->read(_iconList[i].funcInit, 32);
		stream->read(_iconList[i].funcAction, 32);
		stream->read(_iconList[i].funcUse, 32);

		_iconList[i].dir = stream->readUint16LE();
		_iconList[i].level = stream->readUint16LE();
		_iconList[i].value1 = stream->readUint16LE();
		_iconList[i].value2 = stream->readUint16LE();
	}

	g_hdb->setInMapName(_name);

	_mapExplosions = (byte *)calloc(size, 1);
	_mapExpBarrels = (byte *)calloc(size, 1);
	_mapLaserBeams = (byte *)calloc(size, 1);

	int sky = loadTiles();
	g_hdb->_gfx->setSky(sky);
	_mapX = _mapY = 0;

	// Setup animating Tile lists
	for (uint i = 0; i < size; i++) {
		addBGTileAnimation(i % _width, i / _width);
		addFGTileAnimation(i % _width, i / _width);
	}

	struct {
		AIType type;
		AIDir dir;
	} aiInfo[] = {
		{ AI_GUY,			DIR_DOWN },
		{ AI_GUY,			DIR_UP },
		{ AI_GUY,			DIR_RIGHT },
		{ AI_GUY,			DIR_LEFT },

		{ ITEM_ENV_WHITE,	DIR_NONE },
		{ ITEM_ENV_BLUE,	DIR_NONE },
		{ ITEM_ENV_RED,		DIR_NONE },
		{ ITEM_ENV_GREEN,	DIR_NONE },

		{ AI_LASER,			DIR_RIGHT },
		{ AI_LASER,			DIR_DOWN },
		{ AI_LASER,			DIR_LEFT },
		{ AI_LASER,			DIR_UP },

		{ AI_DIVERTER,		DIR_DOWN },
		{ AI_DIVERTER,		DIR_UP },
		{ AI_DIVERTER,		DIR_RIGHT },
		{ AI_DIVERTER,		DIR_LEFT },

		{ AI_FOURFIRER,		DIR_RIGHT },
		{ AI_FOURFIRER,		DIR_DOWN},
		{ AI_FOURFIRER,		DIR_LEFT },
		{ AI_FOURFIRER,		DIR_UP },

		{ INFO_ARROW_TURN,	DIR_DOWN },
		{ INFO_ARROW_TURN,	DIR_UP },
		{ INFO_ARROW_TURN,	DIR_RIGHT },
		{ INFO_ARROW_TURN,	DIR_LEFT },

		{ INFO_ARROW_STOP,	DIR_DOWN },
		{ INFO_ARROW_STOP,	DIR_UP },
		{ INFO_ARROW_STOP,	DIR_RIGHT },
		{ INFO_ARROW_STOP,	DIR_LEFT },

		{ ITEM_CELL,		DIR_NONE },

		{ AI_CRATE,			DIR_NONE },
		{ AI_LIGHTBARREL,	DIR_NONE },
		{ AI_HEAVYBARREL,	DIR_NONE },
		{ AI_BOOMBARREL,	DIR_NONE },

		{ ITEM_TRANSCEIVER, DIR_NONE },
		{ ITEM_CLUB,		DIR_NONE },
		{ ITEM_ROBOSTUNNER, DIR_NONE },
		{ ITEM_SLUGSLINGER, DIR_NONE },

		{ AI_SCIENTIST,		DIR_DOWN },
		{ AI_SCIENTIST,		DIR_UP },
		{ AI_SCIENTIST,		DIR_RIGHT },
		{ AI_SCIENTIST,		DIR_LEFT },

		{ AI_WORKER,		DIR_DOWN },
		{ AI_WORKER,		DIR_UP },
		{ AI_WORKER,		DIR_RIGHT },
		{ AI_WORKER,		DIR_LEFT },

		{ AI_SHOCKBOT,		DIR_DOWN },
		{ AI_SHOCKBOT,		DIR_UP },
		{ AI_SHOCKBOT,		DIR_RIGHT },
		{ AI_SHOCKBOT,		DIR_LEFT },

		{ AI_RIGHTBOT,		DIR_DOWN },
		{ AI_RIGHTBOT,		DIR_UP },
		{ AI_RIGHTBOT,		DIR_RIGHT },
		{ AI_RIGHTBOT,		DIR_LEFT },

		{ AI_PUSHBOT,		DIR_DOWN },
		{ AI_PUSHBOT,		DIR_UP },
		{ AI_PUSHBOT,		DIR_RIGHT },
		{ AI_PUSHBOT,		DIR_LEFT },

		{ AI_LISTENBOT,		DIR_DOWN },
		{ AI_LISTENBOT,		DIR_UP },
		{ AI_LISTENBOT,		DIR_RIGHT },
		{ AI_LISTENBOT,		DIR_LEFT },

		{ ITEM_MONKEYSTONE, DIR_NONE },

		{ INFO_TELEPORTER1,	DIR_NONE },
		{ INFO_TELEPORTER2,	DIR_NONE },
		{ INFO_TELEPORTER3,	DIR_NONE },
		{ INFO_TELEPORTER4,	DIR_NONE },
		{ INFO_TELEPORTER5,	DIR_NONE },
		{ INFO_TELEPORTER6,	DIR_NONE },
		{ INFO_TELEPORTER7,	DIR_NONE },
		{ INFO_TELEPORTER8,	DIR_NONE },
		{ INFO_TELEPORTER9,	DIR_NONE },
		{ INFO_TELEPORTER10,	DIR_NONE },
		{ INFO_TELEPORTER11,	DIR_NONE },
		{ INFO_TELEPORTER12,	DIR_NONE },
		{ INFO_TELEPORTER13,	DIR_NONE },
		{ INFO_TELEPORTER14,	DIR_NONE },
		{ INFO_TELEPORTER15,	DIR_NONE },
		{ INFO_TELEPORTER16,	DIR_NONE },
		{ INFO_TELEPORTER17,	DIR_NONE },
		{ INFO_TELEPORTER18,	DIR_NONE },
		{ INFO_TELEPORTER19,	DIR_NONE },
		{ INFO_TELEPORTER20,	DIR_NONE },

		{ INFO_LEVELEXIT,	DIR_NONE },

		{ INFO_ACTION1,		DIR_NONE },
		{ INFO_ACTION2,		DIR_NONE },
		{ INFO_ACTION3,		DIR_NONE },
		{ INFO_ACTION4,		DIR_NONE },
		{ INFO_ACTION5,		DIR_NONE },
		{ INFO_ACTION6,		DIR_NONE },
		{ INFO_ACTION7,		DIR_NONE },
		{ INFO_ACTION8,		DIR_NONE },
		{ INFO_ACTION9,		DIR_NONE },
		{ INFO_ACTION10,	DIR_NONE },
		{ INFO_ACTION11,	DIR_NONE },
		{ INFO_ACTION12,	DIR_NONE },
		{ INFO_ACTION13,	DIR_NONE },
		{ INFO_ACTION14,	DIR_NONE },
		{ INFO_ACTION15,	DIR_NONE },
		{ INFO_ACTION16,	DIR_NONE },
		{ INFO_ACTION17,	DIR_NONE },
		{ INFO_ACTION18,	DIR_NONE },
		{ INFO_ACTION19,	DIR_NONE },
		{ INFO_ACTION20,	DIR_NONE },

		{ AI_SPACEDUDE,		DIR_DOWN },
		{ AI_SPACEDUDE,		DIR_UP },
		{ AI_SPACEDUDE,		DIR_RIGHT },
		{ AI_SPACEDUDE,		DIR_LEFT },

		{ AI_SERGEANT,		DIR_DOWN },
		{ AI_SERGEANT,		DIR_UP },
		{ AI_SERGEANT,		DIR_RIGHT },
		{ AI_SERGEANT,		DIR_LEFT },

		{ AI_MAINTBOT,		DIR_DOWN },
		{ AI_MAINTBOT,		DIR_UP },
		{ AI_MAINTBOT,		DIR_RIGHT },
		{ AI_MAINTBOT,		DIR_LEFT },

		{ INFO_ACTION_AUTO,	DIR_NONE },

		{ ITEM_GEM_WHITE,	DIR_NONE },
		{ ITEM_GEM_BLUE,	DIR_NONE },
		{ ITEM_GEM_RED,		DIR_NONE },
		{ ITEM_GEM_GREEN,	DIR_NONE },

		{ INFO_SET_MUSIC,	DIR_NONE },
		{ INFO_LUA,			DIR_NONE },
		{ INFO_HERE,		DIR_NONE },

		{ AI_VORTEXIAN,		DIR_DOWN },

		{ AI_CHICKEN,		DIR_DOWN },
		{ AI_CHICKEN,		DIR_UP },
		{ AI_CHICKEN,		DIR_RIGHT },
		{ AI_CHICKEN,		DIR_LEFT },

		{ ITEM_GOO_CUP,		DIR_NONE },
		{ ITEM_TEACUP,		DIR_NONE },
		{ ITEM_COOKIE,		DIR_NONE },
		{ ITEM_BURGER,		DIR_NONE },
		{ ITEM_PDA,			DIR_NONE },
		{ ITEM_BOOK,		DIR_NONE },
		{ ITEM_CLIPBOARD,	DIR_NONE },
		{ ITEM_NOTE,		DIR_NONE },
		{ ITEM_KEYCARD_WHITE,	DIR_NONE },
		{ ITEM_KEYCARD_BLUE,	DIR_NONE },
		{ ITEM_KEYCARD_RED,		DIR_NONE },
		{ ITEM_KEYCARD_GREEN,	DIR_NONE },
		{ ITEM_KEYCARD_PURPLE,	DIR_NONE },
		{ ITEM_KEYCARD_BLACK,	DIR_NONE },
		{ AI_MAGIC_EGG,		DIR_NONE },
		{ AI_ICE_BLOCK,		DIR_NONE },
		{ ITEM_CABKEY,		DIR_NONE },

		{ AI_DEADWORKER,	DIR_NONE },
		{ AI_OMNIBOT,		DIR_DOWN },
		{ AI_OMNIBOT,		DIR_UP },
		{ AI_OMNIBOT,		DIR_RIGHT },
		{ AI_OMNIBOT,		DIR_LEFT },
		{ AI_TURNBOT,		DIR_DOWN },
		{ AI_TURNBOT,		DIR_UP },
		{ AI_TURNBOT,		DIR_RIGHT },
		{ AI_TURNBOT,		DIR_LEFT },
		{ AI_DOLLY,			DIR_DOWN },
		{ AI_DOLLY,			DIR_UP },
		{ AI_DOLLY,			DIR_RIGHT },
		{ AI_DOLLY,			DIR_LEFT },

		{ INFO_TRIGGER,		DIR_NONE },

		{ ITEM_DOLLYTOOL1,	DIR_NONE },
		{ ITEM_DOLLYTOOL2,	DIR_NONE },
		{ ITEM_DOLLYTOOL3,	DIR_NONE },
		{ ITEM_DOLLYTOOL4,	DIR_NONE },

		{ AI_RAILRIDER_ON,	DIR_UP },
		{ AI_RAILRIDER_ON,	DIR_DOWN },
		{ AI_RAILRIDER_ON,	DIR_LEFT },
		{ AI_RAILRIDER_ON,	DIR_RIGHT },
		{ AI_RAILRIDER,		DIR_UP },
		{ AI_RAILRIDER,		DIR_DOWN },
		{ AI_RAILRIDER,		DIR_LEFT },
		{ AI_RAILRIDER,		DIR_RIGHT },

		{ ITEM_SODA,		DIR_NONE },
		{ INFO_ARROW_4WAY,	DIR_NONE },
		{ AI_DEADEYE,		DIR_DOWN },
		{ AI_DEADEYE,		DIR_UP },
		{ AI_DEADEYE,		DIR_RIGHT },
		{ AI_DEADEYE,		DIR_LEFT },
		{ AI_MEERKAT,		DIR_NONE },
		{ AI_FATFROG,		DIR_DOWN },
		{ AI_FATFROG,		DIR_RIGHT },
		{ AI_FATFROG,		DIR_LEFT },
		{ AI_GOODFAIRY,		DIR_DOWN },
		{ AI_GOODFAIRY,		DIR_UP },
		{ AI_GOODFAIRY,		DIR_RIGHT },
		{ AI_GOODFAIRY,		DIR_LEFT },
		{ AI_BADFAIRY,		DIR_DOWN },
		{ AI_BADFAIRY,		DIR_UP },
		{ AI_BADFAIRY,		DIR_RIGHT },
		{ AI_BADFAIRY,		DIR_LEFT },
		{ AI_ACCOUNTANT,	DIR_DOWN },
		{ AI_ACCOUNTANT,	DIR_UP },
		{ AI_ACCOUNTANT,	DIR_RIGHT },
		{ AI_ACCOUNTANT,	DIR_LEFT },
		{ AI_ICEPUFF,		DIR_NONE },
		{ AI_DRAGON,		DIR_NONE },
		{ AI_BUZZFLY,		DIR_DOWN },
		{ AI_BUZZFLY,		DIR_UP },
		{ AI_BUZZFLY,		DIR_RIGHT },
		{ AI_BUZZFLY,		DIR_LEFT },

		{ AI_FROGSTATUE,	DIR_NONE },
		{ ITEM_SLICER,		DIR_NONE },
		{ INFO_FAIRY_SRC,	DIR_NONE },
		{ INFO_FAIRY_SRC2,	DIR_NONE },
		{ INFO_FAIRY_SRC3,	DIR_NONE },
		{ INFO_FAIRY_SRC4,	DIR_NONE },
		{ INFO_FAIRY_SRC5,	DIR_NONE },
		{ INFO_FAIRY_DEST,	DIR_NONE },
		{ INFO_FAIRY_DEST2,	DIR_NONE },
		{ INFO_FAIRY_DEST3,	DIR_NONE },
		{ INFO_FAIRY_DEST4,	DIR_NONE },
		{ INFO_FAIRY_DEST5,	DIR_NONE },
		{ INFO_QMARK,		DIR_NONE },
		{ INFO_DEBUG,		DIR_NONE },
		{ AI_NONE,			DIR_NONE },

		{ AI_NONE,			DIR_NONE }
	};

	// Scan all icons and init all Entities
	g_hdb->setupProgressBar(_iconNum);
	for (int i = 0; i < _iconNum; i++) {
		debug(5, "%s, %d,%d,%s,%s,%s,%d,%d,%d,%d", AIType2Str(aiInfo[_iconList[i].icon].type), _iconList[i].x, _iconList[i].y, _iconList[i].funcInit,
				_iconList[i].funcAction, _iconList[i].funcUse, _iconList[i].dir, _iconList[i].level,
				_iconList[i].value1, _iconList[i].value2);

		g_hdb->makeProgress();

		// Don't spawn Action Mode Entities in Puzzle Mode
		if (!g_hdb->getActionMode()) {
			switch (aiInfo[_iconList[i].icon].type) {
			case AI_DEADEYE:
				if (_iconList[i].value1 == 1)	// For non-moving DeadEyes
					break;
				// fall through
			case AI_FOURFIRER:
			case AI_LISTENBOT:
			case ITEM_CLUB:
			case ITEM_ROBOSTUNNER:
			case ITEM_SLUGSLINGER:
				continue;
			default:
				break;
			}
		}

		// Handle special icons that aren't moving AI entities
		switch (aiInfo[_iconList[i].icon].type) {
		default:
			g_hdb->_ai->spawn(
				aiInfo[_iconList[i].icon].type,
				aiInfo[_iconList[i].icon].dir,
				_iconList[i].x,
				_iconList[i].y,
				_iconList[i].funcInit,
				_iconList[i].funcAction,
				_iconList[i].funcUse,
				(AIDir)_iconList[i].dir,
				_iconList[i].level,
				_iconList[i].value1,
				_iconList[i].value2,
				0
			);
			break;
		case INFO_ARROW_4WAY:
			g_hdb->_ai->addToPathList(
				_iconList[i].x,
				_iconList[i].y,
				2,
				aiInfo[_iconList[i].icon].dir
			);
			break;
		case INFO_ARROW_TURN:
			g_hdb->_ai->addToPathList(
				_iconList[i].x,
				_iconList[i].y,
				1,
				aiInfo[_iconList[i].icon].dir
			);
			break;
		case INFO_ARROW_STOP:
			g_hdb->_ai->addToPathList(
				_iconList[i].x,
				_iconList[i].y,
				0,
				aiInfo[_iconList[i].icon].dir
			);
			break;

		case INFO_ACTION1:
		case INFO_ACTION2:
		case INFO_ACTION3:
		case INFO_ACTION4:
		case INFO_ACTION5:
		case INFO_ACTION6:
		case INFO_ACTION7:
		case INFO_ACTION8:
		case INFO_ACTION9:
		case INFO_ACTION10:
		case INFO_ACTION11:
		case INFO_ACTION12:
		case INFO_ACTION13:
		case INFO_ACTION14:
		case INFO_ACTION15:
		case INFO_ACTION16:
		case INFO_ACTION17:
		case INFO_ACTION18:
		case INFO_ACTION19:
		case INFO_ACTION20:
			g_hdb->_ai->addToActionList(
				aiInfo[_iconList[i].icon].type - INFO_ACTION1,
				_iconList[i].x,
				_iconList[i].y,
				_iconList[i].funcInit,
				_iconList[i].funcUse
			);
			break;
		case INFO_ACTION_AUTO:
			g_hdb->_ai->addToAutoList(
				_iconList[i].x,
				_iconList[i].y,
				_iconList[i].funcInit,
				_iconList[i].funcUse
			);
			break;

		case INFO_TELEPORTER1:
		case INFO_TELEPORTER2:
		case INFO_TELEPORTER3:
		case INFO_TELEPORTER4:
		case INFO_TELEPORTER5:
		case INFO_TELEPORTER6:
		case INFO_TELEPORTER7:
		case INFO_TELEPORTER8:
		case INFO_TELEPORTER9:
		case INFO_TELEPORTER10:
		case INFO_TELEPORTER11:
		case INFO_TELEPORTER12:
		case INFO_TELEPORTER13:
		case INFO_TELEPORTER14:
		case INFO_TELEPORTER15:
		case INFO_TELEPORTER16:
		case INFO_TELEPORTER17:
		case INFO_TELEPORTER18:
		case INFO_TELEPORTER19:
		case INFO_TELEPORTER20:

			if (aiInfo[_iconList[i].icon].type == INFO_TELEPORTER8) {
				if (g_hdb->isPPC() && !scumm_stricmp(g_hdb->currentMapName(), "MAP29.MSM")) {
					if (_iconList[i].x == 45 && _iconList[i].y == 116) {
						warning("PATCHED Teleporter8 in MAP29");

						_iconList[i].level = 1;
					}
				}
			}

			g_hdb->_ai->addToTeleportList(
				aiInfo[_iconList[i].icon].type - INFO_TELEPORTER1,
				_iconList[i].x,
				_iconList[i].y,
				_iconList[i].dir,
				_iconList[i].level,
				_iconList[i].value1,
				_iconList[i].value2,
				_iconList[i].funcUse
			);
			break;

		case INFO_SET_MUSIC:
		case INFO_PROMOTE:
		case INFO_DEMOTE:
			break;

		case INFO_LUA:
			g_hdb->_ai->addToLuaList(
				_iconList[i].x,
				_iconList[i].y,
				_iconList[i].value1,
				_iconList[i].value2,
				_iconList[i].funcInit,
				_iconList[i].funcAction,
				_iconList[i].funcUse
			);
			break;
		case INFO_HERE:
			g_hdb->_ai->addToHereList(
				_iconList[i].funcInit,
				_iconList[i].x,
				_iconList[i].y
			);
			break;
		case INFO_TRIGGER:
			g_hdb->_ai->addToTriggerList(
				_iconList[i].funcInit,
				_iconList[i].funcUse,
				_iconList[i].x,
				_iconList[i].y,
				_iconList[i].value1,
				_iconList[i].value2,
				_iconList[i].funcAction
			);
			break;

		case INFO_FAIRY_SRC:
		case INFO_FAIRY_SRC2:
		case INFO_FAIRY_SRC3:
		case INFO_FAIRY_SRC4:
		case INFO_FAIRY_SRC5:
			g_hdb->_ai->addToFairystones(
				aiInfo[_iconList[i].icon].type - INFO_FAIRY_SRC,
				_iconList[i].x,
				_iconList[i].y,
				0
			);
			break;

		case INFO_FAIRY_DEST:
		case INFO_FAIRY_DEST2:
		case INFO_FAIRY_DEST3:
		case INFO_FAIRY_DEST4:
		case INFO_FAIRY_DEST5:
			g_hdb->_ai->addToFairystones(
				aiInfo[_iconList[i].icon].type - INFO_FAIRY_DEST,
				_iconList[i].x,
				_iconList[i].y,
				1
			);
			break;
		}
	}

	g_hdb->_ai->initAllEnts();
	g_hdb->stopProgress();

	_mapLoaded = true;

	return true;
}

void Map::draw() {
	if (!_mapLoaded)
		return;

	// Calculate Tile Offsets and Panning Offsets
	_mapTileX = _mapX / kTileWidth;
	_mapTileY = _mapY / kTileHeight;
	_mapTileXOff = -(_mapX % kTileWidth);
	_mapTileYOff = -(_mapY % kTileHeight);

	int matrixY = _mapTileY * _width;
	int screenY = _mapTileYOff;

	/*
		Note from Original Source:
		need to set the number of tiles to draw on the screen.  Most of the time
		we need to draw an extra tile because we're displaying a half-tile, but
		sometimes the offset is exactly at 0 and thus we don't need to draw a
		tile offscreen that we'll never see.  In fact, doing this fixes a bug
		that could occur because we would be accessing map data that's outside the map
		when we're at the very bottom of the map.
	*/

	int maxTileX = (_mapTileXOff >= -8) ? g_hdb->_map->_screenXTiles - 1 : g_hdb->_map->_screenXTiles;
	int maxTileY = (!_mapTileYOff) ? g_hdb->_map->_screenYTiles - 1 : g_hdb->_map->_screenYTiles;

	if (matrixY + (maxTileY - 1) * _width > _height * _width) {
		return;
	}

	// Sometimes we're 1 beyond the map, so avoid it
	if (_mapTileX + maxTileX - 1 >= _width)
		maxTileX--;

	_numForegrounds = _numGratings = 0;

	for (int j = 0; j < maxTileY; j++) {
		int screenX = _mapTileXOff;
		for (int i = 0; i < maxTileX; i++) {

			// Draw Background Tile
			int16 tileIndex = _background[matrixY + _mapTileX + i];
			if (tileIndex < 0) {
				tileIndex = 0;
			}

			// Draw if not a sky tile
			if (!g_hdb->_gfx->isSky(tileIndex)) {
				Tile *tile = g_hdb->_gfx->getTile(tileIndex);
				if (tile)
					tile->draw(screenX, screenY);
				else
					warning("Cannot find tile with index %d at %d,%d", tileIndex, _mapTileX + i, _mapTileY + j);
			}

			// Draw Foreground Tile
			tileIndex = _foreground[matrixY + _mapTileX + i];
			if (tileIndex >= 0) {
				Tile *fTile = g_hdb->_gfx->getTile(tileIndex);
				if (fTile && !(fTile->_flags & kFlagInvisible)) {

					if ((fTile->_flags & kFlagGrating) && (_numGratings < kMaxGratings)) {
						// Check for Gratings Flag
						_gratings[_numGratings].x = screenX;
						_gratings[_numGratings].y = screenY;
						_gratings[_numGratings].tile = tileIndex;
						if (_numGratings < kMaxGratings)
							_numGratings++;
					} else if ((fTile->_flags & kFlagForeground)) {
						// Check for Foregrounds Flag
						_foregrounds[_numForegrounds].x = screenX;
						_foregrounds[_numForegrounds].y = screenY;
						_foregrounds[_numForegrounds].tile = tileIndex;
						if (_numForegrounds < kMaxForegrounds)
							_numForegrounds++;
					} else {
						if (fTile->_flags & kFlagMasked) {
							fTile->drawMasked(screenX, screenY);
						} else {
							fTile->draw(screenX, screenY);
						}
					}
				}
			}

			screenX += kTileWidth;
		}
		matrixY += _width;
		screenY += kTileWidth;
	}

	if (g_hdb->isDemo() && g_hdb->isPPC())
		drawEnts();

	// Animate FAST Map Tiles
	if (!(_animCycle % kAnimFastFrames)) {
		for (Common::Array<uint32>::iterator it = _listBGAnimFast.begin(); it != _listBGAnimFast.end(); ++it) {
			_background[(*it)] = g_hdb->_gfx->animateTile(_background[(*it)]);
		}

		for (Common::Array<uint32>::iterator it = _listFGAnimFast.begin(); it != _listFGAnimFast.end(); ++it) {
			_foreground[(*it)] = g_hdb->_gfx->animateTile(_foreground[(*it)]);
		}
	}

	// Animate MEDIUM Map Tiles
	if (!(_animCycle % kAnimMediumFrames)) {
		for (Common::Array<uint32>::iterator it = _listBGAnimMedium.begin(); it != _listBGAnimMedium.end(); ++it) {
			_background[(*it)] = g_hdb->_gfx->animateTile(_background[(*it)]);
		}

		for (Common::Array<uint32>::iterator it = _listFGAnimMedium.begin(); it != _listFGAnimMedium.end(); ++it) {
			_foreground[(*it)] = g_hdb->_gfx->animateTile(_foreground[(*it)]);
		}
	}

	// Animate SLOW Map Tiles
	if (!(_animCycle % kAnimSlowFrames)) {
		for (Common::Array<uint32>::iterator it = _listBGAnimSlow.begin(); it != _listBGAnimSlow.end(); ++it) {
			_background[(*it)] = g_hdb->_gfx->animateTile(_background[(*it)]);
		}

		for (Common::Array<uint32>::iterator it = _listFGAnimSlow.begin(); it != _listFGAnimSlow.end(); ++it) {
			_foreground[(*it)] = g_hdb->_gfx->animateTile(_foreground[(*it)]);
		}
	}

	_animCycle++;
}

void Map::drawEnts() {
	g_hdb->_ai->drawEnts(_mapX, _mapY, g_hdb->_map->_screenXTiles * kTileWidth, g_hdb->_map->_screenYTiles * kTileHeight);
}

void Map::drawGratings() {
	for (int i = 0; i < _numGratings; i++) {
		g_hdb->_gfx->getTile(_gratings[i].tile)->drawMasked(_gratings[i].x, _gratings[i].y);
	}

	debug(8, "Gratings Count: %d", _numGratings);
}

void Map::drawForegrounds() {
	for (int i = 0; i < _numForegrounds; i++) {
		g_hdb->_gfx->getTile(_foregrounds[i].tile)->drawMasked(_foregrounds[i].x, _foregrounds[i].y);
	}

	debug(8, "Foregrounds Count: %d", _numForegrounds);
}

bool Map::onScreen(int x, int y) {
	if ((x >= _mapX / kTileWidth) && (x < (_mapX / kTileWidth) + g_hdb->_map->_screenXTiles) && (y >= _mapY / kTileHeight) && (y < (_mapY / kTileHeight) + g_hdb->_map->_screenYTiles))
		return true;
	return false;
}

uint32 Map::getMapBGTileFlags(int x, int y) {
	if (x < 0 || x >= _width || y < 0 || y >= _height)
		return 0;

	Tile *tile = g_hdb->_gfx->getTile(_background[y * _width + x]);
	if (tile)
		return tile->_flags;
	return 0;
}

uint32 Map::getMapFGTileFlags(int x, int y) {
	if (x < 0 || x >= _width || y < 0 || y >= _height)
		return 0;

	Tile *tile = g_hdb->_gfx->getTile(_foreground[y * _width + x]);
	if (tile)
		return tile->_flags;
	return 0;
}

int16 Map::getMapBGTileIndex(int x, int y) {
	if (x < 0 || x >= _width || y < 0 || y >= _height)
		return 0;

	return _background[y * _width + x];
}

int16 Map::getMapFGTileIndex(int x, int y) {
	if (x < 0 || x >= _width || y < 0 || y >= _height)
		return 0;

	return _foreground[y * _width + x];
}

void Map::setMapBGTileIndex(int x, int y, int index) {
	if (x < 0 || x >= _width || y < 0 || y >= _height)
		return;

	_background[y * _width + x] = index;
}

void Map::setMapFGTileIndex(int x, int y, int index) {
	if (x < 0 || x >= _width || y < 0 || y >= _height)
		return;

	_foreground[y * _width + x] = index;
}

void Map::addBGTileAnimation(int x, int y) {
	int i = y * _width + x;

	Tile *tile = g_hdb->_gfx->getTile(_background[i]);
	if (!tile)
		return;

	uint32 flags = tile->_flags;

	// BACKGROUND
	if ((flags & kFlagAnimFast) == kFlagAnimFast)
		// check 'fast' first since it's a combo of slow & medium
		_listBGAnimFast.push_back(i);
	else if ((flags & kFlagAnimSlow) == kFlagAnimSlow)
		_listBGAnimSlow.push_back(i);
	else if ((flags & kFlagAnimMedium) == kFlagAnimMedium)
		_listBGAnimMedium.push_back(i);
}

void Map::addFGTileAnimation(int x, int y) {

	int i = y * _width + x;

	Tile *tile = g_hdb->_gfx->getTile(_foreground[i]);
	if (!tile)
		return;
	uint32 flags = tile->_flags;

	// FOREGROUND
	if ((flags & kFlagAnimFast) == kFlagAnimFast)
		_listFGAnimFast.push_back(i);
	else if ((flags & kFlagAnimSlow) == kFlagAnimSlow)
		_listFGAnimSlow.push_back(i);
	else if ((flags & kFlagAnimMedium) == kFlagAnimMedium)
		_listFGAnimMedium.push_back(i);
}

void Map::removeBGTileAnimation(int x, int y) {
	uint idx = y * _width + x;

	for (uint i = 0; i < _listBGAnimFast.size(); i++) {
		if (_listBGAnimFast[i] == idx) {
			_listBGAnimFast.remove_at(i);
			return;
		}
	}

	for (uint i = 0; i < _listBGAnimSlow.size(); i++) {
		if (_listBGAnimSlow[i] == idx) {
			_listBGAnimSlow.remove_at(i);
			return;
		}
	}

	for (uint i = 0; i < _listBGAnimMedium.size(); i++) {
		if (_listBGAnimMedium[i] == idx) {
			_listBGAnimMedium.remove_at(i);
			return;
		}
	}
}

void Map::removeFGTileAnimation(int x, int y) {
	uint idx = y * _width + x;

	for (uint i = 0; i < _listFGAnimFast.size(); i++) {
		if (_listFGAnimFast[i] == idx) {
			_listFGAnimFast.remove_at(i);
			return;
		}
	}

	for (uint i = 0; i < _listFGAnimSlow.size(); i++) {
		if (_listFGAnimSlow[i] == idx) {
			_listFGAnimSlow.remove_at(i);
			return;
		}
	}

	for (uint i = 0; i < _listFGAnimMedium.size(); i++) {
		if (_listFGAnimMedium[i] == idx) {
			_listFGAnimMedium.remove_at(i);
			return;
		}
	}
}

void Map::getMapXY(int *x, int *y) {
	*x = _mapX;
	*y = _mapY;
}

void Map::setMapXY(int x, int y) {
	_mapX = CLIP(x, 0, _width * kTileWidth - g_hdb->_screenDrawWidth);
	_mapY = CLIP(y, 0, _height * kTileHeight - g_hdb->_screenDrawHeight);
}

// Sets _mapX and _mapY and tries to center the map around X, Y
void Map::centerMapXY(int x, int y) {
	int checkx = x / kTileWidth;
	int checky = y / kTileHeight;

	// Scan from centerX to right edge
	int maxx = (_width - (g_hdb->_map->_screenTileWidth / 2)) * kTileWidth;
	for (int i = checkx + 1; i <= checkx + (g_hdb->_map->_screenTileWidth / 2); i++) {
		if (!getMapBGTileIndex(i, checky)) {
			maxx = (i - (g_hdb->_map->_screenTileWidth / 2)) * kTileWidth;
			break;
		}
	}

	// Scan from centerX to left edge
	int minx = 0;
	for (int i = checkx - 1; i >= checkx - (g_hdb->_map->_screenTileWidth / 2); i--) {
		if (!getMapBGTileIndex(i, checky)) {
			// +1 because we don't want to see one whole tile
			minx = (1 + i + (g_hdb->_map->_screenTileWidth / 2)) * kTileWidth;
			break;
		}
	}

	// Scan from centerY to bottom edge
	int maxy = (_height - (g_hdb->_map->_screenTileHeight / 2)) * kTileHeight;
	for (int i = checky + 1; i <= checky + (g_hdb->_map->_screenTileHeight / 2); i++) {
		if (!getMapBGTileIndex(checkx, i)) {
			maxy = (i - (g_hdb->_map->_screenTileHeight / 2)) * kTileHeight;
			break;
		}
	}

	// Scan from centerY to top edge
	int miny = 0;
	for (int i = checky - 1; i >= checky - (g_hdb->_map->_screenTileHeight / 2); i--) {
		if (!getMapBGTileIndex(checkx, i)) {
			// +1 because we don't want to see one whole tile
			miny = (1 + i + (g_hdb->_map->_screenTileHeight / 2)) * kTileHeight;
			break;
		}
	}
	
	x = CLIP(x, minx, maxx);
	y = CLIP(y, miny, maxy);

	x -= (g_hdb->_screenDrawWidth / 2);
	y -= (g_hdb->_screenDrawHeight / 2);

	setMapXY(x, y);
}

bool Map::checkEntOnScreen(AIEntity *e) {
	return ((e->x > _mapX - 32) && (e->x < _mapX + g_hdb->_map->_screenXTiles * kTileWidth) && (e->y > _mapY - 32) && (e->y < _mapY + g_hdb->_map->_screenYTiles * kTileHeight));
}

bool Map::checkXYOnScreen(int x, int y) {
	return ((x > _mapX - 32) && (x < _mapX + g_hdb->_map->_screenXTiles * kTileWidth) && (y > _mapY - 32) && (y < _mapY + g_hdb->_map->_screenYTiles * kTileHeight));
}

bool Map::checkOneTileExistInRange(int tileIndex, int count) {
	for (int i = 0; i < _width * _height; i++) {
		if (_background[i] >= tileIndex && _background[i] < tileIndex + count)
			return true;
		if (_foreground[i] >= tileIndex && _foreground[i] < tileIndex + count)
			return true;
	}
	return true;
}

}
