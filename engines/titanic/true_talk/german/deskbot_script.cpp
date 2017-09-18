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

#include "titanic/true_talk/german/deskbot_script.h"
#include "titanic/true_talk/true_talk_manager.h"

namespace Titanic {
namespace German {

int DeskbotScript::preResponse(uint id) {
	int newId = 0;
	if (getValue(1) >= 3 && (id == 41190 || id == 41429 || id == 41755 || id == 41757))
		newId = 241601;

	if (id == 42132)
		CTrueTalkManager::triggerAction(20, 0);

	return newId;
}

int DeskbotScript::doSentenceEntry(int val1, const int *srcIdP, const TTroomScript *roomScript, const TTsentence *sentence) {
	if (val1 != 4501)
		return TTnpcScript::doSentenceEntry(val1, srcIdP, roomScript, sentence);

	int v = getValue(1);
	int tagId = *srcIdP;

	switch (v) {
	case 1:
		tagId = 240336;
		break;

	case 2:
		tagId = addAssignedRoomDialogue();
		break;

	case 3:
		if (tagId == 240431 || tagId == 240432) {
			if (getValue(v) == 1) {
				if (tagId == 240431)
					tagId = 240432;
			} else {
				if (tagId == 240432)
					tagId = 240431;
			}
		}
		break;

	default:
		break;
	}

	addResponse(getDialogueId(tagId));
	applyResponse();
	return 2;
}

} // End of namespace German
} // End of namespace Titanic
