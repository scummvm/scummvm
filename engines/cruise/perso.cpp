/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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
 * $URL$
 * $Id$
 *
 */

#include "cruise/cruise_main.h"
#include "common/util.h"

namespace Cruise {

persoStruct *persoTable[NUM_PERSONS];

int16 computedVar14;

void freePerso(int persoIdx) {
	if (persoTable[persoIdx]) {
		free(persoTable[persoIdx]);
		persoTable[persoIdx] = NULL;
	}
}

void freeAllPerso(void) {
	int i;

	for (i = 0; i < NUM_PERSONS; i++) {
		freePerso(i);
	}

	if (polyStruct) {
		free(polyStruct);
	}

	ctpVar17 = NULL;
	polyStruct = NULL;

	strcpy((char *)currentCtpName, "");
}

int pathVar0;

unsigned int inc_droite2, inc_jo;

int direction(int x1, int y1, int x2, int y2, int inc_jo1, int inc_jo2) {
	int h, v, h1, v1;

	h1 = x1 - x2;
	h = ABS(h1);
	v1 = y1 - y2;
	v = ABS(v1);

	if (v > h) {
		if (h > 30)
			inc_jo = inc_jo1 - inc_jo2;
		else
			inc_jo = inc_jo2;

		if (v1 < 0)
			return (2);
		else
			return (0);
	} else {
		inc_jo = inc_jo1;

		if (h1 < 0)
			return (1);
		else
			return (3);
	}
}

void cor_droite(int x1, int y1, int x2, int y2, int16 cor_joueur[400][2]) {
	int16 *di = (int16 *) cor_joueur;
	int dx;
	int dy;

	int mD0;
	int mD1;

	int mA0;
	int mA1;

	int bp;
	int cx;
	int si;

	int ax;
	int bx;

	di[0] = x1;
	di[1] = y1;
	di += 2;

	dx = x2 - x1;
	dy = y2 - y1;

	mD0 = mD1 = 1;

	if (dx < 0) {
		dx = -dx;
		mD0 = -1;
	}

	if (dy < 0) {
		dy = -dy;
		mD1 = -1;
	}

	if (dx < dy) {
		mA0 = 0;
		bp = dx;
		cx = dy;

		mA1 = mD1;
	} else {
		mA1 = 0;
		bp = dy;
		cx = dx;

		mA0 = mD0;
	}

	bp = bp * 2;
	dx = bp - cx;
	si = dx - cx;

	ax = x1;
	bx = y1;

	while (--cx) {
		if (dx > 0) {
			ax += mD0;
			bx += mD1;
			dx += si;
		} else {
			ax += mA0;
			bx += mA1;
			dx += bp;
		}

		di[0] = ax;
		di[1] = bx;
		di += 2;
	}

	flag_obstacle = 0;
	inc_droite2 = (di - (int16 *) cor_joueur) / 2;
}

void processActorWalk(int16 resx_y[4], int16 *inc_droite, int16 *inc_droite0,
    	int16 *inc_chemin, int16 cor_joueur[400][2],
    	int16 solution0[NUM_NODES + 3][2], int16 *inc_jo1, int16 *inc_jo2,
    	int16 *dir_perso, int16 *inc_jo0, int16 num) {
		int x1, x2, y1, y2;
		int i, u;

	u = 0;
	inc_jo = *inc_jo0;

	i = *inc_chemin;

	if (!*inc_droite) {
		x1 = solution0[i][0];
		y1 = solution0[i][1];
		i++;
		if (solution0[i][0] != -1) {
			do {
				if (solution0[i][0] != -2) {
					x2 = solution0[i][0];
					y2 = solution0[i][1];
					if ((x1 == x2) && (y1 == y2)) {
						resx_y[0] = -1;
						resx_y[1] = -1;
						freePerso(num);

						return;
					}
					cor_droite(x1, y1, x2, y2, cor_joueur);
					*inc_droite0 = inc_droite2;
					*dir_perso = resx_y[2] =
					    direction(x1, y1, x2, y2, *inc_jo1,
					    *inc_jo2);
					*inc_jo0 = inc_jo;
					u = 1;
				} else
					i++;

			} while (solution0[i][0] != -1 && !u);
		}
		if (!u) {
			resx_y[0] = -1;
			resx_y[1] = -1;
			freePerso(num);

			return;
		}
		*inc_chemin = i;
	}

	resx_y[0] = cor_joueur[*inc_droite][0];
	resx_y[1] = cor_joueur[*inc_droite][1];
	resx_y[2] = *dir_perso;
	resx_y[3] = computeZoom(resx_y[1]);

	getPixel(resx_y[0], resx_y[1]);
	resx_y[4] = computedVar14;

	u = subOp23(resx_y[3], inc_jo);
	if (!u)
		u = 1;
	*inc_droite += u;

	if ((*inc_droite) >= (*inc_droite0)) {
		*inc_droite = 0;
		resx_y[0] = solution0[*inc_chemin][0];
		resx_y[1] = solution0[*inc_chemin][1];
	}

}

void affiche_chemin(int16 persoIdx, int16 *returnVar) {
	persoStruct *pPerso = persoTable[persoIdx];

	ASSERT(pPerso);

	processActorWalk(returnVar, &pPerso->inc_droite, &pPerso->inc_droite0,
	    &pPerso->inc_chemin, pPerso->coordinates, pPerso->solution,
	    &pPerso->inc_jo1, &pPerso->inc_jo2, &pPerso->dir_perso,
	    &pPerso->inc_jo0, persoIdx);
}

} // End of namespace Cruise
