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

#include "glk/alan2/acode.h"
#include "glk/alan2/interpreter.h"
#include "glk/alan2/rules.h"
#include "glk/alan2/types.h"
#include "common/debug.h"

namespace Glk {
namespace Alan2 {

// TODO: Refactor these into debug flags
const bool trcflg = false;
const bool stpflg = false;

void Rules::parseRules() {
	bool change = true;
	int i;

	for (i = 1; !endOfTable(&_ruls[i - 1]); i++)
		_ruls[i - 1].run = false;

	while (change) {
		change = false;
		for (i = 1; !endOfTable(&_ruls[i - 1]); i++) {
			if (!_ruls[i - 1].run) {
				if (trcflg) {
					debug("\n<RULE %d (at ", i);	// TODO: Debug output formatting?
					//debugsay(cur.loc);	// TODO
					if (!stpflg)
						debug("), Evaluating");
					else
						debug("), Evaluating:>\n");
				}

				_interpreter->interpret(_ruls[i - 1].exp);

				if (_stack->pop()) {
					change = true;
					_ruls[i - 1].run = true;

					if (trcflg)
						if (!stpflg)
							debug(", Executing:>\n");
						else {
							debug("\nRULE %d (at ", i);
							//debugsay(cur.loc);	// TODO
							debug("), Executing:>\n");
						}

					_interpreter->interpret(_ruls[i - 1].stms);
					
				} else if (trcflg && !stpflg) {
					debug(":>\n");
				}
			}
		}
	}
}

bool Rules::endOfTable(RulElem *addr) {
	Aword *x = (Aword *)addr;
	return *x == EOF;
}

} // End of namespace Alan2
} // Engine of namespace GLK
