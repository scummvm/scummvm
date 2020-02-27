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

#include "common/system.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/substream.h"

#include "engines/util.h"
#include "graphics/primitives.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"
#include "image/bmp.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/images.h"
#include "director/score.h"
#include "director/frame.h"
#include "director/sound.h"
#include "director/sprite.h"
#include "director/stxt.h"
#include "director/util.h"
#include "director/lingo/lingo.h"

namespace Director {

const char *scriptTypes[] = {
	"MovieScript",
	"SpriteScript",
	"FrameScript",
	"CastScript",
	"GlobalScript",
	"ScoreScript"
};

const char *scriptType2str(ScriptType scr) {
	if (scr < 0)
		return "NoneScript";

	if (scr > kMaxScriptType)
		return "<unknown>";

	return scriptTypes[scr];
}


Score::Score(DirectorEngine *vm) {
	_vm = vm;
	_surface = nullptr;
	_trailSurface = nullptr;
	_backSurface = nullptr;
	_lingo = _vm->getLingo();
	_soundManager = _vm->getSoundManager();
	_currentMouseDownSpriteId = 0;
	_mouseIsDown = false;

	// FIXME: TODO: Check whether the original truely does it
	if (_vm->getVersion() <= 3) {
		_lingo->executeScript(kMovieScript, 0, 0);
	}
	_movieScriptCount = 0;
	_labels = NULL;
	_font = NULL;

	_versionMinor = _versionMajor = 0;
	_currentFrameRate = 20;
	_castArrayStart = _castArrayEnd = 0;
	_currentFrame = 0;
	_nextFrame = 0;
	_currentLabel = 0;
	_nextFrameTime = 0;
	_flags = 0;
	_stopPlay = false;
	_stageColor = 0;

	_castIDoffset = 0;

	_movieArchive = nullptr;

	_loadedStxts = nullptr;
	_loadedCast = nullptr;

	_numChannelsDisplayed = 0;
}

void Score::setArchive(Archive *archive) {
	_movieArchive = archive;
	if (archive->hasResource(MKTAG('M', 'C', 'N', 'M'), 0)) {
		_macName = archive->getName(MKTAG('M', 'C', 'N', 'M'), 0).c_str();
	} else {
		_macName = archive->getFileName();
	}

	// Frame Labels
	if (archive->hasResource(MKTAG('V', 'W', 'L', 'B'), -1)) {
		loadLabels(*archive->getFirstResource(MKTAG('V', 'W', 'L', 'B')));
	}
}

void Score::loadArchive() {
	Common::Array<uint16> clutList = _movieArchive->getResourceIDList(MKTAG('C', 'L', 'U', 'T'));

	if (clutList.size() > 1)
		warning("More than one palette was found (%d)", clutList.size());

	if (clutList.size() == 0) {
		warning("CLUT resource not found, using default Mac palette");
		_vm->setPalette(-1);
	} else {
		Common::SeekableSubReadStreamEndian *pal = _movieArchive->getResource(MKTAG('C', 'L', 'U', 'T'), clutList[0]);

		debugC(2, kDebugLoading, "****** Loading Palette CLUT, #%d", clutList[0]);
		loadPalette(*pal);
	}

	// Font Directory
	if (_movieArchive->hasResource(MKTAG('F', 'O', 'N', 'D'), -1)) {
		debug("STUB: Unhandled FOND resource");
	}

	// Score
	assert(_movieArchive->hasResource(MKTAG('V', 'W', 'S', 'C'), -1));
	loadFrames(*_movieArchive->getFirstResource(MKTAG('V', 'W', 'S', 'C')));

	// Configuration Information
	if (_movieArchive->hasResource(MKTAG('V', 'W', 'C', 'F'), -1)) {
		loadConfig(*_movieArchive->getFirstResource(MKTAG('V', 'W', 'C', 'F')));
	} else {
		// TODO: Source this from somewhere!
		_movieRect = Common::Rect(0, 0, 640, 480);
		_stageColor = 1;
	}

	// Cast Information Array
	if (_movieArchive->hasResource(MKTAG('V', 'W', 'C', 'R'), -1)) {
		_castIDoffset = _movieArchive->getResourceIDList(MKTAG('V', 'W', 'C', 'R'))[0];
		loadCastDataVWCR(*_movieArchive->getResource(MKTAG('V', 'W', 'C', 'R'), _castIDoffset));
	}

	// Action list
	if (_movieArchive->hasResource(MKTAG('V', 'W', 'A', 'C'), -1)) {
		loadActions(*_movieArchive->getFirstResource(MKTAG('V', 'W', 'A', 'C')));
	}

	// File Info
	if (_movieArchive->hasResource(MKTAG('V', 'W', 'F', 'I'), -1)) {
		loadFileInfo(*_movieArchive->getFirstResource(MKTAG('V', 'W', 'F', 'I')));
	}

	// Font Mapping
	if (_movieArchive->hasResource(MKTAG('V', 'W', 'F', 'M'), -1)) {
		_vm->_wm->_fontMan->clearFontMapping();

		loadFontMap(*_movieArchive->getFirstResource(MKTAG('V', 'W', 'F', 'M')));
	}

	// Pattern Tiles
	if (_movieArchive->hasResource(MKTAG('V', 'W', 'T', 'L'), -1)) {
		debug("STUB: Unhandled VWTL resource.");
	}

	// Time code
	if (_movieArchive->hasResource(MKTAG('V', 'W', 't', 'c'), -1)) {
		debug("STUB: Unhandled VWtc resource");
	}

	// External sound files
	if (_movieArchive->hasResource(MKTAG('S', 'T', 'R', ' '), -1)) {
		debug("STUB: Unhandled 'STR ' resource");
	}

	// Try to load script context
	if (_vm->getVersion() >= 4) {
		Common::Array<uint16> lctx =  _movieArchive->getResourceIDList(MKTAG('L','c','t','x'));
		if (lctx.size() > 0) {
			debugC(2, kDebugLoading, "****** Loading %d Lctx resources", lctx.size());

			for (Common::Array<uint16>::iterator iterator = lctx.begin(); iterator != lctx.end(); ++iterator) {
				loadLingoContext(*_movieArchive->getResource(MKTAG('L','c','t','x'), *iterator));
			}
		}
	}

	// Try to load script name lists
	if (_vm->getVersion() >= 4) {
		Common::Array<uint16> lnam =  _movieArchive->getResourceIDList(MKTAG('L','n','a','m'));
		if (lnam.size() > 0) {

			int maxLnam = -1;
			for (Common::Array<uint16>::iterator iterator = lnam.begin(); iterator != lnam.end(); ++iterator) {
				maxLnam = MAX(maxLnam, (int)*iterator);
			}
			debugC(2, kDebugLoading, "****** Loading Lnam resource with highest ID (%d)", maxLnam);
			loadLingoNames(*_movieArchive->getResource(MKTAG('L','n','a','m'), maxLnam));
		}
	}

	Common::Array<uint16> vwci = _movieArchive->getResourceIDList(MKTAG('V', 'W', 'C', 'I'));
	if (vwci.size() > 0) {
		debugC(2, kDebugLoading, "****** Loading %d CastInfos VWCI", vwci.size());

		for (Common::Array<uint16>::iterator iterator = vwci.begin(); iterator != vwci.end(); ++iterator)
			loadCastInfo(*_movieArchive->getResource(MKTAG('V', 'W', 'C', 'I'), *iterator), *iterator);
	}

	Common::Array<uint16> cast = _movieArchive->getResourceIDList(MKTAG('C', 'A', 'S', 't'));
	if (!_loadedCast)
		_loadedCast = new Common::HashMap<int, Cast *>();

	if (cast.size() > 0) {
		debugC(2, kDebugLoading, "****** Loading %d CASt resources", cast.size());

		for (Common::Array<uint16>::iterator iterator = cast.begin(); iterator != cast.end(); ++iterator) {
			Common::SeekableSubReadStreamEndian *stream = _movieArchive->getResource(MKTAG('C', 'A', 'S', 't'), *iterator);
			Resource res = _movieArchive->getResourceDetail(MKTAG('C', 'A', 'S', 't'), *iterator);
			loadCastData(*stream, *iterator, &res);
		}
	}

	// PICT resources
	if (_movieArchive->hasResource(MKTAG('P', 'I', 'C', 'T'), -1)) {
		debug("STUB: Unhandled 'PICT' resource");
	}

	// Sound resources
	if (_movieArchive->hasResource(MKTAG('s', 'n', 'd', ' '), -1)) {
		debug("STUB: Unhandled 'snd ' resource");
	}

	// Film Loop resources
	if (_movieArchive->hasResource(MKTAG('S', 'C', 'V', 'W'), -1)) {
		debug("STUB: Unhandled 'SCVW' resource");
	}


	setSpriteCasts();
	loadSpriteImages(false);

	// Now process STXTs
	Common::Array<uint16> stxt = _movieArchive->getResourceIDList(MKTAG('S','T','X','T'));
	debugC(2, kDebugLoading, "****** Loading %d STXT resources", stxt.size());

	_loadedStxts = new Common::HashMap<int, const Stxt *>();

	for (Common::Array<uint16>::iterator iterator = stxt.begin(); iterator != stxt.end(); ++iterator) {
		_loadedStxts->setVal(*iterator,
				 new Stxt(*_movieArchive->getResource(MKTAG('S','T','X','T'), *iterator)));

		// Try to load movie script, it starts with a comment
		if (_vm->getVersion() <= 3) {
			loadScriptText(*_movieArchive->getResource(MKTAG('S','T','X','T'), *iterator));
		}

	}
	copyCastStxts();
}

void Score::copyCastStxts() {
	for (Common::HashMap<int, Cast *>::iterator c = _loadedCast->begin(); c != _loadedCast->end(); ++c) {
		if (c->_value->_type != kCastText && c->_value->_type != kCastButton)
			continue;

		uint stxtid = (_vm->getVersion() < 4) ?
			c->_key + _castIDoffset :
			c->_value->_children[0].index;

		if (_loadedStxts->getVal(stxtid)) {
			const Stxt *stxt = _loadedStxts->getVal(stxtid);
			TextCast *tc = (TextCast *)c->_value;
			tc->importStxt(stxt);
		}
	}
}

void Score::loadSpriteImages(bool isSharedCast) {
	debugC(1, kDebugLoading, "****** Preloading sprite images");

	Score *sharedScore = _vm->getSharedScore();

	for (Common::HashMap<int, Cast *>::iterator c = _loadedCast->begin(); c != _loadedCast->end(); ++c) {
		if (!c->_value)
			continue;

		if (c->_value->_type != kCastBitmap)
			continue;

		BitmapCast *bitmapCast = (BitmapCast *)c->_value;
		uint32 tag = bitmapCast->_tag;
		uint16 imgId = c->_key;
		uint16 realId;

		Image::ImageDecoder *img = NULL;
		Common::SeekableReadStream *pic = NULL;

		if (_vm->getVersion() >= 4 && bitmapCast->_children.size() > 0) {
			imgId = bitmapCast->_children[0].index;
			tag = bitmapCast->_children[0].tag;

			if (_movieArchive->hasResource(tag, imgId))
				pic = _movieArchive->getResource(tag, imgId);
			else if (sharedScore && sharedScore->getArchive()->hasResource(tag, imgId))
				pic = sharedScore->getArchive()->getResource(tag, imgId);
		} else {
			if (_loadedCast->contains(imgId)) {
				bitmapCast->_tag = tag = ((BitmapCast *)_loadedCast->getVal(imgId))->_tag;
				realId = imgId + _castIDoffset;
				pic = _movieArchive->getResource(tag, realId);
			} else if (sharedScore && sharedScore->_loadedCast && sharedScore->_loadedCast->contains(imgId)) {
				bitmapCast->_tag = tag = ((BitmapCast *)sharedScore->_loadedCast->getVal(imgId))->_tag;
				realId = imgId + sharedScore->_castIDoffset;
				pic = sharedScore->getArchive()->getResource(tag, realId);
			}
		}

		if (pic == NULL) {
			warning("Score::loadSpriteImages(): Image %d not found", imgId);
			continue;
		}

		int w = bitmapCast->_initialRect.width();
		int h = bitmapCast->_initialRect.height();

		switch (tag) {
		case MKTAG('D', 'I', 'B', ' '):
			debugC(2, kDebugLoading, "****** Loading 'DIB ' id: %d (%d), %d bytes", imgId, realId, pic->size());
			img = new DIBDecoder();
			break;

		case MKTAG('B', 'I', 'T', 'D'):
			debugC(2, kDebugLoading, "****** Loading 'BITD' id: %d (%d), %d bytes", imgId, realId, pic->size());

			if (w > 0 && h > 0) {
				if (_vm->getVersion() < 6) {
					img = new BITDDecoder(w, h, bitmapCast->_bitsPerPixel, bitmapCast->_pitch);
				} else {
					img = new Image::BitmapDecoder();
				}
			} else {
				warning("Score::loadSpriteImages(): Image %d not found", imgId);
			}

			break;

		default:
			warning("Score::loadSpriteImages(): Unknown Bitmap Cast Tag: [%d] %s", tag, tag2str(tag));
			break;
		}

		img->loadStream(*pic);
		bitmapCast->_surface = img->getSurface();

		debugC(4, kDebugImages, "Score::loadSpriteImages(): id: %d, w: %d, h: %d, flags: %x, bytes: %x, bpp: %d clut: %x",
			imgId, w, h, bitmapCast->_flags, bitmapCast->_bytes, bitmapCast->_bitsPerPixel, bitmapCast->_clut);
	}
}


Score::~Score() {
	if (_surface && _surface->w)
		_surface->free();

	if (_trailSurface && _trailSurface->w)
		_trailSurface->free();

	if (_backSurface && _backSurface->w)
		_backSurface->free();

	delete _backSurface;
	delete _surface;
	delete _trailSurface;

	if (_movieArchive)
		_movieArchive->close();

	delete _font;
	delete _labels;
	delete _loadedStxts;
	delete _loadedCast;
}

void Score::loadPalette(Common::SeekableSubReadStreamEndian &stream) {
	uint16 steps = stream.size() / 6;
	uint16 index = (steps * 3) - 1;
	byte *_palette = new byte[index + 1];

	debugC(3, kDebugLoading, "Score::loadPalette(): %d steps, %d bytes", steps, stream.size());

	if (steps > 256) {
		warning("Score::loadPalette(): steps > 256: %d", steps);
		steps = 256;
	}

	for (int i = 0; i < steps; i++) {
		_palette[index - 2] = stream.readByte();
		stream.readByte();

		_palette[index - 1] = stream.readByte();
		stream.readByte();

		_palette[index] = stream.readByte();
		stream.readByte();
		index -= 3;
	}
	_vm->setPalette(_palette, steps);
}

void Score::loadFrames(Common::SeekableSubReadStreamEndian &stream) {
	debugC(1, kDebugLoading, "****** Loading frames VWSC");

	//stream.hexdump(stream.size());

	uint32 size = stream.readUint32();
	size -= 4;

	if (_vm->getVersion() < 4) {
		_numChannelsDisplayed = 30;
	} else if (_vm->getVersion() == 4) {
		uint32 frame1Offset = stream.readUint32();
		uint32 numFrames = stream.readUint32();
		uint16 version = stream.readUint16();
		uint16 spriteRecordSize = stream.readUint16();
		uint16 numChannels = stream.readUint16();
		size -= 14;

		if (version > 13) {
			_numChannelsDisplayed = stream.readUint16();
		} else {
			if (version <= 7)	// Director5
				_numChannelsDisplayed = 48;
			else
				_numChannelsDisplayed = 120;	// D6

			stream.readUint16(); // Skip
		}

		size -= 2;

		warning("STUB: Score::loadFrames. frame1Offset: %x numFrames: %x version: %x spriteRecordSize: %x numChannels: %x numChannelsDisplayed: %x",
			frame1Offset, numFrames, version, spriteRecordSize, numChannels, _numChannelsDisplayed);
		// Unknown, some bytes - constant (refer to contuinity).
	} else if (_vm->getVersion() > 4) {
		//what data is up the top of D5 VWSC?
		uint32 unk1 = stream.readUint32();
		uint32 unk2 = stream.readUint32();

		uint16 unk3, unk4, unk5, unk6;

		if (unk2 > 0) {
			uint32 blockSize = stream.readUint32() - 1;
			stream.readUint32();
			stream.readUint32();
			stream.readUint32();
			stream.readUint32();
			for (uint32 skip = 0; skip < blockSize * 4; skip++)
				stream.readByte();

			//header number two... this is our actual score entry point.
			unk1 = stream.readUint32();
			unk2 = stream.readUint32();
			stream.readUint32();
			unk3 = stream.readUint16();
			unk4 = stream.readUint16();
			unk5 = stream.readUint16();
			unk6 = stream.readUint16();
		} else {
			unk3 = stream.readUint16();
			unk4 = stream.readUint16();
			unk5 = stream.readUint16();
			unk6 = stream.readUint16();
			size -= 16;
		}
		warning("STUB: Score::loadFrames. unk1: %x unk2: %x unk3: %x unk4: %x unk5: %x unk6: %x", unk1, unk2, unk3, unk4, unk5, unk6);
	}

	uint16 channelSize;
	uint16 channelOffset;

	Frame *initial = new Frame(_vm, _numChannelsDisplayed);
	// Push a frame at frame#0 position.
	// This makes all indexing simpler
	_frames.push_back(initial);

	// This is a representation of the channelData. It gets overridden
	// partically by channels, hence we keep it and read the score from left to right
	//
	// TODO Merge it with shared cast
	byte channelData[kChannelDataSize];
	memset(channelData, 0, kChannelDataSize);

	while (size != 0 && !stream.eos()) {
		uint16 frameSize = stream.readUint16();
		debugC(kDebugLoading, 8, "++++++++++ score frame %d (frameSize %d) size %d", _frames.size(), frameSize, size);

		if (frameSize > 0) {
			Frame *frame = new Frame(_vm, _numChannelsDisplayed);
			size -= frameSize;
			frameSize -= 2;

			while (frameSize != 0) {

				if (_vm->getVersion() < 4) {
					channelSize = stream.readByte() * 2;
					channelOffset = stream.readByte() * 2;
					frameSize -= channelSize + 2;
				} else {
					channelSize = stream.readUint16();
					channelOffset = stream.readUint16();
					frameSize -= channelSize + 4;
				}

				assert(channelOffset + channelSize < kChannelDataSize);
				stream.read(&channelData[channelOffset], channelSize);
			}

			Common::MemoryReadStreamEndian *str = new Common::MemoryReadStreamEndian(channelData, ARRAYSIZE(channelData), stream.isBE());
			// str->hexdump(str->size(), 32);
			frame->readChannels(str);
			delete str;

			debugC(8, kDebugLoading, "Score::loadFrames(): Frame %d actionId: %d", _frames.size(), frame->_actionId);

			_frames.push_back(frame);
		} else {
			warning("zero sized frame!? exiting loop until we know what to do with the tags that follow.");
			size = 0;
		}
	}
}

void Score::loadConfig(Common::SeekableSubReadStreamEndian &stream) {
	debugC(1, kDebugLoading, "****** Loading Config VWCF");

	if (debugChannelSet(5, kDebugLoading))
		stream.hexdump(stream.size());

	uint16 len = stream.readUint16();
	uint16 ver1 = stream.readUint16();
	_movieRect = Score::readRect(stream);

	_castArrayStart = stream.readUint16();
	_castArrayEnd = stream.readUint16();
	_currentFrameRate = stream.readByte();

	if (_currentFrameRate == 0)
		_currentFrameRate = 20;

	byte lightswitch = stream.readByte();
	uint16 unk1 = stream.readUint16();
	uint16 commentFont = stream.readUint16();
	uint16 commentSize = stream.readUint16();
	uint16 commentStyle = stream.readUint16();
	_stageColor = stream.readUint16();
	uint16 bitdepth = stream.readUint16();
	byte color = stream.readByte();	// boolean, color = 1, B/W = 0
	uint16 stageColorR = stream.readUint16();
	uint16 stageColorG = stream.readUint16();
	uint16 stageColorB = stream.readUint16();

	for (int i = 0; i < 0x0b; i++) {
		stream.readByte();
	}

	if (_vm->getVersion() >= 4) {
		for (int i = 0; i < 0x16; i++) {
			stream.readByte();
		}

		int palette = (int16)stream.readUint16();
		_vm->setPalette(palette - 1);

		for (int i = 0; i < 0x08; i++) {
			stream.readByte();
		}
	}

	debugC(1, kDebugLoading, "Score::loadConfig(): len: %d, ver: %d, framerate: %d, light: %d, unk: %d, font: %d, size: %d"
			", style: %d", len, ver1, _currentFrameRate, lightswitch, unk1, commentFont, commentSize, commentStyle);
	debugC(1, kDebugLoading, "Score::loadConfig(): stagecolor: %d, depth: %d, color: %d, rgb: 0x%04x 0x%04x 0x%04x",
			_stageColor, bitdepth, color, stageColorR, stageColorG, stageColorB);
	if (debugChannelSet(1, kDebugLoading))
		_movieRect.debugPrint(1, "Score::loadConfig(): Movie rect: ");
}

void Score::readVersion(uint32 rid) {
	_versionMinor = rid & 0xffff;
	_versionMajor = rid >> 16;

	debug("Version: %d.%d", _versionMajor, _versionMinor);
}

void Score::loadCastDataVWCR(Common::SeekableSubReadStreamEndian &stream) {
	debugC(1, kDebugLoading, "****** Loading Cast rects VWCR. start: %d, end: %d", _castArrayStart, _castArrayEnd);

	_loadedCast = new Common::HashMap<int, Cast *>();

	for (uint16 id = _castArrayStart; id <= _castArrayEnd; id++) {
		byte size = stream.readByte();
		uint32 tag;
		if (size == 0)
			continue;

		if (debugChannelSet(5, kDebugLoading))
			stream.hexdump(size);

		uint8 castType = stream.readByte();

		switch (castType) {
		case kCastBitmap:
			debugC(3, kDebugLoading, "Score::loadCastDataVWCR(): CastTypes id: %d(%s) BitmapCast", id, numToCastNum(id));
			if (_movieArchive->hasResource(MKTAG('B', 'I', 'T', 'D'), id + _castIDoffset))
				tag = MKTAG('B', 'I', 'T', 'D');
			else if (_movieArchive->hasResource(MKTAG('D', 'I', 'B', ' '), id + _castIDoffset))
				tag = MKTAG('D', 'I', 'B', ' ');
			else
				error("Score::loadCastDataVWCR(): non-existent reference to BitmapCast");

			_loadedCast->setVal(id, new BitmapCast(stream, tag, _vm->getVersion()));
			break;
		case kCastText:
			debugC(3, kDebugLoading, "Score::loadCastDataVWCR(): CastTypes id: %d(%s) TextCast", id, numToCastNum(id));
			_loadedCast->setVal(id, new TextCast(stream, _vm->getVersion(), 255 - _stageColor));
			break;
		case kCastShape:
			debugC(3, kDebugLoading, "Score::loadCastDataVWCR(): CastTypes id: %d(%s) ShapeCast", id, numToCastNum(id));
			_loadedCast->setVal(id, new ShapeCast(stream, _vm->getVersion()));
			break;
		case kCastButton:
			debugC(3, kDebugLoading, "Score::loadCastDataVWCR(): CastTypes id: %d(%s) ButtonCast", id, numToCastNum(id));
			_loadedCast->setVal(id, new ButtonCast(stream, _vm->getVersion()));
			break;
		default:
			warning("Score::loadCastDataVWCR(): Unhandled cast id: %d(%s), type: %d, %d bytes", id, numToCastNum(id), castType, size);
			stream.skip(size - 1);
			break;
		}
	}
}

void Score::setSpriteCasts() {
	// Set cast pointers to sprites
	for (uint16 i = 0; i < _frames.size(); i++) {
		for (uint16 j = 0; j < _frames[i]->_sprites.size(); j++) {
			uint16 castId = _frames[i]->_sprites[j]->_castId;

			if (castId == 0)
				continue;

			if (_vm->getSharedScore() && _vm->getSharedScore()->_loadedCast && _vm->getSharedScore()->_loadedCast->contains(castId)) {
				_frames[i]->_sprites[j]->_cast = _vm->getSharedScore()->_loadedCast->getVal(castId);
			} else if (_loadedCast->contains(castId)) {
				_frames[i]->_sprites[j]->_cast = _loadedCast->getVal(castId);
			}
		}
	}
}

void Score::loadCastData(Common::SeekableSubReadStreamEndian &stream, uint16 id, Resource *res) {
	// D4+ variant
	if (stream.size() == 0)
		return;

	// TODO: Determine if there really is a minimum size.
	// This value was too small for Shape Casts.
	if (stream.size() < 10) {
		warning("Score::loadCastData(): CASt data id %d is too small", id);
		return;
	}

	debugC(3, kDebugLoading, "Score::loadCastData(): CASt: id: %d", id);

	if (debugChannelSet(5, kDebugLoading) && stream.size() < 2048)
		stream.hexdump(stream.size());

	uint32 size1, size2, size3, castType, sizeToRead;
	byte unk1 = 0, unk2 = 0, unk3 = 0;

	if (_vm->getVersion() <= 3) {
		size1 = stream.readUint16();
		sizeToRead = size1 +16; // 16 is for bounding rects
		size2 = stream.readUint32();
		size3 = 0;
		castType = stream.readByte();
		unk1 = stream.readByte();
		unk2 = stream.readByte();
		unk3 = stream.readByte();
	} else if (_vm->getVersion() == 4) {
		size1 = stream.readUint16();
		sizeToRead = size1 + 2 + 16; // 16 is for bounding rects
		size2 = stream.readUint32();
		size3 = 0;
		castType = stream.readByte();
		unk1 = stream.readByte();
	} else if (_vm->getVersion() == 5) {
		castType = stream.readUint32();
		size3 = stream.readUint32();
		size2 = stream.readUint32();
		size1 = stream.readUint32();
		if (castType == 1) {
			if (size3 == 0)
				return;
			for (uint32 skip = 0; skip < (size1 - 4) / 4; skip++)
				stream.readUint32();
		}

		sizeToRead = stream.size();
	} else {
		error("Score::loadCastData: unsupported Director version (%d)", _vm->getVersion());
	}

	debugC(3, kDebugLoading, "Score::loadCastData(): CASt: id: %d type: %x size1: %d size2: %d (%x) size3: %d unk1: %d unk2: %d unk3: %d",
		id, castType, size1, size2, size2, size3, unk1, unk2, unk3);

	byte *data = (byte *)calloc(sizeToRead, 1);
	stream.read(data, sizeToRead);

	Common::MemoryReadStreamEndian castStream(data, sizeToRead, stream.isBE());

	switch (castType) {
	case kCastBitmap:
		debugC(3, kDebugLoading, "Score::loadCastData(): loading kCastBitmap (%d children)", res->children.size());
		_loadedCast->setVal(id, new BitmapCast(castStream, res->tag, _vm->getVersion()));
		break;
	case kCastText:
		debugC(3, kDebugLoading, "Score::loadCastData(): loading kCastText (%d children)", res->children.size());
		_loadedCast->setVal(id, new TextCast(castStream, _vm->getVersion(), 255 - _stageColor));
		break;
	case kCastShape:
		debugC(3, kDebugLoading, "Score::loadCastData(): loading kCastShape (%d children)", res->children.size());
		_loadedCast->setVal(id, new ShapeCast(castStream, _vm->getVersion()));
		break;
	case kCastButton:
		debugC(3, kDebugLoading, "Score::loadCastData(): loading kCastButton (%d children)", res->children.size());
		_loadedCast->setVal(id, new ButtonCast(castStream, _vm->getVersion()));
		break;
	case kCastLingoScript:
		debugC(3, kDebugLoading, "Score::loadCastData(): loading kCastLingoScript");
		_loadedCast->setVal(id, new ScriptCast(castStream, _vm->getVersion()));
		break;
	case kCastRTE:
		debugC(3, kDebugLoading, "Score::loadCastData(): loading kCastRTE (%d children)", res->children.size());
		_loadedCast->setVal(id, new RTECast(castStream, _vm->getVersion(), 255 - _stageColor));
		break;
	case kCastFilmLoop:
		warning("STUB: Score::loadCastData(): kCastFilmLoop (%d children)", res->children.size());
		size2 = 0;
		break;
	case kCastPalette:
		warning("STUB: Score::loadCastData(): kCastPalette (%d children)", res->children.size());
		size2 = 0;
		break;
	case kCastPicture:
		warning("STUB: Score::loadCastData(): kCastPicture (%d children)", res->children.size());
		size2 = 0;
		break;
	case kCastSound:
		warning("STUB: Score::loadCastData(): kCastSound (%d children)", res->children.size());
		size2 = 0;
		break;
	case kCastMovie:
		warning("STUB: Score::loadCastData(): kCastMovie (%d children)", res->children.size());
		size2 = 0;
		break;
	case kCastDigitalVideo:
		warning("STUB: Score::loadCastData(): kCastDigitalVideo (%d children)", res->children.size());
		size2 = 0;
		break;
	default:
		warning("Score::loadCastData(): Unhandled cast type: %d [%s] (%d children)", castType, tag2str(castType), res->children.size());
		// also don't try and read the strings... we don't know what this item is.
		size2 = 0;
		break;
	}

	if (_loadedCast->contains(id)) { // Skip unhandled casts
		debugCN(3, kDebugLoading, "Children: ");
		for (uint child = 0; child < res->children.size(); child++) {
			debugCN(3, kDebugLoading, "%d ", res->children[child].index);
			_loadedCast->getVal(id)->_children.push_back(res->children[child]);
		}
		debugCN(3, kDebugLoading, "\n");
	}

	free(data);

	if (size2 && _vm->getVersion() < 5) {
		uint32 entryType = 0;
		Common::Array<Common::String> castStrings = loadStrings(stream, entryType, false);

		debugCN(4, kDebugLoading, "Score::loadCastData(): str(%d): '", castStrings.size());

		for (uint i = 0; i < castStrings.size(); i++) {
			debugCN(4, kDebugLoading, "%s'", castStrings[i].c_str());
			if (i != castStrings.size() - 1)
				debugCN(4, kDebugLoading, ", '");
		}
		debugC(4, kDebugLoading, "'");

		CastInfo *ci = new CastInfo();

		// We have here variable number of strings. Thus, instead of
		// adding tons of ifs, we use this switch()
		switch (castStrings.size()) {
		default:
			warning("Score::loadCastData(): extra %d strings", castStrings.size() - 5);
			// fallthrough
		case 5:
			ci->type = castStrings[4];
			// fallthrough
		case 4:
			ci->fileName = castStrings[3];
			// fallthrough
		case 3:
			ci->directory = castStrings[2];
			// fallthrough
		case 2:
			ci->name = castStrings[1];

			if (!ci->name.empty()) {
				_castsNames[ci->name] = id;
			}
			// fallthrough
		case 1:
			ci->script = castStrings[0];
			// fallthrough
		case 0:
			break;
		}

		// FIXME. Disabled by default, requires --debugflags=bytecode for now
		if (_vm->getVersion() >= 4 && castType == kCastLingoScript && debugChannelSet(-1, kDebugBytecode)) {
			// Try and load the compiled Lingo script associated with this cast
			uint scriptId = ((ScriptCast *)(*_loadedCast)[id])->_id - 1;
			if (scriptId < _castScriptIds.size()) {
				int resourceId = _castScriptIds[scriptId];
				_lingo->addCodeV4(*_movieArchive->getResource(MKTAG('L', 's', 'c', 'r'), resourceId), ((ScriptCast *)_loadedCast->getVal(id))->_scriptType, id);
			} else {
				warning("Score::loadCastData(): Lingo context missing a resource entry for script %d referenced in cast %d", scriptId, id);
			}
		} else {
			if (!ci->script.empty()) {
				if (_loadedCast->getVal(id)->_type == kCastLingoScript) {
					// the script type here could be wrong!
					if (ConfMan.getBool("dump_scripts"))
						dumpScript(ci->script.c_str(), ((ScriptCast *)_loadedCast->getVal(id))->_scriptType, id);

					_lingo->addCode(ci->script.c_str(), ((ScriptCast *)_loadedCast->getVal(id))->_scriptType, id);
				} else {
					warning("Score::loadCastData(): Wrong cast type: %d", _loadedCast->getVal(id)->_type);
				}
			}
		}

		_castsInfo[id] = ci;
	}

	if (size3)
		warning("Score::loadCastData(): size3: %x", size3);
}

void Score::loadCastInto(Sprite *sprite, int castId) {
	sprite->_cast = _loadedCast->getVal(castId);
}

Common::Rect Score::getCastMemberInitialRect(int castId) {
	Cast *cast = _loadedCast->getVal(castId);

	if (!cast) {
		warning("Score::getCastMemberInitialRect(%d): empty cast", castId);
		return Common::Rect(0, 0);
	}

	return cast->_initialRect;
}

void Score::setCastMemberModified(int castId) {
	Cast *cast = _loadedCast->getVal(castId);

	if (!cast) {
		warning("Score::setCastMemberModified(%d): empty cast", castId);
		return;
	}

	cast->_modified = 1;
}

void Score::loadLabels(Common::SeekableSubReadStreamEndian &stream) {
	if (debugChannelSet(5, kDebugLoading)) {
		debug("Score::loadLabels()");
		stream.hexdump(stream.size());
	}

	_labels = new Common::SortedArray<Label *>(compareLabels);
	uint16 count = stream.readUint16() + 1;
	uint32 offset = count * 4 + 2;

	uint16 frame = stream.readUint16();
	uint32 stringPos = stream.readUint16() + offset;

	for (uint16 i = 1; i < count; i++) {
		uint16 nextFrame = stream.readUint16();
		uint32 nextStringPos = stream.readUint16() + offset;
		uint32 streamPos = stream.pos();

		stream.seek(stringPos);
		Common::String label;

		for (uint32 j = stringPos; j < nextStringPos; j++) {
			label += stream.readByte();
		}

		_labels->insert(new Label(label, frame));
		stream.seek(streamPos);

		frame = nextFrame;
		stringPos = nextStringPos;
	}

	Common::SortedArray<Label *>::iterator j;

	debugC(2, kDebugLoading, "****** Loading labels");
	for (j = _labels->begin(); j != _labels->end(); ++j) {
		debugC(2, kDebugLoading, "Frame %d, Label '%s'", (*j)->number, Common::toPrintable((*j)->name).c_str());
	}
}

int Score::compareLabels(const void *a, const void *b) {
	return ((const Label *)a)->number - ((const Label *)b)->number;
}

void Score::loadActions(Common::SeekableSubReadStreamEndian &stream) {
	debugC(2, kDebugLoading, "****** Loading Actions VWAC");

	uint16 count = stream.readUint16() + 1;
	uint32 offset = count * 4 + 2;

	byte id = stream.readByte();

	byte subId = stream.readByte(); // I couldn't find how it used in continuity (except print). Frame actionId = 1 byte.
	uint32 stringPos = stream.readUint16() + offset;

	for (uint16 i = 0; i < count; i++) {
		uint16 nextId = stream.readByte();
		byte nextSubId = stream.readByte();
		uint32 nextStringPos = stream.readUint16() + offset;
		uint32 streamPos = stream.pos();

		stream.seek(stringPos);

		for (uint16 j = stringPos; j < nextStringPos; j++) {
			byte ch = stream.readByte();
			if (ch == 0x0d) {
				ch = '\n';
			}
			_actions[i + 1] += ch;
		}

		debugC(3, kDebugLoading, "Action id: %d nextId: %d subId: %d, code: %s", id, nextId, subId, _actions[id].c_str());

		stream.seek(streamPos);

		id = nextId;
		subId = nextSubId;
		stringPos = nextStringPos;

		if ((int32)stringPos == stream.size())
			break;
	}

	Common::HashMap<uint16, Common::String>::iterator j;

	if (ConfMan.getBool("dump_scripts"))
		for (j = _actions.begin(); j != _actions.end(); ++j) {
			if (!j->_value.empty())
				dumpScript(j->_value.c_str(), kFrameScript, j->_key);
		}

	for (j = _actions.begin(); j != _actions.end(); ++j)
		if (!j->_value.empty()) {
			_lingo->addCode(j->_value.c_str(), kFrameScript, j->_key);

			processImmediateFrameScript(j->_value, j->_key);
		}
}

bool Score::processImmediateFrameScript(Common::String s, int id) {
	s.trim();

	// In D2/D3 this specifies immediately the sprite/field properties
	if (!s.compareToIgnoreCase("moveableSprite") || !s.compareToIgnoreCase("editableText")) {
		_immediateActions[id] = true;
	}

	return false;
}

void Score::loadLingoNames(Common::SeekableSubReadStreamEndian &stream) {
	if (_vm->getVersion() >= 4) {
		_lingo->addNamesV4(stream);
	} else {
		error("Score::loadLingoNames: unsuported Director version (%d)", _vm->getVersion());
	}
}

void Score::loadLingoContext(Common::SeekableSubReadStreamEndian &stream) {
	if (_vm->getVersion() >= 4) {
		debugC(1, kDebugLingoCompile, "Add V4 script context");

		if (debugChannelSet(5, kDebugLoading)) {
			debugC(5, kDebugLoading, "Lctx header:");
			stream.hexdump(0x2a);
		}

		_castScriptIds.clear();

		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		uint16 itemCount = stream.readUint16();
		stream.readUint16();
		/*uint16 itemCount2 = */ stream.readUint16();
		uint16 itemsOffset = stream.readUint16();

		stream.seek(itemsOffset);
		for (uint16 i = 0; i < itemCount; i++) {
			if (debugChannelSet(5, kDebugLoading)) {
				debugC(5, kDebugLoading, "Context entry %d:", i);
				stream.hexdump(0xc);
			}

			stream.readUint16();
			stream.readUint16();
			stream.readUint16();
			uint16 index = stream.readUint16();
			stream.readUint16();
			stream.readUint16();

			_castScriptIds.push_back(index);
		}
	} else {
		error("Score::loadLingoContext: unsuported Director version (%d)", _vm->getVersion());
	}
}

void Score::loadScriptText(Common::SeekableSubReadStreamEndian &stream) {
	/*uint32 unk1 = */ stream.readUint32();
	uint32 strLen = stream.readUint32();
	/*uin32 dataLen = */ stream.readUint32();
	Common::String script;

	for (uint32 i = 0; i < strLen; i++) {
		byte ch = stream.readByte();

		// Convert Mac line endings
		if (ch == 0x0d)
			ch = '\n';

		script += ch;
	}

	// Check if the script has macro. They must start with a comment.
	// See D2 Interactivity Manual pp.46-47 (Ch.2.11. Using a macro)
	if (script.empty() || !script.hasPrefix("--"))
		return;

	int pos = 2;
	while (script[pos] == ' ' || script[pos] == '\t')
		pos++;

	if (script[pos] != '\n')
		return;

	if (ConfMan.getBool("dump_scripts"))
		dumpScript(script.c_str(), kMovieScript, _movieScriptCount);

	if (script.contains("\nmenu:") || script.hasPrefix("menu:"))
		return;

	_lingo->addCode(script.c_str(), kMovieScript, _movieScriptCount);

	_movieScriptCount++;
}

void Score::setStartToLabel(Common::String label) {
	if (!_labels) {
		warning("setStartToLabel: No labels set");
		return;
	}

	Common::SortedArray<Label *>::iterator i;

	for (i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->name.equalsIgnoreCase(label)) {
			_nextFrame = (*i)->number;
			return;
		}
	}
	warning("Label %s not found", label.c_str());
}

