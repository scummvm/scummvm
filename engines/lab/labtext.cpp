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

namespace Lab {

static uint32 SizeOfMemChunk;
static char *BeginOfMemChunk, *CurPlace;


char *LOWERFLOORS, *MIDDLEFLOORS, *UPPERFLOORS, *MEDMAZEFLOORS, *HEDGEMAZEFLOORS, *SURMAZEFLOORS, *CARNIVALFLOOR, *SURMAZEMSG, *FACINGNORTH, *FACINGEAST, *FACINGSOUTH, *FACINGWEST, *LAMPONMSG, *TURNLEFT, *TURNRIGHT, *GOFORWARDDIR, *NOPATH, *TAKEITEM, *SAVETEXT, *LOADTEXT, *BOOKMARKTEXT, *PERSONALTEXT, *DISKTEXT, *SAVEBOOK, *RESTOREBOOK, *SAVEFLASH, *RESTOREFLASH, *SAVEDISK, *RESTOREDISK, *NODISKINDRIVE, *WRITEPROTECTED, *SELECTDISK, *FORMATFLOPPY, *FORMATTING, *NOTHING, *USEONWHAT, *TAKEWHAT, *MOVEWHAT, *OPENWHAT, *CLOSEWHAT, *LOOKWHAT, *USEMAP, *USEJOURNAL, *TURNLAMPON, *TURNLAMPOFF, *USEWHISKEY, *USEPITH, *USEHELMET;


#define LABTEXTFILE "Lab:Rooms/LabText"




/*****************************************************************************/
/* Gets the next string from the list, and changes the end of string marker  */
/* from an end of line to a null character.                                  */
/*****************************************************************************/
static void setString(char **string) {
	*string = CurPlace;

	while (*CurPlace != '\n')
		CurPlace++;

	*CurPlace = 0;
	CurPlace++;
}



/*****************************************************************************/
/* Initializes everything for the Labyrinth text stuff                       */
/*****************************************************************************/
bool initLabText(void) {
	if ((SizeOfMemChunk = sizeOfFile(LABTEXTFILE)))
		if ((BeginOfMemChunk = (char *)calloc(SizeOfMemChunk, 1))) {
			Common::File *file = openPartial(LABTEXTFILE);

			if (file) {
				file->read(BeginOfMemChunk, SizeOfMemChunk);
				file->close();

				CurPlace = BeginOfMemChunk;

				setString(&LOWERFLOORS);
				setString(&MIDDLEFLOORS);
				setString(&UPPERFLOORS);
				setString(&MEDMAZEFLOORS);
				setString(&HEDGEMAZEFLOORS);
				setString(&SURMAZEFLOORS);
				setString(&CARNIVALFLOOR);
				setString(&SURMAZEMSG);

				setString(&FACINGNORTH);
				setString(&FACINGEAST);
				setString(&FACINGSOUTH);
				setString(&FACINGWEST);

				setString(&LAMPONMSG);

				setString(&TURNLEFT);
				setString(&TURNRIGHT);
				setString(&GOFORWARDDIR);
				setString(&NOPATH);
				setString(&TAKEITEM);

				setString(&SAVETEXT);
				setString(&LOADTEXT);
				setString(&BOOKMARKTEXT);
				setString(&PERSONALTEXT);
				setString(&DISKTEXT);

				setString(&SAVEBOOK);
				setString(&RESTOREBOOK);
				setString(&SAVEFLASH);
				setString(&RESTOREFLASH);
				setString(&SAVEDISK);
				setString(&RESTOREDISK);
				setString(&NODISKINDRIVE);
				setString(&WRITEPROTECTED);
				setString(&SELECTDISK);

				setString(&FORMATFLOPPY);
				setString(&FORMATTING);

				setString(&NOTHING);
				setString(&USEONWHAT);
				setString(&TAKEWHAT);
				setString(&MOVEWHAT);
				setString(&OPENWHAT);
				setString(&CLOSEWHAT);
				setString(&LOOKWHAT);

				setString(&USEMAP);
				setString(&USEJOURNAL);
				setString(&TURNLAMPON);
				setString(&TURNLAMPOFF);
				setString(&USEWHISKEY);
				setString(&USEPITH);
				setString(&USEHELMET);

				return true;
			}
		}

	return false;
}


/*****************************************************************************/
/* Frees the memory from the Labyrinth text stuff.                           */
/*****************************************************************************/
void freeLabText(void) {
	if (SizeOfMemChunk && BeginOfMemChunk)
		free(BeginOfMemChunk);
}




/*****************************************************************************/
/* Decrypts a chunk of text.                                                 */
/*****************************************************************************/
void decrypt(byte *text) {
	if (text == NULL)
		return;

	while (*text != '\0') {
		(*text) -= (byte)95;
		text++;
	}
}

} // End of namespace Lab
