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

#include "common/file.h"
#include "common/keyboard.h"
#include "common/memstream.h"
#include "common/zlib.h"

#include "director/director.h"
#include "director/util.h"

namespace Director {

static struct KeyCodeMapping {
	Common::KeyCode scummvm;
	int mac;
} keyCodeMappings[] = {
	{ Common::KEYCODE_ESCAPE,		53 },
	{ Common::KEYCODE_F1,			122 },
	{ Common::KEYCODE_F2,			120 },
	{ Common::KEYCODE_F3,			99 },
	{ Common::KEYCODE_F4,			118 },
	{ Common::KEYCODE_F5,			96 },
	{ Common::KEYCODE_F6,			97 },
	{ Common::KEYCODE_F7,			98 },
	{ Common::KEYCODE_F8,			100 },
	{ Common::KEYCODE_F9,			101 },
	{ Common::KEYCODE_F10,			109 },
	{ Common::KEYCODE_F11,			103 },
	{ Common::KEYCODE_F12,			111 },
	{ Common::KEYCODE_F13,			105 }, // mirrored by print
	{ Common::KEYCODE_F14,			107 }, // mirrored by scroll lock
	{ Common::KEYCODE_F15,			113 }, // mirrored by pause

	{ Common::KEYCODE_BACKQUOTE,	10 },
	{ Common::KEYCODE_1,			18 },
	{ Common::KEYCODE_2,			19 },
	{ Common::KEYCODE_3,			20 },
	{ Common::KEYCODE_4,			21 },
	{ Common::KEYCODE_5,			23 },
	{ Common::KEYCODE_6,			22 },
	{ Common::KEYCODE_7,			26 },
	{ Common::KEYCODE_8,			28 },
	{ Common::KEYCODE_9,			25 },
	{ Common::KEYCODE_0,			29 },
	{ Common::KEYCODE_MINUS,		27 },
	{ Common::KEYCODE_EQUALS,		24 },
	{ Common::KEYCODE_BACKSPACE,	51 },

	{ Common::KEYCODE_TAB,			48 },
	{ Common::KEYCODE_q,			12 },
	{ Common::KEYCODE_w,			13 },
	{ Common::KEYCODE_e,			14 },
	{ Common::KEYCODE_r,			15 },
	{ Common::KEYCODE_t,			17 },
	{ Common::KEYCODE_y,			16 },
	{ Common::KEYCODE_u,			32 },
	{ Common::KEYCODE_i,			34 },
	{ Common::KEYCODE_o,			31 },
	{ Common::KEYCODE_p,			35 },
	{ Common::KEYCODE_LEFTBRACKET,	33 },
	{ Common::KEYCODE_RIGHTBRACKET,	30 },
	{ Common::KEYCODE_BACKSLASH,	42 },

	{ Common::KEYCODE_CAPSLOCK,		57 },
	{ Common::KEYCODE_a,			0 },
	{ Common::KEYCODE_s,			1 },
	{ Common::KEYCODE_d,			2 },
	{ Common::KEYCODE_f,			3 },
	{ Common::KEYCODE_g,			5 },
	{ Common::KEYCODE_h,			4 },
	{ Common::KEYCODE_j,			38 },
	{ Common::KEYCODE_k,			40 },
	{ Common::KEYCODE_l,			37 },
	{ Common::KEYCODE_SEMICOLON,	41 },
	{ Common::KEYCODE_QUOTE,		39 },
	{ Common::KEYCODE_RETURN,		36 },

	{ Common::KEYCODE_LSHIFT,		56 },
	{ Common::KEYCODE_z,			6 },
	{ Common::KEYCODE_x,			7 },
	{ Common::KEYCODE_c,			8 },
	{ Common::KEYCODE_v,			9 },
	{ Common::KEYCODE_b,			11 },
	{ Common::KEYCODE_n,			45 },
	{ Common::KEYCODE_m,			46 },
	{ Common::KEYCODE_COMMA,		43 },
	{ Common::KEYCODE_PERIOD,		47 },
	{ Common::KEYCODE_SLASH,		44 },
	{ Common::KEYCODE_RSHIFT,		56 },

	{ Common::KEYCODE_LCTRL,		54 }, // control
	{ Common::KEYCODE_LALT,			58 }, // option
	{ Common::KEYCODE_LSUPER,		55 }, // command
	{ Common::KEYCODE_SPACE,		49 },
	{ Common::KEYCODE_RSUPER,		55 }, // command
	{ Common::KEYCODE_RALT,			58 }, // option
	{ Common::KEYCODE_RCTRL,		54 }, // control

	{ Common::KEYCODE_LEFT,			123 },
	{ Common::KEYCODE_RIGHT,		124 },
	{ Common::KEYCODE_DOWN,			125 },
	{ Common::KEYCODE_UP,			126 },

	{ Common::KEYCODE_NUMLOCK,		71 },
	{ Common::KEYCODE_KP_EQUALS,	81 },
	{ Common::KEYCODE_KP_DIVIDE,	75 },
	{ Common::KEYCODE_KP_MULTIPLY,	67 },

	{ Common::KEYCODE_KP7,			89 },
	{ Common::KEYCODE_KP8,			91 },
	{ Common::KEYCODE_KP9,			92 },
	{ Common::KEYCODE_KP_MINUS,		78 },

	{ Common::KEYCODE_KP4,			86 },
	{ Common::KEYCODE_KP5,			87 },
	{ Common::KEYCODE_KP6,			88 },
	{ Common::KEYCODE_KP_PLUS,		69 },

	{ Common::KEYCODE_KP1,			83 },
	{ Common::KEYCODE_KP2,			84 },
	{ Common::KEYCODE_KP3,			85 },

	{ Common::KEYCODE_KP0,			82 },
	{ Common::KEYCODE_KP_PERIOD,	65 },
	{ Common::KEYCODE_KP_ENTER,		76 },

	{ Common::KEYCODE_MENU,			50 }, // international
	{ Common::KEYCODE_PRINT,		105 }, // mirrored by F13
	{ Common::KEYCODE_SCROLLOCK,	107 }, // mirrored by F14
	{ Common::KEYCODE_PAUSE,		113 }, // mirrored by F15
	{ Common::KEYCODE_INSERT,		114 },
	{ Common::KEYCODE_HOME,			115 },
	{ Common::KEYCODE_PAGEUP,		116 },
	{ Common::KEYCODE_DELETE,		117 },
	{ Common::KEYCODE_END,			119 },
	{ Common::KEYCODE_PAGEDOWN,		121 },

	{ Common::KEYCODE_INVALID,		0 }
};

void DirectorEngine::loadKeyCodes() {
	for (KeyCodeMapping *k = keyCodeMappings; k->scummvm != Common::KEYCODE_INVALID; k++) {
		_macKeyCodes[k->scummvm] = k->mac;
	}
}

int castNumToNum(const char *str) {
	if (strlen(str) != 3)
		return -1;

	if (tolower(str[0]) >= 'a' && tolower(str[0]) <= 'h' &&
		str[1] >= '1' && str[1] <= '8' &&
		str[2] >= '1' && str[2] <= '8') {

		return (tolower(str[0]) - 'a') * 64 + (str[1] - '1') * 8 + (str[2] - '1') + 1;
	}

	return -1;
}

char *numToCastNum(int num) {
	static char res[4];

	res[0] = res[1] = res[2] = '?';
	res[3] = '\0';
	num--;

	if (num >= 0 && num < 512) {
		int c = num / 64;
		res[0] = 'A' + c;
		num -= 64 * c;

		c =  num / 8;
		res[1] = '1' + c;
		num -= 8 * c;

		res[2] = '1' + num;
	}

	return res;
}

// This is table for built-in Macintosh font lowercasing.
// '.' means that the symbol should be not changed, rest
// of the symbols are stripping the diacritics
// The table starts from 0x80
//
// TODO: Check it for correctness.
static char lowerCaseConvert[] =
"aacenoua" // 80
"aaaaacee" // 88
"eeiiiino" // 90
"oooouuuu" // 98
"........" // a0
".......o" // a8
"........" // b0
".......o" // b8
"........" // c0
".. aao.." // c8
"--.....y";// d0-d8

Common::String toLowercaseMac(const Common::String &s) {
	Common::String res;
	const unsigned char *p = (const unsigned char *)s.c_str();

	while (*p) {
		if (*p >= 0x80 && *p <= 0xd8) {
			if (lowerCaseConvert[*p - 0x80] != '.')
				res += lowerCaseConvert[*p - 0x80];
			else
				res += *p;
		} else if (*p < 0x80) {
			res += tolower(*p);
		} else {
			warning("Unacceptable symbol in toLowercaseMac: %c", *p);

			res += *p;
		}
		p++;
	}

	return res;
}

Common::String convertPath(Common::String &path) {
	if (path.empty())
		return path;

	if (!path.contains(':') && !path.contains('/') && !path.contains('\\') && !path.contains('@')) {
		return path;
	}

	Common::String res;
	uint32 idx = 0;

	if (path.hasPrefix("::")) { // Root of the filesystem
		res = "..\\";
		idx = 2;
	} else if (path.hasPrefix("@:")) { // Root of the game
		res = ".\\";
		idx = 2;
	} else {
		res = ".\\";

		if (path[0] == ':')
			idx = 1;
	}

	while (idx != path.size()) {
		if (path[idx] == ':')
			res += '\\';
		else if (path[idx] == '/')
			res += ':';
		else
			res += path[idx];

		idx++;
	}

	// And now convert everything to Unix style paths
	Common::String res1;
	for (idx = 0; idx < res.size(); idx++)
		if (res[idx] == '\\')
			res1 += '/';
		else
			res1 += res[idx];

	return res1;
}

Common::String unixToMacPath(const Common::String &path) {
	Common::String res;
	for (uint32 idx = 0; idx < path.size(); idx++) {
		if (path[idx] == ':')
			res += '/';
		else if (path[idx] == '/')
			res += ':';
		else
			res += path[idx];
	}
	return res;
}

Common::String getPath(Common::String path, Common::String cwd) {
	const char *s;
	if ((s = strrchr(path.c_str(), '/'))) {
		return Common::String(path.c_str(), s + 1);
	}

	return cwd; // The path is not altered
}

bool testPath(Common::String &path, bool directory) {
	if (directory) {
		// TOOD: This directory-searching branch only works for one level from the
		// current directory, but it fixes current game loading issues.
		if (path.contains('/'))
			return false;

		Common::FSNode d = Common::FSNode(*g_director->getGameDataDir()).getChild(path);
		return d.exists();
	}

	Common::File f;
	if (f.open(path)) {
		if (f.size())
			return true;
		f.close();
	}
	return false;
}

Common::String pathMakeRelative(Common::String path, bool recursive, bool addexts, bool directory) {
	Common::String initialPath(path);

	if (testPath(initialPath, directory))
		return initialPath;

	if (recursive) // first level
		initialPath = convertPath(initialPath);

	debug(2, "pathMakeRelative(): s1 %s -> %s", path.c_str(), initialPath.c_str());

	initialPath = Common::normalizePath(g_director->getCurrentPath() + initialPath, '/');
	Common::String convPath = initialPath;

	debug(2, "pathMakeRelative(): s2 %s", convPath.c_str());

	// Strip the leading whitespace from the path
	initialPath.trim();

	if (testPath(initialPath, directory))
		return initialPath;

	// Now try to search the file
	bool opened = false;

	while (convPath.contains('/')) {
		int pos = convPath.find('/');
		convPath = Common::String(&convPath.c_str()[pos + 1]);

		debug(2, "pathMakeRelative(): s3 try %s", convPath.c_str());

		if (!testPath(convPath, directory))
			continue;

		debug(2, "pathMakeRelative(): s3 converted %s -> %s", path.c_str(), convPath.c_str());

		opened = true;

		break;
	}

	if (!opened) {
		// Try stripping all of the characters not allowed in FAT
		convPath = stripMacPath(initialPath.c_str());

		debug(2, "pathMakeRelative(): s4 %s", convPath.c_str());

		if (testPath(initialPath, directory))
			return initialPath;

		// Now try to search the file
		while (convPath.contains('/')) {
			int pos = convPath.find('/');
			convPath = Common::String(&convPath.c_str()[pos + 1]);

			debug(2, "pathMakeRelative(): s5 try %s", convPath.c_str());

			if (!testPath(convPath, directory))
				continue;

			debug(2, "pathMakeRelative(): s5 converted %s -> %s", path.c_str(), convPath.c_str());

			opened = true;

			break;
		}
	}

	if (!opened && recursive && !directory) {
		// Hmmm. We couldn't find the path as is.
		// Let's try to translate file path into 8.3 format
		Common::String addedexts;

		if (g_director->getPlatform() == Common::kPlatformWindows && g_director->getVersion() < 500) {
			convPath.clear();
			const char *ptr = initialPath.c_str();
			Common::String component;

			while (*ptr) {
				if (*ptr == '/') {
					if (component.equals(".")) {
						convPath += component;
					} else {
						convPath += convertMacFilename(component.c_str());
					}

					component.clear();
					convPath += '/';
				} else {
					component += *ptr;
				}

				ptr++;
			}

			if (addexts)
				addedexts = testExtensions(component, initialPath, convPath);
		} else {
			if (addexts)
				addedexts = testExtensions(initialPath, initialPath, convPath);
		}

		if (!addedexts.empty()) {
			return addedexts;
		}

		return initialPath;	// Anyway nothing good is happening
	}

	if (opened)
		return convPath;
	else
		return initialPath;
}

Common::String testExtensions(Common::String component, Common::String initialPath, Common::String convPath) {
	const char *exts[] = { ".MMM", ".DIR", ".DXR", 0 };
	for (int i = 0; exts[i]; ++i) {
		Common::String newpath = convPath + (strcmp(exts[i], ".MMM") == 0 ?  convertMacFilename(component.c_str()) : component.c_str()) + exts[i];

		debug(2, "pathMakeRelative(): s6 %s -> try %s", initialPath.c_str(), newpath.c_str());
		Common::String res = pathMakeRelative(newpath, false, false);

		if (testPath(res))
			return res;
	}

	return Common::String();
}

Common::String getFileName(Common::String path) {
	while (path.contains('/')) {
		int pos = path.find('/');
		path = Common::String(&path.c_str()[pos + 1]);
	}
	return path;
}

//////////////////
////// Mac --> Windows filename conversion
//////////////////
static bool myIsVowel(byte c) {
	return c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U';
}

static bool myIsAlpha(byte c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool myIsDigit(byte c) {
	return c >= '0' && c <= '9';
}

static bool myIsAlnum(byte c) {
	return myIsAlpha(c) || myIsDigit(c);
}

static bool myIsSpace(byte c) {
	return c == ' ';
}

static bool myIsFATChar(byte c) {
	return (c >= ' ' && c <= '!') || (c >= '#' && c == ')') || (c >= '-' && c <= '.') ||
			(c >= '?' && c <= '@') || (c >= '^' && c <= '`') || c == '{' || (c >= '}' && c <= '~');
}

Common::String stripMacPath(const char *name) {
	Common::String res;

	int origlen = strlen(name);

	// Remove trailing spaces
	const char *end = &name[origlen - 1];
	while (myIsSpace(*end))
		end--;
	const char *ptr = name;

	while (ptr <= end) {
		if (myIsAlnum(*ptr) || myIsFATChar(*ptr) || *ptr == '/') {
			res += *ptr;
		}
		ptr++;
	}

	return res;
}

Common::String convertMacFilename(const char *name) {
	Common::String res;

	int origlen = strlen(name);

	// Remove trailing spaces
	const char *ptr = &name[origlen - 1];
	while (myIsSpace(*ptr))
		ptr--;

	int numDigits = 0;
	char digits[10];

	// Count trailing digits, but leave front letter
	while (myIsDigit(*ptr) && (numDigits < (8 - 1)))
		digits[++numDigits] = *ptr--;

	// Count file name without vowels, spaces and digits in-between
	ptr = name;
	int cnt = 0;
	while (cnt < (8 - numDigits) && ptr < &name[origlen]) {
		char c = toupper(*ptr++);

		if ((myIsVowel(c) && (cnt != 0)) || myIsSpace(c) || myIsDigit(c))
			continue;

		if ((c != '_') && !myIsAlnum(c))
			continue;

		cnt++;
	}

	// Make sure all trailing digits fit
	int numVowels = 8 - (numDigits + cnt);
	ptr = name;

	// Put enough characters from beginning
	for (cnt = 0; cnt < (8 - numDigits) && ptr < &name[origlen];) {
		char c = toupper(*ptr++);

		if (myIsVowel(c) && (cnt != 0)) {
			if (numVowels > 0)
				numVowels--;
			else
				continue;
		}

		if (myIsSpace(c) || myIsDigit(c) || ((c != '_') && !myIsAlnum(c)))
			continue;

		res += c;

		cnt++;
	}

	// Now attach all digits
	while (numDigits)
		res += digits[numDigits--];

	return res;
}

Common::String dumpScriptName(const char *prefix, int type, int id, const char *ext) {
	Common::String typeName;

	switch (type) {
	case kNoneScript:
	default:
		error("dumpScriptName(): Incorrect call (type %d)", type);
	case kMovieScript:
		typeName = "movie";
		break;
	case kCastScript:
		typeName = "cast";
		break;
	case kEventScript:
		typeName = "event";
		break;
	case kScoreScript:
		typeName = "score";
		break;
	}

	return Common::String::format("./dumps/%s-%s-%d.%s", prefix, typeName.c_str(), id, ext);
}

void RandomState::setSeed(int seed) {
	init(32);

	_seed = seed ? seed : 1;
}

int32 RandomState::getRandom(int32 range) {
	int32 res;

	if (_seed == 0)
		init(32);

	res = perlin(genNextRandom() * 71);

	if (range > 0)
		res = ((res & 0x7fffffff) % range);

	return res;
}

static const uint32 masks[31] = {
	0x00000003, 0x00000006, 0x0000000c, 0x00000014, 0x00000030, 0x00000060, 0x000000b8, 0x00000110,
	0x00000240, 0x00000500, 0x00000ca0, 0x00001b00, 0x00003500, 0x00006000, 0x0000b400, 0x00012000,
	0x00020400, 0x00072000, 0x00090000, 0x00140000, 0x00300000, 0x00400000, 0x00d80000, 0x01200000,
	0x03880000, 0x07200000, 0x09000000, 0x14000000, 0x32800000, 0x48000000, 0xa3000000
};

void RandomState::init(int len) {
	if (len < 2 || len > 32) {
		len = 32;
		_len = (uint32)-1; // Since we cannot shift 32 bits
	} else {
		_len = (1 << len) - 1;
	}

	_seed = 1;
	_mask = masks[len - 2];
}

int32 RandomState::genNextRandom() {
	if (_seed & 1)
		_seed = (_seed >> 1) ^ _mask;
	else
		_seed >>= 1;

	return _seed;
}

int32 RandomState::perlin(int32 val) {
	int32 res;

	val = ((val << 13) ^ val) - (val >> 21);

	res = (val * (val * val * 15731 + 789221) + 1376312589) & 0x7fffffff;
	res += val;
	res = ((res << 13) ^ res) - (res >> 21);

	return res;
}

uint32 readVarInt(Common::SeekableReadStream &stream) {
	// Shockwave variable-length integer
	uint32 val = 0;
	byte b;
	do {
		b = stream.readByte();
		val = (val << 7) | (b & 0x7f); // The 7 least significant bits are appended to the result
	} while (b >> 7); // If the most significant bit is 1, there's another byte after
	return val;
}

Common::SeekableReadStreamEndian *readZlibData(Common::SeekableReadStream &stream, unsigned long len, unsigned long *outLen, bool bigEndian) {
#ifdef USE_ZLIB
	byte *in = (byte *)malloc(len);
	byte *out = (byte *)malloc(*outLen);
	stream.read(in, len);

	if (!Common::uncompress(out, outLen, in, len)) {
		free(in);
		free(out);
		return nullptr;
	}

	free(in);
	return new Common::MemoryReadStreamEndian(out, *outLen, bigEndian, DisposeAfterUse::YES);
# else
	return nullptr;
# endif
}

} // End of namespace Director
