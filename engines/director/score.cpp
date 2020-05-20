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

#include "audio/decoders/raw.h"
#include "engines/util.h"
#include "graphics/primitives.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/maceditabletext.h"
#include "director/cachedmactext.h"
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
	_maskSurface = nullptr;
	_backSurface = nullptr;
	_backSurface2 = nullptr;
	_lingo = _vm->getLingo();
	_soundManager = _vm->getSoundManager();
	_currentMouseDownSpriteId = 0;
	_mouseIsDown = false;
	_lastEventTime = _vm->getMacTicks();
	_lastKeyTime = _lastEventTime;
	_lastClickTime = _lastEventTime;
	_lastRollTime = _lastEventTime;
	_lastTimerReset = _lastEventTime;

	// FIXME: TODO: Check whether the original truely does it
	if (_vm->getVersion() <= 3) {
		_lingo->executeScript(kMovieScript, 0, 0);
	}
	_movieScriptCount = 0;
	_labels = nullptr;
	_font = nullptr;

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

	_framesRan = 0; // used by kDebugFewFramesOnly

	_window = nullptr;
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
		Common::SeekableSubReadStreamEndian *r;
		loadLabels(*(r = archive->getFirstResource(MKTAG('V', 'W', 'L', 'B'))));
		delete r;
	}
}

void Score::loadArchive() {
	Common::Array<uint16> clutList = _movieArchive->getResourceIDList(MKTAG('C', 'L', 'U', 'T'));
	Common::SeekableSubReadStreamEndian *r = nullptr;

	if (clutList.size() > 1)
		warning("More than one palette was found (%d)", clutList.size());

	if (clutList.size() == 0) {
		warning("CLUT resource not found, using default Mac palette");
		_vm->setPalette(-1);
	} else {
		Common::SeekableSubReadStreamEndian *pal = _movieArchive->getResource(MKTAG('C', 'L', 'U', 'T'), clutList[0]);

		debugC(2, kDebugLoading, "****** Loading Palette CLUT, #%d", clutList[0]);
		loadPalette(*pal);

		delete pal;
	}

	// Font Directory
	if (_movieArchive->hasResource(MKTAG('F', 'O', 'N', 'D'), -1)) {
		debug("STUB: Unhandled FOND resource");
	}

	// Score
	if (!_movieArchive->hasResource(MKTAG('V', 'W', 'S', 'C'), -1)) {
		warning("Score::loadArchive(): Wrong movie format. VWSC resource missing");
		return;
	}
	loadFrames(*(r = _movieArchive->getFirstResource(MKTAG('V', 'W', 'S', 'C'))));
	delete r;

	// Configuration Information
	if (_movieArchive->hasResource(MKTAG('V', 'W', 'C', 'F'), -1)) {
		loadConfig(*(r = _movieArchive->getFirstResource(MKTAG('V', 'W', 'C', 'F'))));
		delete r;
	} else {
		// TODO: Source this from somewhere!
		_movieRect = Common::Rect(0, 0, 640, 480);
		_stageColor = 1;
	}

	// Cast Information Array
	if (_movieArchive->hasResource(MKTAG('V', 'W', 'C', 'R'), -1)) {
		_castIDoffset = _movieArchive->getResourceIDList(MKTAG('V', 'W', 'C', 'R'))[0];
		loadCastDataVWCR(*(r = _movieArchive->getResource(MKTAG('V', 'W', 'C', 'R'), _castIDoffset)));
		delete r;
	}

	// Action list
	if (_movieArchive->hasResource(MKTAG('V', 'W', 'A', 'C'), -1)) {
		loadActions(*(r = _movieArchive->getFirstResource(MKTAG('V', 'W', 'A', 'C'))));
		delete r;
	}

	// File Info
	if (_movieArchive->hasResource(MKTAG('V', 'W', 'F', 'I'), -1)) {
		loadFileInfo(*(r = _movieArchive->getFirstResource(MKTAG('V', 'W', 'F', 'I'))));
		delete r;
	}

	// Font Mapping
	if (_movieArchive->hasResource(MKTAG('V', 'W', 'F', 'M'), -1)) {
		_vm->_wm->_fontMan->clearFontMapping();

		loadFontMap(*(r = _movieArchive->getFirstResource(MKTAG('V', 'W', 'F', 'M'))));
		delete r;
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
				loadLingoContext(*(r = _movieArchive->getResource(MKTAG('L','c','t','x'), *iterator)));
				delete r;
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
			loadLingoNames(*(r = _movieArchive->getResource(MKTAG('L','n','a','m'), maxLnam)));
			delete r;
		}
	}

	Common::Array<uint16> vwci = _movieArchive->getResourceIDList(MKTAG('V', 'W', 'C', 'I'));
	if (vwci.size() > 0) {
		debugC(2, kDebugLoading, "****** Loading %d CastInfos VWCI", vwci.size());

		for (Common::Array<uint16>::iterator iterator = vwci.begin(); iterator != vwci.end(); ++iterator) {
			loadCastInfo(*(r = _movieArchive->getResource(MKTAG('V', 'W', 'C', 'I'), *iterator)), *iterator);
			delete r;
		}
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
			delete stream;
		}
	}

	// PICT resources
	if (_movieArchive->hasResource(MKTAG('P', 'I', 'C', 'T'), -1)) {
		debug("STUB: Unhandled 'PICT' resource");
	}

	// Film Loop resources
	if (_movieArchive->hasResource(MKTAG('S', 'C', 'V', 'W'), -1)) {
		debug("STUB: Unhandled 'SCVW' resource");
	}

	setSpriteCasts();
	setSpriteBboxes();
	loadSpriteImages(false);
	loadSpriteSounds(false);

	// Now process STXTs
	Common::Array<uint16> stxt = _movieArchive->getResourceIDList(MKTAG('S','T','X','T'));
	debugC(2, kDebugLoading, "****** Loading %d STXT resources", stxt.size());

	_loadedStxts = new Common::HashMap<int, const Stxt *>();

	for (Common::Array<uint16>::iterator iterator = stxt.begin(); iterator != stxt.end(); ++iterator) {
		_loadedStxts->setVal(*iterator,
				 new Stxt(*(r = _movieArchive->getResource(MKTAG('S','T','X','T'), *iterator))));

		delete r;

		// Try to load movie script, it starts with a comment
		if (_vm->getVersion() <= 3) {
			loadScriptText(*(r = _movieArchive->getResource(MKTAG('S','T','X','T'), *iterator)));
			delete r;
		}

	}
	copyCastStxts();
}

