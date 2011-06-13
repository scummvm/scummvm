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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef __MIXER__
#define __MIXER__

#include "cge/vga13h.h"

namespace CGE {

#define MIX_MAX     16                              // count of Leds
#define MIX_Z       64                              // mixer Z position
#define MIX_DELAY   12                              // 6/s
#define MIX_FALL    6                               // in MIX_DELAY units
#define MIX_BHIG    6                               // mixer button high
#define MIX_NAME    105                             // sprite name

class MIXER : public SPRITE {
	BMP_PTR mb[2];
	BMP_PTR lb[MIX_MAX + 1];
	SEQ ls[MIX_MAX];
	SPRITE *Led[2];
	int Fall;
	void Update(void);
public:
	static bool Appear;
	MIXER(int x, int y);
	~MIXER(void);
	void Touch(uint16 mask, int x, int y);
	void Tick(void);
};

} // End of namespace CGE

#endif
