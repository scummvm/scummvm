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

#include "common/config-manager.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/substream.h"

#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"
#include "image/bmp.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/castmember.h"
#include "director/images.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sound.h"
#include "director/stxt.h"
#include "director/util.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"

namespace Director {

const char *scriptTypes[] = {
	"ScoreScript",
	"CastScript",
	"MovieScript",
	"EventScript"
};

const char *scriptType2str(ScriptType scr) {
	if (scr < 0)
		return "NoneScript";

	if (scr > kMaxScriptType)
		return "<unknown>";

	return scriptTypes[scr];
}

Cast::Cast(Movie *movie, bool isShared) {
	_movie = movie;
	_vm = _movie->getVM();
	_lingo = _vm->getLingo();

	_isShared = isShared;

	_lingoArchive = new LingoArchive(this);

	_movieScriptCount = 0;
	_castArrayStart = _castArrayEnd = 0;

	_castIDoffset = 0;

	_castArchive = nullptr;

	_loadedStxts = nullptr;
	_loadedCast = nullptr;

	_defaultPalette = -1;
}

Cast::~Cast() {
	if (_loadedStxts)
		for (Common::HashMap<int, const Stxt *>::iterator it = _loadedStxts->begin(); it != _loadedStxts->end(); ++it)
			delete it->_value;

	if (_castArchive) {
		_castArchive->close();
		delete _castArchive;
		_castArchive = nullptr;
	}

	if (_loadedCast)
		for (Common::HashMap<int, CastMember *>::iterator it = _loadedCast->begin(); it != _loadedCast->end(); ++it)
			delete it->_value;

	delete _loadedStxts;
	delete _loadedCast;
	delete _lingoArchive;
}

CastMember *Cast::getCastMember(int castId) {
	CastMember *result = nullptr;

	if (_loadedCast && _loadedCast->contains(castId)) {
		result = _loadedCast->getVal(castId);
	}
	return result;
}

CastMember *Cast::getCastMemberByName(const Common::String &name) {
	CastMember *result = nullptr;

	if (_castsNames.contains(name)) {
		result = _loadedCast->getVal(_castsNames[name]);
	}
	return result;
}

CastMember *Cast::getCastMemberByScriptId(int scriptId) {
	CastMember *result = nullptr;
	if (_castsScriptIds.contains(scriptId))
		result = _loadedCast->getVal(_castsScriptIds[scriptId]);
	return result;
}

CastMemberInfo *Cast::getCastMemberInfo(int castId) {
	CastMemberInfo *result = nullptr;

	if (_castsInfo.contains(castId)) {
		result = _castsInfo[castId];
	}
	return result;
}

const Stxt *Cast::getStxt(int castId) {
	const Stxt *result = nullptr;

	if (_loadedStxts->contains(castId)) {
		result = _loadedStxts->getVal(castId);
	}
	return result;
}

Common::Rect Cast::getCastMemberInitialRect(int castId) {
	CastMember *cast = _loadedCast->getVal(castId);

	if (!cast) {
		warning("Cast::getCastMemberInitialRect(%d): empty cast", castId);
		return Common::Rect(0, 0);
	}

	return cast->_initialRect;
}

void Cast::setCastMemberModified(int castId) {
	CastMember *cast = _loadedCast->getVal(castId);

	if (!cast) {
		warning("Cast::setCastMemberModified(%d): empty cast", castId);
		return;
	}

	cast->_modified = 1;
}

Common::String Cast::getString(Common::String str) {
	if (str.size() == 0) {
		return str;
	}

	uint8 f = static_cast<uint8>(str.firstChar());

	if (f == 0) {
		return "";
	}

	//TODO: check if all versions need to cut off the first character.
	if (_vm->getVersion() >= 400) {
		str.deleteChar(0);
	}

	if (str.lastChar() == '\x00') {
		str.deleteLastChar();
	}

	return str;
}

void Cast::setArchive(Archive *archive) {
	_castArchive = archive;

	if (archive->hasResource(MKTAG('M', 'C', 'N', 'M'), 0)) {
		_macName = archive->getName(MKTAG('M', 'C', 'N', 'M'), 0).c_str();
	} else {
		_macName = archive->getFileName();
	}
}

bool Cast::loadArchive() {
	// Palette Information
	Common::Array<uint16> clutList = _castArchive->getResourceIDList(MKTAG('C', 'L', 'U', 'T'));
	if (clutList.size() == 0) {
		debugC(2, kDebugLoading, "CLUT resource not found, using default Mac palette");
	} else {
		for (uint i = 0; i < clutList.size(); i++) {
			Common::SeekableReadStreamEndian *pal = _castArchive->getResource(MKTAG('C', 'L', 'U', 'T'), clutList[i]);

			debugC(2, kDebugLoading, "****** Loading Palette CLUT, #%d", clutList[i]);
			PaletteV4 p = loadPalette(*pal);
			g_director->addPalette(clutList[i], p.palette, p.length);
			delete pal;
		}
	}

	// Configuration Information
	Common::SeekableReadStreamEndian *r = nullptr;
	if (_castArchive->hasResource(MKTAG('V', 'W', 'C', 'F'), -1)) {
		loadConfig(*(r = _castArchive->getFirstResource(MKTAG('V', 'W', 'C', 'F'))));
		delete r;
	} else if (!_isShared) {
		// TODO: Source this from somewhere!
		_movie->_movieRect = Common::Rect(0, 0, 639, 479);
		_movie->_stageColor = 1;
	}

	// Font Directory
	if (_castArchive->hasResource(MKTAG('F', 'O', 'N', 'D'), -1)) {
		debug("Cast::loadArchive(): Movie has fonts. Loading....");

		_vm->_wm->_fontMan->loadFonts(_castArchive->getPathName());
	}

	// CastMember Information Array
	if (_castArchive->hasResource(MKTAG('V', 'W', 'C', 'R'), -1)) {
		_castIDoffset = _castArchive->getResourceIDList(MKTAG('V', 'W', 'C', 'R'))[0];
		loadCastDataVWCR(*(r = _castArchive->getResource(MKTAG('V', 'W', 'C', 'R'), _castIDoffset)));
		delete r;
	}

	// Font Mapping
	if (_castArchive->hasResource(MKTAG('V', 'W', 'F', 'M'), -1)) {
		_vm->_wm->_fontMan->clearFontMapping();

		loadFontMap(*(r = _castArchive->getFirstResource(MKTAG('V', 'W', 'F', 'M'))));
		delete r;
	}

	// Pattern Tiles
	if (_castArchive->hasResource(MKTAG('V', 'W', 'T', 'L'), -1)) {
		debug("STUB: Unhandled VWTL resource.");
	}

	// Time code
	// TODO: Is this a score resource?
	if (_castArchive->hasResource(MKTAG('V', 'W', 't', 'c'), -1)) {
		debug("STUB: Unhandled VWtc resource");
	}

	// Tape Key resource. Perhaps a lookup for labels?
	// TODO: Is this a score resource?
	if (_castArchive->hasResource(MKTAG('V', 'W', 't', 'k'), -1)) {
		debug("STUB: Unhandled VWtk resource");
	}

	// External sound files
	if (_castArchive->hasResource(MKTAG('S', 'T', 'R', ' '), -1)) {
		debug("STUB: Unhandled 'STR ' resource");
	}

	Common::Array<uint16> vwci = _castArchive->getResourceIDList(MKTAG('V', 'W', 'C', 'I'));
	if (vwci.size() > 0) {
		debugC(2, kDebugLoading, "****** Loading %d CastInfos VWCI", vwci.size());

		for (Common::Array<uint16>::iterator iterator = vwci.begin(); iterator != vwci.end(); ++iterator) {
			loadCastInfo(*(r = _castArchive->getResource(MKTAG('V', 'W', 'C', 'I'), *iterator)), *iterator - _castIDoffset);
			delete r;
		}
	}

	Common::Array<uint16> cast = _castArchive->getResourceIDList(MKTAG('C', 'A', 'S', 't'));
	if (!_loadedCast)
		_loadedCast = new Common::HashMap<int, CastMember *>();

	if (cast.size() > 0) {
		debugC(2, kDebugLoading, "****** Loading %d CASt resources", cast.size());

		for (Common::Array<uint16>::iterator iterator = cast.begin(); iterator != cast.end(); ++iterator) {
			Common::SeekableReadStreamEndian *stream = _castArchive->getResource(MKTAG('C', 'A', 'S', 't'), *iterator);
			Resource res = _castArchive->getResourceDetail(MKTAG('C', 'A', 'S', 't'), *iterator);
			loadCastData(*stream, res.castId, &res);
			delete stream;
		}
	}

	// For D4+ we may request to force Lingo scripts and skip precompiled bytecode
	if (_vm->getVersion() >= 400 && !debugChannelSet(-1, kDebugNoBytecode)) {
		// Try to load script context
		Common::Array<uint16> lctx =  _castArchive->getResourceIDList(MKTAG('L','c','t','x'));
		if (lctx.size() > 0) {
			debugC(2, kDebugLoading, "****** Loading %d Lctx resources", lctx.size());

			for (Common::Array<uint16>::iterator iterator = lctx.begin(); iterator != lctx.end(); ++iterator) {
				loadLingoContext(*(r = _castArchive->getResource(MKTAG('L','c','t','x'), *iterator)));
				delete r;
			}
		}
	}

	// PICT resources
	if (_castArchive->hasResource(MKTAG('P', 'I', 'C', 'T'), -1)) {
		debug("STUB: Unhandled 'PICT' resource");
	}

	// Film Loop resources
	if (_castArchive->hasResource(MKTAG('S', 'C', 'V', 'W'), -1)) {
		debug("STUB: Unhandled 'SCVW' resource");
	}

	// External Cast Reference resources
	if (_castArchive->hasResource(MKTAG('S', 'C', 'R', 'F'), -1)) {
		debug("STUB: Unhandled 'SCRF' resource");
	}

	// Score Order List resources
	if (_castArchive->hasResource(MKTAG('S', 'o', 'r', 'd'), -1)) {
		debug("STUB: Unhandled 'Sord' resource");
	}

	// Now process STXTs
	Common::Array<uint16> stxt = _castArchive->getResourceIDList(MKTAG('S','T','X','T'));
	debugC(2, kDebugLoading, "****** Loading %d STXT resources", stxt.size());

	_loadedStxts = new Common::HashMap<int, const Stxt *>();

	for (Common::Array<uint16>::iterator iterator = stxt.begin(); iterator != stxt.end(); ++iterator) {
		_loadedStxts->setVal(*iterator - _castIDoffset,
				 new Stxt(this, *(r = _castArchive->getResource(MKTAG('S','T','X','T'), *iterator))));

		delete r;

		// Try to load movie script, it starts with a comment
		if (_vm->getVersion() <= 300) {
			if (debugChannelSet(-1, kDebugFewFramesOnly))
				warning("Compiling STXT %d", *iterator);

			loadScriptText(*(r = _castArchive->getResource(MKTAG('S','T','X','T'), *iterator)));
			delete r;
		}

	}
	copyCastStxts();

	loadCastChildren();
	loadSoundCasts();

	return true;
}

uint16 humanVersion(uint16 ver) {
	if (ver >= 0x79F)
		return 1201;
	if (ver >= 0x783)
		return 1200;
	if (ver >= 0x782)
		return 1150;
	if (ver >= 0x781)
		return 1100;
	if (ver >= 0x73B)
		return 1000;
	if (ver >= 0x6A4)
		return 850;
	if (ver >= 0x582)
		return 800;
	if (ver >= 0x4C8)
		return 700;
	if (ver >= 0x4C2)
		return 600;
	if (ver >= 0x4B1)
		return 500;
	if (ver >= 0x45D)
		return 404;
	if (ver >= 0x45B)
		return 400;
	if (ver >= 0x405)
		return 310;
	if (ver >= 0x404)
		return 300;
	return 200;
}

void Cast::loadConfig(Common::SeekableReadStreamEndian &stream) {
	debugC(1, kDebugLoading, "****** Loading Config VWCF");

	if (debugChannelSet(5, kDebugLoading))
		stream.hexdump(stream.size());

	uint16 len = stream.readUint16();
	uint16 fileVersion = stream.readUint16(); // TODO: very high fileVersion means protected
	uint16 humanFileVersion = humanVersion(fileVersion);
	Common::Rect movieRect = Movie::readRect(stream);
	if (!_isShared)
		_movie->_movieRect = movieRect;

	_castArrayStart = stream.readUint16();
	_castArrayEnd = stream.readUint16();
	byte currentFrameRate = stream.readByte();
	if (!_isShared) {
		_movie->getScore()->_currentFrameRate = currentFrameRate;
		if (_movie->getScore()->_currentFrameRate == 0)
			_movie->getScore()->_currentFrameRate = 20;
	}

	byte lightswitch = stream.readByte();
	uint16 unk1 = stream.readUint16();
	uint16 commentFont = stream.readUint16();
	uint16 commentSize = stream.readUint16();
	uint16 commentStyle = stream.readUint16();
	uint32 stageColor = _vm->transformColor(stream.readUint16());

	if (!_isShared)
		_movie->_stageColor = stageColor;

	uint16 bitdepth = stream.readUint16();

	// byte color = stream.readByte();	// boolean, color = 1, B/W = 0
	// uint16 stageColorR = stream.readUint16();
	// uint16 stageColorG = stream.readUint16();
	// uint16 stageColorB = stream.readUint16();

	uint16 directorVersion = fileVersion;
	uint16 humanDirectorVersion = humanFileVersion;
	if (humanFileVersion >= 300) {
		for (int i = 0; i < 0x06; i++) {
			stream.readByte();
		}

		directorVersion = stream.readUint16();
		humanDirectorVersion = humanVersion(directorVersion);

		for (int i = 0; i < 0x0a; i++) {
			stream.readByte();
		}

		if (humanDirectorVersion >= 400) {
			for (int i = 0; i < 0x16; i++)
				stream.readByte();

			_defaultPalette = (int16)stream.readUint16();

			for (int i = 0; i < 0x08; i++)
				stream.readByte();
		}
	}

	if (humanDirectorVersion > _vm->getVersion()) {
		if (_vm->getVersion() > 0)
			warning("Movie is from later version v%d", humanDirectorVersion);
		_vm->setVersion(humanDirectorVersion);
	} else if (humanDirectorVersion < _vm->getVersion()) {
		warning("Movie is from earlier version v%d", humanDirectorVersion);
		// Don't change version in case there are other movies, factories,
		// etc., which need features from the later version
	}

	debugC(1, kDebugLoading, "Cast::loadConfig(): len: %d, ver: %d, framerate: %d, light: %d, unk: %d, font: %d, size: %d"
			", style: %d", len, fileVersion, currentFrameRate, lightswitch, unk1, commentFont, commentSize, commentStyle);
	debugC(1, kDebugLoading, "Cast::loadConfig(): stagecolor: %d, depth: %d, directorVer: %d",
			stageColor, bitdepth, directorVersion);
	if (debugChannelSet(1, kDebugLoading))
		movieRect.debugPrint(1, "Cast::loadConfig(): Movie rect: ");
}

void Cast::copyCastStxts() {
	for (Common::HashMap<int, CastMember *>::iterator c = _loadedCast->begin(); c != _loadedCast->end(); ++c) {
		if (c->_value->_type != kCastText && c->_value->_type != kCastButton)
			continue;

		uint stxtid;
		if (_vm->getVersion() >= 400 && c->_value->_children.size() > 0)
			stxtid = c->_value->_children[0].index;
		else
			stxtid = c->_key;

		if (_loadedStxts->getVal(stxtid)) {
			const Stxt *stxt = _loadedStxts->getVal(stxtid);
			TextCastMember *tc = (TextCastMember *)c->_value;

			tc->importStxt(stxt);
			tc->_size = stxt->_size;
		}
	}
}

void Cast::loadCastChildren() {
	debugC(1, kDebugLoading, "****** Preloading sprite palettes and images");

	Cast *sharedCast = _movie ? _movie->getSharedCast() : nullptr;
	Common::HashMap<int, PaletteV4>::iterator p = _vm->getLoadedPalettes().find(0);

	for (Common::HashMap<int, CastMember *>::iterator c = _loadedCast->begin(); c != _loadedCast->end(); ++c) {
		if (!c->_value)
			continue;

		// First, handle palettes
		if (c->_value->_type == kCastPalette) {
			PaletteCastMember *member = ((PaletteCastMember *)c->_value);

			// TODO: Verify how palettes work in >D4 versions
			if (_vm->getVersion() >= 400 && _vm->getVersion() < 500 && member->_children.size() == 1) {
				member->_palette = g_director->getPalette(member->_children[0].index);
			} else if (_vm->getVersion() < 400) {
				// D3 palettes are always kept in this ascending order
				member->_palette = g_director->getPalette((++p)->_value.id);
			} else {
				warning("Cast::loadSpriteChildren(): Expected 1 child for palette cast, got %d", member->_children.size());
			}
			continue;
		}

		if (c->_value->_type != kCastBitmap)
			continue;

		// Then handle bitmaps
		BitmapCastMember *bitmapCast = (BitmapCastMember *)c->_value;
		uint32 tag = bitmapCast->_tag;
		uint16 imgId = c->_key;
		uint16 realId = 0;

		Image::ImageDecoder *img = NULL;
		Common::SeekableReadStream *pic = NULL;

		if (_vm->getVersion() >= 400) {
			if (bitmapCast->_children.size() > 0) {
				imgId = bitmapCast->_children[0].index;
				tag = bitmapCast->_children[0].tag;

				if (_castArchive->hasResource(tag, imgId))
					pic = _castArchive->getResource(tag, imgId);
				else if (sharedCast && sharedCast->getArchive()->hasResource(tag, imgId))
					pic = sharedCast->getArchive()->getResource(tag, imgId);
			}
		} else {
			if (_loadedCast->contains(imgId)) {
				bitmapCast->_tag = tag = ((BitmapCastMember *)_loadedCast->getVal(imgId))->_tag;
				realId = imgId + _castIDoffset;
				pic = _castArchive->getResource(tag, realId);
			} else if (sharedCast && sharedCast->_loadedCast && sharedCast->_loadedCast->contains(imgId)) {
				bitmapCast->_tag = tag = ((BitmapCastMember *)sharedCast->_loadedCast->getVal(imgId))->_tag;
				realId = imgId + sharedCast->_castIDoffset;
				pic = sharedCast->getArchive()->getResource(tag, realId);
			}
		}

		if (pic == NULL) {
			warning("Cast::loadCastChildren(): Bitmap image %d not found", imgId);
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
				if (_vm->getVersion() < 600) {
					img = new BITDDecoder(w, h, bitmapCast->_bitsPerPixel, bitmapCast->_pitch, _vm->getPalette());
				} else {
					img = new Image::BitmapDecoder();
				}
			} else {
				warning("Cast::loadCastChildren(): Bitmap image %d not found", imgId);
			}

			break;

		default:
			warning("Cast::loadCastChildren(): Unknown Bitmap CastMember Tag: [%d] %s", tag, tag2str(tag));
			break;
		}

		if (!img)
			continue;

		img->loadStream(*pic);

		bitmapCast->_img = img;
		const Graphics::Surface *surf = img->getSurface();
		bitmapCast->_size = surf->pitch * surf->h + img->getPaletteColorCount() * 3;

		delete pic;

		debugC(4, kDebugImages, "Cast::loadCastChildren(): Bitmap: id: %d, w: %d, h: %d, flags1: %x, flags2: %x bytes: %x, bpp: %d clut: %x", imgId, w, h, bitmapCast->_flags1, bitmapCast->_flags2, bitmapCast->_bytes, bitmapCast->_bitsPerPixel, bitmapCast->_clut);
	}
}

