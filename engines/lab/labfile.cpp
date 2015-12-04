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

#include "lab/lab.h"
#include "lab/labfun.h"
#include "common/file.h"

namespace Lab {


/*****************************************************************************/
/* Reads a block of memory.                                                  */
/*****************************************************************************/
void readBlock(void *Buffer, uint32 Size, byte **File) {
	memcpy(Buffer, *File, (size_t) Size);
	(*File) += Size;
}

static char NewFileName[255];

/*****************************************************************************/
/* Modifies the filename so that paths and stuff are correct.  Should mostly  */
/* deal with assigns and the '/' instead of '\' on IBM systems.              */
/*                                                                           */
/* NOTE: Make a *copy* of the string, and modify that.  It would be a real   */
/* *bad* idea to modify the original.  Since Labyrinth only focuses its      */
/* attention to one file at a time, it would be fine to have one variable    */
/* not on the stack which is used to store the new filename.                 */
/*****************************************************************************/
char *translateFileName(const char *filename) {
	Common::String fileNameStr = filename;
	fileNameStr.toUppercase();
	Common::String fileNameStrFinal;

	if (fileNameStr.hasPrefix("P:")) {
		if (g_lab->_isHiRes)
			fileNameStrFinal = "GAME/SPICT/";
		else
			fileNameStrFinal = "GAME/PICT/";
	} else if (fileNameStr.hasPrefix("LAB:"))
		fileNameStrFinal = "GAME/";
	else if (fileNameStr.hasPrefix("MUSIC:"))
		fileNameStrFinal = "GAME/MUSIC/";

	if (fileNameStr.contains(':')) {
		while (fileNameStr[0] != ':') {
			fileNameStr.deleteChar(0);
		}

		fileNameStr.deleteChar(0);
	}

	fileNameStrFinal += fileNameStr;

	strcpy(NewFileName, fileNameStrFinal.c_str());

	return NewFileName;
}


} // End of namespace Lab
