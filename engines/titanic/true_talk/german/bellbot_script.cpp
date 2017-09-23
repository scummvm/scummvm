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

#include "titanic/true_talk/german/bellbot_script.h"
#include "titanic/true_talk/true_talk_manager.h"

namespace Titanic {
namespace German {

int BellbotScript::doSentenceEntry(int val1, const int *srcIdP, const TTroomScript *roomScript, const TTsentence *sentence) {
	switch (val1) {
	case 5001:
	case 5021:
		return 0;

	case 5002:
		addResponse(getDialogueId(*srcIdP));
		addResponse(getDialogueId(200000));
		applyResponse();
		return 2;

	case 5003:
		addResponse(getDialogueId(*srcIdP));
		return 2;

	case 5022:
	case 5023:
		if (CTrueTalkManager::getStateValue(7)) {
			bool flag = true;

			if (!sentence->localWord("broken") && !sentence->contains("kaputt") &&
				!sentence->contains("im eimer") && !sentence->contains("funktioniert nicht") &&
				CTrueTalkManager::_currentNPC) {
				CNodeItem *node = CTrueTalkManager::_currentNPC->getNode();
				if (node) {
					CString nodeName = node->getName();
					if (nodeName.contains("5") || nodeName.contains("6") || nodeName.contains("7"))
						flag = false;
				}
			}

			if (flag) {
				CTrueTalkManager::triggerAction(29, 1);
				selectResponse(201771);
			}
			else {
				CTrueTalkManager::triggerAction(29, 2);
				selectResponse(201554);
			}
		}
		else {
			selectResponse(21378);
		}

		applyResponse();
		return 2;

	case 5024:
		if (CTrueTalkManager::getStateValue(7)) {
			CTrueTalkManager::triggerAction(29, 3);
			return 0;
		}

		selectResponse(21378);
		applyResponse();
		return 2;

	case 5025:
		if (CTrueTalkManager::getStateValue(7)) {
			CTrueTalkManager::triggerAction(29, 4);
			return 0;
		}

		selectResponse(21378);
		applyResponse();
		return 2;

	default:
		return TTnpcScript::doSentenceEntry(val1, srcIdP, roomScript, sentence);
	}
}

} // End of namespace German
} // End of namespace Titanic