void Cast::loadSoundCasts() {
	debugC(1, kDebugLoading, "****** Preloading sound casts");

	for (Common::HashMap<int, CastMember *>::iterator c = _loadedCast->begin(); c != _loadedCast->end(); ++c) {
		if (!c->_value)
			continue;

		if (c->_value->_type != kCastSound)
			continue;

		SoundCastMember *soundCast = (SoundCastMember *)c->_value;
		uint32 tag = MKTAG('S', 'N', 'D', ' ');
		uint16 sndId = (uint16)(c->_key + _castIDoffset);

		if (_vm->getVersion() >= 400 && soundCast->_children.size() > 0) {
			sndId = soundCast->_children[0].index;
			tag = soundCast->_children[0].tag;
		}

		Common::SeekableReadStreamEndian *sndData = NULL;

		switch (tag) {
		case MKTAG('S', 'N', 'D', ' '):
			if (_castArchive->hasResource(MKTAG('S', 'N', 'D', ' '), sndId)) {
				debugC(2, kDebugLoading, "****** Loading 'SND ' id: %d", sndId);
				sndData = _castArchive->getResource(MKTAG('S', 'N', 'D', ' '), sndId);
			}
			break;
		case MKTAG('s', 'n', 'd', ' '):
			if (_castArchive->hasResource(MKTAG('s', 'n', 'd', ' '), sndId)) {
				debugC(2, kDebugLoading, "****** Loading 'snd ' id: %d", sndId);
				sndData = _castArchive->getResource(MKTAG('s', 'n', 'd', ' '), sndId);
			}
			break;
		}

		if (sndData != NULL) {
			if (sndData->size() == 0) {
				// audio file is linked, load from the filesystem
				AudioFileDecoder *audio = new AudioFileDecoder(_castsInfo[c->_key]->fileName);
				soundCast->_audio = audio;
			} else {
				SNDDecoder *audio = new SNDDecoder();
				audio->loadStream(*sndData);
				soundCast->_audio = audio;
				soundCast->_size = sndData->size();
			}
			delete sndData;
		}
	}
}

