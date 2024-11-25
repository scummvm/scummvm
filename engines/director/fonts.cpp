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

#include "common/substream.h"

#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/cast.h"

namespace Director {

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

		// Map cast font ID to window manager font ID
		FontMapEntry *entry = new FontMapEntry;
		entry->toFont = _vm->_wm->_fontMan->registerFontName(font, id);
		_fontMap[id] = entry;

		debugC(3, kDebugLoading, "Cast::loadFontMap: Mapping font %d (%s) to %d", id, font.c_str(), _fontMap[id]->toFont);
		currentRawPosition = stream.pos();
		stream.seek(positionInfo);
	}
}

void Cast::loadFontMapV4(Common::SeekableReadStreamEndian &stream) {
	if (stream.size() == 0)
		return;

	debugC(2, kDebugLoading, "****** Loading FontMap Fmap");

	uint32 mapLength = stream.readUint32();
	/* uint32 namesLength = */ stream.readUint32();
	uint32 bodyStart = stream.pos();
	uint32 namesStart = bodyStart + mapLength;

	/* uint32 unk1 = */ stream.readUint32();
	/* uint32 unk2 = */ stream.readUint32();
	uint32 entriesUsed = stream.readUint32();
	/* uint32 entriesTotal = */ stream.readUint32();
	/* uint32 unk3 = */ stream.readUint32();
	/* uint32 unk4 = */ stream.readUint32();
	/* uint32 unk5 = */ stream.readUint32();

	for (uint32 i = 0; i < entriesUsed; i++) {
		uint32 nameOffset = stream.readUint32();

		uint32 returnPos = stream.pos();
		stream.seek(namesStart + nameOffset);
		uint32 nameLength = stream.readUint32();
		Common::String name = stream.readString(0, nameLength);
		stream.seek(returnPos);

		Common::Platform platform = platformFromID(stream.readUint16());
		uint16 id = stream.readUint16();

		// Map cast font ID to window manager font ID
		FontMapEntry *entry = new FontMapEntry;
		if (platform == Common::kPlatformWindows && _fontXPlatformMap.contains(name)) {
			FontXPlatformInfo *xinfo = _fontXPlatformMap[name];
			entry->toFont = _vm->_wm->_fontMan->registerFontName(xinfo->toFont, id);
			entry->remapChars = xinfo->remapChars;
			entry->sizeMap = xinfo->sizeMap;
		} else {
			entry->toFont = _vm->_wm->_fontMan->registerFontName(name, id);
		}
		_fontMap[id] = entry;

		debugC(3, kDebugLoading, "Cast::loadFontMapV4: Mapping %s font %d (%s) to %d", getPlatformAbbrev(platform), id, name.c_str(), _fontMap[id]->toFont);
	}
}

enum FXmpTokenType {
	FXMP_TOKEN_WORD,
	FXMP_TOKEN_INT,
	FXMP_TOKEN_STRING,
	FXMP_TOKEN_COLON,
	FXMP_TOKEN_ARROW,
	FXMP_TOKEN_NEWLINE,
	FXMP_TOKEN_EOF,
	FXMP_TOKEN_ERROR
};

const char *const FXmpTokenTypeStrings[] = {
	"WORD",
	"INT",
	"STRING",
	"COLON",
	"ARROW",
	"NEWLINE",
	"EOF",
	"ERROR"
};

struct FXmpToken {
	FXmpTokenType type;
	Common::String str;
};

