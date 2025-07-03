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
#include "common/macresman.h"
#include "common/memstream.h"
#include "common/substream.h"

#include "director/types.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"

#include "video/qt_decoder.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/rte.h"
#include "director/score.h"
#include "director/sound.h"
#include "director/sprite.h"
#include "director/stxt.h"
#include "director/castmember/castmember.h"
#include "director/castmember/bitmap.h"
#include "director/castmember/digitalvideo.h"
#include "director/castmember/filmloop.h"
#include "director/castmember/movie.h"
#include "director/castmember/palette.h"
#include "director/castmember/richtext.h"
#include "director/castmember/script.h"
#include "director/castmember/shape.h"
#include "director/castmember/sound.h"
#include "director/castmember/text.h"
#include "director/castmember/transition.h"
#include "director/lingo/lingo-codegen.h"

#include "director/lingo/lingodec/context.h"
#include "director/lingo/lingodec/names.h"
#include "director/lingo/lingodec/resolver.h"
#include "director/lingo/lingodec/script.h"

namespace Director {

Cast::Cast(Movie *movie, uint16 castLibID, bool isShared, bool isExternal, uint16 libResourceId) {
	_movie = movie;
	_vm = _movie->getVM();
	_lingo = _vm->getLingo();

	_castLibID = castLibID;
	_libResourceId = libResourceId;
	_isShared = isShared;
	_isExternal = isExternal;
	_loadMutex = true;

	_lingoArchive = new LingoArchive(this);

	_castArrayStart = _castArrayEnd = 0;

	_castIDoffset = 0;

	_castArchive = nullptr;
	_version = 0;
	_platform = Common::kPlatformMacintosh;

	_isProtected = false;

	_stageColor = 0;

	_loadedCast = nullptr;

	_defaultPalette = CastMemberID(-1, -1);
	_frameRate = 0;
}

Cast::~Cast() {
	for (auto &it : _loadedStxts)
		delete it._value;

	if (_loadedCast)
		for (auto &it : *_loadedCast)
			if (it._value) {
				it._value->decRefCount();
				it._value = nullptr;
			}

	for (auto &it : _castsInfo)
		delete it._value;

	for (auto &it : _fontXPlatformMap)
		delete it._value;

	for (auto &it : _fontMap)
		delete it._value;

	for (auto &it : _loadedRTE0s)
		delete it._value;

	for (auto &it : _loadedRTE1s)
		delete it._value;

	for (auto &it : _loadedRTE2s)
		delete it._value;

	delete _loadedCast;
	delete _lingoArchive;

	delete _chunkResolver;
	delete _lingodec;
}

CastMember *Cast::getCastMember(int castId, bool load) {
	CastMember *result = nullptr;

	if (_loadedCast && _loadedCast->contains(castId)) {
		result = _loadedCast->getVal(castId);
	}
	if (result && load && _loadMutex) {
		// Archives only support having one stream open at a time,
		// prevent recursive calls to CastMember::load()
		_loadMutex = false;
		result->load();
		while (!_loadQueue.empty()) {
			_loadQueue.back()->load();
			_loadQueue.pop_back();
		}
		_loadMutex = true;
	} else if (result) {
		_loadQueue.push_back(result);
	}
	return result;
}

void Cast::releaseCastMemberWidget() {
	if (_loadedCast)
		for (auto &it : *_loadedCast)
			it._value->releaseWidget();
}

CastMember *Cast::getCastMemberByNameAndType(const Common::String &name, CastType type) {
	if (type == kCastTypeAny) {
		if (_castsNames.contains(name)) {
			return getCastMember(_castsNames[name]);
		}
	} else {
		Common::String cname = Common::String::format("%s:%d", name.c_str(), type);

		if (_castsNames.contains(cname))
			return getCastMember(_castsNames[cname]);
	}
	return nullptr;
}

CastMember *Cast::getCastMemberByScriptId(int scriptId) {
	if (_castsScriptIds.contains(scriptId))
		return getCastMember(_castsScriptIds[scriptId]);
	return nullptr;
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

	if (_loadedStxts.contains(castId)) {
		result = _loadedStxts.getVal(castId);
	}
	return result;
}

int Cast::getCastSize() {
	return _loadedCast->size();
}

int Cast::getCastMaxID() {
	int result = 0;
	for (auto &it : *_loadedCast) {
		result = MAX(result, it._key);
	}
	return result;
}

int Cast::getNextUnusedID() {
	int result = 1;
	while (_loadedCast->contains(result)) {
		result += 1;
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

	cast->setModified(true);
}

CastMember *Cast::setCastMember(int castId, CastMember *cast) {
	eraseCastMember(castId);
	cast->incRefCount();
	_loadedCast->setVal(castId, cast);
	return cast;
}

bool Cast::duplicateCastMember(CastMember *source, CastMemberInfo *info, int targetId) {
	if (_loadedCast->contains(targetId)) {
		eraseCastMember(targetId);
	}
	// duplicating a cast member with a non-existent source
	// is the same as deleting the target
	if (!source)
		return true;
	CastMember *target = source->duplicate(this, targetId);

	if (info) {
		CastMemberInfo *newInfo = new CastMemberInfo(*info);
		_castsInfo[targetId] = newInfo;
	}
	setCastMember(targetId, target);
	if (info) {
		rebuildCastNameCache();
	}
	return true;
}

bool Cast::eraseCastMember(int castId) {
	if (_loadedCast->contains(castId)) {
		CastMember *member = _loadedCast->getVal(castId);
		member->decRefCount();
		_loadedCast->erase(castId);

		if (_castsInfo.contains(castId)) {
			CastMemberInfo *info = _castsInfo.getVal(castId);
			delete info;
			_castsInfo.erase(castId);
		}
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
}

void Cast::loadArchive() {
	loadConfig();
	loadCast();
}

bool Cast::loadConfig() {
	if (!_castArchive) {
		warning("Cast::loadConfig(): No archive specified");
		return false;
	}
	Common::SeekableReadStreamEndian *stream = nullptr;
	stream = _castArchive->getMovieResourceIfPresent(MKTAG('V', 'W', 'C', 'F'));
	if (!stream)
		stream = _castArchive->getMovieResourceIfPresent(MKTAG('D', 'R', 'C', 'F'));
	if (!stream) {
		warning("Cast::loadConfig(): Wrong format. VWCF resource missing");
		return false;
	}

	debugC(1, kDebugLoading, "****** Loading Config VWCF for cast libID %d (%s)", _castLibID, _castName.c_str());

	if (debugChannelSet(5, kDebugLoading))
		stream->hexdump(stream->size());

	// Post D3 Check. The version field was added in D3 and later versions.
	// This is used to check some fields which differed pre-d7 and post-d7
	if (stream->size() > 36) {
		stream->seek(36);					// Seek to version
		_version = stream->readSint16();	// Read version for post-D3
		stream->seek(0);					// Seek to start of stream
	}

	_len = stream->readUint16();
	_fileVersion = stream->readUint16(); // TODO: very high fileVersion means protected

	if (stream->size() <= 36)
		_version = _fileVersion;				// Checking if we have already read the version

	uint humanVer = humanVersion(_version);

	_checkRect = Movie::readRect(*stream);
	if (!g_director->_fixStageSize)
		_movieRect = _checkRect;
	else
		_movieRect = g_director->_fixStageRect;

	_castArrayStart = stream->readUint16();
	_castArrayEnd = stream->readUint16();

	// D3 and below use this, override for D4 and over
	// actual framerates are, on average: { 3.75, 4, 4.35, 4.65, 5, 5.5, 6, 6.6, 7.5, 8.5, 10, 12, 20, 30, 60 }
	Common::Array<int> frameRates = { 3, 4, 4, 4, 5, 5, 6, 6, 7, 8, 10, 12, 15, 20, 30, 60 };
	_readRate = stream->readByte();
	if (_readRate <= 0xF) {
		_frameRate = frameRates[_readRate];
	} else {
		switch (_readRate) {
			// rate when set via the tempo channel
			// these rates are the actual framerates
			case 0x10:
				// defaults to 15 fps on D2 and D3. On D4 it shows as 120 fps
				_frameRate = 15;
				break;
			case 212:
				_frameRate = 1;
				break;
			case 242:
				_frameRate = 2;
				break;
			case 252:
				_frameRate = 3;
				break;
			default:
				warning("BUILDBOT: Cast::loadConfig: unhandled framerate: %i", _readRate);
				_frameRate = _readRate;
		}
	}


	_lightswitch = stream->readByte();

	_unk1 = stream->readSint16();

	// Warning for post-D7 movies (unk1 is stageColorG and stageColorB post-D7)
	if (humanVer >= 700)
		warning("STUB: Cast::loadConfig: 16 bit unk1 read instead of two 8 bit stageColorG and stageColorB. Read value: %04x", _unk1);

	_commentFont = stream->readUint16();
	_commentSize = stream->readUint16();
	_commentStyle = stream->readUint16();
	_stageColor = stream->readUint16();

	// Warning for post-D7 movies (stageColor is isStageColorRGB and stageColorR post-D7)
	if (humanVer >= 700)
		warning("STUB: Cast::loadConfig: 16 bit stageColor read instead of two 8 bit isStageColorRGB and stageColorR. Read value: %04x", _stageColor);

	_bitdepth = stream->readUint16();

	// byte color = stream.readByte();	// boolean, color = 1, B/W = 0
	// uint16 stageColorR = stream.readUint16();
	// uint16 stageColorG = stream.readUint16();
	// uint16 stageColorB = stream.readUint16();

	debugC(1, kDebugLoading, "Cast::loadConfig(): len: %d, fileVersion: %d, light: %d, unk: %d, font: %d, size: %d"
			", style: %d", _len, _fileVersion, _lightswitch, _unk1, _commentFont, _commentSize, _commentStyle);
	debugC(1, kDebugLoading, "Cast::loadConfig(): stagecolor: %d, depth: %d",
			_stageColor, _bitdepth);
	if (debugChannelSet(1, kDebugLoading))
		_movieRect.debugPrint(1, "Cast::loadConfig(): Movie rect: ");

	// Fields required for checksum calculation
	// D3 fields - Macromedia did not increment the fileVersion from D2 to D3
	// so we just have to check if there are more bytes to read.
	if (stream->pos() < stream->size()) {

		//reading these fields for the sake of checksum calculation
		_field17 = stream->readByte();
		_field18 = stream->readByte();
		_field19 = stream->readSint32();

		/* version = */ stream->readUint16();	// We've already read it, this is offset 36

		_field21 = stream->readSint16();
		_field22 = stream->readSint32();
		_field23 = stream->readSint32();
	}

	debugC(1, kDebugLoading, "Cast::loadConfig(): directorVersion: %d", humanVer);

	if (_version >= kFileVer400) {
		_field24 = stream->readSint32();
		_field25 = stream->readSByte();
		_field26 = stream->readSByte();

		_frameRate = stream->readSint16();
		_platformID = stream->readUint16();
		_platform = platformFromID(_platformID);

		_protection = stream->readSint16();
		_isProtected = (_protection % 23) == 0;
		_field29 = stream->readSint32();
		_checksum = stream->readUint32();

		//Calculation and verification of checksum
		uint32 check = _len + 1;
		check *= _fileVersion + 2;
		check /= _checkRect.top + 3;
		check *= _checkRect.left + 4;
		check /= _checkRect.bottom + 5;
		check *= _checkRect.right + 6;
		check -= _castArrayStart + 7;
		check *= _castArrayEnd + 8;
		check -= (int8)_readRate + 9;
		check -= _lightswitch + 10;

		if (humanVer < 700)
			check += _unk1 + 11;
		else
			warning("STUB: skipped using stageColorG, stageColorB for post-D7 movie in checksum calulation");

		check *= _commentFont + 12;
		check += _commentSize + 13;

		if (humanVer < 800)
			check *= (uint8)((_commentStyle >> 8) & 0xFF) + 14;
		else
			check *= _commentStyle + 14;

		if (humanVer < 700)
			check += _stageColor + 15;
		else
			check += (uint8)(_stageColor & 0xFF) + 15;	// Taking lower 8 bits to take into account stageColorR


		check += _bitdepth + 16;
		check += _field17 + 17;
		check *= _field18 + 18;
		check += _field19 + 19;
		check *= _version + 20;
		check += _field21 + 21;
		check += _field22 + 22;
		check += _field23 + 23;
		check += _field24 + 24;
		check *= _field25 + 25;
		check += _frameRate + 26;
		check *= _platformID + 27;
		check *= (_protection * 0xE06) + 0xFF450000u;
		check ^= MKTAG('r', 'a', 'l', 'f');

		if (check != _checksum)
			warning("BUILDBOT: The checksum for this VWCF resource is incorrect. Got %04x, but expected %04x", check, _checksum);

		if (_version >= kFileVer400 && _version < kFileVer500) {
			_field30 = stream->readSint16();

			_defaultPalette.member = stream->readSint16();
			// In this header value, the first builtin palette starts at 0 and
			// continues down into negative numbers.
			// For frames, 0 is used to represent an absence of a palette change,
			// with the builtin palettes starting from -1.
			if (_defaultPalette.member <= 0)
				_defaultPalette.member -= 1;
			else
				_defaultPalette.castLib = DEFAULT_CAST_LIB;
			for (int i = 0; i < 0x08; i++) {
				stream->readByte();
			}
		} else if (_version >= kFileVer500 && _version < kFileVer600) {
			for (int i = 0; i < 0x08; i++) {
				stream->readByte();
			}
			_defaultPalette.castLib = stream->readSint16();
			_defaultPalette.member = stream->readSint16();
			if (_defaultPalette.member <= 0)
				_defaultPalette.member -= 1;

		} else {
			warning("STUB: Cast::loadConfig(): Extended config not yet supported for version %d", _version);
		}
		debugC(1, kDebugLoading, "Cast::loadConfig(): platform: %s, defaultPalette: %s, frameRate: %d", getPlatformAbbrev(_platform), _defaultPalette.asString().c_str(), _frameRate);
	}

	if (humanVer > _vm->getVersion()) {
		if (_vm->getVersion() > 0)
			warning("Movie is from later version v%d", humanVer);
		_vm->setVersion(humanVer);
	}

    if (debugChannelSet(7, kDebugSaving)) {
        // Adding +8 because the stream doesn't include the header and the entry for the size itself
        uint32 configSize = stream->size() + 8;
        byte *dumpData = (byte *)calloc(configSize, sizeof(byte));

        Common::SeekableMemoryWriteStream *writeStream = new Common::SeekableMemoryWriteStream(dumpData, configSize);

        saveConfig(writeStream, 0);
        dumpFile("ConfigData", 0, MKTAG('V', 'W', 'C', 'F'), dumpData, configSize);
        delete writeStream;
    }

	delete stream;
	return true;
}

void Cast::saveConfig(Common::MemoryWriteStream *writeStream, uint32 offset) {
	if (_version < kFileVer400) {
		error("Cast::saveConfig called on a pre-D4 Director movie");
	}

	writeStream->seek(offset);					// This will allow us to write cast config at any offset
	// These offsets are only for Director Version 4 to Director version 6
	// offsets
	writeStream->writeUint16LE(_len);			// 0    // This will change
	writeStream->writeUint16LE(_fileVersion);	    // 2

	Movie::writeRect(writeStream, _checkRect);      // 4, 6, 8, 10

	writeStream->writeUint16LE(_castArrayStart);    // 12
	writeStream->writeUint16LE(_castArrayEnd);      // 14   // This will change

	writeStream->writeByte(_readRate);              // 16
	writeStream->writeByte(_lightswitch);           // 17
	writeStream->writeSint16LE(_unk1);              // 18

	writeStream->writeUint16LE(_commentFont);       // 20
	writeStream->writeUint16LE(_commentSize);       // 22
	writeStream->writeUint16LE(_commentStyle);      // 24
	writeStream->writeUint16LE(_stageColor);        // 26

	writeStream->writeUint16LE(_bitdepth);          // 28

	writeStream->writeByte(_field17);               // 29
	writeStream->writeByte(_field18);               // 30
	writeStream->writeSint32LE(_field19);           // 34

	writeStream->writeUint16LE(_version);   		// 36

	writeStream->writeUint16LE(_field21);           // 38
	writeStream->writeUint32LE(_field22);           // 40
	writeStream->writeUint32LE(_field23);           // 44

	writeStream->writeSint32LE(_field24);           // 48
	writeStream->writeSByte(_field25);              // 52
	writeStream->writeSByte(_field26);              // 53

	writeStream->writeSint16LE(_frameRate);         // 54
	writeStream->writeUint16LE(_platform);          // 56
	writeStream->writeSint16LE(_protection);        // 58
	writeStream->writeSint32LE(_field29);           // 60

	// Currently a stub
	uint32 checksum = computeChecksum();
	writeStream->writeUint32LE(checksum);           // 64

	if (_version >= kFileVer400 && _version < kFileVer500) {
		writeStream->writeSint16LE(_field30);       // 68

		// This loop isn't writing meaningful data currently
		// But it is possible that this data might be needed
		for (int i = 0; i < 0x08; i++) {
			writeStream->writeByte(0);              // 70, 71, 72, 73, 74, 75, 76, 77
		}
	} else if (_version >= kFileVer500 && _version < kFileVer600) {
		for (int i = 0; i < 0x08; i++) {
			writeStream->writeByte(0);              // 68, 69, 70, 71, 72, 73, 74, 75
		}

		writeStream->writeSint16LE(_defaultPalette.castLib);    // 76
		writeStream->writeSint16LE(_defaultPalette.member);     // 77
	}

}

void Cast::loadCast() {
	Common::SeekableReadStreamEndian *r = nullptr;

	// Font Directory
	_vm->_wm->_fontMan->loadFonts(_castArchive->getPathName());

	debugC(1, kDebugLoading, "****** Loading cast member data for for cast libID %d (%s)", _castLibID, _castName.c_str());

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

	// Font Mapping D4
	if ((r = _castArchive->getMovieResourceIfPresent(MKTAG('F', 'm', 'a', 'p'))) != nullptr) {
		loadFontMapV4(*r);
		delete r;
	}

	// Pattern Tiles
	if ((r = _castArchive->getMovieResourceIfPresent(MKTAG('V', 'W', 'T', 'L'))) != nullptr) {
		loadVWTL(*r);
		delete r;
	}

	// External sound files
	if ((r = _castArchive->getMovieResourceIfPresent(MKTAG('S', 'T', 'R', ' '))) != nullptr) {
		loadExternalSound(*r);
		delete r;
	}


	if (_castArchive->hasResource(MKTAG('X', 'C', 'O', 'D'), -1)) {
		Common::Array<uint16> xcod = _castArchive->getResourceIDList(MKTAG('X', 'C', 'O', 'D'));
		for (auto &iterator : xcod) {
			Resource res = _castArchive->getResourceDetail(MKTAG('X', 'C', 'O', 'D'), iterator);
			debug(0, "Detected XObject '%s'", res.name.c_str());
			g_lingo->openXLib(res.name, kXObj, _castArchive->getPathName());
		}
	}

	Common::Array<uint16> cinf = _castArchive->getResourceIDList(MKTAG('C', 'i', 'n', 'f'));
	if (cinf.size() > 0) {
		debugC(2, kDebugLoading, "****** Loading %d CastLibInfos Cinf", cinf.size());

		for (auto &iterator : cinf) {
			loadCastLibInfo(*(r = _castArchive->getResource(MKTAG('C', 'i', 'n', 'f'), iterator)), iterator);
			delete r;
		}
	}

	Common::Array<uint16> vwci = _castArchive->getResourceIDList(MKTAG('V', 'W', 'C', 'I'));
	if (vwci.size() > 0) {
		debugC(2, kDebugLoading, "****** Loading %d CastInfos VWCI", vwci.size());

		for (auto &iterator : vwci) {
			loadCastInfo(*(r = _castArchive->getResource(MKTAG('V', 'W', 'C', 'I'), iterator)), iterator - _castIDoffset);
			delete r;
		}
	}

	Common::Array<uint16> cast = _castArchive->getResourceIDList(MKTAG('C', 'A', 'S', 't'));
	if (!_loadedCast)
		_loadedCast = new Common::HashMap<int, CastMember *>();

	// External casts only have one library ID, so instead
	// we use the movie's mapping.
	uint16 libResourceId = _isExternal ? 1024 : _libResourceId;

	if (cast.size() > 0) {
		debugC(2, kDebugLoading, "****** Loading CASt resources for libId %d (%s), resourceId %d", _castLibID, _castName.c_str(), libResourceId);

		int idx = 0;

		for (auto &iterator : cast) {
			Resource res = _castArchive->getResourceDetail(MKTAG('C', 'A', 'S', 't'), iterator);
			// Only load cast members which belong to the requested library ID.
			// External casts only have one library ID, so instead
			// we use the movie's mapping.
			if (res.libResourceId != libResourceId) {
				debugC(5, kDebugLoading, "SKIPPED - CASt: resource %d, castId %d, libResourceId %d", iterator, res.castId, res.libResourceId);
				continue;
			}
			debugC(2, kDebugLoading, "CASt: resource %d, castId %d, libResourceId %d", iterator, res.castId, res.libResourceId);
			Common::SeekableReadStreamEndian *stream = _castArchive->getResource(MKTAG('C', 'A', 'S', 't'), iterator);
			loadCastData(*stream, res.castId, &res);
			delete stream;

			if (debugChannelSet(-1, kDebugFewFramesOnly) && idx++ > 0 && !(idx % 200))
				debug("Loaded %d cast resources", idx);
		}
	}

	// The CastMemberInfo data should be loaded by now,
	// set up the cache used for cast member name lookups.
	rebuildCastNameCache();

	// Score Order List resources
	if ((r = _castArchive->getMovieResourceIfPresent(MKTAG('S', 'o', 'r', 'd'))) != nullptr) {
		loadSord(*r);
		delete r;
	}

	// Now process STXTs
	Common::Array<uint16> stxt = _castArchive->getResourceIDList(MKTAG('S','T','X','T'));
	debugC(2, kDebugLoading, "****** Loading %d STXT resources", stxt.size());

	for (auto &iterator : stxt) {
		_loadedStxts.setVal(iterator - _castIDoffset, new Stxt(this, *(r = _castArchive->getResource(MKTAG('S','T','X','T'), iterator))));
		debugC(3, kDebugText, "STXT: id %d", iterator - _castIDoffset);
		delete r;

		// Try to load movie script, it starts with a comment
		if (_version < kFileVer400) {
			if (debugChannelSet(-1, kDebugFewFramesOnly))
				warning("Compiling STXT %d", iterator);

			loadScriptV2(*(r = _castArchive->getResource(MKTAG('S','T','X','T'), iterator)), iterator - _castIDoffset);
			delete r;
		}
	}

	Common::Array<uint16> rte0 = _castArchive->getResourceIDList(MKTAG('R','T','E','0'));
	debugC(2, kDebugLoading, "****** Loading %d RTE0 resources", rte0.size());

	for (auto &iterator : rte0) {
		r = _castArchive->getResource(MKTAG('R','T','E','0'), iterator);
		debugC(3, kDebugText, "RTE0: id %d", iterator - _castIDoffset);
		_loadedRTE0s.setVal(iterator, new RTE0(this, *r));
		delete r;
	}

	Common::Array<uint16> rte1 = _castArchive->getResourceIDList(MKTAG('R','T','E','1'));
	debugC(2, kDebugLoading, "****** Loading %d RTE1 resources", rte1.size());

	for (auto &iterator : rte1) {
		r = _castArchive->getResource(MKTAG('R','T','E','1'), iterator);
		debugC(3, kDebugText, "RTE1: id %d", iterator - _castIDoffset);
		_loadedRTE1s.setVal(iterator, new RTE1(this, *r));
		delete r;
	}

	Common::Array<uint16> rte2 = _castArchive->getResourceIDList(MKTAG('R','T','E','2'));
	debugC(2, kDebugLoading, "****** Loading %d RTE2 resources", rte2.size());

	for (auto &iterator : rte2) {
		r = _castArchive->getResource(MKTAG('R','T','E','2'), iterator);
		debugC(3, kDebugText, "RTE2: id %d", iterator - _castIDoffset);
		_loadedRTE2s.setVal(iterator, new RTE2(this, *r));
		delete r;
	}

	// For D4+ we may request to force Lingo scripts and skip precompiled bytecode
	if (_version >= kFileVer400 && !debugChannelSet(-1, kDebugNoBytecode)) {
		// Try to load script context
		// Even for multiple casts, ID is 1024
		if ((r = _castArchive->getFirstResource(MKTAG('L', 'c', 't', 'x'), libResourceId)) != nullptr) {
			loadLingoContext(*r);
			delete r;
		}
	}

	// PICT resources
	if (_castArchive->hasResource(MKTAG('P', 'I', 'C', 'T'), -1)) {
		debug("STUB: Unhandled 'PICT' resource");
	}

	// External Cast Reference resources
	// Used only by authoring tools for referring to the external casts
	if (_castArchive->hasResource(MKTAG('S', 'C', 'R', 'F'), -1)) {
		debugC(4, kDebugLoading, "'SCRF' resource skipped");
	}
}

void Cast::saveCast() {
	// This offset is at which we will start writing our 'CASt' resources
	// In the original file, all the 'CASt' resources don't necessarily appear side by side
	// writeStream->seek(offset);

	Common::Array<uint16> cast = _castArchive->getResourceIDList(MKTAG('C', 'A', 'S', 't'));

	// Okay, this is going to cause confusion
	// In the director movie archive, each CASt resource is given an ID, Not sure how this ID is assigned
	// but it is present in the keytable and is loaded while reading the keytable in RIFXArchive::readKeyTable();
	// The castId on the other hand present in the Resource struct is assigned by ScummVM Director in RIFXArchive::readCast()
	// It is basically the index at which it occurs in the CAS* resource
	// So, RIFXArchive::getResourceDetail will return the list of IDs present in the keytable
	// Whereas, in the _loadedCast, the key of these Cast members is given by castId
	for (auto &it : cast) {
	// I need to call this just because I need the id
		Resource res = _castArchive->getResourceDetail(MKTAG('C', 'A', 'S', 't'), it);
		uint16 id = res.castId + _castArrayStart;               // Never encountered _castArrayStart != 0 till now
		uint32 castSize = 0;

		if (_loadedCast->contains(id)) {
			CastMember *target = _loadedCast->getVal(id);
			// To make it consistent with how the data is stored originally, getResourceSize returns
			// the size excluding 'CASt' header and the entry for size itself. Adding 8 to compensate for that
			castSize = target->getCastResourceSize() + 8;
		} else {
			// The size stored in the memory map (_resources array), as well as the resource itself is the size
			// excluding the 'CASt' header and the entry of size itself. Adding 8 to compensate for that
			castSize = _castArchive->getResourceSize(MKTAG('C', 'A', 'S', 't'), it) + 8;
		}

		debugC(5, kDebugSaving, "Cast::saveCast()::Saving 'CASt' resource, id: %d, size: %d", id, castSize);
		byte *dumpData = (byte *)calloc(castSize, sizeof(byte));

		Common::SeekableMemoryWriteStream *writeStream = new Common::SeekableMemoryWriteStream(dumpData, castSize);
		CastType type = kCastTypeAny;

		if (!_loadedCast->contains(id)) {
			writeStream->writeUint32LE(MKTAG('C', 'A', 'S', 't'));
			Common::SeekableReadStreamEndian *stream = getResource(MKTAG('C', 'A', 'S', 't'), it);
			uint32 size = stream->size();           // This is the size of the Resource without header and size entry itself
			writeStream->writeUint32LE(size);
			writeStream->writeStream(stream);

			delete stream;
		} else {
			CastMember *target = _loadedCast->getVal(id);
			type = target->_type;
			target->writeCAStResource(writeStream, 0);
			break;
		}

		dumpFile(castType2str(type), res.index, MKTAG('C', 'A', 'S', 't'), dumpData, castSize);
		delete writeStream;
	}

}

void Cast::writeCastInfo(Common::MemoryWriteStream *writeStream, uint32 castId) {
	// The structure of the CastMemberInfo is as follows:
	// First some headers: offset, unknown and flags, and then a count of strings to be read
	// (These strings contain properties of the cast member like filename, script attached to it, name, etc.)
	// After the header, we have a sequence of the lengths of the strings,
	// The first int is 0, the second int is 0 + length of the first string, the third int is 0 + length of first string + length of second string and so on
	// After the lengths of the strings are the actual strings

	if (!_castsInfo.contains(castId)) {
		return;
	}
	CastMemberInfo *ci = _castsInfo[castId];

	InfoEntries castInfo;

	castInfo.unk1 = ci->unk1;
	castInfo.unk2 = ci->unk2;
	castInfo.flags = ci->flags;
	castInfo.scriptId = ci->scriptId;
	castInfo.strings.resize(ci->count);

	for (int i = 1; i <= ci->count; i++) {
		castInfo.strings[i - 1].len = getCastInfoStringLength(i, ci);
	}

	for (int i = 1; i <= ci->count; i++) {
		if (!castInfo.strings[i - 1].len) {
			continue;
		}

		switch (i) {
		default:
			debug("Cast::writeCastInfo()::extra strings found, ignoring");
			break;

		case 1:
			castInfo.strings[0].data = (byte *)malloc(castInfo.strings[0].len);
			memcpy(castInfo.strings[0].data, ci->script.c_str(), castInfo.strings[0].len);
			break;

		case 2:
			castInfo.strings[1].data = (byte *)malloc(castInfo.strings[1].len);
			memcpy(castInfo.strings[1].data, ci->name.c_str(), castInfo.strings[1].len);
			break;

		case 3:
			castInfo.strings[2].data = (byte *)malloc(castInfo.strings[2].len);
			memcpy(castInfo.strings[2].data, ci->directory.c_str(), castInfo.strings[2].len);
			break;

		case 4:
			castInfo.strings[3].data = (byte *)malloc(castInfo.strings[3].len);
			memcpy(castInfo.strings[3].data, ci->fileName.c_str(), castInfo.strings[3].len);
			break;

		case 5:
			castInfo.strings[4].data = (byte *)malloc(castInfo.strings[4].len);
			memcpy(castInfo.strings[4].data, ci->type.c_str(), castInfo.strings[4].len);
			break;

		case 6:
			{
				castInfo.strings[5].data = (byte *)malloc(castInfo.strings[5].len);
				Common::MemoryWriteStream *stream = new Common::MemoryWriteStream(castInfo.strings[5].data, castInfo.strings[5].len);

				Movie::writeRect(stream, ci->scriptEditInfo.rect);
				stream->writeUint32BE(ci->scriptEditInfo.selStart);
				stream->writeUint32BE(ci->scriptEditInfo.selEnd);
				stream->writeByte(ci->scriptEditInfo.version);
				stream->writeByte(ci->scriptEditInfo.rulerFlag);
				delete stream;
			}
			break;

		case 7:
			{
				castInfo.strings[6].data = (byte *)malloc(castInfo.strings[6].len);

				Common::MemoryWriteStream *stream = new Common::MemoryWriteStream(castInfo.strings[6].data, castInfo.strings[6].len);
				stream->writeUint16BE(1);			// FIXME: For CastMembers, the count is 1, observed value, need to validate
				ci->scriptStyle.write(stream);

				delete stream;
			}
			break;

		case 8:
			{
				castInfo.strings[7].data = (byte *)malloc(castInfo.strings[7].len);

				Common::MemoryWriteStream *stream = new Common::MemoryWriteStream(castInfo.strings[7].data, castInfo.strings[7].len);
				Movie::writeRect(stream, ci->textEditInfo.rect);
				stream->writeUint32BE(ci->textEditInfo.selStart);
				stream->writeUint32BE(ci->textEditInfo.selEnd);
				stream->writeByte(ci->textEditInfo.version);
				stream->writeByte(ci->textEditInfo.rulerFlag);

				delete stream;
			}
			break;

		case 9:
			castInfo.strings[8].data = (byte *)malloc(castInfo.strings[8].len);
			memcpy(castInfo.strings[8].data, ci->unknown1.data(), castInfo.strings[8].len);
			break;

		case 10:
			castInfo.strings[9].data = (byte *)malloc(castInfo.strings[9].len);
			memcpy(castInfo.strings[9].data, ci->unknown2.data(), castInfo.strings[9].len);
			break;

		case 11:
			castInfo.strings[10].data = (byte *)malloc(castInfo.strings[10].len);
			memcpy(castInfo.strings[10].data, ci->unknown3.data(), castInfo.strings[10].len);
			break;

		case 12:
			castInfo.strings[11].data = (byte *)malloc(castInfo.strings[11].len);
			memcpy(castInfo.strings[11].data, ci->unknown4.data(), castInfo.strings[11].len);
			break;

		case 13:
			castInfo.strings[12].data = (byte *)malloc(castInfo.strings[12].len);
			memcpy(castInfo.strings[12].data, ci->unknown5.data(), castInfo.strings[12].len);
			break;

		case 14:
			castInfo.strings[13].data = (byte *)malloc(castInfo.strings[13].len);
			memcpy(castInfo.strings[13].data, ci->unknown6.data(), castInfo.strings[13].len);
			break;

		case 15:
			castInfo.strings[14].data = (byte *)malloc(castInfo.strings[14].len);
			memcpy(castInfo.strings[14].data, ci->unknown7.data(), castInfo.strings[14].len);
			break;
		}
	}

	Movie::saveInfoEntries(writeStream, castInfo);
}

// This function is called three separate times:
// First, when writing this 'CASt' resource in memory map (in getCastResourceSize())
// Second, when writing the 'CASt' resource itself (in getCastResourceSize())
// Third, when we're writing the info size in the 'CASt' resource
// All three times, it returns the same value, this could be more efficient
uint32 Cast::getCastInfoSize(uint32 castId) {
	CastMemberInfo *ci = getCastMemberInfo(castId);
	if (!ci) {
		return 0;
	}

	uint32 length = 0;
	for (int i = 1; i <= ci->count; i++) {
		length += getCastInfoStringLength(i, ci);
	}

	// The header + total length of the strings + number of length entries for the strings
	return 22 + length + (ci->count + 1) * 4;
}

uint32 Cast::computeChecksum() {
	warning("STUB::ConfigChunk::computeChecksum() is not implemented yet");
	return 0;
}

// The 'CASt' resource has strings containing information about the respective CastMember
// This function is for retrieving the size of each while writing them back
uint32 Cast::getCastInfoStringLength(uint32 stringIndex, CastMemberInfo *ci) {
	switch (stringIndex) {
	default:
		debug("writeCastMemberInfo:: extra string index out of bound");
		return 0;

	case 1:
		return ci->script.size();

	case 2:
		return ci->name.size();

	case 3:
		return ci->directory.size();

	case 4:
		return ci->fileName.size();

	case 5:
		return ci->type.size();

	case 6:
		// Need a better check to see if the script edit info is valid
		if (ci->scriptEditInfo.valid) {
			return 18;		// The length of an edit info
		}
		return 0;

	case 7:
		// Need a better check to see if scriptStyle is valid
		if (ci->scriptStyle.fontId) {
			return 22;		// The length of FontStyle
		}
		return 0;

	case 8:
		// Need a better check to see if text edit info is valid
		if (ci->textEditInfo.valid) {
			return 18;		// The length of an edit info
		}
		return 0;

	case 9:
		return ci->unknown1.size();

	case 10:
		return ci->unknown2.size();

	case 11:
		return ci->unknown3.size();

	case 12:
		return ci->unknown4.size();

	case 13:
		return ci->unknown5.size();

	case 14:
		return ci->unknown6.size();

	case 15:
		return ci->unknown7.size();
	}
}

Common::String Cast::getLinkedPath(int castId) {
	if (!_castsInfo.contains(castId))
		return Common::String();
	Common::String filename = _castsInfo[castId]->fileName;
	if (filename.empty())
		return Common::String();
	Common::String directory = _castsInfo[castId]->directory;
	if (directory.lastChar() != g_director->_dirSeparator)
		directory += g_director->_dirSeparator;
	return directory + filename;
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
		for (auto &it : digitalVideoCast->_children) {
			if (it.tag == MKTAG('M', 'o', 'o', 'V')) {
				videoId = it.index;
				tag = it.tag;
				break;
			}
		}
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
		res = getLinkedPath(castId);
	} else {
		Video::QuickTimeDecoder qt;
		qt.loadStream(videoData);
		videoData = nullptr;
		res = decodeString(qt.getAliasPath());
		if (res.empty()) {
			warning("STUB: Cast::getVideoPath(%d): unsupported non-alias MooV block found", castId);
		}
	}
	if (videoData)
		delete videoData;

	return res;
}

Common::SeekableReadStreamEndian *Cast::getResource(uint32 tag, uint16 id) {
	if (!_castArchive || !_castArchive->hasResource(tag, id))
		return nullptr;

	return _castArchive->getResource(tag, id);
}

PaletteV4 Cast::loadPalette(Common::SeekableReadStreamEndian &stream, int id) {
	int size = stream.size();
	debugC(3, kDebugLoading, "Cast::loadPalette(): %d bytes", size);
	if (debugChannelSet(5, kDebugLoading))
		stream.hexdump(stream.size());

	int steps = size / 6;
	debugC(3, kDebugLoading, "Cast::loadPalette(): %d steps", steps);

	byte *palette = new byte[steps * 3];

	int colorIndex = 0;
	for (int i = 0; i < steps; i++) {
		if (colorIndex >= steps) {
			warning("Cast::loadPalette(): attempted to set invalid color index %d, aborting", colorIndex);
			break;
		}

		palette[3 * colorIndex] = stream.readByte();
		stream.readByte();

		palette[3 * colorIndex + 1] = stream.readByte();
		stream.readByte();

		palette[3 * colorIndex + 2] = stream.readByte();
		stream.readByte();
		colorIndex += 1;
	}
	PaletteV4 pal(CastMemberID(), palette, steps);
	return pal;
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
		CastMember *target = nullptr;
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

			target = new BitmapCastMember(this, id, stream, tag, _version, flags1);
			break;
		case kCastText:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) TextCastMember", id, numToCastNum(id));
			target = new TextCastMember(this, id, stream, _version, flags1);
			break;
		case kCastShape:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) ShapeCastMember", id, numToCastNum(id));
			target = new ShapeCastMember(this, id, stream, _version);
			break;
		case kCastButton:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) ButtonCast", id, numToCastNum(id));
			target =new TextCastMember(this, id, stream, _version, flags1, true);
			break;
		case kCastSound:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) SoundCastMember", id, numToCastNum(id));
			target = new SoundCastMember(this, id, stream, _version);
			break;
		case kCastDigitalVideo:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) DigitalVideoCastMember", id, numToCastNum(id));
			target = new DigitalVideoCastMember(this, id, stream, _version);
			break;
		case kCastPalette:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) PaletteCastMember", id, numToCastNum(id));
			target = new PaletteCastMember(this, id, stream, _version);
			// load the palette now, as there are no CastInfo structs
			target->load();
			break;
		case kCastFilmLoop:
			debugC(3, kDebugLoading, "Cast::loadCastDataVWCR(): CastTypes id: %d(%s) FilmLoopCastMember", id, numToCastNum(id));
			target = new FilmLoopCastMember(this, id, stream, _version);
			break;
		default:
			warning("Cast::loadCastDataVWCR(): Unhandled cast id: %d(%s), type: %d, %d bytes", id, numToCastNum(id), castType, size);
			break;
		}
		if (target)
			setCastMember(id, target);
		stream.seek(returnPos);
	}
}

