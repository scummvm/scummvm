/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef WATCHMAKER_BALL_H
#define WATCHMAKER_BALL_H

#include "watchmaker/types.h"
#include "watchmaker/t3d.h"
#include "watchmaker/struct.h"
#include "watchmaker/globvar.h"

namespace Watchmaker {

struct SPhys {
	t3dF32 Kg = 0.0f;                                           // costante gravitazionale
	t3dF32 Kn = 0.0f;                                           // coefficiente di attrito viscoso
	t3dF32 Kc = 0.0f;                                           // percentuale di energia cinetica che si conserva
	t3dF32 Kt = 0.0f;                                           // percentuale di energia cinetica che si conserva per terra
	t3dF32 Mp = 0.0f;                                           // massa palla
	t3dF32 Mb = 0.0f;                                           // massa braccio
	t3dF32 Rp = 0.0f;                                           // raggio palla
	t3dF32 Td = 0.0f;                                           // differenza di tempo trascorsa tra un istante e l'altro
	t3dF32 Ts = 0.0f;                                           // tempo trascorso dall'ultimo impatto
	t3dF32 Tb = 0.0f;                                           // tempo di carica del braccio

	t3dV3F Fb;                                                  // forza braccio
	t3dV3F Vb;                                                  // velocita' braccio
	t3dV3F Fv;                                                  // forza del vento
	t3dV3F V0;                                                  // velocita' iniziale palla
	t3dV3F S0;                                                  // posizione iniziale palla

	t3dV3F St;                                                  // posizione attuale
	t3dV3F Vt;                                                  // velocita' attuale
	t3dV3F At;                                                  // accelerazione attuale
	t3dF32 MVt = 0.0f;                                          // modulo della velocita' attuale
	t3dF32 MAt = 0.0f;                                          // modulo dell'accelerazione attuale

	uint8 InAir;                                                // Se e' in volo
	t3dV3F Angle;                                               // Angolazione colpo
	t3dF32 TimeLeft = 0.0f;                                     // Tempo Riamente
	SD3DTriangle ViewCone;                              // Cono visivo
};

void InitPhys(struct SPhys *p);
void ContinuePhys(struct SPhys *p);
void ChangePhys(struct SPhys *p, int flags);
void NextGopher(WGame &game);
void ProcessGopherKeyboard();
void UpdateArrow();
void ProcessGopherCamera(WGame &game);
void UpdateBall(WGame &game, struct SPhys *p);

extern struct SPhys Ball[];

} // End of namespace Watchmaker

#endif // WATCHMAKER_BALL_H
