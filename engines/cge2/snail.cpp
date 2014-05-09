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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge2/snail.h"
#include "cge2/fileio.h"

namespace CGE2 {

const char *Snail::comTxt[] = {
	"NOP", "USE", "PAUSE", "INF", "CAVE",
	"SLAVE", "FOCUS", "SETX", "SETY", "SETZ",
	"ADD", "SUB", "MUL", "DIV", "IF", "FLAG",
	"FLASH", "LIGHT", "CYCLE",
	"CLEAR", "TALK", "MOUSE",
	"MAP", "COUNT", "MIDI",
	"SETDLG", "MSKDLG",
	".DUMMY.",
	"WAIT", "HIDE", "ROOM",
	"SAY", "SOUND", "TIME", "KILL",
	"RSEQ", "SEQ", "SEND", "SWAP",
	"KEEP", "GIVE",
	"GETPOS", "GOTO", "MOVEX", "MOVEY",
	"MOVEZ", "TRANS", "PORT",
	"NEXT", "NNEXT", "MTNEXT", "FTNEXT",
	"RNNEXT", "RMTNEXT", "RFTNEXT",
	"RMNEAR", "RMMTAKE", "RMFTAKE",
	"SETREF", "BACKPT",
	"WALKTO", "REACH", "COVER", "UNCOVER",
	NULL };

int Snail::com(const char *com) {
	int i = EncryptedStream::takeEnum(comTxt, com);
	return (i < 0) ? i : i + kSNCom0 + 1;
}

} // End of namespace CGE2.