void Score::dumpScript(const char *script, ScriptType type, uint16 id) {
	Common::DumpFile out;
	Common::String typeName;
	char buf[256];

	switch (type) {
	case kNoneScript:
	default:
		error("Incorrect dumpScript() call (type %d)", type);
	case kFrameScript:
		typeName = "frame";
		break;
	case kMovieScript:
		typeName = "movie";
		break;
	case kSpriteScript:
		typeName = "sprite";
		break;
	case kCastScript:
		typeName = "cast";
		break;
	case kGlobalScript:
		typeName = "global";
		break;
	case kScoreScript:
		typeName = "score";
		break;
	}

	sprintf(buf, "./dumps/%s-%s-%d.txt", _macName.c_str(), typeName.c_str(), id);

	if (!out.open(buf)) {
		warning("Can not open dump file %s", buf);
		return;
	}

	out.write(script, strlen(script));

	out.flush();
	out.close();
}

void Score::loadCastInfo(Common::SeekableSubReadStreamEndian &stream, uint16 id) {
	uint32 entryType = 0;
	Common::Array<Common::String> castStrings = loadStrings(stream, entryType);
	CastInfo *ci = new CastInfo();

	ci->script = castStrings[0];

	if (!ci->script.empty() && ConfMan.getBool("dump_scripts"))
		dumpScript(ci->script.c_str(), kSpriteScript, id);

	if (!ci->script.empty())
		_lingo->addCode(ci->script.c_str(), kSpriteScript, id);

	ci->name = getString(castStrings[1]);
	ci->directory = getString(castStrings[2]);
	ci->fileName = getString(castStrings[3]);
	ci->type = castStrings[4];

	debugC(5, kDebugLoading, "Score::loadCastInfo(): CastInfo: name: '%s' directory: '%s', fileName: '%s', type: '%s'",
				ci->name.c_str(), ci->directory.c_str(), ci->fileName.c_str(), ci->type.c_str());

	if (!ci->name.empty())
		_castsNames[ci->name] = id;

	_castsInfo[id] = ci;
}

