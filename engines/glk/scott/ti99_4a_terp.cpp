#include "ti99_4a_terp.h"
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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "glk/scott/scott.h"
#include "glk/scott/types.h"
#include "glk/scott/globals.h"
#include "glk/scott/ti99_4a_terp.h"

namespace Glk {
namespace Scott {

ActionResultType performTI99Line(uint8_t *actionLine) {
	return ACT_SUCCESS;
}

void runImplicitTI99Actions() {
	int probability;
	uint8_t *ptr;
	int loopFlag;

	ptr = _G(_ti99ImplicitActions);
	loopFlag = 0;

	/* bail if no auto acts in the game. */
	if (*ptr == 0x0)
		loopFlag = 1;

	while (loopFlag == 0) {
		/*
		 p + 0 = chance of happening
		 p + 1 = size of code chunk
		 p + 2 = start of code
		 */

		probability = ptr[0];

		if (g_scott->randomPercent(probability))
			performTI99Line(ptr + 2);

		if (ptr[1] == 0 || ptr - _G(_ti99ImplicitActions) >= _G(_ti99ImplicitExtent))
			loopFlag = 1;

		/* skip code chunk */
		ptr += 1 + ptr[1];
	}
}

ExplicitResultType runExplicitTI99Actions(int verbNum, int nounNum) {
	return ER_NO_RESULT;
}

} // End of namespace Scott
} // End of namespace Glk
