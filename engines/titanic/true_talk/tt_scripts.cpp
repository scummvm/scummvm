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

namespace Titanic {

TTScripts::TTScripts(CTitleEngine *titleEngine) :
		_titleEngine(titleEngine), _field24(0), _field28(0) {
	// Load unnamed scripts
	for (int scriptNum = 100; scriptNum < 133; ++scriptNum)
		addScript(new TTUnnamedScript(scriptNum));

	// Load named scripts
	addScript(new DoorbotScript(104, "Doorbot", 0, "Fentible", 11, 1, -1, -1, -1, 0));
	addScript(new BellbotScript(101, "Bellbot", 0, "Krage", 8, 1));
	addScript(new LiftbotScript(105, "LiftBot", 0, "Nobby", 11, 1, -1, -1, -1, 0));
	addScript(new DeskbotScript(103, "DeskBot", 0, "Marsinta", 11, 2));
	addScript(new BarbotScript(100, "Barbot", 0, "Fortillian", 9, 1, -1, -1, -1, 0));
	addScript(new ParrotScript(107, "Parrot", 0, "The Parrot", 5, 1, -1, -1, -1, 0));
	addScript(new MaitreDScript(112, "MaitreDBot", 0, "Dastrogaaar", 8, 1));
	addScript(new SuccUBusScript(111, "Succubus", 0, "Shorbert", 9, 1, -1, -1, -1, 0));
}

void TTScripts::addScript(TTNamedScript *script) {
	script->proc13();
	_namedScripts.push_back(new TTNamedScriptListItem(script));
}

void TTScripts::addScript(TTUnnamedScript *script) {
	_unnamedScripts.push_back(new TTUnnamedScriptListItem(script));
}

} // End of namespace Titanic
