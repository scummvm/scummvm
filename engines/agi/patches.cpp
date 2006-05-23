/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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
#include "agi/opcodes.h"

namespace Agi {

#ifdef PATCH_LOGIC

#define ip (game.logics[n].cIP)
#define code (game.logics[n].data)
#define size (game.logics[n].size)

/*
 * Patches
 */

static uint8 kq4data_find[] = {
	0x0C, 0x04, 0xFF, 0x07, 0x05, 0xFF, 0x15, 0x00,
	0x03, 0x0A, 0x00, 0x77, 0x83, 0x71, 0x0D, 0x97,
	0x03, 0x98, 0xCE, 0x18, 0x98, 0x19, 0x98, 0x1B,
	0x98, 0x0C, 0x5A, 0x1A, 0x00
};

static uint8 kq4data_fix[] = {
	/* v19 = 0
	 * new.room(96)
	 * return
	 */
	0x03, 0x13, 0x0, 0x12, 0x60, 0x00
};

static uint8 grdata_find[] = {
	0x0C, 0x04, 0xFF, 0x07, 0x05, 0xFF, 0x16, 0x00,
	0x0C, 0x96, 0x03, 0x0A, 0x00, 0x77, 0x83, 0x71,
	0x0D, 0xD9, 0x03, 0xDC, 0xBF, 0x18, 0xDC, 0x19,
	0xDC, 0x1B, 0xDC, 0x0C, 0x95, 0x1A
};

static uint8 grdata_fix[] = {
	/* reset(227)
	 * v19 = 0
	 * v246 = 1
	 * set(15)
	 * new.room(73)
	 */
	0x0D, 0xE3, 0x03, 0x13, 0x00, 0x03, 0xF6, 0x01,
	0x0C, 0x0F, 0x12, 0x49
};

#if 0
static uint8 lsl1data_find[] = {
	0xFF, 0xFD, 0x07, 0x1E, 0xFC, 0x07, 0x6D, 0x01,
	0x5F, 0x03, 0xFC, 0xFF, 0x12, 0x00, 0x0C, 0x6D,
	0x78, 0x8A, 0x77, 0x69, 0x16, 0x18, 0x00, 0x0D,
	0x30, 0x0D, 0x55, 0x78, 0x65, 0x0A
};

static uint8 lsl1data_fix[] = {
	/* set(109)
	 * reset(48)
	 * reset(85)
	 * accept.input()
	 * new.room(11)
	 */
	0x0C, 0x6D, 0x0D, 0x30, 0x0D, 0x55, 0x78, 0x12,
	0x0B
};
#endif

static uint8 mh1data_find[] = {
	0xFF, 0x07, 0x05, 0xFF, 0xE6, 0x00,
	0x03, 0x0A, 0x02, 0x77, 0x83, 0x71,
	0x6F, 0x01, 0x17, 0x00, 0x03, 0x00,
	0x9F, 0x03, 0x37, 0x00, 0x03, 0x32,
	0x03, 0x03, 0x3B, 0x00, 0x6C, 0x03
};

static uint8 mh1data_fix[] = {
	0x0C, 0x05, 0x16, 0x5A, 0x12, 0x99
};

void patch_logic(int n) {
	switch (n) {
#if 0
		/* ALT-X in the questions takes care of that */
	case 6:
		/* lsl1 bypass questions */
		if (!strcmp(game.id, "LLLLL")) {
			if (!memcmp(lsl1data_find, (code + ip), 30))
				memmove((code + ip), lsl1data_fix, 9);
		}
		break;
#endif
	case 125:
		/* gold rush code break */
		if (!strcmp(game.id, "GR")) {
			if (!memcmp(grdata_find, (code + ip), 30))
				memmove((code + ip), grdata_fix, 12);
		}
		break;
	case 140:
		/* kings quest 4 code break */
		if (!strcmp(game.id, "KQ4")) {
			if (memcmp(kq4data_find, (code + ip), 29) == 0)
				memmove((code + ip), kq4data_fix, 6);
		}
		break;
	case 159:
		/* manhunter 1 amiga */
		if (ip + 30 < size && !memcmp(mh1data_find, (code + ip), 30)) {
			memmove((code + ip), mh1data_fix, 6);
		}
		break;
	}
}

#endif

}                             // End of namespace Agi
