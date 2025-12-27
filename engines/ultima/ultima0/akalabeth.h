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

#ifndef ULTIMA_ULTIMA0_AKALABETH_H
#define ULTIMA_ULTIMA0_AKALABETH_H

#include "common/str.h"
#include "ultima/ultima0/struct.h"			/* Our structure file */

namespace Ultima {
namespace Ultima0 {

void HWInitialise(void);					/* HARDWARE.C prototyping */
void HWTerminate(void);
void HWColour(int);
void HWLine(int, int, int, int);
void HWStatus(double, int, int);
void HWChar(int);
int  HWGetKey(void);
void HWClear(void);

void DRAWTile(RECT *, int);					/* DRAW.C prototyping */
void DRAWSetRect(RECT *, int, int, int, int);
void DRAWMonster(int, int, int, double);
void DRAWDungeon(RECT *, RECT *, int, int, int, int, int);
void DRAWText(const char *, ...);

void MONSTAttack(PLAYER *, DUNGEONMAP *);	/* MONST.C prototyping */

int  MAINSuper(void);						/* MAIN.C prototyping */
void MAINSetup(void);
void MAINStart(void);

void PLAYERInit(PLAYER *); 					/* PLAYER.C prototyping */
void PLAYERDebug(PLAYER *);
void PLAYERDemo(PLAYER *);
void PLAYERCharacter(PLAYER *);
void PLAYERInv(PLAYER *);

void DEADCode(PLAYER *);					/* DEAD.C prototyping */
int  DEADGetYesNo(void);

const char *GLOObjName(int);						/* GLOBAL.C prototyping */
const char *GLOAttribName(int);
const char *GLOClassName(char);
const char *GLOMonsterName(int);
int GLOMonsterLevel(int);
void GLOGetInfo(int n, int *, int *, int *);

void TOWNEnter(WORLDMAP *, PLAYER *);   		/* TOWN.C prototyping */

void BRITISHEnter(WORLDMAP *, PLAYER *);		/* BRITISH.C prototyping */

void WORLDCreate(PLAYER *, WORLDMAP *);		/* WORLD.C prototyping */
void WORLDDraw(PLAYER *, WORLDMAP *, int);
int WORLDRead(WORLDMAP *, int, int);

void DUNGEONCreate(PLAYER *, DUNGEONMAP *);	/* DUNGEON.C prototyping */

void DDRAWDraw(PLAYER *, DUNGEONMAP *);		/* DDRAW.C prototyping */
int DDRAWFindMonster(DUNGEONMAP *, COORD *);

void ATTACKMonster(PLAYER *, DUNGEONMAP *);	/* ATTACK.C prototyping */

/* MOVE.C prototyping */
void MOVEEnterExit(WORLDMAP *, PLAYER *, DUNGEONMAP *);
void MOVEMove(int, WORLDMAP *, PLAYER *, DUNGEONMAP *, int, int);
void MOVERotLeft(COORD *);

} // namespace Ultima0
} // namespace Ultima

#endif
