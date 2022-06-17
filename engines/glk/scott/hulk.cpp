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

#include "glk/scott/scott.h"
#include "glk/scott/definitions.h"
#include "glk/scott/resource.h"
#include "glk/scott/globals.h"
#include "glk/scott/types.h"
#include "glk/scott/hulk.h"

namespace Glk {
namespace Scott {

void hulkShowImageOnExamine(int noun) {
	int image = 0;
	switch (noun) {
	case 55: // Dome
		if (_G(_items)[11]._location == MY_LOC)
			image = 28;
		break;
	case 108: // Natter energy egg
		if (_G(_items)[17]._location == MY_LOC || _G(_items)[17]._location == CARRIED)
			image = 30;
		break;
	case 124: // Bio-Gem
	case 41:
		if (_G(_items)[18]._location == MY_LOC || _G(_items)[18]._location == CARRIED)
			image = 29;
		break;
	case 21: // Killer Bees
		if (_G(_items)[24]._location == MY_LOC)
			image = 31;
		break;
	case 83: // Iron ring
		if (_G(_items)[33]._location == MY_LOC)
			image = 32;
		break;
	case 121: // Cage
		if (_G(_items)[47]._location == MY_LOC)
			image = 33;
		break;
	default:
		break;
	}
	if (image) {
		g_scott->drawImage(image);
		g_scott->output(_G(_sys)[HIT_ENTER]);
		g_scott->hitEnter();
	}
}

void hulkLook() {
	g_scott->drawImage(_G(_rooms)[MY_LOC]._image);
	for (int ct = 0; ct <= _G(_gameHeader)->_numItems; ct++) {
		int image = _G(_items)[ct]._image;
		if (_G(_items)[ct]._location == MY_LOC && image != 255) {
			/* Don't draw bio gem in fuzzy area */
			if ((ct == 18 && MY_LOC != 15) ||
				/* Don't draw Dr. Strange until outlet is plugged */
				(ct == 26 && _G(_items)[28]._location != MY_LOC))
				continue;
			g_scott->drawImage(image);
		}
	}
}

void drawHulkImage(int p) {
	int image = 0;
	switch (p) {
	case 85:
		image = 34;
		break;
	case 86:
		image = 35;
		break;
	case 83:
		image = 36;
		break;
	case 84:
		image = 37;
		break;
	case 87:
		image = 38;
		break;
	case 88:
		image = 39;
		break;
	case 89:
		image = 40;
		break;
	case 82:
		image = 41;
		break;
	case 81:
		image = 42;
		break;
	default:

		error("drawHulkImage: Unhandled image number %d", p);
		break;
	}

	if (image != 0) {
		g_scott->drawImage(image);
		g_scott->output(_G(_sys)[HIT_ENTER]);
		g_scott->hitEnter();
	}
}

uint8_t *readHulkDictionary(GameInfo info, uint8_t **pointer) {
	uint8_t *ptr = *pointer;
	char *dictword = new char[info._wordLength + 2];
	char c = 0;
	int wordnum = 0;
	int charindex = 0;

	int nv = info._numberOfVerbs;
	int nn = info._numberOfNouns;

	for (int i = 0; i < nn - nv; i++)
		_G(_verbs)[nv + i] = ".\0";

	for (int i = 0; i < nv - nn; i++)
		_G(_nouns)[nn + i] = ".\0";

	do {
		for (int i = 0; i < info._wordLength; i++) {
			c = *(ptr++);
			if (c == 0) {
				if (charindex == 0) {
					c = *(ptr++);
				}
			}
			dictword[charindex] = c;
			if (c == '*')
				i--;
			charindex++;

			dictword[charindex] = 0;
		}

		if (wordnum < nn) {
			_G(_nouns)[wordnum] = Common::String(dictword, charindex + 1);
		} else {
			_G(_verbs)[wordnum - nn] = Common::String(dictword, charindex + 1);
		}
		wordnum++;

		if (c != 0 && !isascii(c))
			return ptr;

		charindex = 0;
	} while (wordnum <= nv + nn);

	delete[] dictword;
	return ptr;
}

int tryLoadingHulk(GameInfo info, int dictStart) {
	int ni, na, nw, nr, mc, pr, tr, wl, lt, mn, trm;
	int ct;

	Action *ap;
	Room *rp;
	Item *ip;

	/* Load the header */
	uint8_t *ptr = _G(_entireFile);

	_G(_fileBaselineOffset) = dictStart - info._startOfDictionary - 645;

	int offset = info._startOfHeader + _G(_fileBaselineOffset);
	ptr = seekToPos(_G(_entireFile), offset);

	if (ptr == 0)
		return 0;

	readHeader(ptr);

	parseHeader(_G(_header), info._headerStyle, &ni, &na, &nw, &nr, &mc, &pr, &tr, &wl, &lt, &mn, &trm);

	_G(_gameHeader)->_numItems = ni;
	_G(_items).resize(ni + 1);
	_G(_gameHeader)->_numActions = na;
	_G(_actions).resize(na + 1);
	_G(_gameHeader)->_numWords = nw;
	_G(_gameHeader)->_wordLength = wl;
	_G(_verbs).resize(nw + 1);
	_G(_nouns).resize(nw + 1);
	_G(_gameHeader)->_numRooms = nr;
	_G(_rooms).resize(nr + 1);
	_G(_gameHeader)->_maxCarry = mc;
	_G(_gameHeader)->_playerRoom = pr;
	_G(_gameHeader)->_treasures = tr;
	_G(_gameHeader)->_lightTime = lt;
	_G(_lightRefill) = lt;
	_G(_gameHeader)->_numMessages = mn;
	_G(_messages).resize(mn + 1);
	_G(_gameHeader)->_treasureRoom = trm;

	if (_G(_header)[0] != info._wordLength || _G(_header)[1] != info._numberOfWords || _G(_header)[2] != info._numberOfActions || _G(_header)[3] != info._numberOfItems || _G(_header)[4] != info._numberOfMessages || _G(_header)[5] != info._numberOfRooms || _G(_header)[6] != info._maxCarried) {
		return 0;
	}

#pragma mark Dictionary

	if (seekIfNeeded(info._startOfDictionary, &offset, &ptr) == 0)
		return 0;

	readHulkDictionary(info, &ptr);

#pragma mark Rooms

	if (seekIfNeeded(info._startOfRoomDescriptions, &offset, &ptr) == 0)
		return 0;

	ct = 0;

	uint8_t string_length = 0;
	do {
		rp = &_G(_rooms)[ct];
		string_length = *(ptr++);
		if (string_length == 0) {
			rp->_text = ".\0";
		} else {
			for (int i = 0; i < string_length; i++) {
				rp->_text += *(ptr++);
			}
		}
		ct++;
	} while (ct < nr + 1);

#pragma mark Messages

	ct = 0;
	const char *string;

	do {
		string_length = *(ptr++);
		if (string_length == 0) {
			string = ".\0";
			_G(_messages)[ct] = string;
		} else {
			char *s = new char[string_length + 1];
			for (int i = 0; i < string_length; i++) {
				s[i] = *(ptr++);
			}
			s[string_length] = 0;
			_G(_messages)[ct] = s;
		}
		ct++;
	} while (ct < mn + 1);

#pragma mark Items

	if (seekIfNeeded(info._startOfItemDescriptions, &offset, &ptr) == 0)
		return 0;

	ct = 0;

	do {
		ip = &_G(_items)[ct];
		string_length = *(ptr++);
		if (string_length == 0) {
			ip->_text = ".\0";
		} else {
			for (int i = 0; i < string_length; i++) {
				ip->_text += *(ptr++);
			}
			const char *p = strchr(ip->_text.c_str(), '/');
			if (p) {
				ip->_autoGet = Common::String(p);

				/* Some games use // to mean no auto get/drop word! */
				if (!(ip->_autoGet == "//") && !(ip->_autoGet == "/*")) {
					ip->_text = Common::String(ip->_text.c_str(), p);
					ip->_autoGet.deleteChar(0);

					const char *t = strchr(ip->_autoGet.c_str(), '/');
					if (t) {
						ip->_autoGet = Common::String(ip->_autoGet.c_str(), t);
					}
					ptr++;
				}
			}
		}

		ct++;
	} while (ct < ni + 1);

#pragma mark Room connections

	if (seekIfNeeded(info._startOfRoomConnections, &offset, &ptr) == 0)
		return 0;

	/* The room connections are ordered by direction, not room, so all the North
	 * connections for all the rooms come first, then the South connections, and
	 * so on. */
	for (int j = 0; j < 6; j++) {
		ct = 0;

		while (ct < nr + 1) {
			rp = &_G(_rooms)[ct];
			rp->_exits[j] = *(ptr++);
			ptr++;
			ct++;
		}
	}

#pragma mark item locations

	if (seekIfNeeded(info._startOfItemLocations, &offset, &ptr) == 0)
		return 0;

	ct = 0;
	while (ct < ni + 1) {
		ip = &_G(_items)[ct];
		ip->_location = *(ptr++);
		ip->_location += *(ptr++) * 0x100;
		ip->_initialLoc = ip->_location;
		ct++;
	}

#pragma mark room images

	if (seekIfNeeded(info._startOfRoomImageList, &offset, &ptr) == 0)
		return 0;

	for (ct = 0; ct <= _G(_gameHeader)->_numRooms; ct++) {
		rp = &_G(_rooms)[ct];
		rp->_image = *(ptr++);
	}

#pragma mark item images

	if (seekIfNeeded(info._startOfItemImageList, &offset, &ptr) == 0)
		return 0;

	for (ct = 0; ct <= _G(_gameHeader)->_numItems; ct++) {
		ip = &_G(_items)[ct];
		ip->_image = 255;
	}

	int index, image = 10;

	for (index = (*ptr++); index != 255; index = (*ptr++)) {
		_G(_items)[index]._image = image++;
	}

#pragma mark item flags

	/* Hulk does not seem to use item flags */

#pragma mark Actions

	if (seekIfNeeded(info._startOfActions, &offset, &ptr) == 0)
		return 0;

	ct = 0;

	int verb, noun, value, value2, plus;
	while (ct < na + 1) {
		ap = &_G(_actions)[ct];
		plus = na + 1;
		verb = _G(_entireFile)[offset + ct];
		noun = _G(_entireFile)[offset + ct + plus];

		ap->_vocab = verb * 150 + noun;

		for (int j = 0; j < 2; j++) {
			plus += na + 1;
			value = _G(_entireFile)[offset + ct + plus];
			plus += na + 1;
			value2 = _G(_entireFile)[offset + ct + plus];
			ap->_action[j] = 150 * value + value2;
		}

		int offset2 = offset + 0x624;
		plus = 0;

		for (int j = 0; j < 5; j++) {
			value = _G(_entireFile)[offset2 + ct * 2 + plus];
			value2 = _G(_entireFile)[offset2 + ct * 2 + plus + 1];
			ap->_condition[j] = value + value2 * 0x100;
			plus += (na + 1) * 2;
		}
		ct++;
	}
	return 1;
}

} // End of namespace Scott
} // End of namespace Glk
