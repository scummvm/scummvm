/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2003 Sarien Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"

#include "agi/agi.h"

namespace Agi {

int getflag(int n) {
	uint8 *set = (uint8 *) &game.flags;

	set += n >> 3;
	return (*set & (1 << (n & 0x07))) != 0;
}

void setflag(int n, int v) {
	uint8 *set = (uint8 *) &game.flags;

	set += n >> 3;
	if (v)
		*set |= 1 << (n & 0x07);	/* set bit  */
	else
		*set &= ~(1 << (n & 0x07));	/* clear bit */
}

void flipflag(int n) {
	uint8 *set = (uint8 *) & game.flags;

	set += n >> 3;
	*set ^= 1 << (n & 0x07);	/* flip bit */
}

void setvar(int var, int val) {
	game.vars[var] = val;
}

int getvar(int var) {
	return game.vars[var];
}

void decrypt(uint8 *mem, int len) {
	const uint8 *key;
	int i;

	key = opt.agds ? (const uint8 *)CRYPT_KEY_AGDS : (const uint8 *)CRYPT_KEY_SIERRA;

	for (i = 0; i < len; i++)
		*(mem + i) ^= *(key + (i % 11));
}

}                             // End of namespace Agi
