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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#ifndef AVALANCHE_ROOMNUMS_H
#define AVALANCHE_ROOMNUMS_H

#include "common/system.h"

namespace Avalanche {

const byte r__nowhere = 0;
const byte r__yours = 1;
const byte r__outsideyours = 2;
const byte r__outsidespludwicks = 3;
const byte r__yourhall = 5;
const byte r__musicroom = 7;
const byte r__outsideargentpub = 9;
const byte r__argentroad = 10;
const byte r__wisewomans = 11;
const byte r__spludwicks = 12;
const byte r__insideabbey = 13;
const byte r__outsideabbey = 14; // assumed
const byte r__avvysgarden = 15;
const byte r__aylesoffice = 16;
const byte r__argentpub = 19;
const byte r__brummieroad = 20;
const byte r__bridge = 21; // ? not sure
const byte r__lusties = 22;
const byte r__lustiesroom = 23;
const byte r__westhall = 25;
const byte r__easthall = 26;
const byte r__oubliette = 27;
const byte r__geidas = 28;
const byte r__catacombs = 29;

//{ -------------- }

const byte r__entrancehall = 40;
const byte r__robins = 42;
const byte r__outsidenottspub = 46;
const byte r__nottspub = 47;

//{ -------------- }

const byte r__outsideducks = 50;
const byte r__ducks = 51;

//{ -------------- }

const byte r__outsidecardiffcastle = 70;
const byte r__insidecardiffcastle = 71;

//{ -------------- }

// place80 appears to be bogus

//{ -------------- }

const byte r__bosskey = 98; // assumed
const byte r__map = 99;

} // End of namespace Avalanche

#endif // AVALANCHE_ROOMNUMS_H