void Score::gotoLoop() {
	// This command has the playback head contonuously return to the first marker to to the left and then loop back.
	// If no marker are to the left of the playback head, the playback head continues to the right.
	if (_labels == NULL) {
		_nextFrame = 1;
		return;
	} else {
		_nextFrame = _currentLabel;
	}

	_vm->_skipFrameAdvance = true;
}

int Score::getCurrentLabelNumber() {
	Common::SortedArray<Label *>::iterator i;

	int frame = 0;

	for (i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->number <= _currentFrame)
			frame = (*i)->number;
	}

	return frame;
}

void Score::gotoNext() {
	// we can just try to use the current frame and get the next label
	_nextFrame = getNextLabelNumber(_currentFrame);
}

void Score::gotoPrevious() {
	// we actually need the frame of the label prior to the most recent label.
	_nextFrame = getPreviousLabelNumber(getCurrentLabelNumber());
}

int Score::getNextLabelNumber(int referenceFrame) {
	if (_labels == NULL || _labels->size() == 0)
		return 0;

	Common::SortedArray<Label *>::iterator i;

	for (i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->number >= referenceFrame) {
			int n = (*i)->number;
			++i;
			if (i != _labels->end()) {
				// return to the first marker to to the right
				return (*i)->number;
			} else {
				// if no markers are to the right of the playback head,
				// the playback head goes to the first marker to the left
				return n;
			}
		}
	}

	// If there are not markers to the left,
	// the playback head goes to frame 1, (Director frame array start from 1, engine from 0)
	return 0;
}

