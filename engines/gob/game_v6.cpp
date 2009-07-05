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
 */

#include "common/endian.h"
#include "common/stream.h"
#include "common/file.h"

#include "gob/gob.h"
#include "gob/game.h"
#include "gob/helper.h"
#include "gob/global.h"
#include "gob/script.h"
#include "gob/resources.h"
#include "gob/hotspots.h"
#include "gob/inter.h"
#include "gob/draw.h"

namespace Gob {

Game_v6::Game_v6(GobEngine *vm) : Game_v2(vm) {
}

// flagbits: 5 = freeInterVariables, 6 = skipPlay
void Game_v6::totSub(int8 flags, const char *newTotFile) {
	int8 curBackupPos;

	if ((flags == 16) || (flags == 17))
		warning("Urban Stub: Game_v6::totSub(), flags == %d", flags);

	if (_numEnvironments >= Environments::kEnvironmentCount)
		return;

	_environments->set(_numEnvironments);

	curBackupPos = _curEnvironment;
	_numEnvironments++;
	_curEnvironment = _numEnvironments;

	_script = new Script(_vm);
	_resources = new Resources(_vm);

	if (flags & 0x80)
		warning("Urban Stub: Game_v6::totSub(), flags & 0x80");

	if (flags & 5)
		_vm->_inter->_variables = 0;

	strncpy0(_curTotFile, newTotFile, 9);
	strcat(_curTotFile, ".TOT");

	if (_vm->_inter->_terminate != 0) {
		clearUnusedEnvironment();
		return;
	}

	_hotspots->push(0, true);

	if (flags & 6)
		playTot(-1);
	else
		playTot(0);

	if (_vm->_inter->_terminate < 2)
		_vm->_inter->_terminate = 0;

	_hotspots->clear();
	_hotspots->pop();

	if ((flags & 5) && _vm->_inter->_variables) {
		_vm->_inter->delocateVars();
	}

	clearUnusedEnvironment();

	_numEnvironments--;
	_curEnvironment = curBackupPos;
	_environments->get(_numEnvironments);
}

void Game_v6::prepareStart(void) {
	_noCd = false;

	if (Common::File::exists("cd1.itk") && Common::File::exists("cd2.itk") &&
	    Common::File::exists("cd3.itk") && Common::File::exists("cd4.itk")) {
		_noCd = true;
	}

	Game::prepareStart();
}

} // End of namespace Gob
