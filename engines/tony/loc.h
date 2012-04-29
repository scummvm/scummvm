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

#ifndef TONY_LOC_H
#define TONY_LOC_H

#include "common/scummsys.h"
#include "utils.h"

namespace Tony {

#define MAXBOXES	50		// Non si puo' cambiare, comanda cosi' il boxed
#define MAXHOTSPOT 20		// Idem

class RMBox {
public:
	struct T_HOTSPOT {
		int hotx, hoty;          // coordinate HotSpot
		int destination;         // destinazione HotSpot
	};

public:
	int left,top,right,bottom;		// Vertici BoundingBox
	int adj[MAXBOXES];				// Lista di adjacenza
	int numhotspot;					// Numero HotSpot
	uint8 Zvalue;					// Zvalue per quel BoundingBox
	T_HOTSPOT hotspot[MAXHOTSPOT];	// Lista degli HotSpot

	bool attivo;
	bool bReversed;

private:
	void ReadFromStream(RMDataStream &ds);

public:
	friend RMDataStream &operator>>(RMDataStream &ds, RMBox &box);
};


class RMBoxLoc {
public:
	int numbbox;
	RMBox *boxes;

private:
	void ReadFromStream(RMDataStream& ds);

public:
	friend RMDataStream& operator >>(RMDataStream &ds, RMBoxLoc &bl);
	void RecalcAllAdj(void);
};


class RMGameBoxes {	
protected:
	RMBoxLoc *m_allBoxes[200];
	int m_nLocBoxes;

public:
	void Init(void);		
	void Close(void);

	// Prende i box di una locazione
	RMBoxLoc *GetBoxes(int nLoc);
	
	// Calcola in quale box si trova il punto
	int WhichBox(int nLoc, RMPoint pt);

	// Controlla che il punto sia dentro un certo box
	bool IsInBox(int nLoc, int nBox, RMPoint pt);
	
	// Cambia lo stato di un box
	void ChangeBoxStatus(int nLoc, int nBox, int status);

	// Salvataggi
	int GetSaveStateSize(void);
	void SaveState(byte *buf);
	void LoadState(byte *buf);
};

} // End of namespace Tony

#endif /* TONY_H */
