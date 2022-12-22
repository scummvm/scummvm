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
#include "common/macresman.h"
#include "common/memstream.h"
#include "common/punycode.h"
#include "common/tokenizer.h"
#include "common/compression/zlib.h"

#include "director/types.h"
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
	Common::FSNode node;

	// Test if we have it right in the SearchMan. Also accept MacBinary
	// for Mac and Pippin
	if (SearchMan.hasFile(Common::Path(path, g_director->_dirSeparator)) ||
	    ((g_director->getPlatform() == Common::kPlatformMacintoshII
	      || g_director->getPlatform() == Common::kPlatformMacintosh
	      || g_director->getPlatform() == Common::kPlatformPippin) &&
	     Common::MacResManager::exists(Common::Path(path, g_director->_dirSeparator))))
		return true;

	debug(9, "testPath: %s  dir: %d", path.c_str(), directory);

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
			mode = Common::FSNode::kListAll;
		}
		bool hasChildren = d.getChildren(fslist, mode);
		if (!hasChildren)
			continue;

		bool exists = false;
		for (Common::FSList::iterator i = fslist.begin(); i != fslist.end(); ++i) {
			// for each element in the path, choose the first FSNode
			// with a case-insensitive matching name
			if (i->getName().equalsIgnoreCase(token)) {
				// If this the final path component, check if we're allowed to match with a directory
				node = Common::FSNode(*i);
				if (directory_list.empty() && !directory && node.isDirectory()) {
					continue;
				}

				exists = true;
				newPath += i->getName();
				if (!directory_list.empty())
					newPath += (g_director->_dirSeparator);

				d = node;
				break;
			}
		}
		if (!exists) {
			debug(9, "testPath: Not exists");
			return false;
		}
	}
	debug(9, "testPath: ***** HAVE MATCH");
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

			debug(9, "pathMakeRelative(): -- searchPath not found: %s", foundPath.c_str());
		}
	}

	for (auto i = g_director->_extraSearchPath.begin(); i != g_director->_extraSearchPath.end(); ++i) {
		debug(9, "pathMakeRelative(): extraSearchPath: %s", i->c_str());

		foundPath = wrappedPathMakeRelative(*i + path, recursive, addexts, directory);
		if (testPath(foundPath))
			return foundPath;

		debug(9, "pathMakeRelative(): -- extraSearchPath not found: %s", foundPath.c_str());
	}

	return wrappedPathMakeRelative(path, recursive, addexts, directory);
}