void Cast::loadExternalSound(Common::SeekableReadStreamEndian &stream) {
	Common::String str = stream.readString();
	str.trim();
	debugC(1, kDebugLoading, "****** Loading External Sound File %s", str.c_str());
	str = g_director->getCurrentPath() + str;

	Common::Path resPath = findPath(str, true, true, false);

	if (resPath.empty()) {
		warning("Cast::loadExternalSound: could not find external sound file %s", str.c_str());
		return;
	}

	g_director->openArchive(resPath);
}

static void readEditInfo(EditInfo *info, Common::ReadStreamEndian *stream) {
	info->rect = Movie::readRect(*stream);
	info->selStart = stream->readUint32();
	info->selEnd = stream->readUint32();
	info->version = stream->readByte();
	info->rulerFlag = stream->readByte();
	// We're ignoring 2 bytes here
	info->valid = true;
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

	uint32 castDataSize, castInfoSize,  castType, castDataSizeToRead, castDataOffset, castInfoOffset;
	uint8 flags1 = 0xFF;
	byte unk1 = 0, unk2 = 0, unk3 = 0;

	// D2-3 cast members should be loaded in loadCastDataVWCR
#if 0
	if (_version < kFileVer400) {
		size1 = stream.readUint16();
		sizeToRead = size1 +16; // 16 is for bounding rects
		size2 = stream.readUint32();
		castType = stream.readByte();
		unk1 = stream.readByte();
		unk2 = stream.readByte();
		unk3 = stream.readByte();
	}
#endif

	if (_version >= kFileVer400 && _version < kFileVer500) {
		castDataSize = stream.readUint16();
		castDataSizeToRead = castDataSize;
		castInfoSize = stream.readUint32();

		// these bytes are common but included in cast size
		castType = stream.readByte();
		castDataSizeToRead -= 1;
		if (castDataSizeToRead) {
			flags1 = stream.readByte();
			castDataSizeToRead -= 1;
		}
		castDataOffset = stream.pos();
		castInfoOffset = stream.pos() + castDataSizeToRead;
	} else if (_version >= kFileVer500 && _version < kFileVer600) {
		castType = stream.readUint32();
		castInfoSize = stream.readUint32();
		castDataSize = stream.readUint32();
		castDataSizeToRead = castDataSize;
		castInfoOffset = stream.pos();
		castDataOffset = stream.pos() + castInfoSize;
	} else {
		error("Cast::loadCastData: unsupported Director version (%d)", _version);
	}

	debug("Cast::loadCastData(): CASt: id: %d type: %x castDataSize: %d castInfoSize: %d (%x) unk1: %d unk2: %d unk3: %d",
		id, castType, castDataSize, castInfoSize, castInfoSize, unk1, unk2, unk3);

	// read the cast member itself
	byte *data = (byte *)calloc(castDataSizeToRead, 1);
	stream.seek(castDataOffset);
	stream.read(data, castDataSizeToRead);
	Common::MemoryReadStreamEndian castStream(data, castDataSizeToRead, stream.isBE());

	if (_loadedCast->contains(id)) {
		warning("Cast::loadCastData(): Multiple cast members with ID %d, overwriting", id);
		eraseCastMember(id);
	}

	CastMember *target = nullptr;
	switch (castType) {
	case kCastBitmap:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastBitmap (%d children)", res->children.size());
		target = new BitmapCastMember(this, id, castStream, res->tag, _version, flags1);
		break;
	case kCastSound:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastSound (%d children)", res->children.size());
		target = new SoundCastMember(this, id, castStream, _version);
		break;
	case kCastText:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastText (%d children)", res->children.size());
		target = new TextCastMember(this, id, castStream, _version, flags1);
		break;
	case kCastShape:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastShape (%d children)", res->children.size());
		target = new ShapeCastMember(this, id, castStream, _version);
		break;
	case kCastButton:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastButton (%d children)", res->children.size());
		target = new TextCastMember(this, id, castStream, _version, flags1, true);
		break;
	case kCastLingoScript:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastLingoScript");
		target = new ScriptCastMember(this, id, castStream, _version);
		break;
	case kCastRichText:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastRichText (%d children)", res->children.size());
		target = new RichTextCastMember(this, id, castStream, _version);
		break;
	case kCastDigitalVideo:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastDigitalVideo (%d children)", res->children.size());
		target = new DigitalVideoCastMember(this, id, castStream, _version);
		break;
	case kCastFilmLoop:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastFilmLoop (%d children)", res->children.size());
		target = new FilmLoopCastMember(this, id, castStream, _version);
		break;
	case kCastPalette:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastPalette (%d children)", res->children.size());
		target = new PaletteCastMember(this, id, castStream, _version);
		break;
	case kCastPicture:
		warning("BUILDBOT: STUB: Cast::loadCastData(): kCastPicture (id=%d, %d children)! This will be missing from the movie and may cause problems", id, res->children.size());
		castInfoSize = 0;
		break;
	case kCastMovie:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastMovie (id=%d, %d children)",  id, res->children.size());
		target = new MovieCastMember(this, id, castStream, _version);
		break;
	case kCastTransition:
		debugC(3, kDebugLoading, "Cast::loadCastData(): loading kCastTransition (id=%d, %d children)",  id, res->children.size());
		target = new TransitionCastMember(this, id, castStream, _version);
		break;
	default:
		warning("BUILDBOT: STUB: Cast::loadCastData(): Unhandled cast type: %d [%s] (id=%d, %d children)! This will be missing from the movie and may cause problems", castType, tag2str(castType), id, res->children.size());
		// also don't try and read the strings... we don't know what this item is.
		castInfoSize = 0;
		break;
	}
	if (target) {
		target->_castDataSize = castDataSize;
		target->_flags1 = flags1;
		setCastMember(id, target);
	}
	if (castStream.eos()) {
		warning("BUILDBOT: Read past dataStream for id: %d type: %s", id, castType2str((CastType) castType));
	}

	int leftOver = castStream.size() - castStream.pos();
	if (leftOver > 0)
		warning("BUILDBOT: Left over bytes: %d in dataStream for id: %d type: %s", leftOver, id, castType2str((CastType) castType));

	if (target) { // Skip unhandled casts
		debugCN(3, kDebugLoading, "Children: ");
		for (uint child = 0; child < res->children.size(); child++) {
			debugCN(3, kDebugLoading, "%d ", res->children[child].index);
			target->_children.push_back(res->children[child]);
		}
		debugCN(3, kDebugLoading, "\n");
	}

	free(data);

	// read the cast member info
	if (castInfoSize) {
		data = (byte *)calloc(castInfoSize, 1);
		stream.seek(castInfoOffset);
		stream.read(data, castInfoSize);
		Common::MemoryReadStreamEndian castInfoStream(data, castInfoSize, stream.isBE());
		loadCastInfo(castInfoStream, id);
		free(data);
	}
}

