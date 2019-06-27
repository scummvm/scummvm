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

#include "glk/alan2/types.h"
#include "glk/alan2/debug.h"
#include "glk/alan2/exe.h"
#include "glk/alan2/inter.h"
#include "glk/alan2/glkio.h"
#include "glk/alan2/main.h"
#include "glk/alan2/rules.h"
#include "glk/alan2/stack.h"

namespace Glk {
namespace Alan2 {

void rules() {
	Boolean change = TRUE;
	int i;

	for (i = 1; !endOfTable(&ruls[i - 1]); i++)
		ruls[i - 1].run = FALSE;

	while (change) {
		change = FALSE;
		for (i = 1; !endOfTable(&ruls[i - 1]); i++)
			if (!ruls[i - 1].run) {
				if (trcflg) {
					printf("\n<RULE %d (at ", i);
					debugsay(cur.loc);
					if (!stpflg)
						printf("), Evaluating");
					else
						printf("), Evaluating:>\n");
				}
				interpret(ruls[i - 1].exp);
				if (pop()) {
					change = TRUE;
					ruls[i - 1].run = TRUE;
					if (trcflg) {
						if (!stpflg)
							printf(", Executing:>\n");
						else {
							printf("\nRULE %d (at ", i);
							debugsay(cur.loc);
							printf("), Executing:>\n");
						}
					}

					interpret(ruls[i - 1].stms);
				} else if (trcflg && !stpflg)
					printf(":>\n");
			}
	}
}

} // End of namespace Alan2
} // End of namespace Glk
