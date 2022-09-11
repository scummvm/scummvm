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
	struct Rect;
	class SeekableReadStreamEndian;
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
class BitmapCastMember;
class FilmLoopCastMember;
class ScriptCastMember;
class ShapeCastMember;
class TextCastMember;
class PaletteCastMember;
class SoundCastMember;

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
	Cast(Movie *movie, uint16 castLibID, bool shared = false);
	~Cast();

	void loadArchive();
	void setArchive(Archive *archive);
	Archive *getArchive() const { return _castArchive; };
	Common::String getMacName() const { return _macName; }

	bool loadConfig();
	void loadCast();
	void loadCastDataVWCR(Common::SeekableReadStreamEndian &stream);
	void loadCastData(Common::SeekableReadStreamEndian &stream, uint16 id, Resource *res);
	void loadCastInfo(Common::SeekableReadStreamEndian &stream, uint16 id);
	void loadLingoContext(Common::SeekableReadStreamEndian &stream);
	void loadExternalSound(Common::SeekableReadStreamEndian &stream);
	void loadSord(Common::SeekableReadStreamEndian &stream);

	void loadCastMemberData();
	void loadStxtData(int key, TextCastMember *member);
	void loadPaletteData(PaletteCastMember *member, Common::HashMap<int, PaletteV4>::iterator p);
	void loadFilmLoopData(FilmLoopCastMember *member);
	void loadBitmapData(int key, BitmapCastMember *bitmapCast);
	void loadSoundData(int key, SoundCastMember *soundCast);

	int getCastSize();
	Common::Rect getCastMemberInitialRect(int castId);
	void setCastMemberModified(int castId);
	CastMember *setCastMember(CastMemberID castId, CastMember *cast);
	bool eraseCastMember(CastMemberID castId);
	CastMember *getCastMember(int castId);
	CastMember *getCastMemberByName(const Common::String &name);
	CastMember *getCastMemberByScriptId(int scriptId);
	CastMemberInfo *getCastMemberInfo(int castId);
	const Stxt *getStxt(int castId);
	Common::String getVideoPath(int castId);

	// release all castmember's widget, should be called when we are changing movie.
	// because widget is handled by channel, thus we should clear all of those run-time info when we are switching the movie. (because we will create new widgets for cast)
	void releaseCastMemberWidget();

	void dumpScript(const char *script, ScriptType type, uint16 id);
	PaletteV4 loadPalette(Common::SeekableReadStreamEndian &stream);

	Common::CodePage getFileEncoding();
	Common::U32String decodeString(const Common::String &str);

private:
	void loadScriptText(Common::SeekableReadStreamEndian &stream, uint16 id);
	void loadFontMap(Common::SeekableReadStreamEndian &stream);
	void loadFontMapV4(Common::SeekableReadStreamEndian &stream);
	void loadFXmp(Common::SeekableReadStreamEndian &stream);
	bool readFXmpLine(Common::SeekableReadStreamEndian &stream);
	void loadVWTL(Common::SeekableReadStreamEndian &stream);

public:
	Archive *_castArchive;
	uint16 _version;
	Common::Platform _platform;
	uint16 _castLibID;

	CharMap _macCharsToWin;
	CharMap _winCharsToMac;
	FontXPlatformMap _fontXPlatformMap;
	FontMap _fontMap;

	Common::HashMap<int, CastMember *> *_loadedCast;
	Common::HashMap<int, const Stxt *> *_loadedStxts;
	uint16 _castIDoffset;
	uint16 _castArrayStart;
	uint16 _castArrayEnd;

	Common::Rect _movieRect;
	uint16 _stageColor;
	int _defaultPalette;
	TilePatternEntry _tiles[kNumBuiltinTiles];

	LingoArchive *_lingoArchive;

private:
	DirectorEngine *_vm;
	Lingo *_lingo;
	Movie *_movie;

	bool _isShared;

	Common::String _macName;

	Common::HashMap<uint16, CastMemberInfo *> _castsInfo;
	Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _castsNames;
	Common::HashMap<uint16, int> _castsScriptIds;
};

} // End of namespace Director

#endif
