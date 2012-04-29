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
/**************************************************************************
 *                                     様様様様様様様様様様様様様様様様様 *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    様様様様様様様様様様様様様様様様様 *
 *      .d"        d$$$$$b        "b.                                     *
 *   .z$*         d$$$$$$$L        ^*$c.                                  *
 *  #$$$.         $$$$$$$$$         .$$$" Project: Roasted Moths........  *
 *    ^*$b       4$$$$$$$$$F      .d$*"                                   *
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Loc.CPP..............  *
 *        *$.    '$$$$$$$$$     4$P 4                                     *
 *     J   *$     "$$$$$$$"     $P   r    Author:  Giovanni Bajo........  *
 *    z$   '$$$P*4c.*$$$*.z@*R$$$    $.                                   *
 *   z$"    ""       #$F^      ""    '$c  Desc:    Classi di gestione     *
 *  z$$beu     .ue="  $  "=e..    .zed$$c          dei dati di una loca-  *
 *      "#$e z$*"   .  `.   ^*Nc e$""              zione................  *
 *         "$$".  .r"   ^4.  .^$$"                 .....................  *
 *          ^.@*"6L=\ebu^+C$"*b."                                         *
 *        "**$.  "c 4$$$  J"  J$P*"    OS:  [ ] DOS  [X] WIN95  [ ] PORT  *
 *            ^"--.^ 9$"  .--""      COMP:  [ ] WATCOM  [X] VISUAL C++    *
 *                    "                     [ ] EIFFEL  [ ] GCC/GXX/DJGPP *
 *                                                                        *
 * This source code is Copyright (C) Nayma Software.  ALL RIGHTS RESERVED *
 *                                                                        *
 **************************************************************************/

#include "tony/loc.h"
#include "tony/utils.h"
#include "tony/mpal/lzo.h"
#include "tony/mpal/mpalutils.h"

namespace Tony {

using namespace ::Tony::MPAL;

/****************************************************************************\
*       RMBox Methods
\****************************************************************************/

void RMBox::ReadFromStream(RMDataStream &ds) {
	uint16 w;
	int i;
	byte b;

	// Bbox
	ds >> left;
	ds >> top;
	ds >> right;
	ds >> bottom;

	// Adiacenza
	for (i = 0; i < MAXBOXES; i++)
	{
		ds >> adj[i];
	}

	// Misc
	ds >> numhotspot;
	ds >> Zvalue;
	ds >> b;
	attivo = b;
	ds >> b;
	bReversed = b;

	// Spazio di espansione
	ds+=30;

	// Hotspots
	for (i = 0; i < numhotspot; i++) {
		ds >> w; hotspot[i].hotx = w;
		ds >> w; hotspot[i].hoty = w;
		ds >> w; hotspot[i].destination = w;
	}
}

RMDataStream& operator>>(RMDataStream &ds, RMBox &box) {
	box.ReadFromStream(ds);

	return ds;
}

/****************************************************************************\
*       RMBoxLoc Methods
\****************************************************************************/

void RMBoxLoc::ReadFromStream(RMDataStream& ds) {
	int i;
	char buf[2];
	byte ver;

	// ID and versione
	ds >> buf[0] >> buf[1] >> ver;
	assert(buf[0] == 'B' && buf[1] == 'X');
	assert(ver == 3);

	// Numero dei box
	ds >> numbbox;

	// Alloca la memoria per i box
	boxes = new RMBox[numbbox];

	// Li legge da disco
	for (i = 0; i < numbbox; i++)
		ds >> boxes[i];
}


void RMBoxLoc::RecalcAllAdj(void) {
	int i, j;

	for (i = 0; i < numbbox; i++) {
		Common::fill(boxes[i].adj, boxes[i].adj + MAXBOXES, 0);

		for (j=0; j < boxes[i].numhotspot; j++)
			if (boxes[boxes[i].hotspot[j].destination].attivo)
				boxes[i].adj[boxes[i].hotspot[j].destination] = 1;
	}
}

RMDataStream &operator>>(RMDataStream &ds, RMBoxLoc &bl) {
	bl.ReadFromStream(ds);

	return ds;
}

/****************************************************************************\
*       RMGameBoxes methods
\****************************************************************************/

void RMGameBoxes::Init(void) {
	int i;
	RMString fn;
	RMDataStream ds;

	// Load boxes from disk
	m_nLocBoxes = 130;
	for (i=1; i <= m_nLocBoxes; i++) {
		RMRes res(10000 + i);

		ds.OpenBuffer(res);
		
		m_allBoxes[i] = new RMBoxLoc();
		ds >> *m_allBoxes[i];

		m_allBoxes[i]->RecalcAllAdj();

		ds.Close();
	}
}

void RMGameBoxes::Close(void) {
}

RMBoxLoc *RMGameBoxes::GetBoxes(int nLoc) {
	return m_allBoxes[nLoc];
}

bool RMGameBoxes::IsInBox(int nLoc, int nBox, RMPoint pt) {
	RMBoxLoc *cur = GetBoxes(nLoc);

	if ((pt.x >= cur->boxes[nBox].left) && (pt.x <= cur->boxes[nBox].right) &&
			(pt.y >= cur->boxes[nBox].top)  && (pt.y <= cur->boxes[nBox].bottom)) 
		return true;
	else
		return false;
}

int RMGameBoxes::WhichBox(int nLoc, RMPoint punto) {
	int i;
	RMBoxLoc *cur = GetBoxes(nLoc);
	
	if (!cur) return -1;

	for (i=0; i<cur->numbbox; i++)
		if (cur->boxes[i].attivo)
		  if ((punto.x >= cur->boxes[i].left) && (punto.x <= cur->boxes[i].right) &&
					(punto.y >= cur->boxes[i].top)  && (punto.y <= cur->boxes[i].bottom)) 
				return i;

	return -1;
}

void RMGameBoxes::ChangeBoxStatus(int nLoc, int nBox, int status) {
	m_allBoxes[nLoc]->boxes[nBox].attivo=status;
	m_allBoxes[nLoc]->RecalcAllAdj();
}


int RMGameBoxes::GetSaveStateSize(void) {
	int size;
	int i;

	size=4;

	for (i=1; i <= m_nLocBoxes; i++) {
		size += 4;
		size += m_allBoxes[i]->numbbox;
	}

	return size;
}

void RMGameBoxes::SaveState(byte *state) {
	int i,j;
	
	// Save the number of locations with boxes
	WRITE_LE_UINT32(state, m_nLocBoxes);
	state += 4;

	// For each location, write out the number of boxes and their status
	for (i=1; i <= m_nLocBoxes; i++) {
		WRITE_LE_UINT32(state, m_allBoxes[i]->numbbox);
		state+=4;
				
		for (j=0; j < m_allBoxes[i]->numbbox; j++)
			*state++ = m_allBoxes[i]->boxes[j].attivo;
	}
}

void RMGameBoxes::LoadState(byte *state) {
	int i,j;
	int nloc,nbox;

	// Load number of locations with box
	nloc = *(int*)state;
	state+=4;

	// Controlla che siano meno di quelli correnti
	assert(nloc <= m_nLocBoxes);

	// Per ogni locazione, salva il numero di box e il loro stato
	for (i = 1; i <= nloc; i++) {
		nbox = READ_LE_UINT32(state);
		state += 4;

		for (j=0; j<nbox ; j++)
		{
			if (j < m_allBoxes[i]->numbbox)	
				m_allBoxes[i]->boxes[j].attivo = *state;

			state++;
		}

		m_allBoxes[i]->RecalcAllAdj();
	}
}

} // End of namespace Tony