FXmpToken readFXmpToken(Common::SeekableReadStreamEndian &stream) {
	FXmpToken res;
	res.type = FXMP_TOKEN_ERROR;
	res.str = "";

	char ch = stream.readByte();

	// skip non-newline whitespace
	while (!stream.eos() && (ch == ' ' || ch == '\t' || ch == '\v' || ch == '\f')) {
		ch = stream.readByte();
	}
	if (stream.eos()) {
		res.type = FXMP_TOKEN_EOF;
		return res;
	}

	// skip comment
	bool foundComment = false;
	if (ch == ';') {
		foundComment = true;
	} else if (ch == '-') {
		ch = stream.readByte();
		if (stream.eos()) {
			res.type = FXMP_TOKEN_ERROR;
			warning("BUILDBOT: readFXmpToken: Expected '-' but got EOF");
			return res;
		}
		if (ch != '-') {
			res.type = FXMP_TOKEN_ERROR;
			warning("BUILDBOT: readFXmpToken: Expected '-' but got '%c'", ch);
			return res;
		}
		foundComment = true;
	}
	if (foundComment) {
		while (!stream.eos() && ch != '\r') {
			ch = stream.readByte();
		}
		if (stream.eos()) {
			res.type = FXMP_TOKEN_EOF;
			return res;
		}
	}

	if (Common::isAlpha(ch)) {
		res.type = FXMP_TOKEN_WORD;
		do {
			res.str += ch;
			ch = stream.readByte();
		} while (!stream.eos() && Common::isAlpha(ch));
		if (!stream.eos()) {
			stream.seek(-1, SEEK_CUR);
		}
	} else if (Common::isDigit(ch)) {
		res.type = FXMP_TOKEN_INT;
		do {
			res.str += ch;
			ch = stream.readByte();
		} while (!stream.eos() && Common::isDigit(ch));
		if (!stream.eos()) {
			stream.seek(-1, SEEK_CUR);
		}
	} else if (ch == '"') {
		res.type = FXMP_TOKEN_STRING;
		ch = stream.readByte();
		do {
			res.str += ch;
			ch = stream.readByte();
		} while (!stream.eos() && ch != '"');
		if (stream.eos()) {
			res.type = FXMP_TOKEN_ERROR;
			warning("BUILDBOT: readFXmpToken: Expected '\"' but got EOF");
		}
	} else if (ch == ':') {
		res.type = FXMP_TOKEN_COLON;
		res.str += ch;
	} else if (ch == '=') {
		res.str += ch;
		ch = stream.readByte();
		if (stream.eos()) {
			res.type = FXMP_TOKEN_ERROR;
			warning("BUILDBOT: readFXmpToken: Expected '>' but got EOF");
		} else {
			res.str += ch;
			if (ch == '>') {
				res.type = FXMP_TOKEN_ARROW;
			} else {
				res.type = FXMP_TOKEN_ERROR;
				warning("BUILDBOT: readFXmpToken: Expected '>' but got '%c'", ch);
			}
		}
	} else if (ch == '\r') {
		res.type = FXMP_TOKEN_NEWLINE;
		res.str += ch;
		ch = stream.readByte();
		if (!stream.eos()) {
			if (ch == '\n') {
				res.str += ch;
			} else {
				stream.seek(-1, SEEK_CUR);
			}
		}
	} else {
		res.type = FXMP_TOKEN_ERROR;
		warning("BUILDBOT: readFXmpToken: Unexpected '%c'", ch);
	}

	return res;
}

void Cast::loadFXmp(Common::SeekableReadStreamEndian &stream) {
	debugC(2, kDebugLoading, "****** Loading cross-platform font map FXmp");
	while (readFXmpLine(stream)) {}
}