int Score::getPreviousLabelNumber(int referenceFrame) {
	if (_labels == NULL || _labels->size() == 0)
		return 0;

	// One label
	if (_labels->begin() == _labels->end())
		return (*_labels->begin())->number;

	Common::SortedArray<Label *>::iterator previous = _labels->begin();
	Common::SortedArray<Label *>::iterator i;

	for (i = (previous + 1); i != _labels->end(); ++i, ++previous) {
		if ((*i)->number >= referenceFrame)
			return (*previous)->number;
	}

	return 0;
}

Common::String Score::getString(Common::String str) {
	if (str.size() == 0) {
		return str;
	}

	uint8 f = static_cast<uint8>(str.firstChar());

	if (f == 0) {
		return "";
	}

	str.deleteChar(0);

	if (str.lastChar() == '\x00') {
		str.deleteLastChar();
	}

	return str;
}

void Score::loadFileInfo(Common::SeekableSubReadStreamEndian &stream) {
	debugC(2, kDebugLoading, "****** Loading FileInfo VWFI");

	Common::Array<Common::String> fileInfoStrings = loadStrings(stream, _flags);
	_script = fileInfoStrings[0];

	if (!_script.empty() && ConfMan.getBool("dump_scripts"))
		dumpScript(_script.c_str(), kMovieScript, _movieScriptCount);

	if (!_script.empty())
		_lingo->addCode(_script.c_str(), kMovieScript, _movieScriptCount);

	_movieScriptCount++;
	_changedBy = fileInfoStrings[1];
	_createdBy = fileInfoStrings[2];
	_directory = fileInfoStrings[3];
}