Common::String Cast::getVideoPath(int castId) {
	Common::String res;
	CastMember *cast = _loadedCast->getVal(castId);

	if (cast->_type != kCastDigitalVideo)
		return res;

	DigitalVideoCastMember *digitalVideoCast = (DigitalVideoCastMember *)cast;
	uint32 tag = MKTAG('M', 'o', 'o', 'V');
	uint16 videoId = (uint16)(castId + _castIDoffset);

	if (_vm->getVersion() >= 400 && digitalVideoCast->_children.size() > 0) {
		videoId = digitalVideoCast->_children[0].index;
		tag = digitalVideoCast->_children[0].tag;
	}

	Common::SeekableReadStreamEndian *videoData = NULL;

	switch (tag) {
	case MKTAG('M', 'o', 'o', 'V'):
		if (_castArchive->hasResource(MKTAG('M', 'o', 'o', 'V'), videoId)) {
			debugC(2, kDebugLoading, "****** Loading 'MooV' id: %d", videoId);
			videoData = _castArchive->getResource(MKTAG('M', 'o', 'o', 'V'), videoId);
		}
		break;
	}

	if (videoData == NULL || videoData->size() == 0) {
		// video file is linked, load from the filesystem

		Common::String filename = _castsInfo[castId]->fileName;
		Common::String directory = _castsInfo[castId]->directory;

		res = directory + "\\" + filename;
	} else {
		warning("STUB: Cast::getVideoPath(%d): unsupported non-zero MooV block", castId);
	}
	if (videoData)
		delete videoData;

	return res;
}

