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

#ifndef CHAMBER_RESDATA_H
#define CHAMBER_RESDATA_H

namespace Chamber {

typedef struct ResEntry_t {
	char    name[5 + 1 + 3 + 1];
	void    *buffer;
} ResEntry_t;

typedef struct ResEntry_tp {
	char    name[5 + 1 + 3 + 1];
	byte    **buffer;
} ResEntry_tp;

/* Max resource file size among all languages */
#define RES_ALEAT_MAX 256
#define RES_ANICO_MAX 667
#define RES_ANIMA_MAX 2046
#define RES_ARPLA_MAX 7910
#define RES_CARPC_MAX 384
#define RES_GAUSS_MAX 1449
#define RES_ICONE_MAX 2756
#define RES_LUTIN_MAX 2800
#define RES_MURSM_MAX 76
#define RES_SOUCO_MAX 424
#define RES_SOURI_MAX 1152
#define RES_TEMPL_MAX 27337
#define RES_ZONES_MAX 9014
#define RES_PUZZL_MAX 45671
#define RES_SPRIT_MAX 23811
#define RES_PERS1_MAX 14294
#define RES_PERS2_MAX 10587
#define RES_DESCI_MAX 10515
#define RES_DIALI_MAX 9636
#define RES_MOTSI_MAX 1082
#define RES_VEPCI_MAX 1345

extern byte vepci_data[];
extern byte motsi_data[];

extern byte puzzl_data[];
extern byte sprit_data[];

extern byte pers1_data[];
extern byte pers2_data[];

extern byte desci_data[];
extern byte diali_data[];

extern byte *arpla_data;
extern byte *aleat_data;
extern byte *carpc_data;
extern byte *icone_data;
extern byte *souco_data;
extern byte *souri_data;
extern byte *templ_data;
extern byte *mursm_data;
extern byte *gauss_data;
extern byte *lutin_data;
extern byte *anima_data;
extern byte *anico_data;
extern byte *zones_data;

byte *SeekToEntry(byte *bank, uint16 num, byte **end);
byte *SeekToEntryW(byte *bank, uint16 num, byte **end);

uint16 LoadFile(const char *filename, byte *buffer);
uint16 SaveFile(char *filename, byte *buffer, uint16 size);
int16 LoadFilesList(ResEntry_t *entries);

int16 LoadStaticData(void);
int16 LoadFond(void);
int16 LoadSpritesData(void);
int16 LoadPersData(void);

extern ResEntry_t res_texts[];
int16 LoadVepciData(void);

extern ResEntry_t res_desci[];
int16 LoadDesciData(void);

extern ResEntry_t res_diali[];
int16 LoadDialiData(void);

} // End of namespace Chamber

#endif
