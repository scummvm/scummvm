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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

namespace Hugo {

#define HERO 0 // In all enums, HERO is the first element

#define DESCRIPLEN     32                           /* Length of description string */
#define MAX_SOUNDS     64                           /* Max number of sounds */
#define BOOTFILE       "HUGO.BSF"                   /* Name of boot structure file */
#define CONFIGFILE     "CONFIG.DAT"                 /* Name of config file */
#define LEN_MASK       0x3F                         /* Lower 6 bits are length */
#define PBFILE         "playback.dat"

/* Name scenery and objects picture databases */
#define SCENERY_FILE   "scenery.dat"
#define OBJECTS_FILE   "objects.dat"
#define STRING_FILE    "strings.dat"
#define SOUND_FILE     "sounds.dat"

/* User interface database (Windows Only) */
#define UIF_FILE   "uif.dat"

static const int kSavegameVersion = 1;
} // Namespace Hugo

