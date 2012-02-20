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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#ifndef MORTEVIELLE_ACTIONS_H
#define MORTEVIELLE_ACTIONS_H

namespace Mortevielle {

/* NIVEAU 4 */
extern void fctMove();
extern void fctTake();
extern void tsprendre();
extern void fctLift();
extern void fctRead();
extern void tslire();
extern void fctLook();
extern void tsregarder();
extern void fctSearch();
extern void tsfouiller();
extern void fctOpen();
extern void fctPlace();
extern void fctTurn();
extern void fctHideSelf();
extern void fctAttach();
extern void fctClose();
extern void fctKnock();
extern void tposer();
extern void fctListen();
extern void fctEat();
extern void fctEnter();
extern void fctSleep();
extern void fctForce();
extern void fctLeave();
extern void fctWait();
extern void fctSound();
extern void tparler();
extern void fctSmell();
extern void fctScratch();
/* NIVEAU 2 */
extern void endGame();
extern void loseGame();

} // End of namespace Mortevielle
#endif
