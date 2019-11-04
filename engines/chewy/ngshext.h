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

#ifndef CHEWY_NGSHEXT_H
#define CHEWY_NGSHEXT_H

#include "engines/chewy/memory.h"
#include "engines/chewy/mcga.h"
#include "engines/chewy/fehler.h"
#include "engines/chewy/maus.h"

#include "engines/chewy/io_game.h"
#include "engines/chewy/cursor.h"
#include "engines/chewy/flic.h"
#ifdef AIL
#include "engines/chewy/ailclass.h"
#endif

#define MAXPFAD 81

class sblaster;

extern mcga_grafik *out;
extern memory *mem;
extern fehler *err;
extern maus *in;
extern sblaster *snd;
extern io_game *iog;
extern cursor *cur;
#ifdef AIL
extern ailclass *ailsnd;
#endif

extern uint16 _stklen;
extern int16 modul;
extern int16 fcode;

#endif
