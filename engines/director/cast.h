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

#ifndef DIRECTOR_CAST_H
#define DIRECTOR_CAST_H

#include "common/hash-str.h"

namespace Common {
	class ReadStreamEndian;
	class MemoryWriteStream;
	struct Rect;
	class SeekableReadStreamEndian;
}

namespace LingoDec {
	class ChunkResolver;
	struct ScriptContext;
}

namespace Director {

class Archive;
struct CastMemberInfo;
class CastMember;
class DirectorEngine;
class Lingo;
struct LingoArchive;
struct Resource;
class Stxt;
class RTE0;
class RTE1;
class RTE2;
class BitmapCastMember;
class FilmLoopCastMember;
class ScriptCastMember;
class ShapeCastMember;
class TextCastMember;
class PaletteCastMember;
class SoundCastMember;
struct ConfigChunk;

typedef Common::HashMap<byte, byte> CharMap;
typedef Common::HashMap<uint16, uint16> FontSizeMap;
struct FontXPlatformInfo {
	Common::String toFont;
	bool remapChars;
	FontSizeMap sizeMap;

	FontXPlatformInfo() : remapChars(false) {}
};
typedef Common::HashMap<Common::String, FontXPlatformInfo *> FontXPlatformMap;

struct FontMapEntry {
	uint16 toFont;
	bool remapChars;
	FontSizeMap sizeMap;

	FontMapEntry() : toFont(0), remapChars(false) {}
};
typedef Common::HashMap<uint16, FontMapEntry *> FontMap;

#define NUMTILEPATTERNS 8
struct TilePatternEntry {
	CastMemberID bitmapId;
	Common::Rect rect;
};

class Cast {
public:
	Cast(Movie *movie, uint16 castLibID, bool isShared = false, bool isExternal = false, uint16 libResourceId = 1024);
	~Cast();

	void loadArchive();
	void setArchive(Archive *archive);
	Archive *getArchive() const { return _castArchive; };
	Common::String getMacName() const { return _macName; }
	Common::String getCastName() const { return _castName; }
	void setCastName(const Common::String &name) { _castName = name; }

	bool loadConfig();
	void loadCast();
	void loadCastDataVWCR(Common::SeekableReadStreamEndian &stream);
	void loadCastData(Common::SeekableReadStreamEndian &stream, uint16 id, Resource *res);
	void loadCastInfo(Common::SeekableReadStreamEndian &stream, uint16 id);
	void loadCastLibInfo(Common::SeekableReadStreamEndian &stream, uint16 id);
	void loadLingoContext(Common::SeekableReadStreamEndian &stream);
	void loadExternalSound(Common::SeekableReadStreamEndian &stream);
	void loadSord(Common::SeekableReadStreamEndian &stream);
	bool importFileInto(int castId, const Common::Path &path);

	void saveConfig(Common::SeekableWriteStream *writeStream, uint32 offset);
	void saveCastData(Common::SeekableWriteStream *writeStream, Resource *res);
	void saveCastData();
	void writeCastInfo(Common::SeekableWriteStream *writeStream, uint32 castId);
	uint32 getCastInfoSize(uint32 castId);
	uint32 getCastInfoStringLength(uint32 stringIndex, CastMemberInfo *ci);

	uint32 getConfigSize();

	int getCastSize();
	int getCastMaxID();
	int getNextUnusedID();
	Common::Rect getCastMemberInitialRect(int castId);
	void setCastMemberModified(int castId);
	CastMember *setCastMember(int castId, CastMember *cast);
	bool duplicateCastMember(CastMember *source, CastMemberInfo *info, int targetId);
	bool eraseCastMember(int castId);
	CastMember *getCastMember(int castId, bool load = true);
	CastMember *getCastMemberByNameAndType(const Common::String &name, CastType type);
	CastMember *getCastMemberByScriptId(int scriptId);
	int getCastIdByScriptId(uint32 scriptId) const;
	CastMemberInfo *getCastMemberInfo(int castId);
	const Stxt *getStxt(int castId);
	Common::String getLinkedPath(int castId);
	Common::String getVideoPath(int castId);
	Common::SeekableReadStreamEndian *getResource(uint32 tag, uint16 id);
	void rebuildCastNameCache();

	// release all castmember's widget, should be called when we are changing movie.
	// because widget is handled by channel, thus we should clear all of those run-time info when we are switching the movie. (because we will create new widgets for cast)
	void releaseCastMemberWidget();

	void dumpScript(const char *script, ScriptType type, uint16 id);

	Common::CodePage getFileEncoding();
	Common::U32String decodeString(const Common::String &str);

