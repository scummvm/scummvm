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

#ifndef __SCUMM_GLOBALS_H__
#define __SCUMM_GLOBALS_H__

enum {
	GBVARS_DIGSTATEMUSICMAP_INDEX = 0,
	GBVARS_DIGSTATEMUSICTABLE_INDEX,
	GBVARS_COMISTATEMUSICTABLE_INDEX,
	GBVARS_COMISEQMUSICTABLE_INDEX,
	GBVARS_DIGSEQMUSICTABLE_INDEX,
	GBVARS_FTSTATEMUSICTABLE_INDEX,
	GBVARS_FTSEQMUSICTABLE_INDEX,
	GBVARS_GUIFONT_INDEX,
	GBVARS_DEFAULTSCALETABLE_INDEX,
	GBVARS_IMCTABLE_INDEX,
	GBVARS_CODEC47TABLE_INDEX,
	GBVARS_TRANSITIONEFFECTS_INDEX,
	GBVARS_STRINGMAPTABLEV7_INDEX,
	GBVARS_STRINGMAPTABLEV6_INDEX,
	GBVARS_STRINGMAPTABLEV5_INDEX,
	GBVARS_GERMANCHARSETDATAV2_INDEX,
	GBVARS_FRENCHCHARSETDATAV2_INDEX,
	GBVARS_ENGLISHCHARSETDATAV2_INDEX,
	GBVARS_ITALIANCHARSETDATAV2_INDEX,
	GBVARS_SPANISHCHARSETDATAV2_INDEX,
	GBVARS_COSTSCALETABLE_INDEX,
	GBVARS_NOTELENGTHS_INDEX,
	GBVARS_HULLOFFSETS_INDEX,
	GBVARS_HULLS_INDEX,
	GBVARS_FREQMODLENGTHS_INDEX,
	GBVARS_FREQMODOFFSETS_INDEX,
	GBVARS_FREQMODTABLE_INDEX,
	GBVARS_SPKFREQTABLE_INDEX,
	GBVARS_PCJRFREQTABLE_INDEX
};

enum {
	GBVARS_RESOURCETABLEPE100V1_INDEX = 0,
	GBVARS_RESOURCETABLEPE100V2_INDEX,
	GBVARS_RESOURCETABLEPM10_INDEX,
	GBVARS_RESOURCETABLECM10_INDEX
};

void *GlbGetRecord(UInt16 index, UInt16 id);
void GlbReleaseRecord(UInt16 index, UInt16 id);
void GlbOpen();
void GlbClose();

enum {
	GBVARS_SCUMM = 0,
	GBVARS_SIMON,
	GBVARS_SKY,
	GBVARS_SWORD2,
	GBVARS_QUEEN,
	
	GBVARS_COUNT
};

#define _GINIT(x) void initGlobals_##x() {
#define _GEND }
#define _GRELEASE(x) void releaseGlobals_##x() {

#define CALL_INIT(x) initGlobals_##x();
#define CALL_RELEASE(x)	releaseGlobals_##x();

#define _GSETPTR(var,index,format,id)	var = (format *)GlbGetRecord(index,id);
#define _GRELEASEPTR(index,id)			GlbReleaseRecord(index,id);

#define PROTO_GLOBALS(x)		void CALL_INIT(x);\
								void CALL_RELEASE(x);

#ifndef DISABLE_SCUMM
PROTO_GLOBALS(IMuseDigital)
PROTO_GLOBALS(NewGui)
PROTO_GLOBALS(Akos)
PROTO_GLOBALS(Bundle)
PROTO_GLOBALS(Codec47)
PROTO_GLOBALS(Gfx)
PROTO_GLOBALS(Dialogs)
PROTO_GLOBALS(Charset)
PROTO_GLOBALS(Costume)
PROTO_GLOBALS(PlayerV2)
#endif

#ifndef DISABLE_QUEEN
PROTO_GLOBALS(Restables)
#endif

#undef PROTO_GLOBALS

#endif
