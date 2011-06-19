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
 */

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge/btfile.h"
#include "common/system.h"
#include "common/str.h"
#include <string.h>

namespace CGE {

#ifndef BT_SIZE
#define BT_SIZE     K(1)
#endif

#ifndef BT_KEYLEN
#define BT_KEYLEN   13
#endif

BTFILE::BTFILE(const char *name, IOMODE mode, CRYPT *crpt)
	: IOHAND(name, mode, crpt) {
	for (int i = 0; i < BT_LEVELS; i ++) {
		Buff[i].Page = new BT_PAGE;
		Buff[i].PgNo = BT_NONE;
		Buff[i].Indx = -1;
		Buff[i].Updt = false;
		if (Buff[i].Page == NULL)
			error("No core");
	}
}


BTFILE::~BTFILE(void) {
	for (int i = 0; i < BT_LEVELS; i ++) {
		PutPage(i);
		delete Buff[i].Page;
	}
}


void BTFILE::PutPage(int lev, bool hard) {
	if (hard || Buff[lev].Updt) {
		Seek(Buff[lev].PgNo * sizeof(BT_PAGE));
		Write((uint8 *) Buff[lev].Page, sizeof(BT_PAGE));
		Buff[lev].Updt = false;
	}
}


BT_PAGE *BTFILE::GetPage(int lev, uint16 pgn) {
	if (Buff[lev].PgNo != pgn) {
		uint32 pos = pgn * sizeof(BT_PAGE);
		PutPage(lev);
		Buff[lev].PgNo = pgn;
		if (Size() > pos) {
			Seek((uint32) pgn * sizeof(BT_PAGE));
			Read((uint8 *) Buff[lev].Page, sizeof(BT_PAGE));
			Buff[lev].Updt = false;
		} else {
			Buff[lev].Page->Hea.Count = 0;
			Buff[lev].Page->Hea.Down = BT_NONE;
			memset(Buff[lev].Page->Data, '\0', sizeof(Buff[lev].Page->Data));
			Buff[lev].Updt = true;
		}
		Buff[lev].Indx = -1;
	}
	return Buff[lev].Page;
}

// Does this work, or does it have to compare the entire buffer?
#define memicmp(s1, s2, n) scumm_strnicmp((const char *)s1, (const char *)s2, n)
 
BT_KEYPACK *BTFILE::Find(const char *key) {
	int lev = 0;
	uint16 nxt = BT_ROOT;
	while (! Error) {
		BT_PAGE *pg = GetPage(lev, nxt);
		// search
		if (pg->Hea.Down != BT_NONE) {
			int i;
			for (i = 0; i < pg->Hea.Count; i ++)
				if (memicmp(key, pg->Inn[i].Key, BT_KEYLEN) < 0)
					break;
			nxt = (i) ? pg->Inn[i - 1].Down : pg->Hea.Down;
			Buff[lev].Indx = i - 1;
			++ lev;
		} else {
			int i;
			for (i = 0; i < pg->Hea.Count - 1; i ++)
				if (scumm_stricmp((const char *)key, (const char *)pg->Lea[i].Key) <= 0)
					break;
			Buff[lev].Indx = i;
			return &pg->Lea[i];
		}
	}
	return NULL;
}


int keycomp(const void *k1, const void *k2) {
	return memicmp(k1, k2, BT_KEYLEN);
}


void BTFILE::Make(BT_KEYPACK *keypack, uint16 count) {
#if BT_LEVELS != 2
#error This tiny BTREE implementation works with exactly 2 levels!
#endif
	_fqsort(keypack, count, sizeof(*keypack), keycomp);
	uint16 n = 0;
	BT_PAGE *Root = GetPage(0, n++),
	        *Leaf = GetPage(1, n);
	Root->Hea.Down = n;
	PutPage(0, true);
	while (count --) {
		if (Leaf->Hea.Count >= ArrayCount(Leaf->Lea)) {
			PutPage(1, true);     // save filled page
			Leaf = GetPage(1, ++n);   // take empty page
			memcpy(Root->Inn[Root->Hea.Count].Key, keypack->Key, BT_KEYLEN);
			Root->Inn[Root->Hea.Count ++].Down = n;
			Buff[0].Updt = true;
		}
		Leaf->Lea[Leaf->Hea.Count ++] = * (keypack ++);
		Buff[1].Updt = true;
	}
}

} // End of namespace CGE