	Common::String formatCastSummary(int castId);
	PaletteV4 loadPalette(Common::SeekableReadStreamEndian &stream, int id);

private:
	void loadScriptV2(Common::SeekableReadStreamEndian &stream, uint16 id);
	void loadFontMap(Common::SeekableReadStreamEndian &stream);
	void loadFontMapV4(Common::SeekableReadStreamEndian &stream);
	void loadFXmp(Common::SeekableReadStreamEndian &stream);
	bool readFXmpLine(Common::SeekableReadStreamEndian &stream);
	void loadVWTL(Common::SeekableReadStreamEndian &stream);

	uint32 computeChecksum();

public:
	Archive *_castArchive;
	Common::Platform _platform;
	uint16 _castLibID;
	uint16 _libResourceId;
	bool _isExternal;

	CharMap _macCharsToWin;
	CharMap _winCharsToMac;
	FontXPlatformMap _fontXPlatformMap;
	FontMap _fontMap;

	bool _isProtected;

	Common::HashMap<int, CastMember *> *_loadedCast;
	Common::HashMap<int, const Stxt *> _loadedStxts;
	Common::HashMap<uint, const RTE0 *> _loadedRTE0s;
	Common::HashMap<uint, const RTE1 *> _loadedRTE1s;
	Common::HashMap<uint, const RTE2 *> _loadedRTE2s;
	uint16 _castIDoffset;

	Common::Rect _movieRect;
	TilePatternEntry _tiles[kNumBuiltinTiles];

	LingoArchive *_lingoArchive;

	LingoDec::ScriptContext *_lingodec = nullptr;
	LingoDec::ChunkResolver *_chunkResolver = nullptr;

	/* Config Data to be saved */
	/*  0 */ uint16 _len;
	/*  2 */ uint16 _fileVersion;
	/*  4, 6, 8, 10 */ Common::Rect _checkRect;
	/* 12 */ uint16 _castArrayStart;
	/* 14 */ uint16 _castArrayEnd;
	/* 16 */ byte _readRate;
	/* 17 */ byte _lightswitch;

	// Director 6 and below
		/* 18 */ int16 _unk1;	// Mentioned in ProjectorRays as preD7field11

	// Director 7 and above
	// Currently not supporting Director 7
		// /* 18 */ int8 D7stageColorG;
		// /* 19 */ int8 D7stageColorB;

	/* 20 */ uint16 _commentFont;
	/* 22 */ uint16 _commentSize;
	/* 24 */ uint16 _commentStyle;

	// Director 6 and below
		/* 26 */ uint16 _stageColor;
	// Director 7 and above
	// Currently not supporting Director 7
		// /* 26 */ uint8 D7stageColorIsRGB;
		// /* 27 */ uint8 D7stageColorR;

	/* 28 */ uint16 _bitdepth;
	/* 30 */ uint8 _field17;
	/* 31 */ uint8 _field18;
	/* 32 */ int32 _field19;
	/* 36 */ int16 _version;
	/* 38 */ int16 _movieDepth;
	/* 40 */ int32 _field22;
	/* 44 */ int32 _field23;
	/* 48 */ int32 _field24;
	/* 52 */ int8 _field25;
	/* 53 */ uint8 _field26;
	/* 54 */ int16 _frameRate;
	/* 56 */ int16 _platformID;
	/* 58 */ int16 _protection;
	/* 60 */ int32 _field29;
	/* 64 */ uint32 _checksum;
	/* 68 */ uint16 _field30;	// Marked as remnants in ProjectorRays
	/* 70 */ uint16 _defPaletteNum = 0; // _defaultPalette before D5, unused later
	/* 72 */ uint32 _chunkBaseNum = 0;
	/* 76 */ CastMemberID _defaultPalette;

	/****** D6.0-D8.5 *******/
	/* 80 */ int8 _netUnk1 = 0;
	/* 81 */ int8 _netUnk2 = 0;
	/* 82 */ int16 _netPreloadNumFrames = 0;

	/****** post D9-D10 *******/
	/* 84 */ uint32 _windowFlags = 0;
	/* 88 */ CastMemberID _windowIconId;
	/* 92 */ CastMemberID _windowMaskId;
	/* 96 */ CastMemberID _windowDragRegionMaskId;

	/* 100 */ // End of config

private:
	DirectorEngine *_vm;
	Lingo *_lingo;
	Movie *_movie;

	bool _isShared;
	bool _loadMutex;
	Common::Array<CastMember *> _loadQueue;

	Common::String _macName;
	Common::String _castName;

	Common::HashMap<uint16, CastMemberInfo *> _castsInfo;
	Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _castsNames;
	Common::HashMap<uint16, int> _castsScriptIds;

};

} // End of namespace Director

#endif