Common::Array<Common::String> Score::loadStrings(Common::SeekableSubReadStreamEndian &stream, uint32 &entryType, bool hasHeader) {
	Common::Array<Common::String> strings;
	uint32 offset = 0;

	if (hasHeader) {
		offset = stream.readUint32();
		/*uint32 unk1 = */ stream.readUint32();
		/*uint32 unk2 = */ stream.readUint32();
		entryType = stream.readUint32();
		stream.seek(offset);
	}

	uint16 count = stream.readUint16() + 1;

	debugC(3, kDebugLoading, "Score::loadStrings(): Strings: %d entries", count);

	uint32 *entries = (uint32 *)calloc(count, sizeof(uint32));

	for (uint i = 0; i < count; i++)
		entries[i] = stream.readUint32();

	byte *data = (byte *)malloc(entries[count - 1]);
	stream.read(data, entries[count - 1]);

	for (uint16 i = 0; i < count - 1; i++) {
		Common::String entryString;

		for (uint j = entries[i]; j < entries[i + 1]; j++)
			if (data[j] == '\r')
				entryString += '\n';
			else if (j > entries[i] || data[j] >= 0x20) // Skip first byte which is string length
				entryString += data[j];

		strings.push_back(entryString);

		debugC(6, kDebugLoading, "String %d:\n%s\n", i, Common::toPrintable(entryString).c_str());
	}

	free(data);
	free(entries);

	return strings;
}

