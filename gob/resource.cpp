/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/global.h"
#include "gob/video.h"
#include "gob/resource.h"
#include "gob/debug.h"

namespace Gob {

char *resourceBuf = 0;

static char *resourcePtr;

void res_Free(void) {
	if (resourceBuf != 0) {
		free(resourceBuf);
		resourceBuf = 0;
	}
}

void res_Init(void) {
	int16 handle;
	int16 fileSize;
	struct stat statBuf;
	handle = open("ALL.ASK", O_RDONLY);
	if (handle < 0) {
		error("ALL.ASK is missing.");
	}
	if (stat("ALL.ASK", &statBuf) == -1)
		error("res_Init: Error with stat()\n");
	fileSize = statBuf.st_size;

	resourceBuf = (char *)malloc(fileSize * 4);
	read(handle, resourceBuf, fileSize);
	close(handle);
}

void res_Search(char resid) {
	int16 lang;
	if (resourceBuf == 0)
		res_Init();

	lang = (language == 5) ? 2 : language;

	resourcePtr = resourceBuf;
	while (*resourcePtr != '#') {
		if (resourcePtr[0] == '@' && resourcePtr[1] == resid &&
		    ((int16)lang + '0') == (int16)resourcePtr[2]) {
			resourcePtr += 5;
			return;

		}
		resourcePtr++;
	}
	resourcePtr = resourceBuf;
	while (resourcePtr[0] != '#') {
		if (resourcePtr[0] == '@' && resourcePtr[1] == resid) {
			resourcePtr += 5;
			return;
		}
		resourcePtr++;
	}
	return;
}

void res_Get(char *buf) {
	int16 i = 0;
	while (1) {
		if (*resourcePtr == '\r')
			resourcePtr++;

		if (*resourcePtr == '\n') {
			resourcePtr++;
			break;
		}

		buf[i] = *resourcePtr;
		i++;
		resourcePtr++;
	}
	buf[i] = 0;
}

} // End of namespace Gob
