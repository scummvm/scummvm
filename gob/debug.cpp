/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/debug.h"
#include "gob/scenery.h"

namespace Gob {

static int16 logFile = -2;
static char buf[256];

extern uint32 always0_dword_23EC_560;

static void log_close(void) {
	if (logFile != -2)
		close(logFile);
	logFile = -2;
}

static void log_init(void) {
	if (logFile == -2) {
		logFile = open(LOG_NAME, O_WRONLY | O_CREAT);
		if (logFile != -1)
			atexit(&log_close);
	}
}

void log_write(const char *format, ...) {
	va_list lst;
	va_start(lst, format);

	log_init();
	if (logFile >= 0) {
		vsprintf(buf, format, lst);
		write(logFile, buf, strlen(buf));
	}

	va_end(lst);
}

void dbg_printInt(int16 val) {
	log_write("dbg_printInt: %d\n", val);
}

void dbg_printPtr(void *ptr) {
	log_write("dbg_printPtr: %p\n", ptr);
}

void dbg_printStr(char *str) {
	log_write("dbg_printStr: ");
	log_write(str);
	log_write("\n");
}

void dbg_dumpMem(char *ptr, int16 size) {
	int16 i;
	log_write("dbg_dumpMem %p %d:", ptr, size);
	for (i = 0; i < size; i++)
		log_write("%02x ", (uint16)(byte)ptr[i]);
	log_write("\n");
}

void dbg_dumpMemChars(char *ptr, int16 size) {
	int16 i;
	log_write("dbg_dumpMem %p %ld:", ptr, size);
	for (i = 0; i < size; i++)
		log_write("%c ", ptr[i]);
	log_write("\n");
}

void dbg_printDelim() {
	log_write("-------------------\n");
}

void dbg_printHexInt(int16 val) {
	log_write("%02x\n", val);
}

void dbg_dumpStaticScenery(Scen_Static * st) {
	int16 i, j;
	Scen_StaticPlane *ptr;

	log_write("dbg_dumpStaticScenery\n");
	log_write("----------\n");
	log_write("Layers count = %d\n", st->layersCount);

	for (i = 0; i < st->layersCount; i++) {
		log_write("Layer %d:\n", i);
		log_write("Back sprite resource id = %d\n",
		    st->layers[i]->backResId);
		log_write("Plane count = %d\n", st->layers[i]->planeCount);

		for (j = 0; j < st->layers[i]->planeCount; j++) {
			ptr = &st->layers[i]->planes[j];
			log_write
			    ("Plane %d: pictIndex = %d, pieceIndex = %d, drawOrder = %d\n",
			    j, (int16)ptr->pictIndex, (int16)ptr->pieceIndex,
			    (int16)ptr->drawOrder);

			log_write
			    ("destX = %d, destY = %d, transparency = %d\n",
			    ptr->destX, ptr->destY, (char *)ptr->transp);
		}
	}
	log_write("----------\n\n");
}

int16 calcDest(char dest, byte add) {
	if (dest >= 0)
		return dest + ((uint16)add << 7);
	else
		return dest - ((uint16)add << 7);
}

/*
void dbg_dumpFramePiece(Scen_AnimFramePiece* piece, int16 j,  Scen_AnimLayer* layer) {
	log_write("Piece for %d anim, %p: ", j, piece);
	log_write("pictIndex = %x, pieceIndex = %d, destX = %d, destY = %d, not final = %d\n",
		(uint16)piece->pictIndex,
		(uint16)piece->pieceIndex,
		layer->deltaX+calcDest(piece->destX, (char)((piece->pictIndex & 0xc0)>>6)),
		layer->deltaY+calcDest(piece->destY, (char)((piece->pictIndex & 0x30)>>4)),
		(int16)piece->notFinal);
}

void dbg_dumpAnimation(Scen_Animation* anim) {
	int16 i, j;
	Scen_AnimLayer* layer;
	Scen_AnimFramePiece* piece;

	log_write("dbg_dumpAnimation\n");
	log_write("----------\n");
	log_write("Layers count = %d\n", anim->layersCount);

	for(i = 0; i < anim->layersCount; i++)
	{
		layer = anim->layers[i];

		log_write("Layer %d:\n", i);

		log_write("unknown0 = %d\n", layer->unknown0);
		log_write("deltaX = %d\n", layer->deltaX);
		log_write("deltaY = %d\n", layer->deltaY);
		log_write("unknown1 = %d\n", layer->unknown1);
		log_write("unknown2 = %d\n", layer->unknown2);
		log_write("transparency = %d\n", (int16)layer->transp);
		log_write("animsCount %d\n", layer->framesCount);

		piece = layer->frames;
		j = 0;
		while(j < layer->framesCount)
		{
			dbg_dumpFramePiece(piece, j, layer);
			if(piece->notFinal != 1)
				j++;
			piece++;
		}
	}

	log_write("----------\n\n");
}

*/

} // End of namespace Gob
