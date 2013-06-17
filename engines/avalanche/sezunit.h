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

#ifndef __sezunit_h__
#define __sezunit_h__


/*#include "Gyro.h"*/


struct sezheader {
             array<1,2,char> initials;     /* should be "TT" */
             word gamecode;
             word revision; /* as 3- or 4-digit code (eg v1.00 = 100) */
             longint chains; /* number of scroll chains */
             longint size; /* total size of all chains */
};



#ifdef __sezunit_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN array<0,1999,char> chain; /* This chain */
EXTERN word chainsize; /* Length of "chain" */
EXTERN byte sezerror; /* Error code */
EXTERN sezheader sezhead;
#undef EXTERN
#define EXTERN extern


     /* Error codes for "sezerror" */
const integer sezok = 0;
const integer sezgunkyfile = 1;
const integer sezhacked = 2;


void sez_setup();

void getchain(longint number);


#endif
