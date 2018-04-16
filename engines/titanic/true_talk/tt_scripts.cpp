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

#include "titanic/true_talk/tt_scripts.h"
#include "titanic/true_talk/title_engine.h"
#include "titanic/true_talk/barbot_script.h"
#include "titanic/true_talk/bellbot_script.h"
#include "titanic/true_talk/deskbot_script.h"
#include "titanic/true_talk/doorbot_script.h"
#include "titanic/true_talk/liftbot_script.h"
#include "titanic/true_talk/maitred_script.h"
#include "titanic/true_talk/parrot_script.h"
#include "titanic/true_talk/succubus_script.h"
#include "titanic/translation.h"

namespace Titanic {

TTnpcScript *TTnpcScriptList::findById(int charId) const {
	for (TTnpcScriptList::const_iterator i = begin(); i != end(); ++i) {
		const TTnpcScriptListItem *item = *i;
		if (item->_npcScript->_charId == charId)
			return item->_npcScript;
	}

	return nullptr;
}

/*------------------------------------------------------------------------*/

TTroomScript *TTroomScriptList::findById(uint scriptId) const {
	for (TTroomScriptList::const_iterator i = begin(); i != end(); ++i) {
		const TTroomScriptListItem *item = *i;
		if (item->_item->_scriptId == scriptId)
			return item->_item;
	}

	return nullptr;
}

/*------------------------------------------------------------------------*/

TTscripts::TTscripts() {
	// Load room scripts
	for (int scriptNum = 100; scriptNum < 133; ++scriptNum)
		addScript(new TTroomScript(scriptNum));

	// Load npc scripts
	addScript(new BarbotScript(100, "Barbot", 0, "Fortillian", 9, 1, -1, -1, -1, 0), 112);
	addScript(new BellbotScript(101, "Bellbot", 0, "Krage", 8, 1), 110);
	addScript(new DeskbotScript(103, "DeskBot", 0, "Marsinta", 11, 2), 110);
	addScript(new DoorbotScript(104, "Doorbot", 0, "Fentible", 11, 1, -1, -1, -1, 0), 100);
	addScript(new LiftbotScript(105, "LiftBot", 0, "Nobby", 11, 1, -1, -1, -1, 0), 103);
	addScript(new ParrotScript(107, "Parrot", 0, "The Parrot", 5, 1, -1, -1, -1, 0), 111);
	addScript(new SuccUBusScript(111, "Succubus", 0, "Shorbert", 9, 1, -1, -1, -1, 0), 110);
	addScript(new MaitreDScript(112, "MaitreDBot", 0, "Dastrogaaar", 8, 1), 132);
}

void TTscripts::addScript(TTnpcScript *script, int scriptId) {
	// Find the room script this is associated with
	TTroomScript *roomScript = getRoomScript(scriptId);
	assert(roomScript);

	_npcScripts.push_back(new TTnpcScriptListItem(script, roomScript));
}

void TTscripts::addScript(TTroomScript *script) {
	_roomScripts.push_back(new TTroomScriptListItem(script));
}

TTroomScript *TTscripts::getRoomScript(int scriptId) const {
	return _roomScripts.findById(scriptId);
}

TTnpcScript *TTscripts::getNpcScript(int charId) const {
	return _npcScripts.findById(charId);
}

} // End of namespace Titanic
