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

#include "cge/general.h"
#include "cge/text.h"
#include "cge/talk.h"
#include "cge/vol.h"
#include "cge/bitmaps.h"
#include "cge/game.h"
#include "cge/snail.h"
#include "cge/cge_main.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

namespace CGE {

TEXT *Text;
TALK *Talk = NULL;

TEXT::TEXT(const char *fname, int size) {
	Cache = new HAN[size];
	MergeExt(FileName, fname, SAY_EXT);
	if (!INI_FILE::Exist(FileName))
		error("No talk\n");

	for (Size = 0; Size < size; Size ++) {
		Cache[Size].Ref = 0;
		Cache[Size].Txt = NULL;
	}
}


TEXT::~TEXT(void) {
	Clear();
	delete[] Cache;
}


void TEXT::Clear(int from, int upto) {
	HAN *p, * q;
	for (p = Cache, q = p + Size; p < q; p ++) {
		if (p->Ref && p->Ref >= from && p->Ref < upto) {
			p->Ref = 0;
			delete p->Txt;
			p->Txt = NULL;
		}
	}
}


int TEXT::Find(int ref) {
	HAN *p, * q;
	int i = 0;
	for (p = Cache, q = p + Size; p < q; p ++) {
		if (p->Ref == ref)
			break;
		else
			++i;
	}
	return i;
}


void TEXT::Preload(int from, int upto) {
	INI_FILE tf = FileName;
	if (! tf.Error) {
		HAN *CacheLim = Cache + Size;
		char line[LINE_MAX + 1];
		int n;

		while ((n = tf.Read((uint8 *)line)) != 0) {
			char *s;
			int ref;

			if (line[n - 1] == '\n')
				line[-- n] = '\0';
			if ((s = strtok(line, " =,;/\t\n")) == NULL)
				continue;
			if (! IsDigit(*s))
				continue;
			ref = atoi(s);
			if (ref && ref >= from && ref < upto) {
				HAN *p;

				p = &Cache[Find(ref)];
				if (p < CacheLim) {
					delete[] p->Txt;
					p->Txt = NULL;
				} else
					p = &Cache[Find(0)];
				if (p >= CacheLim)
					break;
				s += strlen(s);
				if (s < line + n)
					++s;
				if ((p->Txt = new char[strlen(s) + 1]) == NULL)
					break;
				p->Ref = ref;
				strcpy(p->Txt, s);
			}
		}
	}
}


char *TEXT::Load(int idx, int ref) {
	INI_FILE tf = FileName;
	if (! tf.Error) {
		HAN *p = &Cache[idx];
		char line[LINE_MAX + 1];
		int n;

		while ((n = tf.Read((uint8 *)line)) != 0) {
			char *s;

			if (line[n - 1] == '\n')
				line[-- n] = '\0';
			if ((s = strtok(line, " =,;/\t\n")) == NULL)
				continue;
			if (! IsDigit(*s))
				continue;

			int r = atoi(s);
			if (r < ref)
				continue;
			if (r > ref)
				break;
			// (r == ref)
			s += strlen(s);
			if (s < line + n)
				++s;
			p->Ref = ref;
			if ((p->Txt = new char[strlen(s) + 1]) == NULL)
				return NULL;
			return strcpy(p->Txt, s);
		}
	}
	return NULL;
}


char *TEXT::getText(int ref) {
	int i;
	if ((i = Find(ref)) < Size)
		return Cache[i].Txt;

	if ((i = Find(0)) >= Size) {
		Clear(SYSTXT_MAX);            // clear non-system
		if ((i = Find(0)) >= Size) {
			Clear();              // clear all
			i = 0;
		}
	}
	return Load(i, ref);
}


void Say(const char *txt, SPRITE *spr) {
	KillText();
	Talk = new TALK(txt, ROUND);
	if (Talk) {
		bool east = spr->Flags.East;
		int x = (east) ? (spr->X + spr->W - 2) : (spr->X + 2);
		int y = spr->Y + 2;
		SPRITE *spike = new SPRITE(SP);
		uint16 sw = spike->W;

		if (east) {
			if (x + sw + TEXT_RD + 5 >= SCR_WID)
				east = false;
		} else {
			if (x <= 5 + TEXT_RD + sw)
				east = true;
		}
		x = (east) ? (spr->X + spr->W - 2) : (spr->X + 2 - sw);
		if (spr->Ref == 1)
			x += ((east) ? -10 : 10); // Hero

		Talk->Flags.Kill = true;
		Talk->Flags.BDel = true;
		Talk->SetName(Text->getText(SAY_NAME));
		Talk->Goto(x - (Talk->W - sw) / 2 - 3 + 6 * east, y - spike->H - Talk->H + 1);
		Talk->Z = 125;
		Talk->Ref = SAY_REF;

		spike->Goto(x, Talk->Y + Talk->H - 1);
		spike->Z = 126;
		spike->Flags.Slav = true;
		spike->Flags.Kill = true;
		spike->SetName(Text->getText(SAY_NAME));
		spike->Step(east);
		spike->Ref = SAY_REF;

		Vga->ShowQ->Insert(Talk, Vga->ShowQ->Last());
		Vga->ShowQ->Insert(spike, Vga->ShowQ->Last());
	}
}

void Inf(const char *txt) {
	KillText();
	Talk = new TALK(txt, RECT);
	if (Talk) {
		Talk->Flags.Kill = true;
		Talk->Flags.BDel = true;
		Talk->SetName(Text->getText(INF_NAME));
		Talk->Center();
		Talk->Goto(Talk->X, Talk->Y - 20);
		Talk->Z = 126;
		Talk->Ref = INF_REF;
		Vga->ShowQ->Insert(Talk, Vga->ShowQ->Last());
	}
}


void SayTime(SPRITE *spr) {
	/*
	  static char t[] = "00:00";
	  struct time ti;
	  gettime(&ti);
	  wtom(ti.ti_hour, t+0, 10, 2);
	  wtom(ti.ti_min,  t+3, 10, 2);
	  Say((*t == '0') ? (t+1) : t, spr);
	  */
	warning("STUB: SayTime");
}

void KillText(void) {
	if (Talk) {
		SNPOST_(SNKILL, -1, 0, Talk);
		Talk = NULL;
	}
}

} // End of namespace CGE