bool Cast::readFXmpLine(Common::SeekableReadStreamEndian &stream) {
	// return true to keep reading after this line, false to stop

	FXmpToken tok = readFXmpToken(stream);
	if (tok.type == FXMP_TOKEN_NEWLINE) {
		return true;
	}
	if (tok.type == FXMP_TOKEN_EOF) {
		return false;
	}

	// from
	Common::Platform fromPlatform;
	if (tok.type != FXMP_TOKEN_WORD) {
		warning("BUILDBOT: Cast::readFXmpLine: Expected WORD, got %s", FXmpTokenTypeStrings[tok.type]);
		return false;
	}
	if (tok.str.equalsIgnoreCase("Mac")) {
		fromPlatform = Common::kPlatformMacintosh;
	} else if (tok.str.equalsIgnoreCase("Win")) {
		fromPlatform = Common::kPlatformWindows;
	} else {
		warning("BUILDBOT: Cast::readFXmpLine: Expected 'Mac' or 'Win', got '%s'", tok.str.c_str());
		return false;
	}

	tok = readFXmpToken(stream);
	if (tok.type != FXMP_TOKEN_COLON) {
		warning("BUILDBOT: Cast::readFXmpLine: Expected COLON, got %s", FXmpTokenTypeStrings[tok.type]);
		return false;
	}

	Common::String fromFont;
	tok = readFXmpToken(stream);
	if (tok.type == FXMP_TOKEN_WORD || tok.type == FXMP_TOKEN_STRING) {
		fromFont = tok.str;
		tok = readFXmpToken(stream);
	}

	// arrow
	if (tok.type != FXMP_TOKEN_ARROW) {
		warning("BUILDBOT: Cast::readFXmpLine: Expected ARROW, got %s", FXmpTokenTypeStrings[tok.type]);
		return false;
	}

	// to
	tok = readFXmpToken(stream);
	if (tok.type != FXMP_TOKEN_WORD) {
		warning("BUILDBOT: Cast::readFXmpLine: Expected WORD, got %s", FXmpTokenTypeStrings[tok.type]);
		return false;
	}
	if (fromPlatform == Common::kPlatformMacintosh && !tok.str.equalsIgnoreCase("Win")) {
		warning("BUILDBOT: Cast::readFXmpLine: Expected 'Win', got '%s'", tok.str.c_str());
		return false;
	}
	if (fromPlatform == Common::kPlatformWindows && !tok.str.equalsIgnoreCase("Mac")) {
		warning("BUILDBOT: Cast::readFXmpLine: Expected 'Mac', got '%s'", tok.str.c_str());
		return false;
	}

	tok = readFXmpToken(stream);
	if (tok.type != FXMP_TOKEN_COLON) {
		warning("BUILDBOT: Cast::readFXmpLine: Expected COLON, got %s", FXmpTokenTypeStrings[tok.type]);
		return false;
	}

	if (fromFont.empty()) {
		// character mappings
		tok = readFXmpToken(stream);
		while (tok.type != FXMP_TOKEN_NEWLINE && tok.type != FXMP_TOKEN_EOF) {
			if (tok.type != FXMP_TOKEN_INT) {
				warning("BUILDBOT: Cast::readFXmpLine: Expected INT, got %s", FXmpTokenTypeStrings[tok.type]);
				return false;
			}
			byte fromChar = atoi(tok.str.c_str());

			tok = readFXmpToken(stream);
			if (tok.type != FXMP_TOKEN_ARROW) {
				warning("BUILDBOT: Cast::readFXmpLine: Expected ARROW, got %s", FXmpTokenTypeStrings[tok.type]);
				return false;
			}

			tok = readFXmpToken(stream);
			if (tok.type != FXMP_TOKEN_INT) {
				warning("BUILDBOT: Cast::readFXmpLine: Expected INT, got %s", FXmpTokenTypeStrings[tok.type]);
				return false;
			}
			byte toChar = atoi(tok.str.c_str());

			if (fromPlatform == Common::kPlatformMacintosh) {
				_macCharsToWin[fromChar] = toChar;
				debugC(3, kDebugLoading, "Cast::readFXmpLine: Mapping Mac char %d to Win char %d", fromChar, toChar);
			} else {
				_winCharsToMac[fromChar] = toChar;
				debugC(3, kDebugLoading, "Cast::readFXmpLine: Mapping Win char %d to Mac char %d", fromChar, toChar);
			}

			tok = readFXmpToken(stream);
		}
	} else {
		// font mapping
		FontXPlatformInfo *info = new FontXPlatformInfo;

		// to font
		tok = readFXmpToken(stream);
		if (tok.type != FXMP_TOKEN_WORD && tok.type != FXMP_TOKEN_STRING) {
			warning("BUILDBOT: Cast::readFXmpLine: Expected WORD or STRING, got %s", FXmpTokenTypeStrings[tok.type]);
			delete info;
			return false;
		}
		info->toFont = tok.str;

		tok = readFXmpToken(stream);

		// remap characters?
		info->remapChars = true;
		if (tok.type == FXMP_TOKEN_WORD) {
			if (!tok.str.equalsIgnoreCase("Map")) {
				warning("BUILDBOT: Cast::readFXmpLine: Expected 'Map', got '%s'", tok.str.c_str());
				delete info;
				return false;
			}

			tok = readFXmpToken(stream);
			if (tok.str.equalsIgnoreCase("All")) {
				info->remapChars = true;
			} else if (tok.str.equalsIgnoreCase("None")) {
				info->remapChars = false;
			} else {
				warning("BUILDBOT: Cast::readFXmpLine: Expected 'All' or 'None', got '%s'", tok.str.c_str());
				delete info;
				return false;
			}

			tok = readFXmpToken(stream);
		}

		// size mappings
		while (tok.type != FXMP_TOKEN_NEWLINE && tok.type != FXMP_TOKEN_EOF) {
			if (tok.type != FXMP_TOKEN_INT) {
				warning("BUILDBOT: Cast::readFXmpLine: Expected INT, got %s", FXmpTokenTypeStrings[tok.type]);
				delete info;
				return false;
			}
			uint16 fromSize = atoi(tok.str.c_str());

			tok = readFXmpToken(stream);
			if (tok.type != FXMP_TOKEN_ARROW) {
				warning("BUILDBOT: Cast::readFXmpLine: Expected ARROW, got %s", FXmpTokenTypeStrings[tok.type]);
				delete info;
				return false;
			}

			tok = readFXmpToken(stream);
			if (tok.type != FXMP_TOKEN_INT) {
				warning("BUILDBOT: Cast::readFXmpLine: Expected INT, got %s", FXmpTokenTypeStrings[tok.type]);
				delete info;
				return false;
			}
			uint16 toSize = atoi(tok.str.c_str());

			info->sizeMap[fromSize] = toSize;

			tok = readFXmpToken(stream);
		}

		// TODO: We should fill _fontXPlatformMap with mappings matching the current platform.
		// We only have Mac fonts right now, though, so we'll always use the Win => Mac mappings.
		// We only handle one fontmap per fromFont (happens in Clone Ranger)
		if (fromPlatform == Common::kPlatformWindows) {
			if (_fontXPlatformMap.contains(fromFont)) {
				warning("Cast::readFxmpLine: Skip second map for font '%s'", fromFont.c_str());
				delete info;
			} else {
				_fontXPlatformMap[fromFont] = info;
				debugC(3, kDebugLoading, "Cast::readFXmpLine: Mapping Win font '%s' to Mac font '%s'", fromFont.c_str(), info->toFont.c_str());
				debugC(4, kDebugLoading, "  Remap characters: %d", info->remapChars);
				for (auto &it : info->sizeMap) {
					debugC(4, kDebugLoading, "  Mapping size %d to %d", it._key, it._value);
				}
			}
		} else {
			delete info;
		}
	}

	return true;
}

} // End of namespace Director
