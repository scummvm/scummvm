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

#include "lab/stddefines.h"
#include "lab/labfun.h"
#include "lab/modernsavegame.h"

namespace Lab {

byte g_SaveGameImage[SAVED_IMAGE_SIZE]; // 640 x 358

char g_SaveGamePath[512];
char g_PathSeperator[4];

#define SAVEVERSION         "LBS3"

int getSaveGameList(struct SaveGameInfo *info, int maxNum) {
	warning("STUB: getSaveGameList");

	return 0;

#if 0
	char path[512];
	struct stat statb;
	int total = 0;
	int i;

	for (i = 0; i < maxNum; i++) {
		checkMusic();

		sprintf(path, "%s%s%d", g_SaveGamePath, g_PathSeperator, i);
		statb.st_size = 0;
		stat(path, &statb);

		if (statb.st_size > 0) {
			struct tm *create_date;
			FILE *fh;

			create_date = localtime(&statb.st_ctime);
			strcpy(info->SaveGameDate, asctime(create_date));

			fh = fopen(path, "rb");

			if (fh != NULL) {
				char temp[5];
				unsigned short t;
				int toSeek;

				info->Index = i;

				fread(temp, 1, 4, fh);
				temp[4] = 0;

				fread(&t, 1, 2, fh);
				info->RoomNumber = swapUShort(t);
				fread(&t, 1, 2, fh);
				info->Direction = swapUShort(t);

				toSeek = 2 + Conditions->lastElement / 8 + g_lab->_roomsFound->_lastElement / 8 + 6 + 2 * 16;
				fseek(fh, toSeek, SEEK_CUR);

				info->SaveGameImage = NULL;

				if (strcmp(temp, SAVEVERSION) == 0) {
					info->SaveGameImage = malloc(SAVED_IMAGE_SIZE);

					if (info->SaveGameImage != NULL)
						fread(info->SaveGameImage, 1, SAVED_IMAGE_SIZE, fh);
				} else {
					info->SaveGameImage = malloc(SAVED_IMAGE_SIZE);

					if (info->SaveGameImage != NULL)
						memset(info->SaveGameImage, 0, SAVED_IMAGE_SIZE);
				}

				fclose(fh);

				info++;
				total++;
			}
		}
	}

	return total;
#endif
}

void freeSaveGameList(struct SaveGameInfo *info, int count) {
	int i;

	for (i = 0; i < count; i++) {
		free(info->SaveGameImage);
		++info;
	}
}

} // End of namespace Lab
