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

#include "glk/alan3/checkentry.h"
#include "glk/alan3/inter.h"
#include "glk/alan3/lists.h"
#include "glk/alan3/memory.h"

namespace Glk {
namespace Alan3 {

bool checksFailed(CONTEXT, Aaddr adr, bool execute) {
	CheckEntry *chk = (CheckEntry *) pointerTo(adr);
	bool flag;

	if (chk->exp == 0) {
		if (execute == EXECUTE_CHECK_BODY_ON_FAIL)
			R0CALL1(interpret, chk->stms)
		return TRUE;
	} else {
		while (!isEndOfArray(chk)) {
			R0FUNC1(evaluate, flag, chk->exp)
			if (!flag) {
				if (execute == EXECUTE_CHECK_BODY_ON_FAIL)
					R0CALL1(interpret, chk->stms)
				return TRUE;
			}
			chk++;
		}
		return FALSE;
	}
}

} // End of namespace Alan3
} // End of namespace Glk
