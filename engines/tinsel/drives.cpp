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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * CD/drive handling functions
 */

#include "tinsel/drives.h"
#include "tinsel/scene.h"
#include "tinsel/tinsel.h"
#include "tinsel/sched.h"
#include "tinsel/strres.h"

namespace Tinsel {

static char currentCD = '1';
static uint32 cdFlags[] = { fCd1, fCd2, fCd3, fCd4, fCd5, fCd6, fCd7, fCd8 };

static bool bChangingCD = false;
static char nextCD = '\0';


void CdCD(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	while (bChangingCD) {
		if (g_scheduler->getCurrentProcess()) {
			// FIXME: CdCD gets passed a nullContext in RegisterGlobals() and
			//        PrimeSceneHopper(), because I didn't know how to get a proper
			//        context without converting the whole calling stack to CORO'd
			//        functions. If these functions really get called while a CD
			//        change is requested, this needs to be resolved.
			if (coroParam == nullContext)
				error("CdCD needs context!");
			CORO_SLEEP(1);
		} else
			error("No current process in CdCD()!");
	}

	CORO_END_CODE;
}

int GetCurrentCD(void) {
	// count from 1
	return (currentCD - '1' + 1);
}

void SetCD(int flags) {
	if (flags & cdFlags[currentCD - '1'])
		return;

	error("SetCD() problem");
}

int GetCD(int flags) {
	int i;
	char cd = '\0';

	if (flags & cdFlags[currentCD - '1'])
		return GetCurrentCD();

	for (i = 0; i < 8; i++) {
		if (flags & cdFlags[i]) {
			cd = (char)(i + '1');
			break;
		}
	}
	assert(i != 8);

	nextCD = cd;
	return cd;
}

void DoCdChange(void) {
	if (bChangingCD) {
		_vm->_sound->closeSampleStream();
		_vm->_sound->openSampleFiles();
		ChangeLanguage(TextLanguage());
		bChangingCD = false;
	}
}

void SetNextCD(int cdNumber) {
	assert(cdNumber == 1 || cdNumber == 2);

	nextCD = (char)(cdNumber + '1' - 1);
}

bool GotoCD(void) {
	// WORKAROUND: Somehow, CdDoChange() is called twice... Hopefully, this guard helps
	if (currentCD == nextCD)
		return false;

	currentCD = nextCD;

/*	if (bNoCD) 	{
		strcpy(cdDirectory, hdDirectory);
		cdLastBit[3] = currentCD;
		strcat(cdDirectory, cdLastBit);
	}
*/
	bChangingCD = true;

	return true;
}

} // end of namespace Tinsel