struct LingoContextEntry {
	int32 index;
	int16 nextUnused;
	bool unused;

	LingoContextEntry(int32 i, int16 n);
};

LingoContextEntry::LingoContextEntry(int32 i, int16 n)
	: index(i), nextUnused(n), unused(false) {}

class ChunkResolver : public LingoDec::ChunkResolver {
public:
	ChunkResolver(Cast *cast) : _cast(cast) {}
	~ChunkResolver() {
		for (auto &it : _scripts)
			delete it._value;

		for (auto &it : _scriptnames)
			delete it._value;
	}

	virtual LingoDec::Script *getScript(int32 id) {
		if (_scripts.contains(id))
			return _scripts[id];

		Common::SeekableReadStreamEndian *r;

		r = _cast->_castArchive->getResource(MKTAG('L', 's', 'c', 'r'), id);
		_scripts[id] = new LingoDec::Script(g_director->getVersion());
		_scripts[id]->read(*r);
		delete r;

		return _scripts[id];
	}

	virtual LingoDec::ScriptNames *getScriptNames(int32 id) {
		if (_scriptnames.contains(id))
			return _scriptnames[id];

		Common::SeekableReadStreamEndian *r;

		r = _cast->_castArchive->getResource(MKTAG('L', 'n', 'a', 'm'), id);
		_scriptnames[id] = new LingoDec::ScriptNames(_cast->_version);
		_scriptnames[id]->read(*r);
		delete r;

		return _scriptnames[id];
	}

private:
	Cast *_cast;
	Common::HashMap<int32, LingoDec::Script *> _scripts;
	Common::HashMap<int32, LingoDec::ScriptNames *> _scriptnames;
};

