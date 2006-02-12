/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 Andreas 'Sprawl' Karlsson - Original EPOC port, ESDL
 * Copyright (C) 2003-2005 Lars 'AnotherGuest' Persson - Original EPOC port, Audio System
 * Copyright (C) 2005 Jurgen 'SumthinWicked' Braam - EPOC/CVS maintainer
 * Copyright (C) 2005-2006 The ScummVM project
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

#ifdef USE_VIBRA_SE_PXXX
	"Vibra "
#endif
	"\n"
	
// we want a list of supported engines visible in the program,
// because we also release special builds with only one engine
#ifndef DISABLE_SCUMM
	"SCUMM "
#endif
#ifndef DISABLE_SIMON
	"Simon "
#endif
#ifndef DISABLE_SKY
	"Sky "
#endif
#ifndef DISABLE_QUEEN
	"Queen "
#endif
#ifndef DISABLE_GOB
	"Gob "
#endif
#ifndef DISABLE_SAGA
	"Saga "
#endif
#ifndef DISABLE_KYRA
	"Kyra "
#endif
#ifndef DISABLE_SWORD1
	"Sword1 "
#endif
#ifndef DISABLE_SWORD2
	"Sword2 "
#endif