void Score::loadFontMap(Common::SeekableSubReadStreamEndian &stream) {
	if (stream.size() == 0)
		return;

	debugC(2, kDebugLoading, "****** Loading FontMap VWFM");

	uint16 count = stream.readUint16();
	uint32 offset = (count * 2) + 2;
	uint32 currentRawPosition = offset;

	for (uint16 i = 0; i < count; i++) {
		uint16 id = stream.readUint16();
		uint32 positionInfo = stream.pos();

		stream.seek(currentRawPosition);

		uint16 size = stream.readByte();
		Common::String font;

		for (uint16 k = 0; k < size; k++) {
			font += stream.readByte();
		}

		_fontMap[id] = font;
		_vm->_wm->_fontMan->registerFontMapping(id, font);

		debugC(3, kDebugLoading, "Fontmap. ID %d Font %s", id, font.c_str());
		currentRawPosition = stream.pos();
		stream.seek(positionInfo);
	}
}

Common::Rect Score::readRect(Common::ReadStreamEndian &stream) {
	Common::Rect rect;
	rect.top = stream.readUint16();
	rect.left = stream.readUint16();
	rect.bottom = stream.readUint16();
	rect.right = stream.readUint16();

	return rect;
}

void Score::startLoop() {

	debugC(1, kDebugImages, "Score dims: %dx%d", _movieRect.width(), _movieRect.height());

	initGraphics(_movieRect.width(), _movieRect.height());

	_surface = new Graphics::ManagedSurface;
	_trailSurface = new Graphics::ManagedSurface;
	_backSurface = new Graphics::ManagedSurface;

	_surface->create(_movieRect.width(), _movieRect.height());
	_trailSurface->create(_movieRect.width(), _movieRect.height());
	_backSurface->create(_movieRect.width(), _movieRect.height());

	g_director->_wm->setScreen(_surface);

	_trailSurface->clear(255 - _stageColor);

	_currentFrame = 0;
	_stopPlay = false;
	_nextFrameTime = 0;

	_lingo->processEvent(kEventStartMovie);

	_frames[_currentFrame]->prepareFrame(this);

	while (!_stopPlay) {
		if (_currentFrame >= _frames.size()) {
			if (debugChannelSet(-1, kDebugNoLoop))
				break;

			_currentFrame = 0;
		}

		update();

		if (_currentFrame < _frames.size())
			_vm->processEvents();
	}

	_lingo->processEvent(kEventStopMovie);
}

