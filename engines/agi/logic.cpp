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

#include "agi/agi.h"

namespace Agi {

/**
 * Decode logic resource
 * This function decodes messages from the specified raw logic resource
 * into a message list.
 * @param n  The number of the logic resource to decode.
 */
int AgiEngine::decodeLogic(int16 logicNr) {
	int ec = errOK;
	int mstart, mend, mc;
	uint8 *m0;
	AgiLogic *curLogic = &_game.logics[logicNr];

	// decrypt messages at end of logic + build message list

	// report ("decoding logic #%d\n", n);
	m0 = curLogic->data;

	mstart = READ_LE_UINT16(m0) + 2;
	mc = *(m0 + mstart);
	mend = READ_LE_UINT16(m0 + mstart + 1);
	m0 += mstart + 3;   // cover header info
	mstart = mc << 1;

	// if the logic was not compressed, decrypt the text messages
	// only if there are more than 0 messages
	if ((~_game.dirLogic[logicNr].flags & RES_COMPRESSED) && mc > 0)
		decrypt(m0 + mstart, mend - mstart);    // decrypt messages

	// build message list
	m0 = curLogic->data;
	mstart = READ_LE_UINT16(m0) + 2;    // +2 covers pointer
	_game.logics[logicNr].numTexts = *(m0 + mstart);

	// resetp logic pointers
	curLogic->sIP = 2;
	curLogic->cIP = 2;
	curLogic->size = READ_LE_UINT16(m0) + 2;    // logic end pointer

	// allocate list of pointers to point into our data

	curLogic->texts = (const char **)calloc(1 + curLogic->numTexts, sizeof(char *));

	// cover header info
	m0 += mstart + 3;

	if (curLogic->texts != NULL) {
		// move list of strings into list to make real pointers
		for (mc = 0; mc < curLogic->numTexts; mc++) {
			mend = READ_LE_UINT16(m0 + mc * 2);
			_game.logics[logicNr].texts[mc] = mend ? (const char *)m0 + mend - 2 : (const char *)"";
		}
		// set loaded flag now its all completly loaded
		_game.dirLogic[logicNr].flags |= RES_LOADED;
	} else {
		// unload data
		// Note that not every logic has text
		free(curLogic->data);
		ec = errNotEnoughMemory;
	}

	return ec;
}

/**
 * Unload logic resource
 * This function unloads the specified logic resource, freeing any
 * memory chunks allocated for this resource.
 * @param n  The number of the logic resource to unload
 */
void AgiEngine::unloadLogic(int16 logicNr) {
	if (_game.dirLogic[logicNr].flags & RES_LOADED) {
		free(_game.logics[logicNr].data);
		free(_game.logics[logicNr].texts);
		_game.logics[logicNr].numTexts = 0;
		_game.dirLogic[logicNr].flags &= ~RES_LOADED;
	}

	// if cached, we end up here
	_game.logics[logicNr].sIP = 2;
	_game.logics[logicNr].cIP = 2;
}

} // End of namespace Agi