void Cast::loadLingoContext(Common::SeekableReadStreamEndian &stream) {
	if (_version >= kFileVer400) {
		debugC(1, kDebugCompile, "Add D4 script context");

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
		for (auto &it : _lingoArchive->lctxContexts) {
			ScriptContext *script = it._value;
			if (script->_id >= 0 && !script->isFactory()) {
				if (_lingoArchive->getScriptContext(script->_scriptType, script->_id)) {
					error("Cast::loadLingoContext: Script already defined for type %s, id %d", scriptType2str(script->_scriptType), script->_id);
				}
				_lingoArchive->scriptContexts[script->_scriptType][script->_id] = script;
				_lingoArchive->patchScriptHandler(script->_scriptType, CastMemberID(script->_id, _castLibID));
			} else {
				// Keep track of scripts that are not in scriptContexts
				// Those scripts need to be cleaned up on ~LingoArchive
				script->setOnlyInLctxContexts();
			}
		}
	} else {
		error("Cast::loadLingoContext: unsupported Director version (%d)", _version);
	}

	if (debugChannelSet(-1, kDebugImGui) || ConfMan.getBool("dump_scripts")) {
		// Rewind stream
		stream.seek(0);
		_chunkResolver = new ChunkResolver(this);
		_lingodec = new LingoDec::ScriptContext(_version, _chunkResolver);
		_lingodec->read(stream);

		_lingodec->parseScripts();

		for (auto it = _lingodec->scripts.begin(); it != _lingodec->scripts.end(); ++it) {
			debugC(9, kDebugCompile, "[%d/%d] %s", it->second->castID, it->first, it->second->scriptText("\n", false).c_str());
		}

		if (ConfMan.getBool("dump_scripts")) {
			for (auto it = _lingodec->scripts.begin(); it != _lingodec->scripts.end(); ++it) {
				Common::DumpFile out;
				ScriptType scriptType = kMovieScript;

				if (_loadedCast->contains(it->second->castID)) {
					CastMember *member = _loadedCast->getVal(it->second->castID);
					if (member && member->_type == kCastLingoScript) {
						scriptType = ((ScriptCastMember *)member)->_scriptType;
					}
				}

				// FIXME: castID is set incorrectly for D5+
				Common::Path lingoPath(dumpScriptName(encodePathForDump(_macName).c_str(), scriptType, it->second->castID, "lingo"));

				if (out.open(lingoPath, true)) {
					Common::String decompiled = it->second->scriptText("\n", true);
					out.writeString(decompiled);
					out.flush();
					out.close();
				}
			}
		}
	}
}