void Score::copyCastStxts() {
	for (Common::HashMap<int, Cast *>::iterator c = _loadedCast->begin(); c != _loadedCast->end(); ++c) {
		if (c->_value->_type != kCastText && c->_value->_type != kCastButton)
			continue;

		uint stxtid;
		if (_vm->getVersion() >= 4 && c->_value->_children.size() > 0)
			stxtid = c->_value->_children[0].index;
		else
			stxtid = c->_key + _castIDoffset;

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
		uint16 realId = 0;

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
					img = new BITDDecoder(w, h, bitmapCast->_bitsPerPixel, bitmapCast->_pitch, _vm->getPalette());
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

		if (!img)
			continue;

		img->loadStream(*pic);

		delete pic;

		bitmapCast->_surface = img->getSurface();
		bitmapCast->_img = img;

		debugC(4, kDebugImages, "Score::loadSpriteImages(): id: %d, w: %d, h: %d, flags: %x, bytes: %x, bpp: %d clut: %x",
			imgId, w, h, bitmapCast->_flags, bitmapCast->_bytes, bitmapCast->_bitsPerPixel, bitmapCast->_clut);
	}
}

void Score::loadSpriteSounds(bool isSharedCast) {
	debugC(1, kDebugLoading, "****** Preloading sprite sounds");

	for (Common::HashMap<int, Cast *>::iterator c = _loadedCast->begin(); c != _loadedCast->end(); ++c) {
		if (!c->_value)
			continue;

		if (c->_value->_type != kCastSound)
			continue;

		SoundCast *soundCast = (SoundCast *)c->_value;
		uint32 tag = MKTAG('s', 'n', 'd', ' ');
		uint16 sndId = (uint16)(c->_key + _castIDoffset);

		if (_vm->getVersion() >= 4 && soundCast->_children.size() > 0) {
			sndId = soundCast->_children[0].index;
			tag = soundCast->_children[0].tag;
		}

		Common::SeekableSubReadStreamEndian *sndData = NULL;

		switch (tag) {
		case MKTAG('s', 'n', 'd', ' '):
			if (_movieArchive->hasResource(MKTAG('s', 'n', 'd', ' '), sndId)) {
				debugC(2, kDebugLoading, "****** Loading 'snd ' id: %d", sndId);
				sndData = _movieArchive->getResource(MKTAG('s', 'n', 'd', ' '), sndId);
			}
			break;
		}

		if (sndData != NULL && soundCast != NULL) {
			SNDDecoder *audio = new SNDDecoder();
			audio->loadStream(*sndData);
			delete sndData;
			soundCast->_audio = audio;
		}
	}
}


Score::~Score() {
	if (_maskSurface && _maskSurface->w)
		_maskSurface->free();

	if (_backSurface && _backSurface->w)
		_backSurface->free();

	if (_backSurface2 && _backSurface2->w)
		_backSurface2->free();

	delete _backSurface;
	delete _backSurface2;
	delete _maskSurface;

	if (_window)
		_vm->_wm->removeWindow(_window);

	for (uint i = 0; i < _frames.size(); i++)
		delete _frames[i];

	if (_loadedStxts)
		for (Common::HashMap<int, const Stxt *>::iterator it = _loadedStxts->begin(); it != _loadedStxts->end(); ++it)
			delete it->_value;

	if (_movieArchive) {
		_movieArchive->close();
		delete _movieArchive;
		_movieArchive = nullptr;
	}

	if (_loadedCast)
		for (Common::HashMap<int, Cast *>::iterator it = _loadedCast->begin(); it != _loadedCast->end(); ++it)
			delete it->_value;

	if (_labels)
		for (Common::SortedArray<Label *>::iterator it = _labels->begin(); it != _labels->end(); ++it)
			delete *it;

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
		debugC(8, kDebugLoading, "++++++++++ score frame %d (frameSize %d) size %d", _frames.size(), frameSize, size);

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
	_stageColor = _vm->transformColor(stream.readUint16());
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
			_loadedCast->setVal(id, new TextCast(stream, _vm->getVersion(), _stageColor));
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
	// Update sprite cache of cast pointers/info
	for (uint16 i = 0; i < _frames.size(); i++) {
		for (uint16 j = 0; j < _frames[i]->_sprites.size(); j++) {
			_frames[i]->_sprites[j]->setCast(_frames[i]->_sprites[j]->_castId);
			debugC(1, kDebugImages, "Score::setSpriteCasts(): Frame: %d Channel: %d type: %d", i, j, _frames[i]->_sprites[j]->_spriteType);
		}
	}
}

