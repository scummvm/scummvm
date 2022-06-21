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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "common/str.h"
#include "glk/scott/scott.h"
#include "glk/scott/globals.h"
#include "glk/scott/command_parser.h"
#include "glk/scott/decompress_text.h"
#include "glk/scott/decompress_z80.h"
#include "glk/scott/detection.h"
#include "glk/scott/detection_tables.h"
#include "glk/scott/game_info.h"
#include "glk/scott/hulk.h"
#include "glk/scott/line_drawing.h"
#include "glk/scott/saga_draw.h"
#include "glk/scott/c64_checksums.h"
#include "glk/scott/game_specific.h"
#include "glk/scott/resource.h"

namespace Glk {
namespace Scott {

struct DictionaryKey {
	DictionaryType _dict;
	const char *_signature;
};

DictionaryKey g_dictKeys[] = {
	{FOUR_LETTER_UNCOMPRESSED, "AUTO\0GO\0"},
	{THREE_LETTER_UNCOMPRESSED, "AUT\0GO\0"},
	{FIVE_LETTER_UNCOMPRESSED, "AUTO\0\0GO"},
	{FOUR_LETTER_COMPRESSED, "aUTOgO\0"},
	{GERMAN, "\xc7"
			 "EHENSTEIGE"},
	{FIVE_LETTER_COMPRESSED, "gEHENSTEIGE"}, // Gremlins C64
	{SPANISH, "ANDAENTRAVAN"},
	{FIVE_LETTER_UNCOMPRESSED, "*CROSS*RUN\0\0"}, // Claymorgue
	{ITALIAN, "AUTO\0VAI\0\0*ENTR"}
};

void readHeader(uint8_t *ptr) {
	int i, value;
	for (i = 0; i < 24; i++) {
		value = *ptr + 256 * *(ptr + 1);
		_G(_header)[i] = value;
		ptr += 2;
	}
}

int sanityCheckHeader(void) {
	int16_t v = _G(_gameHeader)->_numItems;
	if (v < 10 || v > 500)
		return 0;
	v = _G(_gameHeader)->_numActions;
	if (v < 100 || v > 500)
		return 0;
	v = _G(_gameHeader)->_numWords; // word pairs
	if (v < 50 || v > 190)
		return 0;
	v = _G(_gameHeader)->_numRooms; // Number of rooms
	if (v < 10 || v > 100)
		return 0;

	return 1;
}

uint8_t *readDictionary(GameInfo info, uint8_t **pointer, int loud) {
	uint8_t *ptr = *pointer;
	char *dictword = new char[info._wordLength + 2];
	char c = 0;
	int wordnum = 0;
	int charindex = 0;

	int nw = info._numberOfWords;

	int nv = info._numberOfVerbs;
	int nn = info._numberOfNouns;

	for (int i = 0; i <= MAX(nn, nw) - nv; i++) {
		_G(_verbs)[nv + i] = ".\0";
	}

	for (int i = 0; i <= MAX(nn, nw) - nn; i++) {
		_G(_nouns)[nn + i] = ".\0";
	}

	do {
		for (int i = 0; i < info._wordLength; i++) {
			c = *(ptr++);

			if (info._dictionary == FOUR_LETTER_COMPRESSED || info._dictionary == FIVE_LETTER_COMPRESSED) {
				if (charindex == 0) {
					if (c >= 'a') {
						c = toupper(c);
					} else if (c != '.' && c != 0) {
						dictword[charindex++] = '*';
					}
				}
				dictword[charindex++] = c;
			} else if (info._subType & LOCALIZED) {
				if (charindex == 0) {
					if (c & 0x80) {
						c = c & 0x7f;
					} else if (c != '.') {
						dictword[charindex++] = '*';
					}
				}
				dictword[charindex++] = c;
			} else {
				if (c == 0) {
					if (charindex == 0) {
						c = *(ptr++);
					}
				}
				if (c != ' ' && charindex > 0 && dictword[charindex - 1] == ' ') {
					i--;
					charindex--;
				}
				if (c == '*') {
					if (charindex != 0)
						charindex = 0;
					i = -1;
				}
				dictword[charindex++] = c;
			}
		}
		dictword[charindex] = 0;
		if (wordnum < nv) {
			_G(_verbs)[wordnum] = Common::String(dictword, charindex + 1);
			if (loud)
				debug("Verb %d: \"%s\"\n", wordnum, _G(_verbs)[wordnum].c_str());
		} else {
			_G(_nouns)[wordnum - nv] = Common::String(dictword, charindex + 1);
			if (loud)
				debug("Noun %d: \"%s\"\n", wordnum - nv, _G(_nouns)[wordnum - nv].c_str());
		}
		wordnum++;

		if (c != 0 && !isascii(c))
			return ptr;

		charindex = 0;
	} while (wordnum <= nv + nn);

	return ptr;
}

int parseHeader(int *h, HeaderType type, int *ni, int *na, int *nw, int *nr, int *mc, int *pr, int *tr, int *wl, int *lt, int *mn, int *trm) {
	switch (type) {
	case NO_HEADER:
		return 0;
	case EARLY:
		*ni = h[1];
		*na = h[2];
		*nw = h[3];
		*nr = h[4];
		*mc = h[5];
		*pr = h[6];
		*tr = h[7];
		*wl = h[8];
		*lt = h[9];
		*mn = h[10];
		*trm = h[11];
		break;
	case LATE:
		*ni = h[1];
		*na = h[2];
		*nw = h[3];
		*nr = h[4];
		*mc = h[5];
		*wl = h[6];
		*mn = h[7];
		*pr = 1;
		*tr = 0;
		*lt = -1;
		*trm = 0;
		break;
	case HULK_HEADER:
		*ni = h[3];
		*na = h[2];
		*nw = h[1];
		*nr = h[5];
		*mc = h[6];
		*pr = h[7];
		*tr = h[8];
		*wl = h[0];
		*lt = h[9];
		*mn = h[4];
		*trm = h[10];
		break;
	case ROBIN_C64_HEADER:
		*ni = h[1];
		*na = h[2];
		*nw = h[6];
		*nr = h[4];
		*mc = h[5];
		*pr = 1;
		*tr = 0;
		*wl = h[7];
		*lt = -1;
		*mn = h[3];
		*trm = 0;
		break;
	case GREMLINS_C64_HEADER:
		*ni = h[1];
		*na = h[2];
		*nw = h[5];
		*nr = h[3];
		*mc = h[6];
		*pr = h[8];
		*tr = 0;
		*wl = h[7];
		*lt = -1;
		*mn = 98;
		*trm = 0;
		break;
	case SUPERGRAN_C64_HEADER:
		*ni = h[3];
		*na = h[1];
		*nw = h[2];
		*nr = h[4];
		*mc = h[8];
		*pr = 1;
		*tr = 0;
		*wl = h[6];
		*lt = -1;
		*mn = h[5];
		*trm = 0;
		break;
	case SEAS_OF_BLOOD_C64_HEADER:
		*ni = h[0];
		*na = h[1];
		*nw = 134;
		*nr = h[3];
		*mc = h[4];
		*pr = 1;
		*tr = 0;
		*wl = h[6];
		*lt = -1;
		*mn = h[2];
		*trm = 0;
		break;
	case MYSTERIOUS_C64_HEADER:
		*ni = h[1];
		*na = h[2];
		*nw = h[3];
		*nr = h[4];
		*mc = h[5] & 0xff;
		*pr = h[5] >> 8;
		*tr = h[6];
		*wl = h[7];
		*lt = h[8];
		*mn = h[9];
		*trm = 0;
		break;
	case ARROW_OF_DEATH_PT_2_C64_HEADER:
		*ni = h[3];
		*na = h[1];
		*nw = h[2];
		*nr = h[4];
		*mc = h[5] & 0xff;
		*pr = h[5] >> 8;
		*tr = h[6];
		*wl = h[7];
		*lt = h[8];
		*mn = h[9];
		*trm = 0;
		break;
	case INDIANS_C64_HEADER:
		*ni = h[1];
		*na = h[2];
		*nw = h[3];
		*nr = h[4];
		*mc = h[5] & 0xff;
		*pr = h[5] >> 8;
		*tr = h[6] & 0xff;
		*wl = h[6] >> 8;
		*lt = h[7] >> 8;
		*mn = h[8] >> 8;
		*trm = 0;
		break;
	default:
		warning("Unhandled header type!\n");
		return 0;
	}
	return 1;
}

void printHeaderInfo(int *h, int ni, int na, int nw, int nr, int mc, int pr, int tr, int wl, int lt, int mn, int trm) {
	uint16_t value;
	for (int i = 0; i < 13; i++) {
		value = h[i];
		debug("b $%X %d: ", 0x494d + 0x3FE5 + i * 2, i);
		debug("\t%d\n", value);
	}

	debug("Number of items =\t%d\n", ni);
	debug("Number of actions =\t%d\n", na);
	debug("Number of words =\t%d\n", nw);
	debug("Number of rooms =\t%d\n", nr);
	debug("Max carried items =\t%d\n", mc);
	debug("Word length =\t%d\n", wl);
	debug("Number of messages =\t%d\n", mn);
	debug("Player start location: %d\n", pr);
	debug("Treasure room: %d\n", tr);
	debug("Lightsource time left: %d\n", lt);
	debug("Number of treasures: %d\n", tr);
}

void loadVectorData(GameInfo info, uint8_t *ptr) {
	int offset;

	if (info._startOfImageData == FOLLOWS)
		ptr++;
	else if (seekIfNeeded(info._startOfImageData, &offset, &ptr) == 0)
		return;

	_G(_lineImages).resize(info._numberOfRooms);
	int ct = 0;
	LineImage *lp = &_G(_lineImages)[0];
	uint8_t byte = *(ptr++);
	do {
		_G(_rooms)[ct]._image = 0;
		if (byte == 0xff) {
			lp->_bgColour = *(ptr++);
			lp->_data = ptr;
		} else {
			error("loadVectorData: Error! Image data does not start with 0xff");
		}
		do {
			byte = *(ptr++);
			if (ptr > _G(_entireFile) && static_cast<size_t>(ptr - _G(_entireFile)) >= _G(_fileLength)) {
				warning("loadVectorData: Error! Image data for image %d cut off", ct);
				if (_G(_gameHeader)->_numRooms - ct > 1)
					g_scott->display(_G(_bottomWindow), "[This copy has %d broken or missing pictures. These have been patched out.]\n\n", _G(_gameHeader)->_numRooms - ct);
				if (lp->_data >= ptr)
					lp->_size = 0;
				else
					lp->_size = ptr - lp->_data - 1;
				for (int i = ct + 2; i < _G(_gameHeader)->_numRooms; i++)
					_G(_rooms)[i]._image = 255;
				return;
			}
		} while (byte != 0xff);

		lp->_size = ptr - lp->_data;
		lp++;
		ct++;
	} while (ct < info._numberOfRooms);
}

uint8_t *seekToPos(uint8_t *buf, size_t offset) {
	if (offset > _G(_fileLength))
		return 0;
	return buf + offset;
}

int seekIfNeeded(int expectedStart, int *offset, uint8_t **ptr) {
	if (expectedStart != FOLLOWS) {
		*offset = expectedStart + _G(_fileBaselineOffset);
		*ptr = seekToPos(_G(_entireFile), *offset);
		if (*ptr == 0)
			return 0;
	}
	return 1;
}

int tryLoadingOld(GameInfo info, int dictStart) {
	int ni, na, nw, nr, mc, pr, tr, wl, lt, mn, trm;
	int ct;

	Action *ap;
	Room *rp;
	Item *ip;
	/* Load the header */

	uint8_t *ptr = _G(_entireFile);
	_G(_fileBaselineOffset) = dictStart - info._startOfDictionary;
	int offset = info._startOfHeader + _G(_fileBaselineOffset);

	ptr = seekToPos(_G(_entireFile), offset);
	if (ptr == 0)
		return 0;

	readHeader(ptr);

	if (!parseHeader(_G(_header), info._headerStyle, &ni, &na, &nw, &nr, &mc, &pr, &tr, &wl, &lt, &mn, &trm))
		return 0;

	if (ni != info._numberOfItems || na != info._numberOfActions || nw != info._numberOfWords || nr != info._numberOfRooms || mc != info._maxCarried) {
		return 0;
	}

	_G(_gameHeader)->_numItems = ni;
	_G(_gameHeader)->_numActions = na;
	_G(_gameHeader)->_numWords = nw;
	_G(_gameHeader)->_wordLength = wl;
	_G(_gameHeader)->_numRooms = nr;
	_G(_gameHeader)->_maxCarry = mc;
	_G(_gameHeader)->_playerRoom = pr;
	_G(_gameHeader)->_treasures = tr;
	_G(_gameHeader)->_lightTime = lt;
	_G(_lightRefill) = lt;
	_G(_gameHeader)->_numMessages = mn;
	_G(_gameHeader)->_treasureRoom = trm;
	_G(_items).resize(ni + 1);
	_G(_actions).resize(na + 1);
	_G(_verbs).resize(nw + 2);
	_G(_nouns).resize(nw + 2);
	_G(_rooms).resize(nr + 1);
	_G(_messages).resize(mn + 1);

	// actions
	if (seekIfNeeded(info._startOfActions, &offset, &ptr) == 0)
		return 0;

	ct = 0;

	uint16_t value, cond, comm;

	while (ct < na + 1) {
		ap = &_G(_actions)[ct];
		value = *(ptr++);
		value += *(ptr++) * 0x100; /* verb/noun */
		ap->_vocab = value;

		cond = 5;
		comm = 2;

		for (int j = 0; j < 5; j++) {
			if (j < cond) {
				value = *(ptr++);
				value += *(ptr++) * 0x100;
			} else
				value = 0;
			ap->_condition[j] = value;
		}
		for (int j = 0; j < 2; j++) {
			if (j < comm) {
				value = *(ptr++);
				value += *(ptr++) * 0x100;
			} else
				value = 0;
			ap->_action[j] = value;
		}
		ct++;
	}

	// room connections
	if (seekIfNeeded(info._startOfRoomConnections, &offset, &ptr) == 0)
		return 0;

	ct = 0;

	while (ct < nr + 1) {
		rp = &_G(_rooms)[ct];
		for (int j = 0; j < 6; j++) {
			rp->_exits[j] = *(ptr++);
		}
		ct++;
	}

	// item locations
	if (seekIfNeeded(info._startOfItemLocations, &offset, &ptr) == 0)
		return 0;

	ct = 0;
	while (ct < ni + 1) {
		ip = &_G(_items)[ct];
		ip->_location = *(ptr++);
		ip->_initialLoc = ip->_location;
		ct++;
	}

	// dictionary
	if (seekIfNeeded(info._startOfDictionary, &offset, &ptr) == 0)
		return 0;

	ptr = readDictionary(info, &ptr, 0);

	// rooms
	if (seekIfNeeded(info._startOfRoomDescriptions, &offset, &ptr) == 0)
		return 0;

	if (info._startOfRoomDescriptions == FOLLOWS)
		ptr++;

	ct = 0;

	char text[256];
	char c = 0;
	int charindex = 0;

	do {
		c = *(ptr++);
		text[charindex] = c;
		rp = &_G(_rooms)[ct];
		if (c == 0) {
			rp->_text = text;
			if (info._numberOfPictures > 0)
				rp->_image = ct - 1;
			else
				rp->_image = 255;
			ct++;
			charindex = 0;
		} else {
			charindex++;
		}
		if (c != 0 && !isascii(c))
			return 0;
	} while (ct < nr + 1);

	// messages
	if (seekIfNeeded(info._startOfMessages, &offset, &ptr) == 0)
		return 0;

	ct = 0;
	charindex = 0;

	while (ct < mn + 1) {
		c = *(ptr++);
		text[charindex] = c;
		if (c == 0) {
			_G(_messages)[ct] = text;
			ct++;
			charindex = 0;
		} else {
			charindex++;
		}
	}

	// items
	if (seekIfNeeded(info._startOfItemDescriptions, &offset, &ptr) == 0)
		return 0;

	ct = 0;
	charindex = 0;

	do {
		ip = &_G(_items)[ct];
		c = *(ptr++);
		text[charindex] = c;
		if (c == 0) {
			ip->_text = text;
			const char *p = strchr(ip->_text.c_str(), '/');
			/* Some games use // to mean no auto get/drop word! */
			if (p) {
				ip->_autoGet = Common::String(p);
				if (!(ip->_autoGet == "//") && !(ip->_autoGet == "/*")) {
					ip->_text = Common::String(ip->_text.c_str(), p);
					ip->_autoGet.deleteChar(0);
					const char *t = strchr(ip->_autoGet.c_str(), '/');
					if (t) {
						ip->_autoGet = Common::String(ip->_autoGet.c_str(), t);
					}
				}
			}
			ct++;
			charindex = 0;
		} else {
			charindex++;
		}
	} while (ct < ni + 1);

	// line images
	if (info._numberOfPictures > 0) {
		loadVectorData(info, ptr);
	}

	// system messages
	ct = 0;
	if (seekIfNeeded(info._startOfSystemMessages, &offset, &ptr) == 0)
		return 1;

	charindex = 0;

	do {
		c = *(ptr++);
		text[charindex] = c;
		if (c == 0 || c == 0x0d) {
			if (charindex > 0) {
				if (c == 0x0d)
					charindex++;
				text[charindex] = '\0';
				_G(_systemMessages)[ct] = Common::String(text, charindex + 1);
				ct++;
				charindex = 0;
			}
		} else {
			charindex++;
		}

		if (c != 0 && c != 0x0d && c != '\x83' && c != '\xc9' && !isascii(c))
			break;
	} while (ct < 40);

	charindex = 0;

	if (seekIfNeeded(info._startOfDirections, &offset, &ptr) == 0)
		return 0;

	ct = 0;
	do {
		c = *(ptr++);
		text[charindex] = c;
		if (c == 0 || c == 0x0d) {
			if (charindex > 0) {
				if (c == 0x0d)
					charindex++;
				text[charindex] = '\0';
				_G(_sys)[ct] = Common::String(text, charindex + 1);
				ct++;
				charindex = 0;
			}
		} else {
			charindex++;
		}

		if (c != 0 && c != 0x0d && !isascii(c))
			break;
	} while (ct < 6);

	return 1;
}

int tryLoading(GameInfo info, int dictStart, int loud) {
	// The Hulk does everything differently so it gets its own function
	if (info._gameID == HULK || info._gameID == HULK_C64)
		return tryLoadingHulk(info, dictStart);

	if (info._type == OLD_STYLE)
		return tryLoadingOld(info, dictStart);

	int ni, na, nw, nr, mc, pr, tr, wl, lt, mn, trm;
	int ct;

	Action *ap;
	Room *rp;
	Item *ip;

	/* Load the header */
	uint8_t *ptr = _G(_entireFile);

	if (loud) {
		debug("dict_start:%x\n", dictStart);
		debug(" info._start_of_dictionary:%x\n", info._startOfDictionary);
	}
	_G(_fileBaselineOffset) = dictStart - info._startOfDictionary;

	if (loud) {
		debug("_fileBaselineOffset:%x (%d)\n", _G(_fileBaselineOffset), _G(_fileBaselineOffset));
	}

	int offset = info._startOfHeader + _G(_fileBaselineOffset);

	ptr = seekToPos(_G(_entireFile), offset);
	if (ptr == 0)
		return 0;

	readHeader(ptr);

	if (!parseHeader(_G(_header), info._headerStyle, &ni, &na, &nw, &nr, &mc, &pr, &tr, &wl, &lt, &mn, &trm))
		return 0;

	if (loud)
		printHeaderInfo(_G(_header), ni, na, nw, nr, mc, pr, tr, wl, lt, mn, trm);

	_G(_gameHeader)->_numItems = ni;
	_G(_gameHeader)->_numActions = na;
	_G(_gameHeader)->_numWords = nw;
	_G(_gameHeader)->_wordLength = wl;
	_G(_gameHeader)->_numRooms = nr;
	_G(_gameHeader)->_maxCarry = mc;
	_G(_gameHeader)->_playerRoom = pr;
	_G(_gameHeader)->_treasures = tr;
	_G(_gameHeader)->_lightTime = lt;
	_G(_lightRefill) = lt;
	_G(_gameHeader)->_numMessages = mn;
	_G(_gameHeader)->_treasureRoom = trm;

	if (sanityCheckHeader() == 0) {
		return 0;
	}

	if (loud) {
		debug("Found a valid header at position 0x%x\n", offset);
		debug("Expected: 0x%x\n", info._startOfHeader + _G(_fileBaselineOffset));
	}

	if (ni != info._numberOfItems || na != info._numberOfActions || nw != info._numberOfWords || nr != info._numberOfRooms || mc != info._maxCarried) {
		if (loud) {
			debug("Non-matching header\n");
		}
		return 0;
	}

	_G(_items).resize(ni + 1);
	_G(_actions).resize(na + 1);
	_G(_verbs).resize(nw + 2);
	_G(_nouns).resize(nw + 2);
	_G(_rooms).resize(nr + 1);
	_G(_messages).resize(mn + 1);

	int compressed = (info._dictionary == FOUR_LETTER_COMPRESSED);

	// room images
	if (info._startOfRoomImageList != 0) {
		if (seekIfNeeded(info._startOfRoomImageList, &offset, &ptr) == 0)
			return 0;

		for (ct = 0; ct <= _G(_gameHeader)->_numRooms; ct++) {
			rp = &_G(_rooms)[ct];
			rp->_image = *(ptr++);
		}
	}

	// item flags
	if (info._startOfItemFlags != 0) {

		if (seekIfNeeded(info._startOfItemFlags, &offset, &ptr) == 0)
			return 0;

		for (ct = 0; ct <= _G(_gameHeader)->_numItems; ct++) {
			ip = &_G(_items)[ct];
			ip->_flag = *(ptr++);
		}
	}

	// item images
	if (info._startOfItemImageList != 0) {

		if (seekIfNeeded(info._startOfItemImageList, &offset, &ptr) == 0)
			return 0;

		for (ct = 0; ct <= _G(_gameHeader)->_numItems; ct++) {
			ip = &_G(_items)[ct];
			ip->_image = *(ptr++);
		}
		if (loud)
			debug("Offset after reading item images: %d\n", static_cast<int>(ptr - _G(_entireFile) - _G(_fileBaselineOffset)));
	}

	// actions
	if (seekIfNeeded(info._startOfActions, &offset, &ptr) == 0)
		return 0;

	ct = 0;

	uint16_t value, cond, comm;

	while (ct < na + 1) {
		ap = &_G(_actions)[ct];
		value = *(ptr++);
		value += *(ptr++) * 0x100; /* verb/noun */
		ap->_vocab = value;

		if (info._actionsStyle == COMPRESSED) {
			value = *(ptr++); /* count of actions/conditions */
			cond = value & 0x1f;
			if (cond > 5) {
				debug("Condition error at action %d!", ct);
				cond = 5;
			}
			comm = (value & 0xe0) >> 5;
			if (comm > 2) {
				debug("Command error at action %d!", ct);
				comm = 2;
			}
		} else {
			cond = 5;
			comm = 2;
		}
		for (int j = 0; j < 5; j++) {
			if (j < cond) {
				value = *(ptr++);
				value += *(ptr++) * 0x100;
			} else
				value = 0;
			ap->_condition[j] = value;
		}
		for (int j = 0; j < 2; j++) {
			if (j < comm) {
				value = *(ptr++);
				value += *(ptr++) * 0x100;
			} else
				value = 0;
			ap->_action[j] = value;
		}

		ct++;
	}
	if (loud)
		debug("Offset after reading actions: %d\n", static_cast<int>(ptr - _G(_entireFile) - _G(_fileBaselineOffset)));

	// dictionary
	if (seekIfNeeded(info._startOfDictionary, &offset, &ptr) == 0)
		return 0;

	ptr = readDictionary(info, &ptr, loud);

	if (loud)
		debug("Offset after reading dictionary: %d\n", static_cast<int>(ptr - _G(_entireFile) - _G(_fileBaselineOffset)));

	// rooms
	if (info._startOfRoomDescriptions != 0) {
		if (seekIfNeeded(info._startOfRoomDescriptions, &offset, &ptr) == 0)
			return 0;

		ct = 0;

		char text[256];
		char c = 0;
		int charindex = 0;

		if (!compressed) {
			do {
				rp = &_G(_rooms)[ct];
				c = *(ptr++);
				text[charindex] = c;
				if (c == 0) {
					rp->_text = text;
					if (loud)
						debug("Room %d: %s\n", ct, rp->_text.c_str());
					ct++;
					charindex = 0;
				} else {
					charindex++;
				}
				if (c != 0 && !isascii(c))
					return 0;
			} while (ct < nr + 1);
		} else {
			do {
				rp = &_G(_rooms)[ct];
				rp->_text = decompressText(ptr, ct);
				if (rp->_text.size() == 0)
					return 0;
				rp->_text.replace(0, 1, Common::String(tolower(rp->_text[0])));
				ct++;
			} while (ct < nr);
		}
	}

	// room connections
	if (seekIfNeeded(info._startOfRoomConnections, &offset, &ptr) == 0)
		return 0;

	ct = 0;

	while (ct < nr + 1) {
		rp = &_G(_rooms)[ct];
		for (int j = 0; j < 6; j++) {
			rp->_exits[j] = *(ptr++);
		}

		ct++;
	}

	// messages
	if (seekIfNeeded(info._startOfMessages, &offset, &ptr) == 0)
		return 0;

	ct = 0;
	char text[256];
	char c = 0;
	int charindex = 0;

	if (compressed) {
		while (ct < mn + 1) {
			_G(_messages)[ct] = decompressText(ptr, ct);
			if (loud)
				debug("Message %d: \"%s\"\n", ct, _G(_messages)[ct].c_str());
			if (_G(_messages)[ct].size() == 0)
				return 0;
			ct++;
		}
	} else {
		while (ct < mn + 1) {
			c = *(ptr++);
			text[charindex] = c;
			if (c == 0) {
				_G(_messages)[ct] = text;
				if (loud)
					debug("Message %d: \"%s\"\n", ct, _G(_messages)[ct].c_str());
				ct++;
				charindex = 0;
			} else {
				charindex++;
			}
		}
	}

	// items
	if (seekIfNeeded(info._startOfItemDescriptions, &offset, &ptr) == 0)
		return 0;

	ct = 0;
	charindex = 0;

	if (compressed) {
		do {
			ip = &_G(_items)[ct];
			ip->_text = decompressText(ptr, ct);
			ip->_autoGet = "";
			if (ip->_text != "" && ip->_text[0] != '.') {
				if (loud)
					debug("Item %d: %s\n", ct, ip->_text.c_str());
				const char *p = strchr(ip->_text.c_str(), '.');
				if (p) {
					ip->_autoGet = Common::String(p);
					ip->_text = Common::String(ip->_text.c_str(), p);
					ip->_autoGet.deleteChar(0);
					ip->_autoGet.deleteChar(0);
					const char *t = strchr(ip->_autoGet.c_str(), '.');
					if (t)
						ip->_autoGet = Common::String(ip->_autoGet.c_str(), t);
					for (int i = 1; i < ip->_autoGet.size(); i++)
						ip->_autoGet.replace(i, 1, Common::String(toupper(ip->_autoGet[i])));
				}
			}
			ct++;
		} while (ct < ni + 1);
	} else {
		do {
			ip = &_G(_items)[ct];
			c = *(ptr++);
			text[charindex] = c;
			if (c == 0) {
				ip->_text = text;
				if (loud)
					debug("Item %d: %s\n", ct, ip->_text.c_str());
				const char *p = strchr(ip->_text.c_str(), '/');
				/* Some games use // to mean no auto get/drop word! */
				if (p) {
					ip->_autoGet = Common::String(p);
					if (!(ip->_autoGet == "//") && !(ip->_autoGet == "/*")) {
						ip->_text = Common::String(ip->_text.c_str(), p);
						ip->_autoGet.deleteChar(0);
						const char *t = strchr(ip->_autoGet.c_str(), '/');
						if (t) {
							ip->_autoGet = Common::String(ip->_autoGet.c_str(), t);
						}
					}
				}
				ct++;
				charindex = 0;
			} else {
				charindex++;
			}
		} while (ct < ni + 1);
	}

	// item locations
	if (seekIfNeeded(info._startOfItemLocations, &offset, &ptr) == 0)
		return 0;

	ct = 0;
	while (ct < ni + 1) {
		ip = &_G(_items)[ct];
		ip->_location = *(ptr++);
		ip->_initialLoc = ip->_location;
		ct++;
	}

	// vector images
	if (info._numberOfPictures > 0 && info._pictureFormatVersion == 99) {
		loadVectorData(info, ptr);
	}

	// system messages
	if (seekIfNeeded(info._startOfSystemMessages, &offset, &ptr) == 0)
		return 1;
jumpSysMess:
	ptr = seekToPos(_G(_entireFile), offset);
	ct = 0;
	charindex = 0;

	do {
		c = *(ptr++);
		if ((ptr > _G(_entireFile) && static_cast<size_t>(ptr - _G(_entireFile)) > _G(_fileLength)) || ptr < _G(_entireFile)) {
			debug("Read out of bounds!\n");
			return 0;
		}
		if (charindex > 255)
			charindex = 0;
		text[charindex] = c;
		if (c == 0 || c == 0x0d) {
			if (charindex > 0) {
				if (c == 0x0d)
					charindex++;
				if (ct == 0 && (info._subType & (C64 | ENGLISH)) == (C64 | ENGLISH) && memcmp(text, "NORTH", 5) != 0) {
					offset--;
					goto jumpSysMess;
				}
				text[charindex] = '\0';
				_G(_systemMessages)[ct] = Common::String(text, charindex + 1);
				if (loud)
					debug("system_messages %d: \"%s\"\n", ct, _G(_systemMessages)[ct].c_str());
				ct++;
				charindex = 0;
			}
		} else {
			charindex++;
		}
	} while (ct < 45);

	if (loud)
		debug("Offset after reading system messages: %d\n", static_cast<int>(ptr - _G(_entireFile)));

	if ((info._subType & (C64 | ENGLISH)) == (C64 | ENGLISH)) {
		return 1;
	}

	if (seekIfNeeded(info._startOfDirections, &offset, &ptr) == 0)
		return 0;

	charindex = 0;

	ct = 0;
	do {
		c = *(ptr++);
		text[charindex] = c;
		if (c == 0 || c == 0x0d) {
			if (charindex > 0) {
				if (c == 0x0d)
					charindex++;
				text[charindex] = '\0';
				_G(_sys)[ct] = Common::String(text, charindex + 1);
				ct++;
				charindex = 0;
			}
		} else {
			charindex++;
		}

		if (c != 0 && c != 0x0d && !isascii(c))
			break;
	} while (ct < 6);

	return 1;
}

DictionaryType getId(int *offset) {
	for (int i = 0; i < 9; i++) {
		*offset = findCode(g_dictKeys[i]._signature, 0);
		if (*offset != -1) {
			if (i == 4 || i == 5) // GERMAN
				*offset -= 5;
			else if (i == 6) // SPANISH
				*offset -= 8;
			else if (i == 7) // Claymorgue
				*offset -= 11;
			return g_dictKeys[i]._dict;
		}
	}

	return NOT_A_GAME;
}

int findCode(const char *x, int base) {
	unsigned const char *p = _G(_entireFile) + base;
	int len = strlen(x);
	if (len < 7)
		len = 7;
	while (p < _G(_entireFile) + _G(_fileLength) - len) {
		if (memcmp(p, x, len) == 0) {
			return p - _G(_entireFile);
		}
		p++;
	}
	return -1;
}

} // End of namespace Scott
} // End of namespace Glk
