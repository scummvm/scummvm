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

#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"

#ifdef USE_BYONLINE
#include "scumm/he/byonline/byonline.h"
#include "scumm/he/byonline/directplay.h"
#endif

namespace Scumm {

/**
 * Logic code for:
 *     Backyard Baseball 2001
 */
class LogicHEbaseball2001 : public LogicHE {
public:
	LogicHEbaseball2001(ScummEngine_v90he *vm) : LogicHE(vm) {}

	int versionID() override;
	int startOfFrame() override;
	int32 dispatch(int op, int numArgs, int32 *args) override;
};

int LogicHEbaseball2001::versionID() {
	return 1;
}

int LogicHEbaseball2001::startOfFrame() {
#ifdef USE_BYONLINE
	_vm->_directPlay->startOfFrame();
	_vm->_byonline->startOfFrame();
#endif

	return 0;
}


int32 LogicHEbaseball2001::dispatch(int op, int numArgs, int32 *args) {
	int res = 0;

	switch (op) {
		case 1492: case 1493: case 1494: case 1495: case 1496:
		case 1497: case 1498: case 1499: case 1500: case 1501:
		case 1502: case 1503: case 1504: case 1505: case 1506:
		case 1507: case 1508: case 1509: case 1510: case 1511:
		case 1512: case 1513: case 1514: case 1555:
			// DirectPlay-related
#ifdef USE_BYONLINE
			res = _vm->_directPlay->dispatch(op, numArgs, args);
#endif
			break;

		case 2200: case 2201: case 2202: case 2203: case 2204:
		case 2205: case 2206: case 2207: case 2208: case 2209:
		case 2210: case 2211: case 2212: case 2213: case 2214:
		case 2215: case 2216: case 2217: case 2218: case 2219:
		case 2220: case 2221: case 2222: case 2223: case 2224:
		case 2225: case 2226: case 2227: case 2228: case 2238:
			// Boneyards-related
			// break;

		case 3000: case 3001: case 3002: case 3003: case 3004:
			// Internet-related
			// 3000: check for updates
			// 3001: check network status
			// 3002: autoupdate
			// 3003: close connection
#ifdef USE_BYONLINE
			res = _vm->_byonline->dispatch(op, numArgs, args);
#endif
			break;

	default:
		LogicHE::dispatch(op, numArgs, args);
	}

	return res;
}

LogicHE *makeLogicHEbaseball2001(ScummEngine_v90he *vm) {
	return new LogicHEbaseball2001(vm);
}

} // End of namespace Scumm
