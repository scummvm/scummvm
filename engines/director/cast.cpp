/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
	"EventScript",
	"TestScript"
};

const char *scriptType2str(ScriptType scr) {
	if (scr < 0)
		return "NoneScript";

	if (scr > kMaxScriptType)
		return "<unknown>";

	return scriptTypes[scr];
}

Cast::Cast(Movie *movie, uint16 castLibID, bool isShared) {
	_movie = movie;
	_vm = _movie->getVM();
	_lingo = _vm->getLingo();

	_castLibID = castLibID;
	_isShared = isShared;

	_lingoArchive = new LingoArchive(this);

	_castArrayStart = _castArrayEnd = 0;

	_castIDoffset = 0;

	_castArchive = nullptr;
	_version = 0;
	_platform = Common::kPlatformMacintosh;

	_stageColor = 0;

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

		g_director->_openResFiles.erase(_castArchive->getPathName());

		delete _castArchive;
		_castArchive = nullptr;
	}

	if (_loadedCast)
		for (Common::HashMap<int, CastMember *>::iterator it = _loadedCast->begin(); it != _loadedCast->end(); ++it)
			if (it->_value) {
				delete it->_value;
				it->_value = nullptr;
			}

	for (Common::HashMap<uint16, CastMemberInfo *>::iterator it = _castsInfo.begin(); it != _castsInfo.end(); ++it)
		delete it->_value;

	for (FontXPlatformMap::iterator it = _fontXPlatformMap.begin(); it != _fontXPlatformMap.end(); ++it)
		delete it->_value;

	for (FontMap::iterator it = _fontMap.begin(); it != _fontMap.end(); ++it)
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