PaletteV4 Cast::loadPalette(Common::SeekableReadStreamEndian &stream) {
	uint16 steps = stream.size() / 6;
	uint16 index = (steps * 3) - 1;
	byte *_palette = new byte[index + 1];

	debugC(3, kDebugLoading, "Cast::loadPalette(): %d steps, %d bytes", steps, stream.size());

	if (steps > 256) {
		warning("Cast::loadPalette(): steps > 256: %d", steps);
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

	return PaletteV4(0, _palette, steps);
}

void Cast::loadCastDataVWCR(Common::SeekableReadStreamEndian &stream) {
	debugC(1, kDebugLoading, "****** Loading CastMember rects VWCR. start: %d, end: %d", _castArrayStart, _castArrayEnd);

	_loadedCast = new Common::HashMap<int, CastMember *>();

	for (uint16 id = _castArrayStart; id <= _castArrayEnd; id++) {
		byte size = stream.readByte();
		uint32 tag;
		if (size == 0)
			continue;

		if (debugChannelSet(5, kDebugLoading))
			stream.hexdump(size);

		// these bytes are common but included in cast size
		uint8 castType = stream.readByte();
		size -= 1;
		uint8 flags1 = 0;
		if (size) {
			flags1 = stream.readByte();
			size -= 1;
		}

		int returnPos = stream.pos() + size;
		switch (castType) {
		case kCastBitmap:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) BitmapCastMember", id, numToCastNum(id));
			if (_castArchive->hasResource(MKTAG('B', 'I', 'T', 'D'), id + _castIDoffset))
				tag = MKTAG('B', 'I', 'T', 'D');
			else if (_castArchive->hasResource(MKTAG('D', 'I', 'B', ' '), id + _castIDoffset))
				tag = MKTAG('D', 'I', 'B', ' ');
			else
				error("Cast::loadCastDataVWCR(): non-existent reference to BitmapCastMember");

			_loadedCast->setVal(id, new BitmapCastMember(this, id, stream, tag, _vm->getVersion(), flags1));
			break;
		case kCastText:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) TextCastMember", id, numToCastNum(id));
			_loadedCast->setVal(id, new TextCastMember(this, id, stream, _vm->getVersion(), flags1));
			break;
		case kCastShape:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) ShapeCastMember", id, numToCastNum(id));
			_loadedCast->setVal(id, new ShapeCastMember(this, id, stream, _vm->getVersion()));
			break;
		case kCastButton:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) ButtonCast", id, numToCastNum(id));
			_loadedCast->setVal(id, new TextCastMember(this, id, stream, _vm->getVersion(), flags1, true));
			break;
		case kCastSound:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) SoundCastMember", id, numToCastNum(id));
			_loadedCast->setVal(id, new SoundCastMember(this, id, stream, _vm->getVersion()));
			break;
		case kCastDigitalVideo:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) DigitalVideoCastMember", id, numToCastNum(id));
			_loadedCast->setVal(id, new DigitalVideoCastMember(this, id, stream, _vm->getVersion()));
			break;
		case kCastPalette:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) PaletteCastMember", id, numToCastNum(id));
			_loadedCast->setVal(id, new PaletteCastMember(this, id, stream, _vm->getVersion()));
			break;
		default:
			warning("Cast::loadCastDataVWCR(): Unhandled cast id: %d(%s), type: %d, %d bytes", id, numToCastNum(id), castType, size);
			break;
		}
		stream.seek(returnPos);
	}
}

