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

#include "glk/alan3/actor.h"
#include "glk/alan3/instance.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/lists.h"
#include "glk/alan3/inter.h"
#include "glk/alan3/msg.h"
#include "glk/alan3/container.h"

namespace Glk {
namespace Alan3 {

/*======================================================================*/
ScriptEntry *scriptOf(int actor) {
	ScriptEntry *scr;

	if (admin[actor].script != 0) {
		for (scr = (ScriptEntry *) pointerTo(header->scriptTableAddress); !isEndOfArray(scr); scr++)
			if (scr->code == admin[actor].script)
				break;
		if (!isEndOfArray(scr))
			return scr;
	}
	return NULL;
}


/*======================================================================*/
StepEntry *stepOf(int actor) {
	StepEntry *step;
	ScriptEntry *scr = scriptOf(actor);

	if (scr == NULL) return NULL;

	step = (StepEntry *)pointerTo(scr->steps);
	step = &step[admin[actor].step];

	return step;
}


/*======================================================================*/
void describeActor(CONTEXT, int actor) {
	ScriptEntry *script = scriptOf(actor);

	if (script != NULL && script->description != 0) {
		CALL1(interpret, script->description)
	} else if (hasDescription(actor)) {
		CALL1(describeAnything, actor)
	} else {
		printMessageWithInstanceParameter(M_SEE_START, actor);
		printMessage(M_SEE_END);

		if (instances[actor].container != 0)
			CALL1(describeContainer, actor)
	}
	admin[actor].alreadyDescribed = TRUE;
}

} // End of namespace Alan3
} // End of namespace Glk
