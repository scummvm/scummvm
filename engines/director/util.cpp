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

#include "common/file.h"
#include "common/fs.h"
#include "common/keyboard.h"
#include "common/memstream.h"
#include "common/punycode.h"
#include "common/tokenizer.h"
#include "common/zlib.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macfontmanager.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/lingo/lingo.h"
#include "director/util.h"

namespace Director {

static struct MacKeyCodeMapping {
	Common::KeyCode scummvm;
	int mac;
} MackeyCodeMappings[] = {
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

static struct WinKeyCodeMapping {
	Common::KeyCode scummvm;
	int win;
} WinkeyCodeMappings[] = {
	{ Common::KEYCODE_BACKSPACE,	0x08 },
	{ Common::KEYCODE_TAB,			0x09 },
	{ Common::KEYCODE_CAPSLOCK,		0x14 },

	{ Common::KEYCODE_SPACE,		0x20 },
	{ Common::KEYCODE_MENU,			0x12 },
	{ Common::KEYCODE_CANCEL, 		0x03 },
	{ Common::KEYCODE_RETURN,		0x0D },
	{ Common::KEYCODE_PAUSE,		0x13 },
	{ Common::KEYCODE_CAPSLOCK,		0x14 },
	{ Common::KEYCODE_PRINT,		0x2A },

	{ Common::KEYCODE_0,			0x30 },
	{ Common::KEYCODE_1,			0x31 },
	{ Common::KEYCODE_2,			0x32 },
	{ Common::KEYCODE_3,			0x33 },
	{ Common::KEYCODE_4,			0x34 },
	{ Common::KEYCODE_5,			0x35 },
	{ Common::KEYCODE_6,			0x36 },
	{ Common::KEYCODE_7,			0x37 },
	{ Common::KEYCODE_8,			0x38 },
	{ Common::KEYCODE_9,			0x39 },

	{ Common::KEYCODE_a,			0x41 },
	{ Common::KEYCODE_b,			0x42 },
	{ Common::KEYCODE_c,			0x43 },
	{ Common::KEYCODE_d,			0x44 },
	{ Common::KEYCODE_e,			0x45 },
	{ Common::KEYCODE_f,			0x46 },
	{ Common::KEYCODE_g,			0x47 },
	{ Common::KEYCODE_h,			0x48 },
	{ Common::KEYCODE_i,			0x49 },
	{ Common::KEYCODE_j,			0x4A },
	{ Common::KEYCODE_k,			0x4B },
	{ Common::KEYCODE_l,			0x4C },
	{ Common::KEYCODE_m,			0x4D },
	{ Common::KEYCODE_n,			0x4E },
	{ Common::KEYCODE_o,			0x4F },
	{ Common::KEYCODE_p,			0x50 },
	{ Common::KEYCODE_q,			0x51 },
	{ Common::KEYCODE_r,			0x52 },
	{ Common::KEYCODE_s,			0x53 },
	{ Common::KEYCODE_t,			0x54 },
	{ Common::KEYCODE_u,			0x55 },
	{ Common::KEYCODE_v,			0x56 },
	{ Common::KEYCODE_w,			0x57 },
	{ Common::KEYCODE_x,			0x58 },
	{ Common::KEYCODE_y,			0x59 },
	{ Common::KEYCODE_z,			0x5A },

	{ Common::KEYCODE_KP0,			0x60 },
	{ Common::KEYCODE_KP1,			0x61 },
	{ Common::KEYCODE_KP2,			0x62 },
	{ Common::KEYCODE_KP3,			0x63 },
	{ Common::KEYCODE_KP4,			0x64 },
	{ Common::KEYCODE_KP5,			0x65 },
	{ Common::KEYCODE_KP6,			0x66 },
	{ Common::KEYCODE_KP7,			0x67 },
	{ Common::KEYCODE_KP8,			0x68 },
	{ Common::KEYCODE_KP9,			0x69 },

	{ Common::KEYCODE_KP_PLUS,		0x6B },
	{ Common::KEYCODE_KP_MULTIPLY,	0x6A },
	{ Common::KEYCODE_KP_DIVIDE,	0x6F },
	{ Common::KEYCODE_KP_MINUS,		0x6D },

	{ Common::KEYCODE_F1,			0x70 },
	{ Common::KEYCODE_F2,			0x71 },
	{ Common::KEYCODE_F3,			0x72 },
	{ Common::KEYCODE_F4,			0x73 },
	{ Common::KEYCODE_F5,			0x74 },
	{ Common::KEYCODE_F6,			0x75 },
	{ Common::KEYCODE_F7,			0x76 },
	{ Common::KEYCODE_F8,			0x77 },
	{ Common::KEYCODE_F9,			0x78 },
	{ Common::KEYCODE_F10,			0x79 },
	{ Common::KEYCODE_F11,			0x7A },
	{ Common::KEYCODE_F12,			0x7B },
	{ Common::KEYCODE_F13,			0x7C },
	{ Common::KEYCODE_F14,			0x7D },
	{ Common::KEYCODE_F15,			0x7E },
	{ Common::KEYCODE_F16,			0x7F },
	{ Common::KEYCODE_F17,			0x80 },
	{ Common::KEYCODE_F18,			0x81 },

	{ Common::KEYCODE_LEFT,			0x25 },
	{ Common::KEYCODE_RIGHT,		0x27 },
	{ Common::KEYCODE_DOWN,			0x28 },
	{ Common::KEYCODE_UP,			0x26 },

	{ Common::KEYCODE_NUMLOCK,		0x90 },
	{ Common::KEYCODE_SCROLLOCK,	0x91 },
	{ Common::KEYCODE_SLEEP,		0x5F },
	{ Common::KEYCODE_INSERT,		0x2D },
	{ Common::KEYCODE_HELP,			0x2F },
	{ Common::KEYCODE_SELECT, 		0x29 },
	{ Common::KEYCODE_HOME,			0x24 },
	{ Common::KEYCODE_PRINT,		0x2A },
	{ Common::KEYCODE_ESCAPE, 		0x18 },
	{ Common::KEYCODE_PAGEUP,		0x21 },
	{ Common::KEYCODE_DELETE,		0x2E },
	{ Common::KEYCODE_END,			0x23 },
	{ Common::KEYCODE_PAGEDOWN,		0x22 },

	{ Common::KEYCODE_INVALID,		0 }
};

void DirectorEngine::loadKeyCodes() {
	if (g_director->getPlatform() == Common::kPlatformWindows) {
		for (WinKeyCodeMapping *k = WinkeyCodeMappings; k->scummvm != Common::KEYCODE_INVALID; k++)
			_KeyCodes[k->scummvm] = k->win;
	} else {
		for (MacKeyCodeMapping *k = MackeyCodeMappings; k->scummvm != Common::KEYCODE_INVALID; k++)
			_KeyCodes[k->scummvm] = k->mac;
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

Common::String CastMemberID::asString() const {
	Common::String res = Common::String::format("member %d", member);

	if (g_director->getVersion() < 400 || g_director->getCurrentMovie()->_allowOutdatedLingo)
		res += "(" + Common::String(numToCastNum(member)) + ")";
	else if (g_director->getVersion() >= 500)
		res += Common::String::format(" of castLib %d", castLib);

	return res;
}

Common::String convertPath(Common::String &path) {
	if (path.empty())
		return path;

	if (!path.contains(':') && !path.contains('\\') && !path.contains('@')) {
		return path;
	}

	Common::String res;
	uint32 idx = 0;

	if (path.hasPrefix("::")) { // Parent directory
		idx = 2;
	} else if (path.hasPrefix("@:")) { // Root of the game
		idx = 2;
	} else if (path.size() >= 3
					&& Common::isAlpha(path[0])
					&& path[1] == ':'
					&& path[2] == '\\') { // Windows drive letter
		idx = 3;
	} else if (path[0] == ':') {
		idx = 1;
	}

	while (idx < path.size()) {
		if (path[idx] == ':' || path[idx] == '\\')
			res += g_director->_dirSeparator;
		else
			res += path[idx];

		idx++;
	}

	return res;
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
	if ((s = strrchr(path.c_str(), g_director->_dirSeparator))) {
		return Common::String(path.c_str(), s + 1);
	}

	return cwd; // The path is not altered
}

bool testPath(Common::String &path, bool directory) {
	Common::FSNode d = Common::FSNode(*g_director->getGameDataDir());

	// Test if we have it right in the SearchMan
	if (SearchMan.hasFile(Common::Path(path, g_director->_dirSeparator)))
		return true;

	// check for the game data dir
	if (!path.contains(g_director->_dirSeparator) && path.equalsIgnoreCase(d.getName())) {
		if (!directory)
			return false;
		path = "";
		return true;
	}

	Common::StringTokenizer directory_list(path, Common::String(g_director->_dirSeparator));
	Common::String newPath;

	Common::FSList fslist;
	while (!directory_list.empty()) {
		Common::String token = directory_list.nextToken();
		fslist.clear();
		Common::FSNode::ListMode mode = Common::FSNode::kListDirectoriesOnly;
		if (directory_list.empty() && !directory) {
			mode = Common::FSNode::kListFilesOnly;
		}
		d.getChildren(fslist, mode);

		bool exists = false;
		for (Common::FSList::iterator i = fslist.begin(); i != fslist.end(); ++i) {
			// for each element in the path, choose the first FSNode
			// with a case-insensitive matcing name
			if (i->getName().equalsIgnoreCase(token)) {
				exists = true;
				newPath += i->getName();
				if (!directory_list.empty())
					newPath += (g_director->_dirSeparator);

				d = Common::FSNode(*i);
				break;
			}
		}
		if (!exists)
			return false;
	}
	// write back path with correct case
	path = newPath;
	return true;
}


Common::String pathMakeRelative(Common::String path, bool recursive, bool addexts, bool directory) {
	//Wrap pathMakeRelative to search in extra paths defined by the game
	Common::String foundPath;

	Datum searchPath = g_director->getLingo()->_searchPath;
	if (searchPath.type == ARRAY && searchPath.u.farr->arr.size() > 0) {
		for (uint i = 0; i < searchPath.u.farr->arr.size(); i++) {
			Common::String searchIn = searchPath.u.farr->arr[i].asString();
			debug(9, "pathMakeRelative(): searchPath: %s", searchIn.c_str());

			foundPath = wrappedPathMakeRelative(searchIn + path, recursive, addexts, directory);
			if (testPath(foundPath))
				return foundPath;
		}
	}

	return wrappedPathMakeRelative(path, recursive, addexts, directory);
}


// if we are finding the file path, then this func will return exactly the executable file path
// if we are finding the directory path, then we will get the path relative to the game data dir.
// e.g. if we have game data dir as SSwarlock, then "A:SSwarlock" -> "", "A:SSwarlock:Nav" -> "Nav"
Common::String wrappedPathMakeRelative(Common::String path, bool recursive, bool addexts, bool directory) {

	Common::String initialPath(path);

	debug(9, "pathMakeRelative(): s0 %s -> %s", path.c_str(), initialPath.c_str());

	if (recursive) // first level
		initialPath = convertPath(initialPath);

	debug(9, "pathMakeRelative(): s1 %s -> %s", path.c_str(), initialPath.c_str());

	initialPath = Common::normalizePath(g_director->getCurrentPath() + initialPath, g_director->_dirSeparator);
	Common::String convPath = initialPath;

	debug(9, "pathMakeRelative(): s2 %s", convPath.c_str());

	// Strip the leading whitespace from the path
	initialPath.trim();

	if (testPath(initialPath, directory))
		return initialPath;

	// Now try to search the file
	bool opened = false;

	while (convPath.contains(g_director->_dirSeparator)) {
		int pos = convPath.find(g_director->_dirSeparator);
		convPath = Common::String(&convPath.c_str()[pos + 1]);

		debug(9, "pathMakeRelative(): s3 try %s", convPath.c_str());

		if (!testPath(convPath, directory)) {
			// If we were supplied with parh with subdirectories,
			// attempt to combine it with the current movie path at every iteration
			Common::String locPath = Common::normalizePath(g_director->getCurrentPath() + convPath, g_director->_dirSeparator);
			debug(9, "pathMakeRelative(): s3.1 try %s", locPath.c_str());

			if (!testPath(locPath, directory))
				continue;
		}

		debug(9, "pathMakeRelative(): s3 converted %s -> %s", path.c_str(), convPath.c_str());

		opened = true;

		break;
	}

	if (!opened) {
		// Try stripping all of the characters not allowed in FAT
		convPath = stripMacPath(initialPath.c_str());

		debug(9, "pathMakeRelative(): s4 %s", convPath.c_str());

		if (testPath(initialPath, directory))
			return initialPath;

		// Now try to search the file
		while (convPath.contains(g_director->_dirSeparator)) {
			int pos = convPath.find(g_director->_dirSeparator);
			convPath = Common::String(&convPath.c_str()[pos + 1]);

			debug(9, "pathMakeRelative(): s5 try %s", convPath.c_str());

			if (!testPath(convPath, directory))
				continue;

			debug(9, "pathMakeRelative(): s5 converted %s -> %s", path.c_str(), convPath.c_str());

			opened = true;

			break;
		}
	}

	if (!opened && recursive && !directory) {
		// Hmmm. We couldn't find the path as is.
		// Let's try to translate file path into 8.3 format
		Common::String addedexts;

		convPath.clear();
		const char *ptr = initialPath.c_str();
		Common::String component;

		while (*ptr) {
			if (*ptr == g_director->_dirSeparator) {
				if (component.equals(".")) {
					convPath += component;
				} else {
					convPath += convertMacFilename(component.c_str());
				}

				component.clear();
				convPath += g_director->_dirSeparator;
			} else {
				component += *ptr;
			}

			ptr++;
		}

		if (g_director->getPlatform() == Common::kPlatformWindows) {
			if (hasExtension(component)) {
				Common::String nameWithoutExt = component.substr(0, component.size() - 4);
				Common::String ext = component.substr(component.size() - 4);
				Common::String newpath = convPath + convertMacFilename(nameWithoutExt.c_str()) + ext;

				debug(9, "pathMakeRelative(): s6 %s -> try %s", initialPath.c_str(), newpath.c_str());
				Common::String res = wrappedPathMakeRelative(newpath, false, false);

				if (testPath(res))
					return res;
			}
		}

		if (addexts)
			addedexts = testExtensions(component, initialPath, convPath);

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

bool hasExtension(Common::String filename) {
	uint len = filename.size();
	return len >= 4 && filename[len - 4] == '.'
					&& Common::isAlpha(filename[len - 3])
					&& Common::isAlpha(filename[len - 2])
					&& Common::isAlpha(filename[len - 1]);
}

Common::String testExtensions(Common::String component, Common::String initialPath, Common::String convPath) {
	const char *extsD3[] = { ".MMM", nullptr };
	const char *extsD4[] = { ".DIR", ".DXR", nullptr };

	const char **exts = (g_director->getVersion() >= 400) ? extsD4 : extsD3;
	for (int i = 0; exts[i]; ++i) {
		Common::String newpath = convPath + convertMacFilename(component.c_str()) + exts[i];

		debug(9, "testExtensions(): sT %s -> try %s, comp: %s", initialPath.c_str(), newpath.c_str(), component.c_str());
		Common::String res = wrappedPathMakeRelative(newpath, false, false);

		if (testPath(res))
			return res;
	}

	return Common::String();
}

Common::String getFileName(Common::String path) {
	while (path.contains(g_director->_dirSeparator)) {
		int pos = path.find(g_director->_dirSeparator);
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
		if (myIsAlnum(*ptr) || myIsFATChar(*ptr) || *ptr == g_director->_dirSeparator) {
			res += *ptr;
		}
		ptr++;
	}

	return res;
}

Common::String convertMacFilename(const char *name) {
	Common::String res;

	int origlen = strlen(name);

	if (g_director->getVersion() < 400) {
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
	} else {
		const char *ptr = name;
		for (int cnt = 0; cnt < 8 && ptr < &name[origlen];) {
			char c = toupper(*ptr++);

			if (myIsSpace(c) || (!myIsAlnum(c) && !myIsFATChar(c)))
				continue;

			res += c;

			cnt++;
		}

		// If the result filename ends with '.', remove it
		if (res.hasSuffix("."))
			res = res.substr(0, res.size() - 1);
	}

	return res;
}

Common::String dumpScriptName(const char *prefix, int type, int id, const char *ext) {
	Common::String typeName;

	switch (type) {
	case kNoneScript:
		typeName = "unknown";
		break;
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
	default:
		error("dumpScriptName(): Incorrect call (type %d)", type);
		break;
	}

	return Common::String::format("./dumps/%s-%s-%d.%s", prefix, typeName.c_str(), id, ext);
}

Common::String dumpFactoryName(const char *prefix, const char *name, const char *ext) {
	return Common::String::format("./dumps/%s-factory-%s.%s", prefix, name, ext);
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

uint16 humanVersion(uint16 ver) {
	if (ver >= kFileVer1201)
		return 1201;
	if (ver >= kFileVer1200)
		return 1200;
	if (ver >= kFileVer1150)
		return 1150;
	if (ver >= kFileVer1100)
		return 1100;
	if (ver >= kFileVer1000)
		return 1000;
	if (ver >= kFileVer850)
		return 850;
	if (ver >= kFileVer800)
		return 800;
	if (ver >= kFileVer700)
		return 700;
	if (ver >= kFileVer600)
		return 600;
	if (ver >= kFileVer500)
		return 500;
	if (ver >= kFileVer404)
		return 404;
	if (ver >= kFileVer400)
		return 400;
	if (ver >= kFileVer310)
		return 310;
	if (ver >= kFileVer300)
		return 300;
	return 200;
}

Common::Platform platformFromID(uint16 id) {
	switch (id) {
	case 1:
		return Common::kPlatformMacintosh;
	case 2:
		return Common::kPlatformWindows;
	default:
		warning("platformFromID: Unknown platform ID %d", id);
		break;
	}
	return Common::kPlatformUnknown;
}

bool isButtonSprite(SpriteType spriteType) {
	return spriteType == kButtonSprite || spriteType == kCheckboxSprite || spriteType == kRadioButtonSprite;
}

Common::CodePage getEncoding(Common::Platform platform, Common::Language language) {
	switch (language) {
	case Common::JA_JPN:
		return Common::kWindows932; // Shift JIS
	default:
		break;
	}
	return (platform == Common::kPlatformWindows)
				? Common::kWindows1252
				: Common::kMacRoman;
}

Common::CodePage detectFontEncoding(Common::Platform platform, uint16 fontId) {
	return getEncoding(platform, g_director->_wm->_fontMan->getFontLanguage(fontId));
}

int charToNum(Common::u32char_type_t ch) {
	Common::String encodedCh = Common::U32String(ch).encode(g_director->getPlatformEncoding());
	int res = 0;
	while (encodedCh.size()) {
		res = (res << 8) | (byte)encodedCh.firstChar();
		encodedCh.deleteChar(0);
	}
	return res;
}

Common::u32char_type_t numToChar(int num) {
	Common::String encodedCh;
	while (num) {
		encodedCh.insertChar((char)(num & 0xFF), 0);
		num >>= 8;
	}
	Common::U32String str = encodedCh.decode(g_director->getPlatformEncoding());
	return str.lastChar();
}

int compareStrings(const Common::String &s1, const Common::String &s2) {
	Common::U32String u32S1 = s1.decode(Common::kUtf8);
	Common::U32String u32S2 = s2.decode(Common::kUtf8);
	const Common::u32char_type_t *p1 = u32S1.c_str();
	const Common::u32char_type_t *p2 = u32S2.c_str();

	uint32 c1, c2;
	do {
		c1 = charToNum(*p1);
		c2 = charToNum(*p2);
		p1++;
		p2++;
	} while (c1 == c2 && c1);
	return c1 - c2;
}

Common::String encodePathForDump(const Common::String &path) {
	return punycode_encodepath(Common::Path(path, g_director->_dirSeparator)).toString();
}

Common::String utf8ToPrintable(const Common::String &str) {
	return Common::toPrintable(Common::U32String(str));
}

Common::String castTypeToString(const CastType &type) {
	Common::String res;
	switch(type) {
	case kCastBitmap:
		res = "bitmap";
		break;
	case kCastPalette:
		res = "palette";
		break;
	case kCastButton:
		res = "button";
		break;
	case kCastPicture:
		res = "picture";
		break;
	case kCastDigitalVideo:
		res = "digitalVideo";
		break;
	case kCastLingoScript:
		res = "script";
		break;
	case kCastShape:
		res = "shape";
		break;
	case kCastFilmLoop:
		res = "filmLoop";
		break;
	case kCastSound:
		res = "sound";
		break;
	case kCastMovie:
		res = "movie";
		break;
	case kCastText:
		res = "text";
		break;
	default:
		res = "empty";
		break;
	}
	return res;
}

Common::String decodePlatformEncoding(Common::String input) {
	return input.decode(g_director->getPlatformEncoding());
}

} // End of namespace Director
