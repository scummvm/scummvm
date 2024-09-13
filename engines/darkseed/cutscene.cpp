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

#include "cutscene.h"
#include "darkseed.h"

void Darkseed::Cutscene::play(char cutsceneId) {
	_cutsceneId = cutsceneId;
	_movieStep = 1;
	g_engine->_sound->waitForSpeech();
	g_engine->fadeOut();
}


void Darkseed::Cutscene::update() {
	switch(_cutsceneId) {
	case 'B' : _movieStep = 9999; break;
	case 'C' : _movieStep = 9999; break;
	case 'D' : _movieStep = 9999; break;
	case 'E' : _movieStep = 9999; break;
	case 'G' : _movieStep = 9999; break;
	case 'H' : _movieStep = 9999; break;
	case 'I' : introScene(); break;
	case 'J' : _movieStep = 9999; break;
	case 'Y' : _movieStep = 9999; break;
	case 'Z' : _movieStep = 9999; break;
	}

	if (_movieStep == 9999) {
		if (_cutsceneId == 'E') {
			g_engine->_previousRoomNumber = 38;
			g_engine->changeToRoom(7);
		} else if (_cutsceneId == 'Z') {
			g_engine->restartGame();
		}
	}
}

bool Darkseed::Cutscene::introScene() {
	switch (_movieStep) {
	case 1: g_engine->fadeOut(); break;
	default: _movieStep = 9999; return false;
	}
	_movieStep++;
	return true;
}
