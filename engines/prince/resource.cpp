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

#include "common/fs.h"
#include "common/config-manager.h"

#include "prince/prince.h"
#include "prince/graphics.h"
#include "prince/debugger.h"
#include "prince/script.h"
#include "prince/hero.h"
#include "prince/resource.h"
#include "prince/archive.h"

namespace Prince {

Common::SeekableReadStream *Resource::getDecompressedStream(Common::SeekableReadStream *stream) {
	if (!(((PrinceEngine *)g_engine)->getFeatures() & GF_EXTRACTED))
		return stream;

	byte header[4];

	stream->read(header, 4);
	stream->seek(0);

	if (READ_BE_UINT32(header) == MKTAG('M', 'A', 'S', 'M')) {
		byte *buffer = (byte *)malloc(stream->size());
		stream->read(buffer, stream->size());

		Decompressor dec;
		uint32 decompLen = READ_BE_UINT32(buffer + 14);
		byte *decompData = (byte *)malloc(decompLen);
		dec.decompress(buffer + 18, decompData, decompLen);
		free(buffer);

		debug(8, "Resource::getDecompressedStream: decompressed %d to %d bytes", stream->size(), decompLen);

		return new Common::MemoryReadStream(decompData, decompLen, DisposeAfterUse::YES);
	} else {
		return stream;
	}
}

bool AnimListItem::loadFromStream(Common::SeekableReadStream &stream) {
	int32 pos = stream.pos();

	uint16 type = stream.readUint16LE();
	if (type == 0xFFFF) {
		return false;
	}
	_type = type;
	_fileNumber = stream.readUint16LE();
	_startPhase = stream.readUint16LE();
	_endPhase = stream.readUint16LE();
	_loopPhase = stream.readUint16LE();
	_x = stream.readSint16LE();
	_y = stream.readSint16LE();
	_loopType = stream.readUint16LE();
	_nextAnim = stream.readUint16LE();
	_flags = stream.readUint16LE();

	debug(7, "AnimListItem type %d, fileNumber %d, x %d, y %d, flags %d", _type, _fileNumber, _x, _y, _flags);
	debug(7, "startPhase %d, endPhase %d, loopPhase %d", _startPhase, _endPhase, _loopPhase);

	// 32 byte aligment
	stream.seek(pos + 32);

	return true;
}

bool PrinceEngine::loadLocation(uint16 locationNr) {

	blackPalette();

	_flicPlayer.close();

	memset(_textSlots, 0, sizeof(_textSlots));
	freeAllSamples();

	debugEngine("PrinceEngine::loadLocation %d", locationNr);
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.remove(Common::String::format("%02d", _locationNr));

	_locationNr = locationNr;
	_debugger->_locationNr = locationNr;

	_flags->setFlagValue(Flags::CURRROOM, _locationNr);
	_interpreter->stopBg();

	changeCursor(0);

	const Common::String locationNrStr = Common::String::format("%02d", _locationNr);
	debugEngine("loadLocation %s", locationNrStr.c_str());

	if (!(getFeatures() & GF_EXTRACTED)) {
		PtcArchive *locationArchive = new PtcArchive();
		if (!locationArchive->open(locationNrStr + "/databank.ptc"))
			error("Can't open location %s", locationNrStr.c_str());

		SearchMan.add(locationNrStr, locationArchive);
	} else {
		SearchMan.addSubDirectoryMatching(gameDataDir, locationNrStr);
	}

	loadMusic(_locationNr);

	// load location background, replace old one
	Resource::loadResource(_roomBmp, "room", true);
	if (_roomBmp->getSurface()) {
		_sceneWidth = _roomBmp->getSurface()->w;
	}

	loadZoom(_zoomBitmap, kZoomBitmapLen, "zoom");
	loadShadow(_shadowBitmap, kShadowBitmapSize, "shadow", "shadow2");
	loadTrans(_transTable, "trans");
	loadPath("path");

	for (uint32 i = 0; i < _pscrList.size(); i++) {
		delete _pscrList[i];
	}
	_pscrList.clear();
	Resource::loadResource(_pscrList, "pscr.lst", false);

	loadMobPriority("mobpri");

	_mobList.clear();
	if (getGameType() == kPrinceDataDE) {
		const Common::String mobLstName = Common::String::format("mob%02d.lst", _locationNr);
		debug(3, "moblist name: %s", mobLstName.c_str());
		Resource::loadResource(_mobList, mobLstName.c_str(), false);
	} else if (getGameType() == kPrinceDataPL) {
		Resource::loadResource(_mobList, "mob.lst", false);
	}
	if (getFeatures() & GF_TRANSLATED) {
		// update Mob texts for translated version
		setMobTranslationTexts();
	}

	_animList.clear();
	Resource::loadResource(_animList, "anim.lst", false);

	for (uint32 i = 0; i < _objList.size(); i++) {
		delete _objList[i];
	}
	_objList.clear();
	Resource::loadResource(_objList, "obj.lst", false);

	_room->loadRoom(_script->getRoomOffset(_locationNr));

	for (uint i = 0; i < _maskList.size(); i++) {
		free(_maskList[i]._data);
	}
	_maskList.clear();
	_script->loadAllMasks(_maskList, _room->_nak);

	_picWindowX = 0;

	_lightX = _script->getLightX(_locationNr);
	_lightY = _script->getLightY(_locationNr);
	setShadowScale(_script->getShadowScale(_locationNr));

	for (uint i = 0; i < _mobList.size(); i++) {
		_mobList[i]._visible = _script->getMobVisible(_room->_mobs, i);
	}

	_script->installObjects(_room->_obj);

	freeAllNormAnims();

	clearBackAnimList();
	_script->installBackAnims(_backAnimList, _room->_backAnim);

	_graph->makeShadowTable(70, _graph->_shadowTable70);
	_graph->makeShadowTable(50, _graph->_shadowTable50);

	_mainHero->freeOldMove();
	_secondHero->freeOldMove();

	_mainHero->scrollHero();

	return true;
}

bool PrinceEngine::loadAnim(uint16 animNr, bool loop) {
	Common::String streamName = Common::String::format("AN%02d", animNr);
	Common::SeekableReadStream *flicStream = SearchMan.createReadStreamForMember(streamName);

	if (!flicStream) {
		error("Can't open %s", streamName.c_str());
		return false;
	}

	flicStream = Resource::getDecompressedStream(flicStream);

	if (!_flicPlayer.loadStream(flicStream)) {
		error("Can't load flic stream %s", streamName.c_str());
	}

	debugEngine("%s loaded", streamName.c_str());
	_flicLooped = loop;
	_flicPlayer.start();
	playNextFLCFrame();
	return true;
}

bool PrinceEngine::loadZoom(byte *zoomBitmap, uint32 dataSize, const char *resourceName) {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName);
	if (!stream) {
		delete stream;
		return false;
	}
	stream = Resource::getDecompressedStream(stream);

