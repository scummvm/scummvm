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

#include <typeinfo>
#include "common/events.h"

#include "hypno/grammar.h"
#include "hypno/hypno.h"

namespace Hypno {

//Actions

void HypnoEngine::runMenu(Hotspots hs) {
	const Hotspot h = *hs.begin();
	assert(h.type == MakeMenu);
	debugC(1, kHypnoDebugScene, "hotspot actions size: %d", h.actions.size());
	for (Actions::const_iterator itt = h.actions.begin(); itt != h.actions.end(); ++itt) {
		Action *action = *itt;
		if (typeid(*action) == typeid(Quit))
			runQuit((Quit *)action);
		else if (typeid(*action) == typeid(Background))
			runBackground((Background *)action);
		else if (typeid(*action) == typeid(Overlay))
			runOverlay((Overlay *)action);
		else if (typeid(*action) == typeid(Ambient))
			runAmbient((Ambient *)action);

		//else if (typeid(*action) == typeid(Mice))
		//	runMice(h, (Mice*) action);
	}

	//if (h.stype == "SINGLE_RUN")
	//	loadImage("int_main/mainbutt.smk", 0, 0);
	if (h.stype == "AUTO_BUTTONS" && _conversation.empty())
		loadImage("int_main/resume.smk", 0, 0, true);
}

void HypnoEngine::runBackground(Background *a) {
	if (a->condition.size() > 0 && !_sceneState[a->condition])
		return;
	loadImage(a->path, a->origin.x, a->origin.y, false);
}

void HypnoEngine::runOverlay(Overlay *a) {
	loadImage(a->path, a->origin.x, a->origin.y, false);
}

void HypnoEngine::runMice(Mice *a) {
	changeCursor(a->path, a->index);
}

void HypnoEngine::runEscape(Escape *a) {
	_nextHotsToRemove = stack.back();
}

void HypnoEngine::runCutscene(Cutscene *a) {
	stopSound();
	disableCursor();
	_music.clear();
	_nextSequentialVideoToPlay.push_back(MVideo(a->path, Common::Point(0, 0), false, true, false));
}

void HypnoEngine::runGlobal(Global *a) {
	if (a->command == "TURNON")
		_sceneState[a->variable] = 1;
	else if (a->command == "TURNOFF")
		_sceneState[a->variable] = 0;
	else
		error("Invalid command %s", a->command.c_str());
}

void HypnoEngine::runPlay(Play *a) {
	if (a->condition.size() > 0 && !_sceneState[a->condition])
		return;

	if (a->flag == "/BITMAP")
		loadImage(a->path, a->origin.x, a->origin.y, false);
	else {
		_nextSequentialVideoToPlay.push_back(MVideo(a->path, a->origin, false, false, false));
	}
}

void HypnoEngine::runAmbient(Ambient *a) {
	if (a->flag == "/BITMAP")
		loadImage(a->path, a->origin.x, a->origin.y, false);
	else {
		_nextSequentialVideoToPlay.push_back(MVideo(a->path, a->origin, false, a->fullscreen, a->flag == "/LOOP"));
	}
}

void HypnoEngine::runWalN(WalN *a) {
	if (a->condition.size() > 0 && !_sceneState[a->condition])
		return;
	if (a->flag == "/BITMAP")
		loadImage(a->path, a->origin.x, a->origin.y, false);
	else {
		_nextSequentialVideoToPlay.push_back(MVideo(a->path, a->origin, false, false, false));
	}
}

void HypnoEngine::runQuit(Quit *a) {
	quitGame();
}

void HypnoEngine::runChangeLevel(ChangeLevel *a) {
	_nextLevel = a->level;
}

void HypnoEngine::runTalk(Talk *a) {
	_conversation.push_back(a);
	_refreshConversation = true;
}

} // End of namespace Hypno