// if we are finding the file path, then this func will return exactly the executable file path
// if we are finding the directory path, then we will get the path relative to the game data dir.
// e.g. if we have game data dir as SSwarlock, then "A:SSwarlock" -> "", "A:SSwarlock:Nav" -> "Nav"
Common::String wrappedPathMakeRelative(Common::String path, bool recursive, bool addexts, bool directory) {

	Common::String initialPath(path);

	debug(9, "wrappedPathMakeRelative(): s0 %s -> %s", path.c_str(), initialPath.c_str());

	if (recursive) // first level
		initialPath = convertPath(initialPath);

	debug(9, "wrappedPathMakeRelative(): s1 %s -> %s", path.c_str(), initialPath.c_str());

	initialPath = Common::normalizePath(g_director->getCurrentPath() + initialPath, g_director->_dirSeparator);
	Common::String convPath = initialPath;

	debug(9, "wrappedPathMakeRelative(): s2 %s", convPath.c_str());

	if (testPath(initialPath, directory))
		return initialPath;

	debug(9, "wrappedPathMakeRelative(): s2.1 -- not found %s", initialPath.c_str());

	// Now try to search the file
	bool opened = false;

	while (convPath.contains(g_director->_dirSeparator)) {
		int pos = convPath.find(g_director->_dirSeparator);
		convPath = Common::String(&convPath.c_str()[pos + 1]);

		debug(9, "wrappedPathMakeRelative(): s3 try %s", convPath.c_str());

		if (!testPath(convPath, directory)) {
			// If we were supplied a path with subdirectories,
			// attempt to combine it with the current movie path at every iteration
			Common::String locPath = Common::normalizePath(g_director->getCurrentPath() + convPath, g_director->_dirSeparator);
			debug(9, "wrappedPathMakeRelative(): s3.1 try %s", locPath.c_str());

			if (!testPath(locPath, directory)) {
				debug(9, "wrappedPathMakeRelative(): s3.1 -- not found %s", locPath.c_str());
				continue;
			}
		}

		debug(9, "wrappedPathMakeRelative(): s3 converted %s -> %s", path.c_str(), convPath.c_str());

		opened = true;

		break;
	}

	if (!opened) {
		// Try stripping all of the characters not allowed in FAT
		convPath = stripMacPath(initialPath.c_str());

		debug(9, "wrappedPathMakeRelative(): s4 %s", convPath.c_str());

		if (testPath(initialPath, directory))
			return initialPath;

		debug(9, "wrappedPathMakeRelative(): s4.1 -- not found %s", initialPath.c_str());

		// Now try to search the file
		while (convPath.contains(g_director->_dirSeparator)) {
			int pos = convPath.find(g_director->_dirSeparator);
			convPath = Common::String(&convPath.c_str()[pos + 1]);

			debug(9, "wrappedPathMakeRelative(): s5 try %s", convPath.c_str());

			if (!testPath(convPath, directory)) {
				debug(9, "wrappedPathMakeRelative(): s5 -- not found %s", convPath.c_str());
				continue;
			}

			debug(9, "wrappedPathMakeRelative(): s5 converted %s -> %s", path.c_str(), convPath.c_str());

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

				debug(9, "wrappedPathMakeRelative(): s6 %s -> try %s", initialPath.c_str(), newpath.c_str());
				Common::String res = wrappedPathMakeRelative(newpath, false, false);

				if (testPath(res))
					return res;

				debug(9, "wrappedPathMakeRelative(): s6 -- not found %s", res.c_str());
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
		Common::String newpath = convPath + component.c_str() + exts[i];

		debug(9, "testExtensions(): sT %s -> try %s, comp: %s", initialPath.c_str(), newpath.c_str(), component.c_str());
		Common::String res = wrappedPathMakeRelative(newpath, false, false);

		if (testPath(res))
			return res;
	}
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

Common::String encodePathForDump(const Common::String &path) {
	return Common::Path(path, g_director->_dirSeparator).punycodeEncode().toString();
}

Common::String utf8ToPrintable(const Common::String &str) {
	return Common::toPrintable(Common::U32String(str));
}

Common::String castTypeToString(const CastType &type) {
	Common::String res;
	switch (type) {
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

/////////////////////////////////////////////////////////////
// String comparison order tables
//
// Director is using weird order of comparison for strings
// It is declared as case-insensitive, ignoring diacritics,
// but the reality is more complex.
//
// The tables below contain the actual weight of the letter in
// the given position

//
// Director 2.0-4.0 Mac, MacRoman encoding
//
// ................................
//   !"«»“”#$%&'‘’()*+,-./012345678
// 9:;<=>?@AÁÀÂÄÃÅaáàâäãåÆæBbCÇcçDd
// EÉÈÊËeéèêëFfﬁﬂGgHhIÍÌÎÏı.íìîïJjK
// kLlMmNÑnñOÓÒÔÖΩØoóòôöõøŒœPpQqRrS
// sßTtUÚÙÛÜuúùûüVvWwXxYyÿŸZz[\]^_`
// {|}~.†°¢£§•¶®©™´¨≠∞±≤≥¥µ∂∑∏π∫ªºΩ
// ¿¡¬√ƒ≈Δ…–—÷◊⁄¤‹›‡·‚„‰��¯˘˙˚¸˝˛.

const byte orderTableD2mac[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x22, 0x23, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
	0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
	0x47, 0x48, 0x58, 0x5a, 0x5e, 0x60, 0x6a, 0x6e, 0x70, 0x72, 0x7d, 0x7f, 0x81, 0x83, 0x85, 0x89,
	0x99, 0x9b, 0x9d, 0x9f, 0xa2, 0xa4, 0xae, 0xb0, 0xb2, 0xb4, 0xb8, 0xba, 0xbb, 0xbc, 0xbd, 0xbe,
	0xbf, 0x4f, 0x59, 0x5c, 0x5f, 0x65, 0x6b, 0x6f, 0x71, 0x77, 0x7e, 0x80, 0x82, 0x84, 0x87, 0x90,
	0x9a, 0x9c, 0x9e, 0xa0, 0xa3, 0xa9, 0xaf, 0xb1, 0xb3, 0xb5, 0xb9, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4,
	0x4c, 0x4e, 0x5b, 0x61, 0x86, 0x8d, 0xa8, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x5d, 0x66, 0x67,
	0x68, 0x69, 0x79, 0x7a, 0x7b, 0x7c, 0x88, 0x91, 0x92, 0x93, 0x94, 0x95, 0xaa, 0xab, 0xac, 0xad,
	0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xa1, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0x56, 0x8f,
	0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0x57, 0x96,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0x24, 0x25, 0xe7, 0x21, 0x4a, 0x4d, 0x8e, 0x97, 0x98,
	0xe8, 0xe9, 0x26, 0x27, 0x2d, 0x2e, 0xea, 0xeb, 0xb6, 0xb7, 0xec, 0xed, 0xee, 0xef, 0x6c, 0x6d,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0x4b, 0x63, 0x49, 0x64, 0x62, 0x73, 0x75, 0x76, 0x74, 0x8a, 0x8c,
	0xf5, 0x8b, 0xa5, 0xa7, 0xa6, 0x77, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

//
// Director 4.0-5.0 Win, cp1250 encoding
//
// ................................
//   !"«»#$%&'‘’()*+,-./0123456789:
// ;<=>?@AÁŔÂÄĂĹĆaáŕâäăĺćBbCÇcçDĐdđ
// EÉČĘËeéčęëFfGgHhIÍĚÎĎiíěîďJjKkLl
// MmNŃnńOÓŇÔÖŐŘŚoóňôöőřśPpQqRrSŠsš
// ßTtUÚŮŰÜuúůűüVvWwXxYÝźyý.Zz[\]^_
// `{|}~.€�‚�„…†‡�‰‹ŤŽŹ�“”•–—�™›ťžˇ
// ˘Ł¤Ą¦§¨©Ş¬®Ż°±˛ł´µ¶·¸ąşĽ˝ľż×Ţţ÷

const byte orderTableD4win[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x22, 0x23, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34,
	0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44,
	0x45, 0x46, 0x56, 0x58, 0x5c, 0x60, 0x6a, 0x6c, 0x6e, 0x70, 0x7a, 0x7c, 0x7e, 0x80, 0x82, 0x86,
	0x96, 0x98, 0x9a, 0x9c, 0xa1, 0xa3, 0xad, 0xaf, 0xb1, 0xb3, 0xb9, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0xc0, 0x4e, 0x57, 0x5a, 0x5e, 0x65, 0x6b, 0x6d, 0x6f, 0x75, 0x7b, 0x7d, 0x7f, 0x81, 0x84, 0x8e,
	0x97, 0x99, 0x9b, 0x9e, 0xa2, 0xa8, 0xae, 0xb0, 0xb2, 0xb6, 0xba, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5,
	0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0x9d, 0xd0, 0x8d, 0xd1, 0xd2, 0xd3,
	0xd4, 0x2b, 0x2c, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0x9f, 0xdc, 0x95, 0xdd, 0xde, 0xb5,
	0x21, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0x24, 0xe9, 0xea, 0xeb, 0xec,
	0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0x25, 0xf8, 0xf9, 0xfa, 0xfb,
	0x48, 0x47, 0x49, 0x4b, 0x4a, 0x4c, 0x4d, 0x59, 0x62, 0x61, 0x63, 0x64, 0x72, 0x71, 0x73, 0x74,
	0x5d, 0x83, 0x88, 0x87, 0x89, 0x8b, 0x8a, 0xfc, 0x8c, 0xa5, 0xa4, 0xa6, 0xa7, 0xb4, 0xfd, 0xa0,
	0x50, 0x4f, 0x51, 0x53, 0x52, 0x54, 0x55, 0x5b, 0x67, 0x66, 0x68, 0x69, 0x77, 0x76, 0x78, 0x79,
	0x5f, 0x85, 0x90, 0x8f, 0x91, 0x93, 0x92, 0xff, 0x94, 0xaa, 0xa9, 0xab, 0xac, 0xb7, 0xfe, 0xb8,
};


//
// Director 6.0 Win, cp1250 encoding
//
// ............................�ŤŹ�
// ť'-–—  .....!"#$%&()*,./:;?@[\]
// ^�_`{|}~ˇ¦¨Ż´¸ż�‘’‚“”„‹›+<=>±«»×
// ÷˘Ł¤Ą§©¬®°µ¶·†‡•…‰€0Ľ˝ľ1ą2˛3ł456
// 789aAŞáÁŕŔâÂäÄăĂĺĹćĆbBcCçÇdDđĐeE
// éÉčČęĘëËfF�gGhHiIíÍěĚîÎďĎjJkKlLm
// MnNńŃoOşóÓňŇôÔöÖőŐřŘśŚpPqQrRsSšŠ
// ßtTţŢ™uUúÚůŮűŰüÜvVwWxXyYýÝ.źzZžŽ

const byte orderTableD6win[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x09, 0x0a,
	0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
	0x26, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x21, 0x33, 0x34, 0x35, 0x58, 0x36, 0x22, 0x37, 0x38,
	0x73, 0x77, 0x79, 0x7b, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x39, 0x3a, 0x59, 0x5a, 0x5b, 0x3b,
	0x3c, 0x84, 0x95, 0x97, 0x9b, 0x9f, 0xa9, 0xac, 0xae, 0xb0, 0xba, 0xbc, 0xbe, 0xc0, 0xc2, 0xc6,
	0xd7, 0xd9, 0xdb, 0xdd, 0xe2, 0xe7, 0xf1, 0xf3, 0xf5, 0xf7, 0xfd, 0x3d, 0x3e, 0x3f, 0x40, 0x42,
	0x43, 0x83, 0x94, 0x96, 0x9a, 0x9e, 0xa8, 0xab, 0xad, 0xaf, 0xb9, 0xbb, 0xbd, 0xbf, 0xc1, 0xc5,
	0xd6, 0xd8, 0xda, 0xdc, 0xe1, 0xe6, 0xf0, 0xf2, 0xf4, 0xf6, 0xfc, 0x44, 0x45, 0x46, 0x47, 0x1b,
	0x72, 0x1c, 0x52, 0xaa, 0x55, 0x70, 0x6d, 0x6e, 0x41, 0x71, 0xdf, 0x56, 0xd5, 0x1d, 0xff, 0x1e,
	0x1f, 0x50, 0x51, 0x53, 0x54, 0x6f, 0x24, 0x25, 0x4f, 0xe5, 0xde, 0x57, 0xd4, 0x20, 0xfe, 0xfb,
	0x27, 0x48, 0x61, 0x62, 0x63, 0x64, 0x49, 0x65, 0x4a, 0x66, 0x85, 0x5d, 0x67, 0x23, 0x68, 0x4b,
	0x69, 0x5c, 0x7a, 0x7c, 0x4c, 0x6a, 0x6b, 0x6c, 0x4d, 0x78, 0xc7, 0x5e, 0x74, 0x75, 0x76, 0x4e,
	0x89, 0x87, 0x8b, 0x8f, 0x8d, 0x91, 0x93, 0x99, 0xa3, 0xa1, 0xa5, 0xa7, 0xb4, 0xb2, 0xb6, 0xb8,
	0x9d, 0xc4, 0xcb, 0xc9, 0xcd, 0xd1, 0xcf, 0x5f, 0xd3, 0xeb, 0xe9, 0xed, 0xef, 0xf9, 0xe4, 0xe0,
	0x88, 0x86, 0x8a, 0x8e, 0x8c, 0x90, 0x92, 0x98, 0xa2, 0xa0, 0xa4, 0xa6, 0xb3, 0xb1, 0xb5, 0xb7,
	0x9c, 0xc3, 0xca, 0xc8, 0xcc, 0xd0, 0xce, 0x60, 0xd2, 0xea, 0xe8, 0xec, 0xee, 0xf8, 0xe3, 0xfa,
};

static int getCharOrder(Common::u32char_type_t ch) {
	int num = charToNum(ch);

	if (num > 255)
		return num;

	Common::Platform pl = g_director->getPlatform();
	Common::Language lang = g_director->getLanguage();
	int version = g_director->getVersion();

	if (pl == Common::kPlatformMacintosh && lang != Common::JA_JPN && version < 500)
		return orderTableD2mac[num];

	if (pl == Common::kPlatformWindows && lang != Common::JA_JPN && version < 600)
		return orderTableD4win[num];

	if (pl == Common::kPlatformWindows && lang != Common::JA_JPN && version < 700)
		return orderTableD6win[num];

	return num;
}

int compareStrings(const Common::String &s1, const Common::String &s2) {
	Common::U32String u32S1 = s1.decode(Common::kUtf8);
	Common::U32String u32S2 = s2.decode(Common::kUtf8);
	const Common::u32char_type_t *p1 = u32S1.c_str();
	const Common::u32char_type_t *p2 = u32S2.c_str();

	uint32 c1, c2;
	do {
		c1 = getCharOrder(*p1);
		c2 = getCharOrder(*p2);
		p1++;
		p2++;
	} while (c1 == c2 && c1);
	return c1 - c2;
}

} // End of namespace Director