void Score::setSpriteBboxes() {
	for (uint16 i = 0; i < _frames.size(); i++) {
		for (uint16 j = 0; j < _frames[i]->_sprites.size(); j++) {
			Sprite *sp = _frames[i]->_sprites[j];

			if (sp->_castId == 0)
				continue;

			CastType castType = sp->_castType;

			switch (castType) {
			case kCastShape:
				sp->_startBbox = Common::Rect(sp->_currentPoint.x,
											  sp->_currentPoint.y,
											  sp->_currentPoint.x + sp->_width,
											  sp->_currentPoint.y + sp->_height);
				break;
			case kCastRTE:
			case kCastText: {
				TextCast *textCast = (TextCast*)sp->_cast;
				int x = sp->_currentPoint.x; // +rectLeft;
				int y = sp->_currentPoint.y; // +rectTop;
				int height = textCast->_initialRect.height(); //_sprites[spriteId]->_height;
				int width;
				Common::Rect *textRect = NULL;

				if (_vm->getVersion() >= 4) {
					// where does textRect come from?
					if (textRect == NULL) {
						width = textCast->_initialRect.right;
					} else {
						width = textRect->width();
					}
				} else {
					width = textCast->_initialRect.width(); //_sprites[spriteId]->_width;
				}

				sp->_startBbox = Common::Rect(x, y, x + width, y + height);
				break;
			}
			case kCastButton: {
				uint16 castId = sp->_castId;

				// This may not be a button cast. It could be a textcast with the
				// channel forcing it to be a checkbox or radio button!
				ButtonCast *button = (ButtonCast *)_vm->getCurrentScore()->_loadedCast->getVal(castId);

				// Sometimes, at least in the D3 Workshop Examples, these buttons are
				// just TextCast. If they are, then we just want to use the spriteType
				// as the button type. If they are full-bown Cast members, then use the
				// actual cast member type.
				int buttonType = sp->_spriteType;
				if (buttonType == kCastMemberSprite) {
					switch (button->_buttonType) {
					case kTypeCheckBox:
						buttonType = kCheckboxSprite;
						break;
					case kTypeButton:
						buttonType = kButtonSprite;
						break;
					case kTypeRadio:
						buttonType = kRadioButtonSprite;
						break;
					}
				}

				uint32 rectLeft = button->_initialRect.left;
				uint32 rectTop = button->_initialRect.top;

				int x = sp->_currentPoint.x;
				int y = sp->_currentPoint.y;

				if (_vm->getVersion() > 3) {
					x += rectLeft;
					y += rectTop;
				}

				int height = button->_initialRect.height();
				int width = button->_initialRect.width() + 3;

				switch (buttonType) {
				case kCheckboxSprite:
					// Magic numbers: checkbox square need to move left about 5px from
					// text and 12px side size (D4)
					sp->_startBbox = Common::Rect(x, y + 2, x + 12, y + 14);
					break;
				case kButtonSprite:
					sp->_startBbox = Common::Rect(x, y, x + width, y + height + 3);
					break;
				case kRadioButtonSprite:
					sp->_startBbox = Common::Rect(x, y + 2, x + 12, y + 14);
					break;
				default:
					warning("Score::setSpriteBboxes: Unknown buttonType");
				}
				break;
			}
			case kCastBitmap: {
				BitmapCast *bc = (BitmapCast *)sp->_cast;

				int32 regX = bc->_regX;
				int32 regY = bc->_regY;
				int32 rectLeft = bc->_initialRect.left;
				int32 rectTop = bc->_initialRect.top;

				int x = sp->_currentPoint.x - regX + rectLeft;
				int y = sp->_currentPoint.y - regY + rectTop;
				int height = sp->_height;
				int width = _vm->getVersion() > 4 ? bc->_initialRect.width() : sp->_width;

				// If one of the dimensions is invalid, invalidate whole thing
				if (width == 0 || height == 0)
					width = height = 0;

				sp->_startBbox = Common::Rect(x, y, x + width, y + height);
				break;
			}
			default:
				warning("Score::setSpriteBboxes(): Unhandled cast type: %d", castType);
			}
			sp->_currentBbox = sp->_startBbox;
			sp->_dirtyBbox = sp->_startBbox;
		}
	}
}

void Score::loadCastData(Common::SeekableSubReadStreamEndian &stream, uint16 id, Resource *res) {
	// IDs are stored as relative to the start of the cast array.
	id += _castArrayStart;

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
	case kCastSound:
		debugC(3, kDebugLoading, "Score::loadCastData(): loading kCastSound (%d children)", res->children.size());
		_loadedCast->setVal(id, new SoundCast(castStream, _vm->getVersion()));
		break;
	case kCastText:
		debugC(3, kDebugLoading, "Score::loadCastData(): loading kCastText (%d children)", res->children.size());
		_loadedCast->setVal(id, new TextCast(castStream, _vm->getVersion(), _stageColor));
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
		_loadedCast->setVal(id, new RTECast(castStream, _vm->getVersion(), _stageColor));
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

		Cast *member = _loadedCast->getVal(id);
		// FIXME. Bytecode disabled by default, requires --debugflags=bytecode for now
		if (_vm->getVersion() >= 4 && castType == kCastLingoScript && debugChannelSet(-1, kDebugBytecode)) {
			// Try and load the compiled Lingo script associated with this cast
			uint scriptId = ((ScriptCast *)member)->_id - 1;
			if (scriptId < _castScriptIds.size()) {
				int resourceId = _castScriptIds[scriptId];
				Common::SeekableSubReadStreamEndian *r;
				_lingo->addCodeV4(*(r = _movieArchive->getResource(MKTAG('L', 's', 'c', 'r'), resourceId)), ((ScriptCast *)member)->_scriptType, id);
				delete r;
			} else {
				warning("Score::loadCastData(): Lingo context missing a resource entry for script %d referenced in cast %d", scriptId, id);
			}
		} else {
			if (!ci->script.empty()) {
				if (member->_type == kCastLingoScript) {
					// the script type here could be wrong!
					if (ConfMan.getBool("dump_scripts"))
						dumpScript(ci->script.c_str(), ((ScriptCast *)member)->_scriptType, id);

					_lingo->addCode(ci->script.c_str(), ((ScriptCast *)member)->_scriptType, id);
				} else {
					warning("Score::loadCastData(): Wrong cast type: %d", member->_type);
				}
			}
		}

		_castsInfo[id] = ci;
	}

	if (size3)
		warning("Score::loadCastData(): size3: %x", size3);
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

	bool *scriptRefs = (bool *)calloc(_actions.size() + 1, sizeof(int));

	// Now let's scan which scripts are actually referenced
	for (uint i = 0; i < _frames.size(); i++) {
		if (_frames[i]->_actionId <= _actions.size())
			scriptRefs[_frames[i]->_actionId] = true;

		for (uint16 j = 0; j <= _frames[i]->_numChannels; j++) {
			if (_frames[i]->_sprites[j]->_scriptId <= _actions.size())
				scriptRefs[_frames[i]->_sprites[j]->_scriptId] = true;
		}
	}

	Common::HashMap<uint16, Common::String>::iterator j;

	if (ConfMan.getBool("dump_scripts"))
		for (j = _actions.begin(); j != _actions.end(); ++j) {
			if (!j->_value.empty())
				dumpScript(j->_value.c_str(), kFrameScript, j->_key);
		}

	for (j = _actions.begin(); j != _actions.end(); ++j) {
		if (!scriptRefs[j->_key]) {
			warning("Action id %d is not referenced, the code is:\n-----\n%s\n------", j->_key, j->_value.c_str());
			// continue;
		}
		if (!j->_value.empty()) {
			_lingo->addCode(j->_value.c_str(), kFrameScript, j->_key);

			processImmediateFrameScript(j->_value, j->_key);
		}
	}

	free(scriptRefs);
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

	castStrings.clear();

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

	//TODO: check if all versions need to cut off the first character.
	if (_vm->getVersion() > 3) {
		str.deleteChar(0);
	}

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

		uint start = i == 1 ? entries[i] + 1 : entries[i]; // Skip first byte which is string length

		for (uint j = start; j < entries[i + 1]; j++)
			if (data[j] == '\r')
				entryString += '\n';
			else if (data[j] >= 0x20)
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

	_window = _vm->_wm->addWindow(false, false, true);
	_window->disableBorder();
	_window->resize(_movieRect.width(), _movieRect.height());

	_surface = _window->getWindowSurface();
	_maskSurface = new Graphics::ManagedSurface;
	_backSurface = new Graphics::ManagedSurface;
	_backSurface2 = new Graphics::ManagedSurface;

	_maskSurface->create(_movieRect.width(), _movieRect.height());
	_backSurface->create(_movieRect.width(), _movieRect.height());
	_backSurface2->create(_movieRect.width(), _movieRect.height());

	_sprites.resize(_frames[0]->_sprites.size());

	if (_vm->_backSurface.w > 0) {
		// Persist screen between the movies
		// TODO: this is a workaround until the rendering pipeline is reworked

		_backSurface2->copyFrom(g_director->_backSurface);
		_surface->copyFrom(g_director->_backSurface);

		_vm->_backSurface.free();
	}

	_vm->_backSurface.create(_movieRect.width(), _movieRect.height());

	_vm->_wm->setScreen(_surface);

	_surface->clear(_stageColor);

	_currentFrame = 0;
	_stopPlay = false;
	_nextFrameTime = 0;

	_sprites = _frames[_currentFrame]->_sprites;
	_lingo->processEvent(kEventStartMovie);

	_sprites = _frames[_currentFrame]->_sprites;
	renderFrame(_currentFrame, true);

	if (_frames.size() <= 1) {	// We added one empty sprite
		warning("Score::startLoop(): Movie has no frames");
		_stopPlay = true;
	}

	while (!_stopPlay) {
		if (_currentFrame >= _frames.size()) {
			if (debugChannelSet(-1, kDebugNoLoop))
				break;

			_currentFrame = 0;
		}

		update();

		if (_currentFrame < _frames.size())
			_vm->processEvents();

		if (debugChannelSet(-1, kDebugFewFramesOnly) && _framesRan > 9) {
			warning("Score::startLoop(): exiting due to debug few frames only");
			break;
		}
	}

	_lingo->processEvent(kEventStopMovie);
}