void Cast::loadScriptV2(Common::SeekableReadStreamEndian &stream, uint16 id) {
	// In Director 2 (and Director 3 with compatibility mode), any text cast
	// member which begins with a comment "--" gets parsed as a movie script.
	// In this mode, fewer top-level keywords are recognised; this is indicated
	// by passing kLPPForceD2 to the Lingo preprocessor.

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

	_lingoArchive->addCode(script.decode(Common::kMacRoman), kMovieScript, id, nullptr, kLPPForceD2|kLPPTrimGarbage);
}

void Cast::dumpScript(const char *script, ScriptType type, uint16 id) {
	Common::DumpFile out;
	Common::Path buf(dumpScriptName(encodePathForDump(_macName).c_str(), type, id, "txt"));

	if (!out.open(buf, true)) {
		warning("Cast::dumpScript(): Can not open dump file %s", buf.toString(Common::Path::kNativeSeparator).c_str());
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

	CastMemberInfo *ci = new CastMemberInfo();
	Common::MemoryReadStreamEndian *entryStream;
	CastMember *member = _loadedCast->getVal(id);

	// We need this data while saving the cast information back
	// Is it possible that the count changes?
	ci->unk1 = castInfo.unk1;
	ci->unk2 = castInfo.unk2;
	ci->count = castInfo.strings.size();

	// If possible, we won't store flags
	ci->flags = castInfo.flags;

	// We have here variable number of strings. Thus, instead of
	// adding tons of ifs, we use this switch()
	switch (castInfo.strings.size()) {
	default:
		warning("Cast::loadCastInfo(): BUILDBOT: extra %d strings for castid %d", castInfo.strings.size() - 15, id);
		// fallthrough
	case 15:
		if (castInfo.strings[14].len) {
			warning("Cast::loadCastInfo(): BUILDBOT: string #%d for castid %d", 14, id);
			Common::hexdump(castInfo.strings[14].data, castInfo.strings[14].len);
			ci->unknown7 = Common::Array<byte>(castInfo.strings[14].data, castInfo.strings[14].len);
		}
		// fallthrough
	case 14:
		if (castInfo.strings[13].len) {
			warning("Cast::loadCastInfo(): BUILDBOT: string #%d for castid %d", 13, id);
			Common::hexdump(castInfo.strings[13].data, castInfo.strings[13].len);
			ci->unknown6 = Common::Array<byte>(castInfo.strings[13].data, castInfo.strings[13].len);
		}
		// fallthrough
	case 13:
		if (castInfo.strings[12].len) {
			warning("Cast::loadCastInfo(): BUILDBOT: string #%d for castid %d", 12, id);
			Common::hexdump(castInfo.strings[12].data, castInfo.strings[12].len);
			ci->unknown5 = Common::Array<byte>(castInfo.strings[12].data, castInfo.strings[12].len);
		}
		// fallthrough
	case 12:
		if (castInfo.strings[11].len) {
			warning("Cast::loadCastInfo(): BUILDBOT: string #%d for castid %d", 11, id);
			Common::hexdump(castInfo.strings[11].data, castInfo.strings[11].len);
			ci->unknown4 = Common::Array<byte>(castInfo.strings[11].data, castInfo.strings[11].len);
		}
		// fallthrough
	case 11:
		if (castInfo.strings[10].len) {
			warning("Cast::loadCastInfo(): BUILDBOT: string #%d for castid %d", 11, id);
			Common::hexdump(castInfo.strings[10].data, castInfo.strings[10].len);
			ci->unknown3 =Common::Array<byte>(castInfo.strings[10].data, castInfo.strings[10].len);
		}
		// fallthrough
	case 10:
		if (castInfo.strings[9].len) {
			warning("Cast::loadCastInfo(): BUILDBOT: string #%d for castid %d", 10, id);
			Common::hexdump(castInfo.strings[9].data, castInfo.strings[9].len);
			ci->unknown2 = Common::Array<byte>(castInfo.strings[9].data, castInfo.strings[9].len);
		}
		// fallthrough
	case 9:
		if (castInfo.strings[8].len) {
			warning("Cast::loadCastInfo(): BUILDBOT: string #%d for castid %d", 9, id);
			Common::hexdump(castInfo.strings[8].data, castInfo.strings[8].len);
			ci->unknown1 = Common::Array<byte>(castInfo.strings[8].data, castInfo.strings[8].len);
		}
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
		// fallthrough
	case 1:
		ci->script = castInfo.strings[0].readString(false);
		// fallthrough
	case 0:
		break;
	}
	debugC(4, kDebugLoading, "Cast::loadCastInfo(): castId: %d, size: %d, script: %s, name: %s, directory: %s, fileName: %s, type: %s",
			id, castInfo.strings.size(), ci->script.c_str(), ci->name.c_str(), ci->directory.c_str(), ci->fileName.c_str(), ci->type.c_str());

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
	if (_version >= kFileVer400 && _version < kFileVer600 && member->_type == kCastSound) {
		((SoundCastMember *)member)->_looping = castInfo.flags & 16 ? 0 : 1;
	} else if (_version >= kFileVer600 && member->_type == kCastSound) {
		warning("STUB: Cast::loadCastInfo(): Sound cast member info not yet supported for version %d", _version);
	}

	// For PaletteCastMember, run load() as we need it right now
	if (member->_type == kCastPalette)
		member->load();

	ci->autoHilite = castInfo.flags & 2;
	ci->scriptId = castInfo.scriptId;
	if (ci->scriptId != 0)
		_castsScriptIds[ci->scriptId] = id;

	_castsInfo[id] = ci;
}

void Cast::loadCastLibInfo(Common::SeekableReadStreamEndian &stream, uint16 id) {
	if (debugChannelSet(8, kDebugLoading)) {
		stream.hexdump(stream.size());
	}
	debugC(5, kDebugLoading, "Cast::loadCastLibInfo(): %d", id);
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
	uint16 castLibId = DEFAULT_CAST_LIB; // default for pre-D5
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
	uint16 castLibId = DEFAULT_CAST_LIB; // default for pre-D5
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

Common::String Cast::formatCastSummary(int castId = -1) {
	Common::String result;
	Common::Array<int> castIds;
	for (auto it = _loadedCast->begin(); it != _loadedCast->end(); ++it) {
		castIds.push_back(it->_key);
	}
	Common::sort(castIds.begin(), castIds.end());
	for (auto it = castIds.begin(); it != castIds.end(); ++it) {
		if (castId > -1 &&  *it != castId)
			continue;
		CastMember *castMember = getCastMember(*it, false);
		CastMemberInfo *castMemberInfo = getCastMemberInfo(*it);
		Common::String info = castMember->formatInfo();
		result += Common::String::format("%5d", *it);
		if (_version < kFileVer400) {
			result += Common::String::format(" (%s)", numToCastNum(*it));
		}
		result += Common::String::format(": type=%s, name=\"%s\"",
			castType2str(castMember->_type),
			castMemberInfo ? castMemberInfo->name.c_str() : ""
		);

		if (castMemberInfo) {
			if (!castMemberInfo->fileName.empty())
				result += ", filename=\"" + castMemberInfo->directory + g_director->_dirSeparator + castMemberInfo->fileName + "\"";
			if (!castMemberInfo->script.empty())
				result += ", script=\"" + formatStringForDump(castMemberInfo->script) + "\"";
		}

		if (!info.empty()) {
			result += ", ";
			result += info;
		}
		result += "\n";
	}
	return result;
}

void Cast::rebuildCastNameCache() {
	_castsNames.clear();
	for (auto &it : _castsInfo) {
		if (!it._value->name.empty()) {
			// Multiple casts can have the same name. In director only the earliest one is used for lookups.
			if (!_castsNames.contains(it._value->name) || (_castsNames.getVal(it._value->name) > it._key)) {
				_castsNames[it._value->name] = it._key;
			}

			// Store name with type
			CastMember *member = _loadedCast->getVal(it._key);
			Common::String cname = Common::String::format("%s:%d", it._value->name.c_str(), member->_type);
			if (!_castsNames.contains(cname) || (_castsNames.getVal(cname) > it._key)) {
				_castsNames[cname] = it._key;
			} else {
				debugC(4, kDebugLoading, "Cast::rebuildCastNameCache(): duplicate cast name: %s for castIDs: %d %d ", cname.c_str(), it._key, _castsNames[it._value->name]);
			}
		}
	}
}

} // End of namespace Director