	if (stream->read(zoomBitmap, dataSize) != dataSize) {
		free(zoomBitmap);
		delete stream;
		return false;
	}
	delete stream;
	return true;
}

bool PrinceEngine::loadShadow(byte *shadowBitmap, uint32 dataSize, const char *resourceName1, const char *resourceName2) {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName1);
	if (!stream) {
		delete stream;
		return false;
	}

	stream = Resource::getDecompressedStream(stream);

	if (stream->read(shadowBitmap, dataSize) != dataSize) {
		free(shadowBitmap);
		delete stream;
		return false;
	}

	Common::SeekableReadStream *stream2 = SearchMan.createReadStreamForMember(resourceName2);
	if (!stream2) {
		delete stream;
		delete stream2;
		return false;
	}

	stream2 = Resource::getDecompressedStream(stream2);

	byte *shadowBitmap2 = shadowBitmap + dataSize;
	if (stream2->read(shadowBitmap2, dataSize) != dataSize) {
		free(shadowBitmap);
		delete stream;
		delete stream2;
		return false;
	}

	delete stream;
	delete stream2;
	return true;
}

bool PrinceEngine::loadTrans(byte *transTable, const char *resourceName) {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName);
	if (!stream) {
		delete stream;
		for (int i = 0; i < 256; i++) {
			for (int j = 0; j < 256; j++) {
				transTable[i * 256 + j] = j;
			}
		}
		return true;
	}

	stream = Resource::getDecompressedStream(stream);

	if (stream->read(transTable, kTransTableSize) != kTransTableSize) {
		delete stream;
		return false;
	}
	delete stream;
	return true;
}

bool PrinceEngine::loadPath(const char *resourceName) {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName);
	if (!stream) {
		delete stream;
		return false;
	}

	stream = Resource::getDecompressedStream(stream);

	if (stream->read(_roomPathBitmap, kPathBitmapLen) != kPathBitmapLen) {
		delete stream;
		return false;
	}
	delete stream;
	return true;
}

bool PrinceEngine::loadAllInv() {
	for (int i = 0; i < kMaxInv; i++) {
		InvItem tempInvItem;

		const Common::String invStreamName = Common::String::format("INV%02d", i);
		Common::SeekableReadStream *invStream = SearchMan.createReadStreamForMember(invStreamName);
		if (!invStream) {
			delete invStream;
			return true;
		}

		invStream = Resource::getDecompressedStream(invStream);

		tempInvItem._x = invStream->readUint16LE();
		tempInvItem._y = invStream->readUint16LE();
		int width = invStream->readUint16LE();
		int height = invStream->readUint16LE();
		tempInvItem._surface = new Graphics::Surface();
		tempInvItem._surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());

		for (int h = 0; h < tempInvItem._surface->h; h++) {
			invStream->read(tempInvItem._surface->getBasePtr(0, h), tempInvItem._surface->w);
		}

		_allInvList.push_back(tempInvItem);
		delete invStream;
	}

	return true;
}

bool PrinceEngine::loadMobPriority(const char *resourceName) {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName);
	if (!stream) {
		delete stream;
		return false;
	}

	stream = Resource::getDecompressedStream(stream);

	_mobPriorityList.clear();
	uint mobId;
	while (1) {
		mobId = stream->readUint32LE();
		if (mobId == 0xFFFFFFFF) {
			break;
		}
		_mobPriorityList.push_back(mobId);
	}
	delete stream;
	return true;
}

} // End of namespace Prince
