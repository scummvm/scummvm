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

#include "lab/labfun.h"
#include "lab/parsetypes.h"
#include "lab/parsefun.h"
#include "lab/stddefines.h"

namespace Lab {

#define MAXSTRINGLENGTH 250

static bool UseMemory = false;



/* Global parser data */

extern RoomData *Rooms;
extern InventoryData *Inventory;
extern uint16 NumInv, ManyRooms, HighestCondition, Direction;

static uint16 allocroom;


static bool rallocate(void **Ptr, uint32 Size) {
	if (UseMemory)
		return ((*Ptr = calloc(Size, 1)) != 0);
	else {
		allocRoom(Ptr, (uint16) Size, allocroom);
		return true;
	}
}



/*****************************************************************************/
/* Reads in the RoomData.                                                    */
/*****************************************************************************/
bool readRoomData(const char *fileName) {
	byte **file;
	uint16 Counter;
	char Temp[5];

	if ((file = g_music->newOpen(fileName)) != NULL) {
		readBlock(Temp, 4L, file);
		Temp[4] = '\0';

		if (strcmp(Temp, "DOR1") != 0)
			return false;

		readBlock(&ManyRooms, 2L, file);
		swapUShortPtr(&ManyRooms, 1);
		readBlock(&HighestCondition, 2L, file);
		swapUShortPtr(&HighestCondition, 1);

		if ((Rooms = (RoomData *)calloc(ManyRooms + 1, sizeof(RoomData)))) {
			for (Counter = 1; Counter <= ManyRooms; Counter++) {
				readBlock(&(Rooms[Counter].NorthDoor), 2L, file);
				readBlock(&(Rooms[Counter].SouthDoor), 2L, file);
				readBlock(&(Rooms[Counter].EastDoor),  2L, file);
				readBlock(&(Rooms[Counter].WestDoor),  2L, file);

				swapUShortPtr(&(Rooms[Counter].NorthDoor), 1);
				swapUShortPtr(&(Rooms[Counter].SouthDoor), 1);
				swapUShortPtr(&(Rooms[Counter].EastDoor), 1);
				swapUShortPtr(&(Rooms[Counter].WestDoor), 1);

				readBlock(&(Rooms[Counter].WipeType),  1L, file);
			}
		} else
			return false;
	} else
		return false;

	return true;
}




/*****************************************************************************/
/* Reads in a NULL terminated string, and allocates memory for it.           */
/*****************************************************************************/
static bool readString(char **string, byte **file) {
	char len;
	uint32 counter = 0L;


	readBlock(&len, 1L, file);

	if (len) {
		counter = len;

		if (rallocate((void **) string, counter)) {
			readBlock(*string, counter, file);
			(*string)[counter - 1] = 0; /* Sanity modification */
			decrypt((byte *)*string);
			return true;
		}
	} else {
		*string = NULL;
		return true;
	}

	return false;
}



/*****************************************************************************/
/* Reads in the Inventory data.                                              */
/*****************************************************************************/
bool readInventory(const char *fileName) {
	byte **file;
	uint16 Counter;
	char Temp[5];

	if ((file = g_music->newOpen(fileName)) != NULL) {
		readBlock(Temp, 4L, file);
		Temp[4] = '\0';

		if (strcmp(Temp, "INV1") != 0)
			return false;

		readBlock(&NumInv, 2L, file);
		swapUShortPtr(&NumInv, 1);

		UseMemory = true;

		if (rallocate((void **) &Inventory, (NumInv + 1) * sizeof(InventoryData))) {
			for (Counter = 1; Counter <= NumInv; Counter++) {
				readBlock(&(Inventory[Counter].Many), 2L, file);
				swapUShortPtr(&(Inventory[Counter].Many), 1);

				if (!readString(&(Inventory[Counter].name), file)) {
					UseMemory = false;
					return false;
				}

				if (!readString(&(Inventory[Counter].BInvName), file)) {
					UseMemory = false;
					return false;
				}
			}
		} else {
			UseMemory = false;
			return false;
		}
	} else
		return false;

	UseMemory = false;
	return true;
}






/*****************************************************************************/
/* Converts a number to a string.                                            */
/*****************************************************************************/
char *numtostr(char *text, uint16 Num) {
	uint16 Counter = 0;

	if (Num == 0) {
		text[0] = '0';
		text[1] = 0;
		return text;
	}

	strcpy(text, "00000");

	if (Num >= 10000) {
		text[0] = (Num / 10000) + '0';
		Num -= (Num / 10000) * 10000;
	}

	if (Num >= 1000) {
		text[1] = (Num / 1000) + '0';
		Num -= (Num / 1000) * 1000;
	}

	if (Num >= 100) {
		text[2] = (Num / 100) + '0';
		Num -= (Num / 100) * 100;
	}

	if (Num >= 10) {
		text[3] = (Num / 10) + '0';
		Num -= (Num / 10) * 10;
	}

	text[4] = Num + '0';
	text[5] = 0;

	while (text[Counter] == '0')
		Counter++;

	return (&text[Counter]);
}





/*****************************************************************************/
/* Reads in a list of conditions.                                            */
/*****************************************************************************/
static int16 *readConditions(byte **file) {
	int16 Counter = 0, last, list[25], *ptr;

	do {
		readBlock(&last, 2L, file);
		swapUShortPtr((uint16 *)&last, 1);

		if (Counter < 25) {
			list[Counter] = last;
			Counter++;
		} else
			list[Counter] = 0;
	} while (last);

	if (!rallocate((void **) & (ptr), Counter * 2L))
		return NULL;

	memcpy(ptr, list, (size_t)(Counter * 2L));

	return ptr;
}




/*****************************************************************************/
/* Reads in some CloseUp data.                                               */
/*****************************************************************************/
static bool readCloseUps(CloseDataPtr *CPtr, uint16 depth, byte **file) {
	char c;

	while (1) {
		*CPtr = NULL;

		readBlock(&c, 1L, file);

		if (c != '\0') {
			if (rallocate((void **) CPtr, sizeof(CloseData))) {
				(*CPtr)->SubCloseUps = NULL;
				(*CPtr)->NextCloseUp = NULL;
				(*CPtr)->depth       = depth;

				readBlock(*CPtr, 10L, file);
				swapUShortPtr((uint16 *)*CPtr, 5);

				if (!readString(&((*CPtr)->GraphicName), file))
					return false;

				if (!readString(&((*CPtr)->Message), file))
					return false;

				if (!readCloseUps(&((*CPtr)->SubCloseUps), depth + 1, file))
					return false;

				CPtr = &((*CPtr)->NextCloseUp);
			} else
				return false;
		} else
			return true;
	}
}




/*****************************************************************************/
/* Reads in a View.                                                          */
/*****************************************************************************/
static bool readView(ViewDataPtr *VPtr, byte **file) {
	char c;

	while (1) {
		*VPtr = NULL;

		readBlock(&c, 1L, file);

		if (c == 1) {
			if (rallocate((void **) VPtr, sizeof(viewData))) {
				(*VPtr)->closeUps = NULL;
				(*VPtr)->NextCondition = NULL;

				(*VPtr)->Condition = readConditions(file);

				if (!(*VPtr)->Condition)
					return false;

				if (!readString(&((*VPtr)->GraphicName), file))
					return false;

				readCloseUps(&((*VPtr)->closeUps), 0, file);

				VPtr = &((*VPtr)->NextCondition);
			} else
				return false;
		} else
			return true;
	}
}




/*****************************************************************************/
/* Reads in an Action.                                                       */
/*****************************************************************************/
static bool readAction(ActionPtr *APtr, byte **file) {
	char c;
	byte **ptrarray;
	uint16 counter;

	while (1) {
		*APtr = NULL;

		readBlock(&c, 1L, file);

		if (c == 1) {
			if (rallocate((void **) APtr, sizeof(Action))) {
				readBlock(*APtr, 8L, file);
				swapShortPtr((int16 *)*APtr, 4);

				if ((*APtr)->ActionType == SHOWMESSAGES) {
					if (!rallocate((void **) &ptrarray, 4L * (*APtr)->Param1))
						return false;

					for (counter = 0; counter < (*APtr)->Param1; counter++)
						readString((char **) & (ptrarray[counter]), file);

					(*APtr)->Data = (byte *)ptrarray;
				} else
					readString((char **) & ((*APtr)->Data), file);

				APtr = &((*APtr)->NextAction);
			} else
				return false;
		} else
			return true;
	}
}





/*****************************************************************************/
/* Reads in a rule.                                                          */
/*****************************************************************************/
static bool readRule(RulePtr *RPtr, byte **file) {
	char c;

	while (1) {
		*RPtr = NULL;
		readBlock(&c, 1L, file);

		if (c == 1) {
			if (rallocate((void **) RPtr, sizeof(Rule))) {
				readBlock(*RPtr, 6L, file);
				swapShortPtr((int16 *)*RPtr, 3);

				(*RPtr)->Condition = readConditions(file);

				if (!(*RPtr)->Condition)
					return false;

				readAction(&((*RPtr)->ActionList), file);
			} else
				return false;

			RPtr = &((*RPtr)->NextRule);
		} else
			return true;
	}
}





/*****************************************************************************/
/* Reads in the views of a room.                                             */
/*****************************************************************************/
bool readViews(uint16 RoomNum, const char *Path) {
	char Temp[10], *RoomString, fileName[250];
	byte **file;

	allocroom = RoomNum;

	RoomString = numtostr(Temp, RoomNum);
	strcpy(fileName, Path);
	strcat(fileName, RoomString);

	if ((file = g_music->newOpen(fileName)) != NULL) {
		readBlock(Temp, 4L, file);
		Temp[4] = '\0';

		if (strcmp(Temp, "ROM4") != 0)
			return false;

		readString(&(Rooms[RoomNum].RoomMsg), file);

		readView(&Rooms[RoomNum].NorthView, file);
		g_music->checkMusic();

		readView(&Rooms[RoomNum].SouthView, file);
		g_music->checkMusic();

		readView(&Rooms[RoomNum].EastView, file);
		g_music->checkMusic();

		readView(&Rooms[RoomNum].WestView, file);
		g_music->checkMusic();

		readRule(&Rooms[RoomNum].RuleList, file);

		return true;
	} else
		return false;

	return false;
}

} // End of namespace Lab
