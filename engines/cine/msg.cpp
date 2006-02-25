/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/scummsys.h"

#include "cine/msg.h"
#include "cine/flip_support.h"
#include "cine/various.h"

namespace Cine {

uint16 msgVar0;

void loadMsg(char *pMsgName) {
	uint16 i;
	uint8 *ptr;

	checkDataDisk(-1);

	msgVar0 = 0;

	for (i = 0; i < NUM_MAX_MESSAGE; i++) {
		messageTable[i].len = 0;

		if (messageTable[i].ptr) {
			ASSERT_PTR(messageTable[i].ptr);

			free(messageTable[i].ptr);
		}

		messageTable[i].ptr = NULL;
	}

	ptr = readBundleFile(findFileInBundle(pMsgName));

	processPendingUpdates(1);

	msgVar0 = *(uint16 *) ptr;
	ptr += 2;
	flipU16(&msgVar0);

	ASSERT(msgVar0 <= NUM_MAX_MESSAGE);

	for (i = 0; i < msgVar0; i++) {
		messageTable[i].len = *(uint16 *) ptr;
		ptr += 2;
		flipU16(&messageTable[i].len);
	}

	for (i = 0; i < msgVar0; i++) {
		if (messageTable[i].len) {
			messageTable[i].ptr = (uint8 *) malloc(messageTable[i].len);

			ASSERT_PTR(messageTable[i].ptr);

			memcpy(messageTable[i].ptr, ptr, messageTable[i].len);
			ptr += messageTable[i].len;
		}
	}
}

} // End of namespace Cine
