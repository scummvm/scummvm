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

namespace Lab {

bool IsHiRes = false;

/*****************************************************************************/
/* Scales the x co-ordinates to that of the new display.  In the room parser */
/* file, co-ordinates are set up on a 360x336 display.                       */
/*****************************************************************************/
uint16 scaleX(uint16 x) {
	if (IsHiRes)
		return (uint16)((x * 16) / 9);
	else
		return (uint16)((x * 8) / 9);
}




/*****************************************************************************/
/* Scales the y co-ordinates to that of the new display.  In the room parser */
/* file, co-ordinates are set up on a 368x336 display.                       */
/*****************************************************************************/
uint16 scaleY(uint16 y) {
	if (IsHiRes)
		return (y + (y / 14));
	else
		return ((y * 10) / 24);
}




/*****************************************************************************/
/* Scales the VGA cords to SVGA if necessary; otherwise, returns VGA cords.  */
/*****************************************************************************/
uint16 VGAScaleX(uint16 x) {
	if (IsHiRes)
		return (x * 2);
	else
		return x;
}




/*****************************************************************************/
/* Scales the VGA cords to SVGA if necessary; otherwise, returns VGA cords.  */
/*****************************************************************************/
uint16 VGAScaleY(uint16 y) {
	if (IsHiRes)
		return ((y * 12) / 5);
	else
		return y;
}



/*****************************************************************************/
/* Scales the VGA cords to SVGA if necessary; otherwise, returns VGA cords.  */
/*****************************************************************************/
int16 VGAScaleXs(int16 x) {
	if (IsHiRes)
		return (x * 2);
	else
		return x;
}




/*****************************************************************************/
/* Scales the VGA cords to SVGA if necessary; otherwise, returns VGA cords.  */
/*****************************************************************************/
int16 VGAScaleYs(int16 y) {
	if (IsHiRes)
		return ((y * 12) / 5);
	else
		return y;
}




uint16 SVGACord(uint16 cord) {
	if (IsHiRes)
		return cord;
	else
		return 0;
}




/*****************************************************************************/
/* Converts SVGA cords to VGA if necessary, otherwise returns VGA cords.     */
/*****************************************************************************/
uint16 VGAUnScaleX(uint16 x) {
	if (IsHiRes)
		return (x / 2);
	else
		return x;
}




/*****************************************************************************/
/* Converts SVGA cords to VGA if necessary, otherwise returns VGA cords.     */
/*****************************************************************************/
uint16 VGAUnScaleY(uint16 y) {
	if (IsHiRes)
		return ((y * 5) / 12);
	else
		return y;
}




#if defined(WIN32)
/*****************************************************************************/
/* Replaces all occurences of a character in a string with a new one.        */
/*****************************************************************************/
static void strrplc(char *text, char orig, char replace) {
	uint16 counter;

	counter = 0;

	while (text[counter]) {
		if (text[counter] == orig)
			text[counter] = replace;

		counter++;
	}
}
#endif


/*****************************************************************************/
/* Checks to see if all the characters in the second string are at the start */
/* of the first.                                                             */
/*****************************************************************************/
static bool strstart(const char **Source, const char *Start) {
	uint16 len1, len2, counter;

	len1 = strlen(*Source);
	len2 = strlen(Start);

	if (len1 < len2)
		return false;

	for (counter = 0; counter < len2; counter++)
		if ((*Source)[counter] != Start[counter])
			return false;

	(*Source) += len2;
	return true;
}


static char NewFileName[255];

/*****************************************************************************/
/* Modifes the filename so that paths and stuff are correct.  Should mostly  */
/* deal with assigns and the '/' instead of '\' on IBM systems.              */
/*                                                                           */
/* NOTE: Make a *copy* of the string, and modify that.  It would be a real   */
/* *bad* idea to modify the original.  Since Labyrinth only focuses its      */
/* attention to one file at a time, it would be fine to have one variable    */
/* not on the stack which is used to store the new filename.                 */
/*****************************************************************************/
static void mystrupr(char *s) {
	char c;

	while ((c = *s) != 0)
		*s++ = toupper(c);
}

char *translateFileName(const char *filename) {
	char tempfilename[255];
	char *dot;

	strcpy(tempfilename, filename);
	mystrupr(tempfilename);

	*NewFileName = 0;
	filename = tempfilename;

	if (strstart(&filename, "P:")) {
		if (IsHiRes)
			strcat(NewFileName, "GAME/SPICT/");
		else
			strcat(NewFileName, "GAME/PICT/");
	} else if (strstart(&filename, "LAB:"))
		strcat(NewFileName, "GAME/");
	else if (strstart(&filename, "MUSIC:"))
		strcat(NewFileName, "GAME/MUSIC/");

	strcat(NewFileName, filename);

	dot = strrchr(NewFileName, '.');

	if (dot != NewFileName && dot != NULL && dot[4] != '/') { // Linux may start with '.'
		dot[4] = 0; // Back to the days of 8.3, even if your OS was never DOSish!!
	}

	return NewFileName;
}

#if defined(USE_SWAP)

uint16 swapUShort(uint16 value) {
	char *b = (char *)&value;
	char t = b[0];
	b[0] = b[1];
	b[1] = t;
	return value;
}
uint16 *swapUShortPtr(uint16 *ptr, int count) {
	while (count-- > 0) {
		char *b = (char *)ptr++;
		char t = b[0];
		b[0] = b[1];
		b[1] = t;
	}

	return ptr;
}
int16 swapShort(int16 value) {
	char *b = (char *)&value;
	char t = b[0];
	b[0] = b[1];
	b[1] = t;
	return value;
}
int16 *swapShortPtr(int16 *ptr, int count) {
	while (count-- > 0) {
		char *b = (char *)ptr++;
		char t = b[0];
		b[0] = b[1];
		b[1] = t;
	}

	return ptr;
}
unsigned int swapUInt(unsigned int value) {
	char *b = (char *)&value;
	char t = b[0];
	b[0] = b[3];
	b[3] = t;
	t = b[1];
	b[1] = b[2];
	b[2] = t;
	return value;
}
unsigned int *swapUIntPtr(unsigned int *ptr, int count) {
	while (count-- > 0) {
		char *b = (char *)ptr++;
		char t = b[0];
		b[0] = b[3];
		b[3] = t;
		t = b[1];
		b[1] = b[2];
		b[2] = t;
	}

	return ptr;
}
int swapInt(int value) {
	char *b = (char *)&value;
	char t = b[0];
	b[0] = b[3];
	b[3] = t;
	t = b[1];
	b[1] = b[2];
	b[2] = t;
	return value;
}
int *swapIntPtr(int *ptr, int count) {
	while (count-- > 0) {
		char *b = (char *)ptr++;
		char t = b[0];
		b[0] = b[3];
		b[3] = t;
		t = b[1];
		b[1] = b[2];
		b[2] = t;
	}

	return ptr;
}
uint32 swapULong(uint32 value) {
	char *b = (char *)&value;
	char t = b[0];
	b[0] = b[3];
	b[3] = t;
	t = b[1];
	b[1] = b[2];
	b[2] = t;
	return value;
}
uint32 *swapULongPtr(uint32 *ptr, int count) {
	while (count-- > 0) {
		char *b = (char *)ptr++;
		char t = b[0];
		b[0] = b[3];
		b[3] = t;
		t = b[1];
		b[1] = b[2];
		b[2] = t;
	}

	return ptr;
}
int32 swapLong(int32 value) {
	char *b = (char *)&value;
	char t = b[0];
	b[0] = b[3];
	b[3] = t;
	t = b[1];
	b[1] = b[2];
	b[2] = t;
	return value;
}
int32 *swapLongPtr(int32 *ptr, int count) {
	while (count-- > 0) {
		char *b = (char *)ptr++;
		char t = b[0];
		b[0] = b[3];
		b[3] = t;
		t = b[1];
		b[1] = b[2];
		b[2] = t;
	}

	return ptr;
}
#endif

} // End of namespace Lab
