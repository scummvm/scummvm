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
#include "agi/opcodes.h"

namespace Agi {

int AgiEngine::v2id_game() {
	int ver;

	ver = this->_gameDescription->version;
	game.ver = ver;
	debugC(2, kDebugLevelMain, "game.ver = 0x%x", game.ver);
	agiSetRelease(ver);

	return setup_v2_game(ver, 0);
}

/*
 * Currently, there is no known difference between v3.002.098 -> v3.002.149
 * So version emulated;
 *
 * 0x0086,
 * 0x0149
 */

int AgiEngine::v3id_game() {
	int ver;

	ver = this->_gameDescription->version;
	game.ver = ver;
	debugC(2, kDebugLevelMain, "game.ver = 0x%x", game.ver);
	agiSetRelease(ver);

	return setup_v3_game(ver, 0);
}

/**
 *
 */
int AgiEngine::setup_v2_game(int ver, uint32 crc) {
	int ec = err_OK;

	if (ver == 0) {
		report("Unknown v2 Sierra game: %08x\n\n", crc);
		agiSetRelease(0x2917);
	}

	/* setup the differences in the opcodes and other bits in the
	 * AGI v2 specs
	 */
	if (opt.emuversion)
		agiSetRelease(opt.emuversion);

	if (opt.agds)
		agiSetRelease(0x2440);	/* ALL AGDS games built for 2.440 */

	report("Seting up for version 0x%04X\n", ver);

	switch (agiGetRelease()) {
	case 0x2089:
		logic_names_cmd[0x86].num_args = 0;	/* quit: 0 args */
		logic_names_cmd[0x97].num_args = 3;	/* print.at: 3 args */
		logic_names_cmd[0x98].num_args = 3;	/* print.at.v: 3 args */
		break;
	case 0x2272:
		/* KQ3 0x88673 (2.272) requires print.at with 4 arguments */
		break;
	case 0x2440:
		break;
	case 0x2917:
		break;
	case 0x2936:
		break;
	default:
		report("** Cannot setup for unknown version\n");
		ec = err_UnknownAGIVersion;
		break;
	}

	return ec;
}

/**
 *
 */
int AgiEngine::setup_v3_game(int ver, uint32 crc) {
	int ec = err_OK;

	if (ver == 0) {
		report("Unknown v3 Sierra game: %08x\n\n", crc);
		agiSetRelease(ver = 0x3149);
	}

	if (opt.emuversion)
		agiSetRelease(ver = opt.emuversion);

	report("Seting up for version 0x%04X\n", ver);

	switch (agiGetRelease()) {
	case 0x3086:
		logic_names_cmd[0xad].num_args = 1;	/* 173 : 1 args */
		break;
	case 0x3149:
		logic_names_cmd[0xad].num_args = 0;	/* 173 : 0 args */
		break;
	default:
		report("Error: cannot setup for unknown version\n");
		ec = err_UnknownAGIVersion;
		break;
	}

	return ec;
}

}                             // End of namespace Agi
