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

#ifndef DIRECTOR_CAST_H
#define DIRECTOR_CAST_H

#include "common/hash-str.h"

namespace Common {
	class ReadStreamEndian;
	struct Rect;
	class SeekableSubReadStreamEndian;
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
class ScriptCastMember;
class ShapeCastMember;
class TextCastMember;

class Cast {
public:
	Cast(Movie *movie, bool shared = false);
	~Cast();

	bool loadArchive();
	void setArchive(Archive *archive);
	Archive *getArchive() const { return _castArchive; };

	void loadConfig(Common::SeekableSubReadStreamEndian &stream);
	void loadCastDataVWCR(Common::SeekableSubReadStreamEndian &stream);
	void loadCastData(Common::SeekableSubReadStreamEndian &stream, uint16 id, Resource *res);
	void loadCastInfo(Common::SeekableSubReadStreamEndian &stream, uint16 id);
	void loadLingoNames(Common::SeekableSubReadStreamEndian &stream);
	void loadLingoContext(Common::SeekableSubReadStreamEndian &stream);

	void loadSpriteImages();
	void loadSpriteSounds();

	void copyCastStxts();
	void createCastWidgets();
	Common::Rect getCastMemberInitialRect(int castId);
	void setCastMemberModified(int castId);
	CastMember *getCastMember(int castId);
	CastMember *getCastMemberByName(const Common::String &name);
	CastMemberInfo *getCastMemberInfo(int castId);
	const Stxt *getStxt(int castId);

	void dumpScript(const char *script, ScriptType type, uint16 id);

private:
	void loadPalette(Common::SeekableSubReadStreamEndian &stream);
	void loadScriptText(Common::SeekableSubReadStreamEndian &stream);
	void loadFontMap(Common::SeekableSubReadStreamEndian &stream);
	Common::String getString(Common::String str);

public:
	Archive *_castArchive;
	Common::HashMap<uint16, Common::String> _fontMap;

	Common::HashMap<int, CastMember *> *_loadedCast;
	Common::HashMap<int, const Stxt *> *_loadedStxts;
	uint16 _castIDoffset;
	uint16 _castArrayStart;
	uint16 _castArrayEnd;

	uint16 _movieScriptCount;
	LingoArchive *_lingoArchive;

private:
	DirectorEngine *_vm;
	Lingo *_lingo;
	Movie *_movie;

	bool _isShared;

	Common::String _macName;

	Common::HashMap<uint16, CastMemberInfo *> _castsInfo;
	Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _castsNames;
};

} // End of namespace Director

#endif