void Score::update() {
	if (g_system->getMillis() < _nextFrameTime) {
		renderZoomBox(true);

		_vm->_wm->draw();

		return;
	}

	// For previous frame
	if (_currentFrame > 0) {
		// When Lingo::func_goto* is called, _nextFrame is set
		// and _skipFrameAdvance is set to true.
		// However, the exitFrame event can overwrite the value
		// for _nextFrame before it can be used.
		// Because we still want to call exitFrame, we check if
		// a goto call has been made and if so, cache the value
		// of _nextFrame so it doesn't get wiped.
		if (_vm->_skipFrameAdvance) {
			uint16 nextFrameCache = _nextFrame;
			_lingo->processEvent(kEventExitFrame);
			_nextFrame = nextFrameCache;
		} else {
			_lingo->processEvent(kEventExitFrame);
		}
	}

	if (!_vm->_playbackPaused) {
		if (_nextFrame)
			_currentFrame = _nextFrame;
		else
			_currentFrame++;
	}

	_nextFrame = 0;

	_vm->_skipFrameAdvance = false;

	if (_currentFrame >= _frames.size())
		return;

	Common::SortedArray<Label *>::iterator i;
	if (_labels != NULL) {
		for (i = _labels->begin(); i != _labels->end(); ++i) {
			if ((*i)->number == _currentFrame) {
				_currentLabel = _currentFrame;
			}
		}
	}

	debugC(1, kDebugImages, "******************************  Current frame: %d", _currentFrame);

	_surface->clear(255 - _stageColor);
	_surface->copyFrom(*_trailSurface);

	_lingo->executeImmediateScripts(_frames[_currentFrame]);

	if (_vm->getVersion() >= 6) {
		_lingo->processEvent(kEventBeginSprite);
		// TODO Director 6 step: send beginSprite event to any sprites whose span begin in the upcoming frame
		_lingo->processEvent(kEventPrepareFrame);
		// TODO: Director 6 step: send prepareFrame event to all sprites and the script channel in upcoming frame
	}

	_frames[_currentFrame]->prepareFrame(this);
	// Stage is drawn between the prepareFrame and enterFrame events (Lingo in a Nutshell, p.100)

	// Enter and exit from previous frame (Director 4)
	_lingo->processEvent(kEventEnterFrame);
	_lingo->processEvent(kEventNone);
	// TODO Director 6 - another order

	byte tempo = _frames[_currentFrame]->_tempo;

	if (tempo) {
		if (tempo > 161) {
			// Delay
			_nextFrameTime = g_system->getMillis() + (256 - tempo) * 1000;

			return;
		} else if (tempo <= 60) {
			// FPS
			_nextFrameTime = g_system->getMillis() + (float)tempo / 60 * 1000;
			_currentFrameRate = tempo;
		} else if (tempo >= 136) {
			// TODO Wait for channel tempo - 135
			warning("STUB: tempo >= 136");
		} else if (tempo == 128) {
			// TODO Wait for Click/Key
			warning("STUB: tempo == 128");
		} else if (tempo == 135) {
			// Wait for sound channel 1
			while (_soundManager->isChannelActive(1)) {
				_vm->processEvents();
			}
		} else if (tempo == 134) {
			// Wait for sound channel 2
			while (_soundManager->isChannelActive(2)) {
				_vm->processEvents();
			}
		}
	}

	_nextFrameTime = g_system->getMillis() + 1000.0 / (float)_currentFrameRate;

	if (debugChannelSet(-1, kDebugSlow))
		_nextFrameTime += 1000;

	if (debugChannelSet(-1, kDebugFast))
		_nextFrameTime = g_system->getMillis();
}

