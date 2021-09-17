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

#include "common/file.h"

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/resdata.h"
#include "chamber/decompr.h"

namespace Chamber {

extern void AskDisk2(void);
extern int16 LoadSplash(const char *filename);

/*
Get bank entry
TODO: port SeekToString to this routine
*/
byte *SeekToEntry(byte *bank, uint16 num, byte **end) {
	byte len;
	byte *p = bank;

	while (num--) {
		len = *p;
		p += len;
	}
	len = *p;
	*end = p + len;
	return p + 1;
}

byte *SeekToEntryW(byte *bank, uint16 num, byte **end) {
	uint16 len;
	byte *p = bank;

	while (num--) {
		len = p[0] | (p[1] << 8);
		p += len;
	}
	len = p[0] | (p[1] << 8);
	*end = p + len;
	return p + 2;
}

uint16 LoadFile(const char *filename, byte *buffer) {
	Common::File in;

	in.open(filename);

	if (!in.isOpen())
		return 0;

	return in.read(buffer, 0xFFFF0);
}

uint16 SaveFile(char *filename, byte *buffer, uint16 size) {
	warning("STUB: SaveFile(%s, buffer, %d)", filename, size);
	return 0;
#if 0
	int16 f;
	int16 wlen;
	f = open(filename, O_RDONLY | O_BINARY);
	if (f == -1)
		return 0;
	wlen = write(f, buffer, size);
	close(f);
	if (wlen == -1)
		return 0;
	return (uint16)wlen;
#endif
}

int16 LoadFilesList(ResEntry_t *entries) {
	int16 i;
	for (i = 0; entries[i].name[0] != '$'; i++) {
		if (!LoadFile(entries[i].name, (byte *)entries[i].buffer))
			return 0;
	}
	return 1;
}


byte arpla_data[RES_ARPLA_MAX];
byte aleat_data[RES_ALEAT_MAX];
byte icone_data[RES_ICONE_MAX];
byte souco_data[RES_SOUCO_MAX];
byte souri_data[RES_SOURI_MAX];
byte mursm_data[RES_MURSM_MAX];
byte gauss_data[RES_GAUSS_MAX];
byte lutin_data[RES_LUTIN_MAX];
byte anima_data[RES_ANIMA_MAX];
byte anico_data[RES_ANICO_MAX];
byte zones_data[RES_ZONES_MAX];

ResEntry_t res_static[] = {
	{"ARPLA.BIN", arpla_data},
	{"ALEAT.BIN", aleat_data},
	{"ICONE.BIN", icone_data},
	{"SOUCO.BIN", souco_data},
	{"CARPC.BIN", carpc_data},
	{"SOURI.BIN", souri_data},
	{"TEMPL.BIN", templ_data},
	{"MURSM.BIN", mursm_data},
	{"GAUSS.BIN", gauss_data},
	{"LUTIN.BIN", lutin_data},
	{"ANIMA.BIN", anima_data},
	{"ANICO.BIN", anico_data},
	{"ZONES.BIN", zones_data},
	{"$", NULL}
};

/*
Load resident data files. Original game has all these data files embedded in the executable.
NB! Static data includes the font file, don't use any text print routines before it's loaded.
*/
int16 LoadStaticData() {
	return LoadFilesList(res_static);
}

ResEntry_t res_texts[] = {
	{"VEPCI.BIN", vepci_data},
	{"MOTSI.BIN", motsi_data},
	{"$", NULL}
};

/*
Load strings data (commands/names)
*/
int16 LoadVepciData() {
	return LoadFilesList(res_texts);
}

int16 LoadFond(void) {
	return LoadSplash("FOND.BIN");
}

ResEntry_t res_sprites[] = {
	{"PUZZL.BIN", puzzl_data},
	{"SPRIT.BIN", sprit_data},
	{"$", NULL}
};

int16 LoadSpritesData(void) {
	return LoadFilesList(res_sprites);
}

ResEntry_t res_person[] = {
	{"PERS1.BIN", pers1_data},
	{"PERS2.BIN", pers2_data},
	{"$", NULL}
};

int16 LoadPersData(void) {
	/*Originally it tries to load pers1 + pers2 as a single contiguos resource, if have enough memory*/
	/*If memory is low, neccessary file is loaded on demand, according to requested bank resource index*/
	/*Here we load both parts to their own memory buffers then select one in LoadPersSprit()*/
	return LoadFilesList(res_person);
}

ResEntry_t res_desci[] = {
	{"DESCI.BIN", desci_data},
	{"$", NULL}
};

/*
Load strings data (obj. descriptions)
*/
int16 LoadDesciData(void) {
	while (!LoadFilesList(res_desci))
		AskDisk2();
	return 1;
}

ResEntry_t res_diali[] = {
	{"DIALI.BIN", diali_data},
	{"$", NULL}
};

/*
Load strings data (dialogs)
*/
int16 LoadDialiData(void) {
	while (!LoadFilesList(res_diali))
		AskDisk2();
	return 1;
}

} // End of namespace Chamber
