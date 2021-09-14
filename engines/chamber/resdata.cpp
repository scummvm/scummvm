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

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/resdata.h"
#include "chamber/decompr.h"

namespace Chamber {

/*
Get bank entry
TODO: port SeekToString to this routine
*/
unsigned char *SeekToEntry(unsigned char *bank, unsigned int num, unsigned char **end) {
	unsigned char len;
	unsigned char *p = bank;

	while (num--) {
		len = *p;
		p += len;
	}
	len = *p;
	*end = p + len;
	return p + 1;
}

unsigned char *SeekToEntryW(unsigned char *bank, unsigned int num, unsigned char **end) {
	unsigned int len;
	unsigned char *p = bank;

	while (num--) {
		len = p[0] | (p[1] << 8);
		p += len;
	}
	len = p[0] | (p[1] << 8);
	*end = p + len;
	return p + 2;
}

unsigned int LoadFile(char *filename, unsigned char *buffer) {
	int f;
	int rlen;
	f = open(filename, O_RDONLY | O_BINARY);
	if (f == -1)
		return 0;
	rlen = read(f, buffer, 0xFFF0);
	close(f);
	if (rlen == -1)
		return 0;
	return (unsigned int)rlen;
}

unsigned int SaveFile(char *filename, unsigned char *buffer, unsigned int size) {
	int f;
	int wlen;
	f = open(filename, O_RDONLY | O_BINARY);
	if (f == -1)
		return 0;
	wlen = write(f, buffer, size);
	close(f);
	if (wlen == -1)
		return 0;
	return (unsigned int)wlen;
}

int LoadFilesList(ResEntry_t *entries) {
	int i;
	for (i = 0; entries[i].name[0] != '$'; i++) {
		if (!LoadFile(entries[i].name, entries[i].buffer))
			return 0;
	}
	return 1;
}


unsigned char arpla_data[RES_ARPLA_MAX];
unsigned char aleat_data[RES_ALEAT_MAX];
unsigned char icone_data[RES_ICONE_MAX];
unsigned char souco_data[RES_SOUCO_MAX];
unsigned char souri_data[RES_SOURI_MAX];
unsigned char mursm_data[RES_MURSM_MAX];
unsigned char gauss_data[RES_GAUSS_MAX];
unsigned char lutin_data[RES_LUTIN_MAX];
unsigned char anima_data[RES_ANIMA_MAX];
unsigned char anico_data[RES_ANICO_MAX];
unsigned char zones_data[RES_ZONES_MAX];

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
	{"$"}
};

/*
Load resident data files. Original game has all these data files embedded in the executable.
NB! Static data includes the font file, don't use any text print routines before it's loaded.
*/
int LoadStaticData() {
	return LoadFilesList(res_static);
}

ResEntry_t res_texts[] = {
	{"VEPCI.BIN", vepci_data},
	{"MOTSI.BIN", motsi_data},
	{"$"}
};

/*
Load strings data (commands/names)
*/
int LoadVepciData() {
	return LoadFilesList(res_texts);
}

int LoadFond(void) {
	return LoadSplash("FOND.BIN");
}

ResEntry_t res_sprites[] = {
	{"PUZZL.BIN", puzzl_data},
	{"SPRIT.BIN", sprit_data},
	{"$"}
};

int LoadSpritesData(void) {
	return LoadFilesList(res_sprites);
}

ResEntry_t res_person[] = {
	{"PERS1.BIN", pers1_data},
	{"PERS2.BIN", pers2_data},
	{"$"}
};

int LoadPersData(void) {
	/*Originally it tries to load pers1 + pers2 as a single contiguos resource, if have enough memory*/
	/*If memory is low, neccessary file is loaded on demand, according to requested bank resource index*/
	/*Here we load both parts to their own memory buffers then select one in LoadPersSprit()*/
	return LoadFilesList(res_person);
}

ResEntry_t res_desci[] = {
	{"DESCI.BIN", desci_data},
	{"$"}
};

/*
Load strings data (obj. descriptions)
*/
int LoadDesciData(void) {
	while (!LoadFilesList(res_desci))
		AskDisk2();
}

ResEntry_t res_diali[] = {
	{"DIALI.BIN", diali_data},
	{"$"}
};

/*
Load strings data (dialogs)
*/
int LoadDialiData(void) {
	while (!LoadFilesList(res_diali))
		AskDisk2();
}

} // End of namespace Chamber