void Cast::releaseCastMemberWidget() {
	if (_loadedCast)
		for (Common::HashMap<int, CastMember *>::iterator it = _loadedCast->begin(); it != _loadedCast->end(); ++it)
			it->_value->releaseWidget();
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

int Cast::getCastSize() {
	return _loadedCast->size();
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

	cast->setModified(true);
}

CastMember *Cast::setCastMember(CastMemberID castId, CastMember *cast) {
	if (_loadedCast->contains(castId.member)) {
		_loadedCast->erase(castId.member);
	}

	_loadedCast->setVal(castId.member, cast);
	return cast;
}

bool Cast::eraseCastMember(CastMemberID castId) {
	if (_loadedCast->contains(castId.member)) {
		_loadedCast->erase(castId.member);
		return true;
	}

	return false;
}

void Cast::setArchive(Archive *archive) {
	_castArchive = archive;

	if (archive->hasResource(MKTAG('M', 'C', 'N', 'M'), 0)) {
		_macName = archive->getName(MKTAG('M', 'C', 'N', 'M'), 0).c_str();
	} else {
		_macName = archive->getFileName();
	}

	// Register the resfile so that Cursor::readFromResource can find it
	g_director->_openResFiles.setVal(archive->getPathName(), archive);
}

void Cast::loadArchive() {
	loadConfig();
	loadCast();
}

bool Cast::loadConfig() {
	if (!_castArchive->hasResource(MKTAG('V', 'W', 'C', 'F'), -1)) {
		warning("Cast::loadConfig(): Wrong format. VWCF resource missing");
		return false;
	}

	Common::SeekableReadStreamEndian *stream = nullptr;

	stream = _castArchive->getMovieResourceIfPresent(MKTAG('V', 'W', 'C', 'F'));

	debugC(1, kDebugLoading, "****** Loading Config VWCF");

	if (debugChannelSet(5, kDebugLoading))
		stream->hexdump(stream->size());

	// Post D3 Check. The version field was added in D3 and later versions.
	// This is used to check some fields which differed pre-d7 and post-d7
	if (stream->size() > 36) {
		stream->seek(36);					// Seek to version
		_version = stream->readSint16();	// Read version for post-D3
		stream->seek(0);					// Seek to start of stream
	}

	uint16 len = stream->readUint16();
	uint16 fileVersion = stream->readUint16(); // TODO: very high fileVersion means protected

	if (stream->size() <= 36)
		_version = fileVersion;				// Checking if we have already read the version

	uint humanVer = humanVersion(_version);

	Common::Rect checkRect = Movie::readRect(*stream);
	if (!g_director->_fixStageSize)
		_movieRect = checkRect;
	else
		_movieRect = g_director->_fixStageRect;

	if (!_isShared)
		_movie->_movieRect = _movieRect;

	_castArrayStart = stream->readUint16();
	_castArrayEnd = stream->readUint16();

	// v3 and below use this, override for v4 and over
	// actual framerates are, on average: { 3.75, 4, 4.35, 4.65, 5, 5.5, 6, 6.6, 7.5, 8.5, 10, 12, 20, 30, 60 }
	Common::Array<int> frameRates = { 3, 4, 4, 4, 5, 5, 6, 6, 7, 8, 10, 12, 15, 20, 30, 60 };
	byte readRate = stream->readByte();
	int16 currentFrameRate;
	if (readRate <= 0xF) {
		currentFrameRate = frameRates[readRate];
	} else {
		switch (readRate) {
			// rate when set via the tempo channel
			// these rates are the actual framerates
			case 0x10:
				// defaults to 15 fps on D2 and D3. On D4 it shows as 120 fps
				currentFrameRate = 15;
				break;
			case 212:
				currentFrameRate = 1;
				break;
			case 242:
				currentFrameRate = 2;
				break;
			case 252:
				currentFrameRate = 3;
				break;
			default:
				warning("BUILDBOT: Cast::loadConfig: unhandled framerate: %i", readRate);
				currentFrameRate = readRate;
		}
	}

	byte lightswitch = stream->readByte();

	int16 unk1 = stream->readSint16();

	// Warning for post-D7 movies (unk1 is stageColorG and stageColorB post-D7)
	if (humanVer >= 700)
		warning("STUB: Cast::loadConfig: 16 bit unk1 read instead of two 8 bit stageColorG and stageColorB. Read value: %04x", unk1);

	uint16 commentFont = stream->readUint16();
	uint16 commentSize = stream->readUint16();
	uint16 commentStyle = stream->readUint16();
	_stageColor = stream->readUint16();

	// Warning for post-D7 movies (stageColor is isStageColorRGB and stageColorR post-D7)
	if (humanVer >= 700)
		warning("STUB: Cast::loadConfig: 16 bit stageColor read instead of two 8 bit isStageColorRGB and stageColorR. Read value: %04x", _stageColor);

	if (!_isShared)
		_movie->_stageColor = _vm->transformColor(_stageColor);

	uint16 bitdepth = stream->readUint16();

	// byte color = stream.readByte();	// boolean, color = 1, B/W = 0
	// uint16 stageColorR = stream.readUint16();
	// uint16 stageColorG = stream.readUint16();
	// uint16 stageColorB = stream.readUint16();

	debugC(1, kDebugLoading, "Cast::loadConfig(): len: %d, fileVersion: %d, light: %d, unk: %d, font: %d, size: %d"
			", style: %d", len, fileVersion, lightswitch, unk1, commentFont, commentSize, commentStyle);
	debugC(1, kDebugLoading, "Cast::loadConfig(): stagecolor: %d, depth: %d",
			_stageColor, bitdepth);
	if (debugChannelSet(1, kDebugLoading))
		_movieRect.debugPrint(1, "Cast::loadConfig(): Movie rect: ");

	// Fields required for checksum calculation
	uint8 field17 = 0, field18 = 0;
	int16 field21 = 0;
	int32 field19 = 0, field22 = 0, field23 = 0;

	// D3 fields - Macromedia did not increment the fileVersion from D2 to D3
	// so we just have to check if there are more bytes to read.
	if (stream->pos() < stream->size()) {

		//reading these fields for the sake of checksum calculation
		field17 = stream->readByte();
		field18 = stream->readByte();
		field19 = stream->readSint32();

		/* _version = */ stream->readUint16();

		field21 = stream->readSint16();
		field22 = stream->readSint32();
		field23 = stream->readSint32();

		debugC(1, kDebugLoading, "Cast::loadConfig(): directorVersion: %d", _version);
	}

	if (_version >= kFileVer400) {
		int32 field24 = stream->readSint32();
		int8 field25 = stream->readSByte();
		/* int8 field26 = */ stream->readSByte();

		currentFrameRate = stream->readSint16();
		uint16 platform = stream->readUint16();
		_platform = platformFromID(platform);

		int16 protection = stream->readSint16();
		/* int32 field29 = */ stream->readSint32();
		uint32 checksum = stream->readUint32();

		//Calculation and verification of checksum
		uint32 check = len + 1;
		check *= fileVersion + 2;
		check /= checkRect.top + 3;
		check *= checkRect.left + 4;
		check /= checkRect.bottom + 5;
		check *= checkRect.right + 6;
		check -= _castArrayStart + 7;
		check *= _castArrayEnd + 8;
		check -= (int8)readRate + 9;
		check -= lightswitch + 10;

		if (humanVer < 700)
			check += unk1 + 11;
		else
			warning("STUB: skipped using stageColorG, stageColorB for post-D7 movie in checksum calulation");

		check *= commentFont + 12;
		check += commentSize + 13;

		if (humanVer < 800)
			check *= (uint8)((commentStyle >> 8) & 0xFF) + 14;
		else
			check *= commentStyle + 14;

		if (humanVer < 700)
			check += _stageColor + 15;
		else
			check += (uint8)(_stageColor & 0xFF) + 15;	// Taking lower 8 bits to take into account stageColorR


		check += bitdepth + 16;
		check += field17 + 17;
		check *= field18 + 18;
		check += field19 + 19;
		check *= _version + 20;
		check += field21 + 21;
		check += field22 + 22;
		check += field23 + 23;
		check += field24 + 24;
		check *= field25 + 25;
		check += currentFrameRate + 26;
		check *= platform + 27;
		check *= (protection * 0xE06) + 0xFFF450000;
		check ^= MKTAG('r', 'a', 'l', 'f');

		if (check != checksum)
			warning("BUILDBOT: The checksum for this VWCF resource is incorrect. Got %04x, but expected %04x", check, checksum);

		_defaultPalette = stream->readSint16();
		// In this header value, the first builtin palette starts at 0 and
		// continues down into negative numbers.
		// For frames, 0 is used to represent an absence of a palette change,
		// with the builtin palettes starting from -1.
		if (_defaultPalette <= 0)
			_defaultPalette -= 1;
		for (int i = 0; i < 0x08; i++) {
			stream->readByte();
		}
		debugC(1, kDebugLoading, "Cast::loadConfig(): platform: %s, defaultPalette: %d", getPlatformAbbrev(_platform), _defaultPalette);
	}

	if (!_isShared) {
		debugC(1, kDebugLoading, "Cast::loadConfig(): currentFrameRate: %d", currentFrameRate);
		_movie->getScore()->_currentFrameRate = currentFrameRate;
	}

	if (humanVer > _vm->getVersion()) {
		if (_vm->getVersion() > 0)
			warning("Movie is from later version v%d", humanVer);
		_vm->setVersion(humanVer);
	}

	delete stream;
	return true;
}

void Cast::loadCast() {
	// Palette Information
	Common::Array<uint16> clutList = _castArchive->getResourceIDList(MKTAG('C', 'L', 'U', 'T'));
	if (clutList.size() == 0) {
		debugC(2, kDebugLoading, "CLUT resource not found, using default Mac palette");
	} else {
		for (uint i = 0; i < clutList.size(); i++) {
			Common::SeekableReadStreamEndian *pal = _castArchive->getResource(MKTAG('C', 'L', 'U', 'T'), clutList[i]);

			debugC(2, kDebugLoading, "****** Loading Palette CLUT, #%d", clutList[i]);
			PaletteV4 p = loadPalette(*pal);

			// for D2, we are using palette cast member id to resolve palette Id, so we are using lowest 1 bit to represent cast id. see Also loadCastChildren
			if (_version < kFileVer300)
				g_director->addPalette(clutList[i] & 0xff, p.palette, p.length);
			else
				g_director->addPalette(clutList[i], p.palette, p.length);

			delete pal;
		}
	}

	Common::SeekableReadStreamEndian *r = nullptr;

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
	if ((r = _castArchive->getMovieResourceIfPresent(MKTAG('V', 'W', 'F', 'M'))) != nullptr) {
		loadFontMap(*r);
		delete r;
	}

	// Cross-Platform Font Mapping
	if ((r = _castArchive->getMovieResourceIfPresent(MKTAG('F', 'X', 'm', 'p'))) != nullptr) {
		loadFXmp(*r);
		delete r;
	}

	// Font Mapping V4
	if ((r = _castArchive->getMovieResourceIfPresent(MKTAG('F', 'm', 'a', 'p'))) != nullptr) {
		loadFontMapV4(*r);
		delete r;
	}

	// Pattern Tiles
	if ((r = _castArchive->getMovieResourceIfPresent(MKTAG('V', 'W', 'T', 'L'))) != nullptr) {
		loadVWTL(*r);
		delete r;
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
	if ((r = _castArchive->getMovieResourceIfPresent(MKTAG('S', 'T', 'R', ' '))) != nullptr) {
		loadExternalSound(*r);
		delete r;
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
	if (_version >= kFileVer400 && !debugChannelSet(-1, kDebugNoBytecode)) {
		// Try to load script context
		if ((r = _castArchive->getMovieResourceIfPresent(MKTAG('L', 'c', 't', 'x'))) != nullptr) {
			loadLingoContext(*r);
			delete r;
		}
	}

	// PICT resources
	if (_castArchive->hasResource(MKTAG('P', 'I', 'C', 'T'), -1)) {
		debug("STUB: Unhandled 'PICT' resource");
	}

	// External Cast Reference resources
	if (_castArchive->hasResource(MKTAG('S', 'C', 'R', 'F'), -1)) {
		debug("STUB: Unhandled 'SCRF' resource");
	}

	// Score Order List resources
	if ((r = _castArchive->getMovieResourceIfPresent(MKTAG('S', 'o', 'r', 'd'))) != nullptr) {
		loadSord(*r);
		delete r;
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
		if (_version < kFileVer400) {
			if (debugChannelSet(-1, kDebugFewFramesOnly))
				warning("Compiling STXT %d", *iterator);

			loadScriptText(*(r = _castArchive->getResource(MKTAG('S','T','X','T'), *iterator)), *iterator - _castIDoffset);
			delete r;
		}

	}

	loadCastMemberData();
}

void Cast::loadStxtData(int key, TextCastMember *member) {
	uint stxtid;
	if (_version >= kFileVer400 && member->_children.size() > 0)
		stxtid = member->_children[0].index;
	else
		stxtid = key;

	if (_loadedStxts->getVal(stxtid)) {
		const Stxt *stxt = _loadedStxts->getVal(stxtid);
		member->importStxt(stxt);
		member->_size = stxt->_size;
	}
}

void Cast::loadPaletteData(PaletteCastMember *member, Common::HashMap<int, PaletteV4>::iterator p) {
	// TODO: Verify how palettes work in >D4 versions
	if (_version >= kFileVer400 && _version < kFileVer500 && member->_children.size() == 1) {
		member->_palette = g_director->getPalette(member->_children[0].index);
	} else if (_version >= kFileVer300 && _version < kFileVer400) {
		// D3 palettes are always kept in this ascending order
		member->_palette = g_director->getPalette((++p)->_value.id);
	} else if (_version < kFileVer300) {
		// for D2, we shall use the castId to get the palette
		member->_palette = g_director->getPalette(member->getID());
	} else {
		warning("Cast::loadPaletteData(): Expected 1 child for palette cast, got %d", member->_children.size());
	}
}

void Cast::loadFilmLoopData(FilmLoopCastMember *member) {
	if (_version >= kFileVer400 && _version < kFileVer500) {
		if (member->_children.size() == 1) {
			uint16 filmLoopId = member->_children[0].index;
			uint32 tag = member->_children[0].tag;
			if (_castArchive->hasResource(tag, filmLoopId)) {
				Common::SeekableReadStreamEndian *loop = _castArchive->getResource(tag, filmLoopId);
				debugC(2, kDebugLoading, "****** Loading '%s' id: %d, %d bytes", tag2str(tag), filmLoopId, (int)loop->size());
				member->loadFilmLoopData(*loop);
			} else {
				warning("Cast::loadFilmLoopData(): Film loop not found");
			}
		} else {
			warning("Cast::loadFilmLoopData(): Expected 1 child for film loop cast, got %d", member->_children.size());
		}
	} else {
		warning("STUB: Cast::loadFilmLoopData(): Film loops not supported for version %d", _version);
	}
}

void Cast::loadBitmapData(int key, BitmapCastMember *bitmapCast) {
	uint32 tag = bitmapCast->_tag;
	uint16 imgId = key;
	uint16 realId = 0;
	Cast *sharedCast = _movie ? _movie->getSharedCast() : nullptr;

	Image::ImageDecoder *img = nullptr;
	Common::SeekableReadStream *pic = nullptr;

	if (_version >= kFileVer400) {
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

	if (pic == nullptr) {
		warning("Cast::loadBitmapData(): Bitmap image %d not found", imgId);
		return;
	}

	int w = bitmapCast->_initialRect.width();
	int h = bitmapCast->_initialRect.height();

	switch (tag) {
	case MKTAG('D', 'I', 'B', ' '):
		debugC(2, kDebugLoading, "****** Loading 'DIB ' id: %d (%d), %d bytes", imgId, realId, (int)pic->size());
		img = new DIBDecoder();
		break;

	case MKTAG('B', 'I', 'T', 'D'):
		debugC(2, kDebugLoading, "****** Loading 'BITD' id: %d (%d), %d bytes", imgId, realId, (int)pic->size());

		if (w > 0 && h > 0) {
			if (_version < kFileVer600) {
				img = new BITDDecoder(w, h, bitmapCast->_bitsPerPixel, bitmapCast->_pitch, _vm->getPalette(), _version);
			} else {
				img = new Image::BitmapDecoder();
			}
		} else {
			warning("Cast::loadBitmapData(): Bitmap image %d not found", imgId);
		}

		break;

	default:
		warning("Cast::loadBitmapData(): Unknown Bitmap CastMember Tag: [%d] %s", tag, tag2str(tag));
		break;
	}

	if (!img)
		return;

	img->loadStream(*pic);

	bitmapCast->_img = img;
	const Graphics::Surface *surf = img->getSurface();
	bitmapCast->_size = surf->pitch * surf->h + img->getPaletteColorCount() * 3;

	delete pic;

	debugC(4, kDebugImages, "Cast::loadBitmapData(): Bitmap: id: %d, w: %d, h: %d, flags1: %x, flags2: %x bytes: %x, bpp: %d clut: %x", imgId, w, h, bitmapCast->_flags1, bitmapCast->_flags2, bitmapCast->_bytes, bitmapCast->_bitsPerPixel, bitmapCast->_clut);
}

void Cast::loadSoundData(int key, SoundCastMember *soundCast) {
	uint32 tag = MKTAG('S', 'N', 'D', ' ');
	uint16 sndId = (uint16)(key + _castIDoffset);

	if (_version >= kFileVer400 && soundCast->_children.size() > 0) {
		sndId = soundCast->_children[0].index;
		tag = soundCast->_children[0].tag;
	}

	Common::SeekableReadStreamEndian *sndData = nullptr;

	if (!_castArchive->hasResource(tag, sndId)) {
		if (_castArchive->hasResource(MKTAG('s', 'n', 'd', ' '), sndId))
			tag = MKTAG('s', 'n', 'd', ' ');
	}

	if (_castArchive->hasResource(tag, sndId)) {
		debugC(2, kDebugLoading, "****** Loading '%s' id: %d", tag2str(tag), sndId);
		sndData = _castArchive->getResource(tag, sndId);
	}

	if (sndData != nullptr) {
		if (sndData->size() == 0) {
			// audio file is linked, load from the filesystem
			Common::String filename = _castsInfo[key]->fileName;

			if (!_castsInfo[key]->directory.empty())
				filename = _castsInfo[key]->directory + g_director->_dirSeparator + _castsInfo[key]->fileName;

			AudioFileDecoder *audio = new AudioFileDecoder(filename);
			soundCast->_audio = audio;
		} else {
			SNDDecoder *audio = new SNDDecoder();
			audio->loadStream(*sndData);
			soundCast->_audio = audio;
			soundCast->_size = sndData->size();
			if (_version < kFileVer400) {
				// The looping flag wasn't added to sound cast members until D4.
				// In older versions, always loop sounds that contain a loop start and end.
				soundCast->_looping = audio->hasLoopBounds();
			}
		}
		delete sndData;
	}
}

void Cast::loadCastMemberData() {
	debugC(1, kDebugLoading, "****** Loading casts data: sprite palettes, images, filmloops, sounds and texts.");

	Common::HashMap<int, PaletteV4>::iterator p = _vm->getLoadedPalettes().find(0);

	for (Common::HashMap<int, CastMember *>::iterator c = _loadedCast->begin(); c != _loadedCast->end(); ++c) {
		if (!c->_value)
			continue;

		switch (c->_value->_type){
			case kCastPalette:
				loadPaletteData((PaletteCastMember *)c->_value, p);
				break;
			case kCastFilmLoop:
				loadFilmLoopData((FilmLoopCastMember *)c->_value);
				break;
			case kCastBitmap:
				loadBitmapData(c->_key, (BitmapCastMember *)c->_value);
				break;
			case kCastSound:
				loadSoundData(c->_key, (SoundCastMember *)c->_value);
				break;
			case kCastText:
			case kCastButton:
				loadStxtData(c->_key, (TextCastMember *)c->_value);
			default:
				break;
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

	if (_version >= kFileVer400 && digitalVideoCast->_children.size() > 0) {
		videoId = digitalVideoCast->_children[0].index;
		tag = digitalVideoCast->_children[0].tag;
	}

	Common::SeekableReadStreamEndian *videoData = nullptr;

	switch (tag) {
	case MKTAG('M', 'o', 'o', 'V'):
		if (_castArchive->hasResource(MKTAG('M', 'o', 'o', 'V'), videoId)) {
			debugC(2, kDebugLoading, "****** Loading 'MooV' id: %d", videoId);
			videoData = _castArchive->getResource(MKTAG('M', 'o', 'o', 'V'), videoId);
		}
		break;
	}

	if (videoData == nullptr || videoData->size() == 0) {
		// video file is linked, load from the filesystem

		Common::String filename = _castsInfo[castId]->fileName;
		Common::String directory = _castsInfo[castId]->directory;

		res = directory + g_director->_dirSeparator + filename;
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

	debugC(3, kDebugLoading, "Cast::loadPalette(): %d steps, %d bytes", steps, (int)stream.size());

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
			else {
				warning("Cast::loadCastDataVWCR(): non-existent reference to BitmapCastMember");
				break;
			}

			_loadedCast->setVal(id, new BitmapCastMember(this, id, stream, tag, _version, flags1));
			break;
		case kCastText:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) TextCastMember", id, numToCastNum(id));
			_loadedCast->setVal(id, new TextCastMember(this, id, stream, _version, flags1));
			break;
		case kCastShape:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) ShapeCastMember", id, numToCastNum(id));
			_loadedCast->setVal(id, new ShapeCastMember(this, id, stream, _version));
			break;
		case kCastButton:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) ButtonCast", id, numToCastNum(id));
			_loadedCast->setVal(id, new TextCastMember(this, id, stream, _version, flags1, true));
			break;
		case kCastSound:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) SoundCastMember", id, numToCastNum(id));
			_loadedCast->setVal(id, new SoundCastMember(this, id, stream, _version));
			break;
		case kCastDigitalVideo:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) DigitalVideoCastMember", id, numToCastNum(id));
			_loadedCast->setVal(id, new DigitalVideoCastMember(this, id, stream, _version));
			break;
		case kCastPalette:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) PaletteCastMember", id, numToCastNum(id));
			_loadedCast->setVal(id, new PaletteCastMember(this, id, stream, _version));
			break;
		default:
			warning("Cast::loadCastDataVWCR(): Unhandled cast id: %d(%s), type: %d, %d bytes", id, numToCastNum(id), castType, size);
			break;
		}
		stream.seek(returnPos);
	}
}

