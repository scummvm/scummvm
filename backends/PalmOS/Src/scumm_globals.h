/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
 
#define GBVARS_DIGSTATEMUSICMAP_INDEX		0
#define GBVARS_DIGSTATEMUSICTABLE_INDEX		1
#define GBVARS_COMISTATEMUSICTABLE_INDEX	2
#define GBVARS_COMISEQMUSICTABLE_INDEX		3
#define GBVARS_DIGSEQMUSICTABLE_INDEX		4
#define GBVARS_FTSTATEMUSICTABLE_INDEX		5
#define GBVARS_FTSEQMUSICTABLE_INDEX		6
#define GBVARS_GUIFONT_INDEX				7
#define GBVARS_DEFAULTSCALETABLE_INDEX 		8
#define GBVARS_CODEC37TABLE_INDEX			9
#define GBVARS_CODEC47TABLE_INDEX			10
#define GBVARS_TRANSITIONEFFECTS_INDEX		11
#define GBVARS_STRINGMAPTABLEV7_INDEX		12
#define GBVARS_STRINGMAPTABLEV6_INDEX		13
#define GBVARS_STRINGMAPTABLEV5_INDEX		14
#define GBVARS_GERMANCHARSETDATAV2_INDEX	15
#define GBVARS_FRENCHCHARSETDATAV2_INDEX	16
#define GBVARS_ENGLISHCHARSETDATAV2_INDEX	17
#define GBVARS_ITALIANCHARSETDATAV2_INDEX	18
#define GBVARS_SPANISHCHARSETDATAV2_INDEX	19

#define GBVARS_SCUMM	0
#define GBVARS_SIMON	1
#define GBVARS_SKY		2

#define GSETPTR(var,index,format,id)	var = (format *)GBGetRecord(index,id);
#define GRELEASEPTR(index,id)			GBReleaseRecord(index,id);

void *GBGetRecord(UInt16 index, UInt16 id);
void GBReleaseRecord(UInt16 index, UInt16 id);

void IMuseDigital_initGlobals();
void IMuseDigital_releaseGlobals();

void NewGui_initGlobals();
void NewGui_releaseGlobals();

void Codec47_initGlobals();
void Codec47_releaseGlobals();

void Gfx_initGlobals();
void Gfx_releaseGlobals();

void Akos_initGlobals();
void Akos_releaseGlobals();

void Dialogs_initGlobals();
void Dialogs_releaseGlobals();

void Charset_initGlobals();
void Charset_releaseGlobals();
