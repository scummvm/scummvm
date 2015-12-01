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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#ifndef LAB_PARSEFUN_H
#define LAB_PARSEFUN_H

#include "lab/labfun.h"
#include "lab/parsetypes.h"

namespace Lab {

/* From Parser.c */

bool parse(const char *InputFile);


/* From allocRoom.c */

bool initRoomBuffer();
void freeRoomBuffer();
void allocRoom(void **Ptr, uint16 Size, uint16 RoomNum);


/* From ProcessRoom.c */

ViewData *getViewData(uint16 RoomNum, uint16 Direction);
char *getPictName(CloseDataPtr *LCPtr);
void drawDirection(CloseDataPtr LCPtr);
bool processArrow(uint16 *Direction, uint16 Arrow);
void setCurClose(Common::Point pos, CloseDataPtr *cptr, bool useAbsoluteCoords = false);
bool takeItem(uint16 x, uint16 y, CloseDataPtr *cptr);
bool doActionRule(Common::Point pos, int16 action, int16 RoomNum, CloseDataPtr *LCPtr);
bool doOperateRule(int16 x, int16 y, int16 ItemNum, CloseDataPtr *LCPtr);
bool doGoForward(CloseDataPtr *LCPtr);
bool doTurn(uint16 from, uint16 to, CloseDataPtr *LCPtr);
bool doMainView(CloseDataPtr *LCPtr);

} // End of namespace Lab

#endif /* LAB_PARSEFUN_H */