void Cast::loadExternalSound(Common::SeekableReadStreamEndian &stream) {
	Common::String str = stream.readString();
	str.trim();
	debugC(1, kDebugLoading, "****** Loading External Sound File %s", str.c_str());

	Common::String resPath = g_director->getCurrentPath() + str;

	if (!g_director->_openResFiles.contains(resPath)) {
		MacArchive *resFile = new MacArchive();

		if (resFile->openFile(resPath)) {
			g_director->_openResFiles.setVal(resPath, resFile);
		} else {
			delete resFile;
		}
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
	if (_version < kFileVer400) {
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

	if (_version >= kFileVer400 && _version < kFileVer500) {
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
	} else if (_version >= kFileVer500 && _version < kFileVer600) {
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
		error("Cast::loadCastData: unsupported Director version (%d)", _version);
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
		_loadedCast->setVal(id, new BitmapCastMember(this, id, castStream, res->tag, _version, flags1));
		break;
	case kCastSound:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastSound (%d children)", res->children.size());
		_loadedCast->setVal(id, new SoundCastMember(this, id, castStream, _version));
		break;
	case kCastText:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastText (%d children)", res->children.size());
		_loadedCast->setVal(id, new TextCastMember(this, id, castStream, _version, flags1));
		break;
	case kCastShape:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastShape (%d children)", res->children.size());
		_loadedCast->setVal(id, new ShapeCastMember(this, id, castStream, _version));
		break;
	case kCastButton:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastButton (%d children)", res->children.size());
		_loadedCast->setVal(id, new TextCastMember(this, id, castStream, _version, flags1, true));
		break;
	case kCastLingoScript:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastLingoScript");
		_loadedCast->setVal(id, new ScriptCastMember(this, id, castStream, _version));
		break;
	case kCastRTE:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastRTE (%d children)", res->children.size());
		_loadedCast->setVal(id, new RTECastMember(this, id, castStream, _version));
		break;
	case kCastDigitalVideo:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastDigitalVideo (%d children)", res->children.size());
		_loadedCast->setVal(id, new DigitalVideoCastMember(this, id, castStream, _version));
		break;
	case kCastFilmLoop:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastFilmLoop (%d children)", res->children.size());
		_loadedCast->setVal(id, new FilmLoopCastMember(this, id, castStream, _version));
		break;
	case kCastPalette:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastPalette (%d children)", res->children.size());
		_loadedCast->setVal(id, new PaletteCastMember(this, id, castStream, _version));
		break;
	case kCastPicture:
		warning("BUILDBOT: STUB: Cast::loadCastData(): kCastPicture (id=%d, %d children)! This will be missing from the movie and may cause problems", id, res->children.size());
		castInfoSize = 0;
		break;
	case kCastMovie:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastMovie (id=%d, %d children)",  id, res->children.size());
		_loadedCast->setVal(id, new MovieCastMember(this, id, castStream, _version));
		break;
	default:
		warning("Cast::loadCastData(): Unhandled cast type: %d [%s] (id=%d, %d children)! This will be missing from the movie and may cause problems", castType, tag2str(castType), id, res->children.size());
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

	if (castInfoSize && _version < kFileVer500) {
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
	if (_version >= kFileVer400) {
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
		int16 nextUnused = firstUnused;
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
			_lingoArchive->addCodeV4(*(r = _castArchive->getResource(MKTAG('L', 's', 'c', 'r'), entry.index)), i, _macName, _version);
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
		error("Cast::loadLingoContext: unsupported Director version (%d)", _version);
	}
}

void Cast::loadScriptText(Common::SeekableReadStreamEndian &stream, uint16 id) {
	/*uint32 unk1 = */ stream.readUint32();
	uint32 strLen = stream.readUint32();
	/*uin32 dataLen = */ stream.readUint32();
	Common::String script = stream.readString(0, strLen);

	// Check if this is a script. It must start with a comment.
	// See D2 Interactivity Manual pp.46-47 (Ch.2.11. Using a macro)
	if (script.empty() || !script.hasPrefix("--"))
		return;

	if (ConfMan.getBool("dump_scripts"))
		dumpScript(script.c_str(), kMovieScript, id);

	if (script.contains("\nmenu:") || script.hasPrefix("menu:"))
		return;

	_lingoArchive->addCode(script.decode(Common::kMacRoman), kMovieScript, id);
}

void Cast::dumpScript(const char *script, ScriptType type, uint16 id) {
	Common::DumpFile out;
	Common::String buf = dumpScriptName(encodePathForDump(_macName).c_str(), type, id, "txt");

	if (!out.open(buf, true)) {
		warning("Cast::dumpScript(): Can not open dump file %s", buf.c_str());
		return;
	}

	uint len = strlen(script);
	char *scriptCopy = (char *)malloc(len + 1);
	Common::strlcpy(scriptCopy, script, len + 1);

	for (uint i = 0; i < len; i++)
		if (scriptCopy[i] == '\r' && scriptCopy[i + 1] != '\n') // It is safe to check [i + 1], as '\0' != '\n'
			scriptCopy[i] = '\n';

	out.write(scriptCopy, len);

	out.flush();
	out.close();

	free(scriptCopy);

}

void Cast::loadCastInfo(Common::SeekableReadStreamEndian &stream, uint16 id) {
	if (!_loadedCast->contains(id))
		return;

	InfoEntries castInfo = Movie::loadInfoEntries(stream, _version);

	debugCN(4, kDebugLoading, "Cast::loadCastInfo(): castId: %s str(%d): '", numToCastNum(id), castInfo.strings.size());

	for (uint i = 0; i < castInfo.strings.size(); i++) {
		debugCN(4, kDebugLoading, "%s'", utf8ToPrintable(castInfo.strings[i].readString()).c_str());
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
				ci->scriptStyle.read(*entryStream, this);
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
			// Multiple casts can have the same name. In director only the first one is used.
			if (!_castsNames.contains(ci->name)) {
				_castsNames[ci->name] = id;
			} else {
				debugC(4, kDebugLoading, "Cast::loadCastInfo(): duplicate cast name: %s for castIDs: %s %s", ci->name.c_str(), numToCastNum(id), numToCastNum(_castsNames[ci->name]));
			}
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
	if (_version < kFileVer400 || debugChannelSet(-1, kDebugNoBytecode)) {
		if (!ci->script.empty()) {
			ScriptType scriptType = kCastScript;
			// the script type here could be wrong!
			if (member->_type == kCastLingoScript) {
				scriptType = ((ScriptCastMember *)member)->_scriptType;
			}

			if (ConfMan.getBool("dump_scripts"))
				dumpScript(ci->script.c_str(), scriptType, id);

			_lingoArchive->addCode(ci->script, scriptType, id, ci->name.c_str());
		}
	}

	// For SoundCastMember, read the flags in the CastInfo
	if (_version >= kFileVer400 && _version < kFileVer500 && member->_type == kCastSound) {
		((SoundCastMember *)member)->_looping = castInfo.flags & 16 ? 0 : 1;
	}

	// For FilmLoopCastMember, read the flags in the CastInfo
	if (_version >= kFileVer400 && _version < kFileVer500 && member->_type == kCastFilmLoop) {
		((FilmLoopCastMember *)member)->_looping = castInfo.flags & 64 ? 0 : 1;
		((FilmLoopCastMember *)member)->_enableSound = castInfo.flags & 8 ? 1 : 0;
		((FilmLoopCastMember *)member)->_crop = castInfo.flags & 2 ? 0 : 1;
		((FilmLoopCastMember *)member)->_center = castInfo.flags & 1 ? 1 : 0;
	}

	ci->autoHilite = castInfo.flags & 2;
	ci->scriptId = castInfo.scriptId;
	if (ci->scriptId != 0)
		_castsScriptIds[ci->scriptId] = id;

	_castsInfo[id] = ci;
}

Common::CodePage Cast::getFileEncoding() {
	// Returns the default encoding for the file this cast is contained in.
	// This depends on which platform the file was made on.
	return getEncoding(_platform, _vm->getLanguage());
}

Common::U32String Cast::decodeString(const Common::String &str) {
	Common::CodePage encoding = getFileEncoding();

	Common::String fixedStr;
	if (encoding == Common::kWindows1252) {
		/**
		 * Director for Windows stores strings in a screwed up version of Mac Roman
		 * where characters map directly to Windows-1252 characters.
		 * We need to map this screwed up Mac Roman back to Windows-1252 before using it.
		 * Comment from FXmp:
		 *   Note: Some characters are not available in both character sets.
		 *   However, the bi-directional mapping table below preserves these
		 *   characters even if they are mapped to a different platform and
		 *   later re-mapped back to the original platform.
		 */

		for (uint i = 0; i < str.size(); i++) {
			if (_macCharsToWin.contains(str[i]))
				fixedStr += _macCharsToWin[str[i]];
			else
				fixedStr += str[i];
		}
	} else {
		fixedStr = str;
	}

	return fixedStr.decode(encoding);
}

// Score order, 'Sord' resource
//
// Enlists all cast members as they're used in the movie
// It is used for more effective preloading
//
// And as such, not currently used by the ScummVM
void Cast::loadSord(Common::SeekableReadStreamEndian &stream) {
	stream.readUint32();
	stream.readUint32();
	stream.readUint32();
	stream.readUint32();
	stream.readUint16();
	stream.readUint16();

	uint numEntries = 0;
	uint16 castLibId = 0; // default for pre-D5
	uint16 memberId;
	while (!stream.eos()) {

		if (_version >= kFileVer500)
			castLibId = stream.readUint16LE();

		memberId = stream.readUint16LE();

		debugC(2, kDebugLoading, "Cast::loadSord(): entry %d - %u:%u", numEntries, castLibId, memberId);

		numEntries++;
	}

	debugC(1, kDebugLoading, "Cast::loadSord(): number of entries: %d", numEntries);
}

// Pattern tiles
//
// Basically, a reference to Bitmap cast accompanied with rectrangle
void Cast::loadVWTL(Common::SeekableReadStreamEndian &stream) {
	debugC(1, kDebugLoading, "****** Loading CastMember petterns VWTL");

	Common::Rect r;
	uint16 castLibId = 0; // default for pre-D5
	uint16 memberId;

	for (int i = 0; i < kNumBuiltinTiles; i++) {
		stream.readUint32(); // unused

		if (_version >= kFileVer500)
			castLibId = stream.readUint16();

		memberId = stream.readUint16();

		r = Movie::readRect(stream);

		_tiles[i].bitmapId.castLib = castLibId;
		_tiles[i].bitmapId.member = memberId;
		_tiles[i].rect = r;

		debugC(2, kDebugLoading, "Cast::loadCastDataVWCR(): entry %d - %u:%u [%d, %d, %d, %d]", i, castLibId, memberId,
				r.left, r.top, r.right, r.bottom);
	}

}

} // End of namespace Director
