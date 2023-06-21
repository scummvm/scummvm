/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/wscript/ws_cruncher.h"
#include "m4/wscript/ws_hal.h"
#include "m4/core/errors.h"
#include "m4/mem/mem.h"
#include "m4/globals.h"

namespace M4 {

#define VERIFY_INTIALIZED(s) if (!_G(cruncherInitialized)) error_show(FL, 'WSCI', "%s failed.", s);

static int32 dataFormats[] = { 0, 5, 8, 12, 16 };

int32 *ws_GetDataFormats() {
	return &dataFormats[0];
}

bool ws_InitCruncher(void) {
	//	int32		i;

		//make sure the cruncher has not been initialized
	if (_G(cruncherInitialized))
		error_show(FL, 'WSCR');

	// Register the end of sequence struct with the stash manager
	mem_register_stash_type(&_G(memtypeEOS), sizeof(EOSreq), 32, "+EOS");
	if (_G(memtypeEOS) < 0)
		error_show(FL, 'WSCE');

	if ((_G(myCruncher) = (cruncher *)mem_alloc(sizeof(cruncher), "cruncher")) == NULL)
		error_show(FL, 'OOM!', "%ld bytes.", sizeof(cruncher));

	_G(myCruncher)->backLayerAnim8 = NULL;
	_G(myCruncher)->frontLayerAnim8 = NULL;
	_G(myCruncher)->firstAnim8ToCrunch = NULL;
	_G(myCruncher)->lastAnim8ToCrunch = NULL;

	// Set up stack
	_G(stackSize) = 2048;
	if ((_G(stackBase) = (uint32 *)mem_alloc(_G(stackSize), "crunchstack")) == NULL) {
		error_show(FL, 'OOM!', "%ld bytes.", _G(stackSize));
	}
	_G(stackTop) = _G(stackBase);
	_G(stackLimit) = (uint32 *)((uint32)_G(stackBase) + (uint32)_G(stackSize));

	_G(cruncherInitialized) = true;

	return true;
}

void ws_KillCruncher(void) {
	Anim8 *myAnim8;

	//make sure the cruncher has been initialized
	VERIFY_INTIALIZED("ws_KillCruncher()");

	myAnim8 = _G(myCruncher)->firstAnim8ToCrunch;
	while (myAnim8) {
		_G(myCruncher)->firstAnim8ToCrunch = myAnim8->next;
		if (myAnim8->myCCB) {
			KillCCB(myAnim8->myCCB, false);
		}
		mem_free((void *)myAnim8->myRegs);
		myAnim8 = _G(myCruncher)->firstAnim8ToCrunch;
	}
	mem_free((void *)_G(myCruncher));
	if (_G(stackBase)) {
		mem_free(_G(stackBase));
	}

	_G(cruncherInitialized) = false;
}

} // End of namespace M4