static void readEditInfo(EditInfo *info, Common::ReadStreamEndian *stream) {
	info->rect = Movie::readRect(*stream);
	info->selStart = stream->readUint32();
	info->selEnd = stream->readUint32();
	info->version = stream->readByte();
	info->rulerFlag = stream->readByte();

	if (debugChannelSet(3, kDebugLoading)) {
		info->rect.debugPrint(0, "EditInfo: ");
		debug("selStart: %d  selEnd: %d  version: %d  rulerFlag: %d", info->selStart,info->selEnd, info->version, info->rulerFlag);
	}
}

void Cast::loadCastData(Common::SeekableReadStreamEndian &stream, uint16 id, Resource *res) {
	// IDs are stored as relative to the start of the cast array.
	id += _castArrayStart;

	// D4+ variant
	if (stream.size() == 0)
		return;

	// TODO: Determine if there really is a minimum size.
	// This value was too small for Shape Casts.
	if (stream.size() < 10) {
		warning("Cast::loadCastData(): CASt data id %d is too small", id);
		return;
	}

	debugC(3, kDebugLoading, "Cast::loadCastData(): CASt: id: %d", id);

	if (debugChannelSet(5, kDebugLoading) && stream.size() < 2048)
		stream.hexdump(stream.size());

	uint32 castSize, castInfoSize, size3, castType, castSizeToRead;
	byte flags1 = 0, unk1 = 0, unk2 = 0, unk3 = 0;

	// D2-3 cast members should be loaded in loadCastDataVWCR
#if 0
	if (_vm->getVersion() < 400) {
		size1 = stream.readUint16();
		sizeToRead = size1 +16; // 16 is for bounding rects
		size2 = stream.readUint32();
		size3 = 0;
		castType = stream.readByte();
		unk1 = stream.readByte();
		unk2 = stream.readByte();
		unk3 = stream.readByte();
	}
#endif

	if (_vm->getVersion() >= 400 && _vm->getVersion() < 500) {
		castSize = stream.readUint16();
		castSizeToRead = castSize;
		castInfoSize = stream.readUint32();
		size3 = 0;

		// these bytes are common but included in cast size
		castType = stream.readByte();
		castSizeToRead -= 1;
		if (castSizeToRead) {
			flags1 = stream.readByte();
			castSizeToRead -= 1;
		}
	} else if (_vm->getVersion() >= 500 && _vm->getVersion() < 600) {
		castType = stream.readUint32();
		size3 = stream.readUint32();
		castInfoSize = stream.readUint32();
		castSize = stream.readUint32();
		if (castType == 1) {
			if (size3 == 0)
				return;
			for (uint32 skip = 0; skip < (castSize - 4) / 4; skip++)
				stream.readUint32();
		}

		castSizeToRead = stream.size();
	} else {
		error("Cast::loadCastData: unsupported Director version (%d)", _vm->getVersion());
	}

	debugC(3, kDebugLoading, "Cast::loadCastData(): CASt: id: %d type: %x castSize: %d castInfoSize: %d (%x) size3: %d unk1: %d unk2: %d unk3: %d",
		id, castType, castSize, castInfoSize, castInfoSize, size3, unk1, unk2, unk3);

	// read the cast member itself

	byte *data = (byte *)calloc(castSizeToRead, 1);
	stream.read(data, castSizeToRead);

	Common::MemoryReadStreamEndian castStream(data, castSizeToRead, stream.isBE());

	switch (castType) {
	case kCastBitmap:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastBitmap (%d children)", res->children.size());
		_loadedCast->setVal(id, new BitmapCastMember(this, id, castStream, res->tag, _vm->getVersion(), flags1));
		break;
	case kCastSound:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastSound (%d children)", res->children.size());
		_loadedCast->setVal(id, new SoundCastMember(this, id, castStream, _vm->getVersion()));
		break;
	case kCastText:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastText (%d children)", res->children.size());
		_loadedCast->setVal(id, new TextCastMember(this, id, castStream, _vm->getVersion(), flags1));
		break;
	case kCastShape:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastShape (%d children)", res->children.size());
		_loadedCast->setVal(id, new ShapeCastMember(this, id, castStream, _vm->getVersion()));
		break;
	case kCastButton:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastButton (%d children)", res->children.size());
		_loadedCast->setVal(id, new TextCastMember(this, id, castStream, _vm->getVersion(), flags1, true));
		break;
	case kCastLingoScript:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastLingoScript");
		_loadedCast->setVal(id, new ScriptCastMember(this, id, castStream, _vm->getVersion()));
		break;
	case kCastRTE:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastRTE (%d children)", res->children.size());
		_loadedCast->setVal(id, new RTECastMember(this, id, castStream, _vm->getVersion()));
		break;
	case kCastDigitalVideo:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastDigitalVideo (%d children)", res->children.size());
		_loadedCast->setVal(id, new DigitalVideoCastMember(this, id, castStream, _vm->getVersion()));
		break;
	case kCastFilmLoop:
		warning("STUB: Cast::loadCastData(): kCastFilmLoop (%d children)", res->children.size());
		castInfoSize = 0;
		break;
	case kCastPalette:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastPalette (%d children)", res->children.size());
		_loadedCast->setVal(id, new PaletteCastMember(this, id, castStream, _vm->getVersion()));
		break;
	case kCastPicture:
		warning("BUILDBOT: STUB: Cast::loadCastData(): kCastPicture (%d children)", res->children.size());
		castInfoSize = 0;
		break;
	case kCastMovie:
		warning("STUB: Cast::loadCastData(): kCastMovie (%d children)", res->children.size());
		castInfoSize = 0;
		break;
	default:
		warning("Cast::loadCastData(): Unhandled cast type: %d [%s] (%d children)", castType, tag2str(castType), res->children.size());
		// also don't try and read the strings... we don't know what this item is.
		castInfoSize = 0;
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

	// read the cast member info

	if (castInfoSize && _vm->getVersion() < 500) {
		loadCastInfo(stream, id);
	}

	if (size3)
		warning("Cast::loadCastData(): size3: %x", size3);
}

struct LingoContextEntry {
	int32 index;
	int16 nextUnused;
	bool unused;

	LingoContextEntry(int32 i, int16 n);
};

LingoContextEntry::LingoContextEntry(int32 i, int16 n)
	: index(i), nextUnused(n), unused(false) {}

void Cast::loadLingoContext(Common::SeekableReadStreamEndian &stream) {
	if (_vm->getVersion() >= 400) {
		debugC(1, kDebugCompile, "Add V4 script context");

		if (debugChannelSet(5, kDebugLoading)) {
			debugC(5, kDebugLoading, "Lctx header:");
			stream.hexdump(0x2a);
		}

		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		int32 itemCount = stream.readSint32();
		/* int32 itemCount2 = */ stream.readSint32();
		uint16 itemsOffset = stream.readUint16();
		/* uint16 entrySize = */ stream.readUint16();
		/* uint32 unk1 = */ stream.readUint32();
		/* uint32 fileType = */ stream.readUint32();
		/* uint32 unk2 = */ stream.readUint32();
		int32 nameTableId = stream.readSint32();
		/* int16 validCount = */ stream.readSint16();
		/* uint16 flags = */ stream.readUint16();
		int16 firstUnused = stream.readSint16();

		Common::SeekableReadStreamEndian *r;
		debugC(2, kDebugLoading, "****** Loading Lnam resource (%d)", nameTableId);
		_lingoArchive->addNamesV4(*(r = _castArchive->getResource(MKTAG('L','n','a','m'), nameTableId)));
		delete r;

		Common::Array<LingoContextEntry> entries;
		stream.seek(itemsOffset);
		for (int16 i = 1; i <= itemCount; i++) {
			if (debugChannelSet(5, kDebugLoading)) {
				debugC(5, kDebugLoading, "Context entry %d:", i);
				stream.hexdump(0xc);
			}

			stream.readUint32();
			int32 index = stream.readSint32();
			/* uint16 entryFlags = */ stream.readUint16();
			int16 nextUnused = stream.readSint16();
			entries.push_back(LingoContextEntry(index, nextUnused));
		}

		// mark unused entries
		int16 nextUnused = firstUnused ;
		while (0 <= nextUnused && nextUnused < (int16)entries.size()) {
			LingoContextEntry &entry = entries[nextUnused];
			entry.unused = true;
			nextUnused = entry.nextUnused;
		}

		// compile scripts
		for (int16 i = 1; i <= (int16)entries.size(); i++) {
			LingoContextEntry &entry = entries[i - 1];
			if (entry.unused && entry.index < 0) {
				debugC(1, kDebugCompile, "Cast::loadLingoContext: Script %d is unused and empty", i);
				continue;
			}
			if (entry.unused) {
				debugC(1, kDebugCompile, "Cast::loadLingoContext: Script %d is unused but not empty", i);
				continue;
			}
			if (entry.index < 0) {
				debugC(1, kDebugCompile, "Cast::loadLingoContext: Script %d is used but empty", i);
				continue;
			}
			_lingoArchive->addCodeV4(*(r = _castArchive->getResource(MKTAG('L', 's', 'c', 'r'), entry.index)), i, _macName);
			delete r;
		}

		// actually define scripts
		for (ScriptContextHash::iterator it = _lingoArchive->lctxContexts.begin(); it != _lingoArchive->lctxContexts.end(); ++it) {
			ScriptContext *script = it->_value;
			if (script->_id >= 0 && !script->isFactory()) {
				if (_lingoArchive->getScriptContext(script->_scriptType, script->_id)) {
					error("Cast::loadLingoContext: Script already defined for type %s, id %d", scriptType2str(script->_scriptType), script->_id);
				}
				_lingoArchive->scriptContexts[script->_scriptType][script->_id] = script;
			}
		}
	} else {
		error("Cast::loadLingoContext: unsuported Director version (%d)", _vm->getVersion());
	}
}

void Cast::loadScriptText(Common::SeekableReadStreamEndian &stream) {
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

	// Check if this is a script. It must start with a comment.
	// See D2 Interactivity Manual pp.46-47 (Ch.2.11. Using a macro)
	if (script.empty() || !script.hasPrefix("--"))
		return;

	if (ConfMan.getBool("dump_scripts"))
		dumpScript(script.c_str(), kMovieScript, _movieScriptCount);

	if (script.contains("\nmenu:") || script.hasPrefix("menu:"))
		return;

	_lingoArchive->addCode(script.c_str(), kMovieScript, _movieScriptCount);

	_movieScriptCount++;
}

void Cast::dumpScript(const char *script, ScriptType type, uint16 id) {
	Common::DumpFile out;
	Common::String buf = dumpScriptName(_macName.c_str(), type, id, "txt");

	if (!out.open(buf, true)) {
		warning("Cast::dumpScript(): Can not open dump file %s", buf.c_str());
		return;
	}

	out.write(script, strlen(script));

	out.flush();
	out.close();
}

void Cast::loadCastInfo(Common::SeekableReadStreamEndian &stream, uint16 id) {
	if (!_loadedCast->contains(id))
		return;

	InfoEntries castInfo = Movie::loadInfoEntries(stream);

	debugCN(4, kDebugLoading, "Cast::loadCastInfo(): str(%d): '", castInfo.strings.size());

	for (uint i = 0; i < castInfo.strings.size(); i++) {
		debugCN(4, kDebugLoading, "%s'", castInfo.strings[i].readString().c_str());
		if (i != castInfo.strings.size() - 1)
			debugCN(4, kDebugLoading, ", '");
	}
	debugC(4, kDebugLoading, "'");

	CastMemberInfo *ci = new CastMemberInfo();
	Common::MemoryReadStreamEndian *entryStream;

	// We have here variable number of strings. Thus, instead of
	// adding tons of ifs, we use this switch()
	switch (castInfo.strings.size()) {
	default:
		warning("Cast::loadCastInfo(): BUILDBOT: extra %d strings", castInfo.strings.size() - 8);
		// fallthrough
	case 8:
		if (castInfo.strings[7].len) {
			entryStream = new Common::MemoryReadStreamEndian(castInfo.strings[7].data, castInfo.strings[7].len, stream.isBE());
			readEditInfo(&ci->textEditInfo, entryStream);
			delete entryStream;
		}
		// fallthrough
	case 7:
		if (castInfo.strings[6].len) {
			entryStream = new Common::MemoryReadStreamEndian(castInfo.strings[6].data, castInfo.strings[6].len, stream.isBE());

			int16 count = entryStream->readUint16();

			for (int16 i = 0; i < count; i++)
				ci->scriptStyle.read(*entryStream);
			delete entryStream;
		}
		// fallthrough
	case 6:
		if (castInfo.strings[5].len) {
			entryStream = new Common::MemoryReadStreamEndian(castInfo.strings[5].data, castInfo.strings[5].len, stream.isBE());
			readEditInfo(&ci->scriptEditInfo, entryStream);
			delete entryStream;
		}
		// fallthrough
	case 5:
		ci->type = castInfo.strings[4].readString();
		// fallthrough
	case 4:
		ci->fileName = castInfo.strings[3].readString();
		// fallthrough
	case 3:
		ci->directory = castInfo.strings[2].readString();
		// fallthrough
	case 2:
		ci->name = castInfo.strings[1].readString();

		if (!ci->name.empty()) {
			_castsNames[ci->name] = id;
		}
		// fallthrough
	case 1:
		ci->script = castInfo.strings[0].readString(false);
		// fallthrough
	case 0:
		break;
	}

	CastMember *member = _loadedCast->getVal(id);
	// For D4+ we may force Lingo scripts
	if (_vm->getVersion() < 400 || debugChannelSet(-1, kDebugNoBytecode)) {
		if (!ci->script.empty()) {
			ScriptType scriptType = kCastScript;
			// the script type here could be wrong!
			if (member->_type == kCastLingoScript) {
				scriptType = ((ScriptCastMember *)member)->_scriptType;
			}

			if (ConfMan.getBool("dump_scripts"))
				dumpScript(ci->script.c_str(), scriptType, id);

			_lingoArchive->addCode(ci->script.c_str(), scriptType, id, ci->name.c_str());
		}
	}

	ci->scriptId = castInfo.scriptId;
	if (ci->scriptId != 0)
		_castsScriptIds[ci->scriptId] = id;

	_castsInfo[id] = ci;
}

void Cast::loadFontMap(Common::SeekableReadStreamEndian &stream) {
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

} // End of namespace Director
