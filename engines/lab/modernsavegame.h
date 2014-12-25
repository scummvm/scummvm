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

#ifndef LAB_MODERNGAMESAVE_H
#define LAB_MODERNGAMESAVE_H

namespace Lab {

#define MAX_SAVED_GAMES 15
#define SAVED_IMAGE_SIZE (128 * 72)

extern byte g_SaveGameImage[SAVED_IMAGE_SIZE]; // 640 x 358

struct SaveGameInfo {
	unsigned short Index;
	unsigned short RoomNumber;
	unsigned short Direction;
	byte *SaveGameImage;
	char SaveGameDate[128];
};

int getSaveGameList(SaveGameInfo *info, int maxNum);
void freeSaveGameList(SaveGameInfo *info, int count);

} // End of namespace Lab

#endif /* LAB_MODERNGAMESAVE_H */