Sprite *Score::getSpriteById(uint16 id) {
	if (_currentFrame >= _frames.size() || id >= _frames[_currentFrame]->_sprites.size()) {
		warning("Score::getSpriteById(%d): out of bounds. frame: %d", id, _currentFrame);
		return nullptr;
	}
	if (_frames[_currentFrame]->_sprites[id]) {
		return _frames[_currentFrame]->_sprites[id];
	} else {
		warning("Sprite on frame %d width id %d not found", _currentFrame, id);
		return nullptr;
	}
}

void Score::addZoomBox(ZoomBox *box) {
	_zoomBoxes.push_back(box);
}

void Score::renderZoomBox(bool redraw) {
	if (!_zoomBoxes.size())
		return;

	ZoomBox *box = _zoomBoxes.front();
	uint32 t = g_system->getMillis();

	if (box->nextTime > t)
		return;

	if (redraw) {
		_surface->copyFrom(*_backSurface);
	}

	const int numSteps = 14;
	// We have 15 steps in total, and we have flying rectange
	// from switching 3/4 frames

	int start, end;
	// Determine, how many rectangles and what are their numbers
	if (box->step < 5) {
		start = 1;
		end = box->step;
	} else {
		start = box->step - 4;
		end = MIN(start + 3 - box->step % 2, 8);
	}

	Graphics::MacPlotData pd(_surface, &_vm->_wm->getPatterns(), Graphics::kPatternCheckers, 0, 0, 1, 0);

	for (int i = start; i <= end; i++) {
		Common::Rect r(box->start.left   + (box->end.left   - box->start.left)   * i / 8,
					   box->start.top    + (box->end.top    - box->start.top)    * i / 8,
					   box->start.right  + (box->end.right  - box->start.right)  * i / 8,
					   box->start.bottom + (box->end.bottom - box->start.bottom) * i / 8);

		Graphics::drawLine(r.left,  r.top,    r.right, r.top,    0xffff, Graphics::macDrawPixel, &pd);
		Graphics::drawLine(r.right, r.top,    r.right, r.bottom, 0xffff, Graphics::macDrawPixel, &pd);
		Graphics::drawLine(r.left,  r.bottom, r.right, r.bottom, 0xffff, Graphics::macDrawPixel, &pd);
		Graphics::drawLine(r.left,  r.top,    r.left,  r.bottom, 0xffff, Graphics::macDrawPixel, &pd);
	}

	box->step++;

	if (box->step >= numSteps) {
		_zoomBoxes.remove_at(0);
	}

	box->nextTime = box->startTime + 1000 * box->step * box->delay / 60;

	if (redraw) {
		g_system->copyRectToScreen(_surface->getPixels(), _surface->pitch, 0, 0, _surface->getBounds().width(), _surface->getBounds().height()); // zoomBox
	}
}

} // End of namespace Director