void Score::update() {
	if (g_system->getMillis() < _nextFrameTime) {
		renderZoomBox(true);

		if (!_vm->_newMovieStarted)
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

	if (_frames[_currentFrame]->_transType != 0 && !_vm->_newMovieStarted)	// Store screen, so we could draw a nice transition
		_backSurface2->copyFrom(*_surface);

	_vm->_newMovieStarted = false;

	// _surface->clear(_stageColor);
	// _surface->copyFrom(*_trailSurface);

	_lingo->executeImmediateScripts(_frames[_currentFrame]);

	if (_vm->getVersion() >= 6) {
		_lingo->processEvent(kEventBeginSprite);
		// TODO Director 6 step: send beginSprite event to any sprites whose span begin in the upcoming frame
		_lingo->processEvent(kEventPrepareFrame);
		// TODO: Director 6 step: send prepareFrame event to all sprites and the script channel in upcoming frame
	}

	renderFrame(_currentFrame);
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
			_vm->waitForClick();
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

	if (debugChannelSet(-1, kDebugFewFramesOnly))
		_framesRan++;
}

void Score::renderFrame(uint16 frameId, bool forceUpdate, bool updateStageOnly) {
	_maskSurface->clear(0);

	Frame *currentFrame = _frames[frameId];

	for (uint16 i = 0; i < _sprites.size(); i++) {
		Sprite *currentSprite = _sprites[i];
		Sprite *nextSprite;

		if (currentSprite->_puppet)
			nextSprite = currentSprite;
		else
			nextSprite = currentFrame->_sprites[i];

		bool needsUpdate = (currentSprite->_currentBbox != nextSprite->_currentBbox || currentSprite->_currentBbox != currentSprite->_dirtyBbox);

		if (needsUpdate || forceUpdate)
			unrenderSprite(i);

		_sprites[i] = nextSprite;
	}

	for (uint i = 0; i < _sprites.size(); i++)
		renderSprite(i);

	if (!updateStageOnly) {
		renderZoomBox();

		_vm->_wm->draw();

		if (currentFrame->_transType != 0)
			// TODO Handle changing area case
			currentFrame->playTransition(this);

		if (currentFrame->_sound1 != 0 || currentFrame->_sound2 != 0) {
			playSoundChannel(frameId);
		}

		if (_vm->getCurrentScore()->haveZoomBox())
			_backSurface->copyFrom(*_surface);
	}

	g_system->copyRectToScreen(_surface->getPixels(), _surface->pitch, 0, 0, _surface->getBounds().width(), _surface->getBounds().height());
}

void Score::unrenderSprite(uint16 spriteId) {
	Sprite *currentSprite = _sprites[spriteId];

	if (!currentSprite->_trails) {
		_maskSurface->fillRect(currentSprite->_currentBbox, 1);
		_surface->fillRect(currentSprite->_currentBbox, _stageColor);
	}

	currentSprite->_currentBbox = currentSprite->_dirtyBbox;
}

void Score::renderSprite(uint16 id) {
	Sprite *sprite = _sprites[id];

	if (!sprite)
		return;

	CastType castType = sprite->_castType;

	_maskSurface->fillRect(sprite->_currentBbox, 1);

	if (castType == kCastTypeNull)
		return;

	debugC(1, kDebugImages, "Score::renderFrame(): channel: %d,  castType: %d", id, castType);
	// this needs precedence to be hit first... D3 does something really tricky
	// with cast IDs for shapes. I don't like this implementation 100% as the
	// 'cast' above might not actually hit a member and be null?
	if (castType == kCastShape) {
		renderShape(id);
	} else if (castType == kCastText || castType == kCastRTE) {
		renderText(id, NULL);
	} else if (castType == kCastButton) {
		renderButton(id);
	} else {
		if (!sprite->_cast || sprite->_cast->_type != kCastBitmap) {
			warning("Score::renderFrame(): No cast ID for sprite %d", id);
			return;
		}
		if (sprite->_cast->_surface == nullptr) {
			warning("Score::renderFrame(): No cast surface for sprite %d", id);
			return;
		}

		renderBitmap(id);
	}
}

void Score::renderShape(uint16 spriteId) {
	Sprite *sp = _sprites[spriteId];

	InkType ink = sp->_ink;
	byte spriteType = sp->_spriteType;
	byte foreColor = sp->_foreColor;
	byte backColor = sp->_backColor;
	int lineSize = sp->_thickness & 0x3;

	if (_vm->getVersion() >= 3 && spriteType == kCastMemberSprite) {
		if (!sp->_cast) {
			warning("Score::renderShape(): kCastMemberSprite has no cast defined");
			return;
		}
		switch (sp->_cast->_type) {
		case kCastShape:
			{
				ShapeCast *sc = (ShapeCast *)sp->_cast;
				switch (sc->_shapeType) {
				case kShapeRectangle:
					spriteType = sc->_fillType ? kRectangleSprite : kOutlinedRectangleSprite;
					break;
				case kShapeRoundRect:
					spriteType = sc->_fillType ? kRoundedRectangleSprite : kOutlinedRoundedRectangleSprite;
					break;
				case kShapeOval:
					spriteType = sc->_fillType ? kOvalSprite : kOutlinedOvalSprite;
					break;
				case kShapeLine:
					spriteType = sc->_lineDirection == 6 ? kLineBottomTopSprite : kLineTopBottomSprite;
					break;
				default:
					break;
				}
				if (_vm->getVersion() > 3) {
					foreColor = sc->_fgCol;
					backColor = sc->_bgCol;
					lineSize = sc->_lineThickness;
					ink = sc->_ink;
				}
				// shapes should be rendered with transparency by default
				if (ink == kInkTypeCopy) {
					ink = kInkTypeTransparent;
				}
			}
			break;
		default:
			warning("Score::renderShape(): Unhandled cast type: %d", sp->_cast->_type);
			break;
		}
	}

	// for outlined shapes, line thickness of 1 means invisible.
	lineSize -= 1;

	Common::Rect shapeRect = sp->_currentBbox;

	Graphics::ManagedSurface tmpSurface, maskSurface;
	tmpSurface.create(shapeRect.width(), shapeRect.height(), Graphics::PixelFormat::createFormatCLUT8());
	tmpSurface.clear(backColor);

	maskSurface.create(shapeRect.width(), shapeRect.height(), Graphics::PixelFormat::createFormatCLUT8());
	maskSurface.clear(0);

	// Draw fill
	Common::Rect fillRect((int)shapeRect.width(), (int)shapeRect.height());
	Graphics::MacPlotData plotFill(&tmpSurface, &maskSurface, &_vm->getPatterns(), sp->getPattern(), -shapeRect.left, -shapeRect.top, 1, backColor);
	switch (spriteType) {
	case kRectangleSprite:
		Graphics::drawFilledRect(fillRect, foreColor, Graphics::macDrawPixel, &plotFill);
		break;
	case kRoundedRectangleSprite:
		Graphics::drawRoundRect(fillRect, 12, foreColor, true, Graphics::macDrawPixel, &plotFill);
		break;
	case kOvalSprite:
		Graphics::drawEllipse(fillRect.left, fillRect.top, fillRect.right, fillRect.bottom, foreColor, true, Graphics::macDrawPixel, &plotFill);
		break;
	case kCastMemberSprite: 		// Face kit D3
		Graphics::drawFilledRect(fillRect, foreColor, Graphics::macDrawPixel, &plotFill);
		break;
	default:
		break;
	}

	// Draw stroke
	Common::Rect strokeRect(MAX((int)shapeRect.width() - lineSize, 0), MAX((int)shapeRect.height() - lineSize, 0));
	Graphics::MacPlotData plotStroke(&tmpSurface, &maskSurface, &_vm->getPatterns(), 1, -shapeRect.left, -shapeRect.top, lineSize, backColor);
	switch (spriteType) {
	case kLineTopBottomSprite:
		Graphics::drawLine(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, foreColor, Graphics::macDrawPixel, &plotStroke);
		break;
	case kLineBottomTopSprite:
		Graphics::drawLine(strokeRect.left, strokeRect.bottom, strokeRect.right, strokeRect.top, foreColor, Graphics::macDrawPixel, &plotStroke);
		break;
	case kRectangleSprite:
		// fall through
	case kOutlinedRectangleSprite:	// this is actually a mouse-over shape? I don't think it's a real button.
		Graphics::drawRect(strokeRect, foreColor, Graphics::macDrawPixel, &plotStroke);
		//tmpSurface.fillRect(Common::Rect(shapeRect.width(), shapeRect.height()), (_vm->getCurrentScore()->_currentMouseDownSpriteId == spriteId ? 0 : 0xff));
		break;
	case kRoundedRectangleSprite:
		// fall through
	case kOutlinedRoundedRectangleSprite:
		Graphics::drawRoundRect(strokeRect, 12, foreColor, false, Graphics::macDrawPixel, &plotStroke);
		break;
	case kOvalSprite:
		// fall through
	case kOutlinedOvalSprite:
		Graphics::drawEllipse(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, foreColor, false, Graphics::macDrawPixel, &plotStroke);
		break;
	default:
		break;
	}

	inkBasedBlit(&maskSurface, tmpSurface, ink, shapeRect, spriteId);
}


void Score::renderButton(uint16 spriteId) {
	uint16 castId = _sprites[spriteId]->_castId;

	// This may not be a button cast. It could be a textcast with the channel forcing it
	// to be a checkbox or radio button!
	Cast *member = _vm->getCastMember(castId);
	if (!member) {
		warning("renderButton: unknown cast id %d", castId);
	} else if (member->_type != kCastButton) {
		warning("renderButton: cast id %d not of type kCastButton", castId);
		return;
	}
	ButtonCast *button = (ButtonCast *)member;

	// Sometimes, at least in the D3 Workshop Examples, these buttons are just TextCast.
	// If they are, then we just want to use the spriteType as the button type.
	// If they are full-bown Cast members, then use the actual cast member type.
	int buttonType = _sprites[spriteId]->_spriteType;
	if (buttonType == kCastMemberSprite) {
		switch (button->_buttonType) {
		case kTypeCheckBox:
			buttonType = kCheckboxSprite;
			break;
		case kTypeButton:
			buttonType = kButtonSprite;
			break;
		case kTypeRadio:
			buttonType = kRadioButtonSprite;
			break;
		}
	}

	bool invert = spriteId == _vm->getCurrentScore()->_currentMouseDownSpriteId;

	// TODO: review all cases to confirm if we should use text height.
	// height = textRect.height();

	Common::Rect _rect = _sprites[spriteId]->_currentBbox;
	int16 x = _rect.left;
	int16 y = _rect.top;

	Common::Rect textRect(0, 0, _rect.width(), _rect.height());

	// WORKAROUND, HACK
	// Because we're not drawing text with transparency
	// We swap drawing depending on whether the button is
	// inverted or not, to prevent destroying the border
	if (!invert)
		renderText(spriteId, &textRect);

	Graphics::MacPlotData plotStroke(_surface, nullptr, &_vm->getPatterns(), 1, 0, 0, 1, 0);

	switch (buttonType) {
	case kCheckboxSprite:
		_surface->frameRect(_rect, 0);
		break;
	case kButtonSprite: {
			Graphics::MacPlotData pd(_surface, nullptr, &_vm->getMacWindowManager()->getPatterns(), Graphics::MacGUIConstants::kPatternSolid, 0, 0, 1, invert ? Graphics::kColorBlack : Graphics::kColorWhite);

			Graphics::drawRoundRect(_rect, 4, 0, invert, Graphics::macDrawPixel, &pd);
		}
		break;
	case kRadioButtonSprite:
		Graphics::drawEllipse(x, y + 2, x + 11, y + 13, 0, false, Graphics::macDrawPixel, &plotStroke);
		break;
	default:
		warning("renderButton: Unknown buttonType");
		break;
	}

	if (invert)
		renderText(spriteId, &textRect);
}

void Score::renderText(uint16 spriteId, Common::Rect *textRect) {
	TextCast *textCast = (TextCast*)_sprites[spriteId]->_cast;
	if (textCast == nullptr) {
		warning("Score::renderText(): TextCast #%d is a nullptr", spriteId);
		return;
	}

	Score *score = _vm->getCurrentScore();
	Sprite *sprite = _sprites[spriteId];

	Common::Rect bbox = sprite->_currentBbox;
	int width = bbox.width();
	int height = bbox.height();
	int x = bbox.left;
	int y = bbox.top;

	if (_vm->getCurrentScore()->_fontMap.contains(textCast->_fontId)) {
		// We need to make sure that the Shared Cast fonts have been loaded in?
		// might need a mapping table here of our own.
		// textCast->fontId = _vm->_wm->_fontMan->getFontIdByName(_vm->getCurrentScore()->_fontMap[textCast->fontId]);
	}

	if (width == 0 || height == 0) {
		warning("Score::renderText(): Requested to draw on an empty surface: %d x %d", width, height);
		return;
	}

	if (sprite->_editable) {
		if (!textCast->_widget) {
			warning("Creating MacEditableText with '%s'", toPrintable(textCast->_ftext).c_str());
			textCast->_widget = new Graphics::MacEditableText(score->_window, x, y, width, height, g_director->_wm, textCast->_ftext, new Graphics::MacFont(), 0, 255, width);
			warning("Finished creating MacEditableText");
		}

		textCast->_widget->draw();

		InkType ink = sprite->_ink;

		// if (spriteId == score->_currentMouseDownSpriteId)
		// 	ink = kInkTypeReverse;

		inkBasedBlit(nullptr, textCast->_widget->getSurface()->rawSurface(), ink, Common::Rect(x, y, x + width, y + height), spriteId);

		return;
	}

	debugC(3, kDebugText, "renderText: sprite: %d x: %d y: %d w: %d h: %d fontId: '%d' text: '%s'", spriteId, x, y, width, height, textCast->_fontId, Common::toPrintable(textCast->_ftext).c_str());

	uint16 boxShadow = (uint16)textCast->_boxShadow;
	uint16 borderSize = (uint16)textCast->_borderSize;
	if (textRect != NULL)
		borderSize = 0;
	uint16 padding = (uint16)textCast->_gutterSize;
	uint16 textShadow = (uint16)textCast->_textShadow;

	//uint32 rectLeft = textCast->initialRect.left;
	//uint32 rectTop = textCast->initialRect.top;

	textCast->_cachedMacText->clip(width);
	const Graphics::ManagedSurface *textSurface = textCast->_cachedMacText->getSurface();

	if (!textSurface)
		return;

	height = textSurface->h;
	if (textRect != NULL) {
		// TODO: this offset could be due to incorrect fonts loaded!
		textRect->bottom = height + textCast->_cachedMacText->getLineCount();
	}

	uint16 textX = 0, textY = 0;

	if (textRect == NULL) {
		if (borderSize > 0) {
			if (_vm->getVersion() <= 3) {
				height += (borderSize * 2);
				textX += (borderSize + 2);
			} else {
				height += borderSize;
				textX += (borderSize + 1);
			}
			textY += borderSize;
		} else {
			x += 1;
		}

		if (padding > 0) {
			width += padding * 2;
			height += padding;
			textY += padding / 2;
		}

		if (textCast->_textAlign == kTextAlignRight)
			textX -= 1;

		if (textShadow > 0)
			textX--;
	} else {
		x++;
		if (width % 2 != 0)
			x++;

		if (sprite->_spriteType != kCastMemberSprite) {
			y += 2;
			switch (sprite->_spriteType) {
			case kCheckboxSprite:
				textX += 16;
				break;
			case kRadioButtonSprite:
				textX += 17;
				break;
			default:
				break;
			}
		} else {
			ButtonType buttonType = ((ButtonCast*)textCast)->_buttonType;
			switch (buttonType) {
			case kTypeCheckBox:
				width += 4;
				textX += 16;
				break;
			case kTypeRadio:
				width += 4;
				textX += 17;
				break;
			case kTypeButton:
				width += 4;
				y += 2;
				break;
			default:
				warning("Score::renderText(): Expected button but got unexpected button type: %d", buttonType);
				y += 2;
				break;
			}
		}
	}

	switch (textCast->_textAlign) {
	case kTextAlignLeft:
	default:
		break;
	case kTextAlignCenter:
		textX = (width / 2) - (textSurface->w / 2) + (padding / 2) + borderSize;
		break;
	case kTextAlignRight:
		textX = width - (textSurface->w + 1) + (borderSize * 2) - (textShadow * 2) - (padding);
		break;
	}

	Graphics::ManagedSurface textWithFeatures(width + (borderSize * 2) + boxShadow + textShadow, height + borderSize + boxShadow + textShadow);
	textWithFeatures.fillRect(Common::Rect(textWithFeatures.w, textWithFeatures.h), score->getStageColor());

	if (textRect == NULL && boxShadow > 0) {
		textWithFeatures.fillRect(Common::Rect(boxShadow, boxShadow, textWithFeatures.w + boxShadow, textWithFeatures.h), 0);
	}

	if (textRect == NULL && borderSize != kSizeNone) {
		for (int bb = 0; bb < borderSize; bb++) {
			Common::Rect borderRect(bb, bb, textWithFeatures.w - bb - boxShadow - textShadow, textWithFeatures.h - bb - boxShadow - textShadow);
			textWithFeatures.fillRect(borderRect, 0xff);
			textWithFeatures.frameRect(borderRect, 0);
		}
	}

	if (textShadow > 0)
		textWithFeatures.transBlitFrom(textSurface->rawSurface(), Common::Point(textX + textShadow, textY + textShadow), 0xff);

	textWithFeatures.transBlitFrom(textSurface->rawSurface(), Common::Point(textX, textY), 0xff);

	InkType ink = sprite->_ink;

	// if (spriteId == score->_currentMouseDownSpriteId)
	// 	ink = kInkTypeReverse;

	inkBasedBlit(nullptr, textWithFeatures, ink, Common::Rect(x, y, x + width, y + height), spriteId);
}

void Score::renderBitmap(uint16 spriteId) {
	InkType ink;
	Sprite *sprite = _sprites[spriteId];

	// if (spriteId == _vm->getCurrentScore()->_currentMouseDownSpriteId)
	// 	ink = kInkTypeReverse;
	// else
		ink = sprite->_ink;

	BitmapCast *bc = (BitmapCast *)sprite->_cast;
	Common::Rect drawRect = sprite->_currentBbox;

	inkBasedBlit(nullptr, *(bc->_surface), ink, drawRect, spriteId);
}

void Score::inkBasedBlit(Graphics::ManagedSurface *maskSurface, const Graphics::Surface &spriteSurface, InkType ink, Common::Rect drawRect, uint spriteId) {
	// drawRect could be bigger than the spriteSurface. Clip it
	Common::Rect t(spriteSurface.w, spriteSurface.h);
	t.moveTo(drawRect.left, drawRect.top);
	bool nullMask = false;

	// combine the given mask with the maskSurface
	if (!maskSurface) {
		nullMask = true;
		maskSurface = new Graphics::ManagedSurface;
		maskSurface->create(spriteSurface.w, spriteSurface.h, Graphics::PixelFormat::createFormatCLUT8());
		maskSurface->clear(0);
	}

	drawRect.clip(Common::Rect(_maskSurface->w, _maskSurface->h));


	if (drawRect.isEmpty()) {
		warning("Score::inkBasedBlit(): empty drawRect");
		return;
	}

	maskSurface->blitFrom(*_maskSurface, drawRect, Common::Point(0, 0));

	drawRect.clip(t);

	switch (ink) {
	case kInkTypeCopy:
		if (maskSurface)
			_surface->transBlitFrom(spriteSurface, Common::Point(drawRect.left, drawRect.top), *maskSurface);
		else
			_surface->blitFrom(spriteSurface, Common::Point(drawRect.left, drawRect.top));
		break;
	case kInkTypeTransparent:
		// FIXME: is it always white (last entry in pallette)?
		_surface->transBlitFrom(spriteSurface, Common::Point(drawRect.left, drawRect.top), _vm->getPaletteColorCount() - 1);
		break;
	case kInkTypeBackgndTrans:
		drawBackgndTransSprite(spriteSurface, drawRect, spriteId);
		break;
	case kInkTypeMatte:
		drawMatteSprite(spriteSurface, drawRect);
		break;
	case kInkTypeGhost:
		drawGhostSprite(spriteSurface, drawRect);
		break;
	case kInkTypeReverse:
		drawReverseSprite(spriteSurface, drawRect, spriteId);
		break;
	default:
		warning("Score::inkBasedBlit(): Unhandled ink type %d", ink);
		_surface->blitFrom(spriteSurface, Common::Point(drawRect.left, drawRect.top));
		break;
	}

	if (nullMask)
		delete maskSurface;
}

void Score::drawBackgndTransSprite(const Graphics::Surface &sprite, Common::Rect &drawRect, int spriteId) {
	byte skipColor = _sprites[spriteId]->_backColor;
	Common::Rect srcRect(sprite.w, sprite.h);

	if (!_surface->clip(srcRect, drawRect))
		return; // Out of screen

	for (int ii = 0; ii < srcRect.height(); ii++) {
		const byte *src = (const byte *)sprite.getBasePtr(srcRect.left, srcRect.top + ii);
		byte *dst = (byte *)_surface->getBasePtr(drawRect.left, drawRect.top + ii);

		for (int j = 0; j < srcRect.width(); j++) {
			if (*src != skipColor)
				*dst = *src;

			src++;
			dst++;
		}
	}
}

void Score::drawGhostSprite(const Graphics::Surface &sprite, Common::Rect &drawRect) {
	Common::Rect srcRect(sprite.w, sprite.h);

	if (!_surface->clip(srcRect, drawRect))
		return; // Out of screen

	uint8 skipColor = _vm->getPaletteColorCount() - 1;
	for (int ii = 0; ii < srcRect.height(); ii++) {
		const byte *src = (const byte *)sprite.getBasePtr(srcRect.left, srcRect.top + ii);
		byte *dst = (byte *)_surface->getBasePtr(drawRect.left, drawRect.top + ii);

		for (int j = 0; j < srcRect.width(); j++) {
			if ((getSpriteIDFromPos(Common::Point(drawRect.left + j, drawRect.top + ii)) != 0) && (*src != skipColor))
				*dst = (_vm->getPaletteColorCount() - 1) - *src; // Oposite color

			src++;
			dst++;
		}
	}
}

void Score::drawReverseSprite(const Graphics::Surface &sprite, Common::Rect &drawRect, uint16 spriteId) {
	Common::Rect srcRect(sprite.w, sprite.h);

	if (!_surface->clip(srcRect, drawRect))
		return; // Out of screen

	uint8 skipColor = _vm->getPaletteColorCount() - 1;
	for (int ii = 0; ii < srcRect.height(); ii++) {
		const byte *src = (const byte *)sprite.getBasePtr(srcRect.left, srcRect.top + ii);
		byte *dst = (byte *)_surface->getBasePtr(drawRect.left, drawRect.top + ii);
		byte srcColor = *src;

		for (int j = 0; j < srcRect.width(); j++) {
			if (_sprites[spriteId]->_cast->_type == kCastShape)
				srcColor = 0x0;
			else
				srcColor = *src;
			uint16 targetSprite = getSpriteIDFromPos(Common::Point(drawRect.left + j, drawRect.top + ii));
			if ((targetSprite != 0)) {
				// TODO: This entire reverse colour attempt needs a lot more testing on
				// a lot more colour depths.
				if (srcColor != skipColor) {
					if (_sprites[targetSprite]->_cast->_type != kCastBitmap) {
						if (*dst == 0 || *dst == 255) {
							*dst = _vm->transformColor(*dst);
						} else if (srcColor == 255 || srcColor == 0) {
							*dst = _vm->transformColor(*dst - 40);
						} else {
							*dst = _vm->transformColor(*src - 40);
						}
					} else {
						if (*dst == 0 && _vm->getVersion() == 3 &&
							((BitmapCast*)_sprites[spriteId]->_cast)->_bitsPerPixel > 1) {
							*dst = _vm->transformColor(*src - 40);
						} else {
							*dst ^= _vm->transformColor(srcColor);
						}
					}
				}
			} else if (srcColor != skipColor) {
				*dst = _vm->transformColor(srcColor);
			}
			src++;
			dst++;
		}
	}
}

void Score::drawMatteSprite(const Graphics::Surface &sprite, Common::Rect &drawRect) {
	// Like background trans, but all white pixels NOT ENCLOSED by coloured pixels are transparent
	Graphics::Surface tmp;
	tmp.copyFrom(sprite);
	Common::Rect srcRect(sprite.w, sprite.h);

	if (!_surface->clip(srcRect, drawRect))
		return; // Out of screen

	// Searching white color in the corners
	int whiteColor = -1;

	for (int y = 0; y < tmp.h; y++) {
		for (int x = 0; x < tmp.w; x++) {
			byte color = *(byte *)tmp.getBasePtr(x, y);

			if (_vm->getPalette()[color * 3 + 0] == 0xff &&
				_vm->getPalette()[color * 3 + 1] == 0xff &&
				_vm->getPalette()[color * 3 + 2] == 0xff) {
				whiteColor = color;
				break;
			}
		}
	}

	if (whiteColor == -1) {
		debugC(1, kDebugImages, "Score::drawMatteSprite(): No white color for Matte image");

		for (int yy = 0; yy < srcRect.height(); yy++) {
			const byte *src = (const byte *)tmp.getBasePtr(srcRect.left, srcRect.top + yy);
			byte *dst = (byte *)_surface->getBasePtr(drawRect.left, drawRect.top + yy);

			for (int xx = 0; xx < drawRect.width(); xx++, src++, dst++)
				*dst = *src;
		}
	} else {
		Graphics::FloodFill ff(&tmp, whiteColor, 0, true);

		for (int yy = 0; yy < tmp.h; yy++) {
			ff.addSeed(0, yy);
			ff.addSeed(tmp.w - 1, yy);
		}

		for (int xx = 0; xx < tmp.w; xx++) {
			ff.addSeed(xx, 0);
			ff.addSeed(xx, tmp.h - 1);
		}
		ff.fillMask();

		for (int yy = 0; yy < srcRect.height(); yy++) {
			const byte *src = (const byte *)tmp.getBasePtr(srcRect.left, srcRect.top + yy);
			const byte *mask = (const byte *)ff.getMask()->getBasePtr(srcRect.left, srcRect.top + yy);
			byte *dst = (byte *)_surface->getBasePtr(drawRect.left, drawRect.top + yy);

			for (int xx = 0; xx < srcRect.width(); xx++, src++, dst++, mask++)
				if (*mask == 0)
					*dst = *src;
		}
	}

	tmp.free();
}

uint16 Score::getSpriteIDFromPos(Common::Point pos) {
	for (int i = _sprites.size() - 1; i >= 0; i--)
		if (_sprites[i]->_currentBbox.contains(pos))
			return i;

	return 0;
}

bool Score::checkSpriteIntersection(uint16 spriteId, Common::Point pos) {
	if (_sprites[spriteId]->_currentBbox.contains(pos))
		return true;

	return false;
}

Common::Rect *Score::getSpriteRect(uint16 spriteId) {
	return &_sprites[spriteId]->_currentBbox;
}

Sprite *Score::getSpriteById(uint16 id) {
	if (id >= _sprites.size()) {
		warning("Score::getSpriteById(%d): out of bounds. frame: %d", id, _currentFrame);
		return nullptr;
	}
	if (_sprites[id]) {
		return _sprites[id];
	} else {
		warning("Sprite on frame %d width id %d not found", _currentFrame, id);
		return nullptr;
	}
}

void Score::playSoundChannel(uint16 frameId) {
	Frame *frame = _frames[frameId];
	debug(0, "STUB: playSoundChannel(), Sound1 %d Sound2 %d", frame->_sound1, frame->_sound2);
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

	Graphics::MacPlotData pd(_surface, nullptr, &_vm->_wm->getPatterns(), Graphics::kPatternCheckers, 0, 0, 1, 0);

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

Cast *Score::getCastMember(int castId) {
	Cast *result = nullptr;

	if (_loadedCast->contains(castId)) {
		result = _loadedCast->getVal(castId);
	}
	return result;
}

} // End of namespace Director
