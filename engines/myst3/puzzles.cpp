/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/myst3/puzzles.h"
#include "engines/myst3/ambient.h"
#include "engines/myst3/menu.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/node.h"
#include "engines/myst3/state.h"
#include "engines/myst3/sound.h"

#include "common/config-manager.h"

namespace Myst3 {

Puzzles::Puzzles(Myst3Engine *vm) :
		_vm(vm) {
}

Puzzles::~Puzzles() {
}

void Puzzles::run(uint16 id, uint16 arg0, uint16 arg1, uint16 arg2) {
	switch (id) {
	case 1:
		leversBall(arg0);
		break;
	case 2:
		tesla(arg0, arg1, arg2);
		break;
	case 3:
		resonanceRingControl();
		break;
	case 4:
		resonanceRingsLaunchBall();
		break;
	case 5:
		resonanceRingsLights();
		break;
	case 6:
		pinball(arg0);
		break;
	case 7:
		weightDrag(arg0, arg1);
		break;
	case 8:
		journalSaavedro(arg0);
		break;
	case 9:
		journalAtrus(arg0, arg1);
		break;
	case 10:
		symbolCodesInit(arg0, arg1, arg2);
		break;
	case 11:
		symbolCodesClick(arg0);
		break;
	case 12:
		railRoadSwitchs();
		break;
	case 13:
		rollercoaster();
		break;
	case 14:
		projectorLoadBitmap(arg0);
		break;
	case 15:
		projectorAddSpotItem(arg0, arg1, arg2);
		break;
	case 16:
		projectorUpdateCoordinates();
		break;
	case 17:
		_vm->settingsLoadToVars();
		break;
	case 18:
		_vm->settingsApplyFromVars();
		break;
	case 19:
		settingsSave();
		break;
	case 20:
		_vm->_menu->saveLoadAction(arg0, arg1);
		break;
	case 21:
		mainMenu(arg0);
		break;
	case 22:
		updateSoundScriptTimer();
		break;
	case 23:
		_vm->loadNodeSubtitles(arg0);
		break;
	case 25:
		checkCanSave(); // Xbox specific
		break;
	default:
		warning("Puzzle %d is not implemented", id);
	}
}

void Puzzles::_drawForVarHelper(int16 var, int32 startValue, int32 endValue) {
	uint startTick = _vm->_state->getTickCount();
	uint currentTick = startTick;
	uint numValues = abs(endValue - startValue);
	uint endTick = startTick + 2 * numValues;

	int16 var2 = var;

	if (var < 0)
		var = -var;
	if (var2 < 0)
		var2 = -var2 + 1;

	if (startTick < endTick) {
		int currentValue = -9999;
		while (1) {
			int nextValue = (currentTick - startTick) / 2;
			if (currentValue != nextValue) {
				currentValue = nextValue;

				int16 varValue;
				if (endValue > startValue)
					varValue = startValue + currentValue;
				else
					varValue = startValue - currentValue;

				_vm->_state->setVar(var, varValue);
				_vm->_state->setVar(var2, varValue);
			}

			_vm->processInput(false);
			_vm->drawFrame();
			currentTick = _vm->_state->getTickCount();

			if (currentTick > endTick || _vm->shouldQuit())
				break;
		}
	}

	_vm->_state->setVar(var, endValue);
	_vm->_state->setVar(var2, endValue);
}

void Puzzles::_drawXTicks(uint16 ticks) {
	uint32 endTick = _vm->_state->getTickCount() + ticks;

	while (_vm->_state->getTickCount() < endTick && !_vm->shouldQuit()) {
		_vm->processInput(false);
		_vm->drawFrame();
	}
}

void Puzzles::leversBall(int16 var) {
	struct NewPosition {
		bool newLeft;
		bool newRight;
		uint16 newBallPosition;
		uint16 movieStart;
		uint16 movieEnd;
		uint16 movieBallStart;
		uint16 movieBallEnd;
	};

	struct Move {
		int16 oldLeft;
		int16 oldRight;
		uint16 oldBallPosition;
		NewPosition p[2];
	};

	static const Move moves[] =	{
		{   0,   1,   2, { { 1,   1,   2, 127, 147,   0,   0 }, { 0,   0,   0, 703, 735,   0,   0 } } },
		{   0,   0,   4, { { 1,   0,   4,  43,  63,   0,   0 }, { 0,   1,   4,  64,  84,   0,   0 } } },
		{   0,   0,   1, { { 1,   0,   1,  85, 105,   0,   0 }, { 0,   1,   1,  22,  42,   0,   0 } } },
		{   1,   0,   4, { { 1,   1,   3, 514, 534, 169, 217 }, { 0,   0,   4, 577, 597,   0,   0 } } },
		{   1,   0,   3, { { 1,   1,   3, 493, 513,   0,   0 }, { 0,   0,   4, 451, 471, 410, 450 } } },
		{   1,   0,   1, { { 1,   1,   2, 472, 492, 312, 360 }, { 0,   0,   1, 598, 618,   0,   0 } } },
		{   0,   1,   4, { { 1,   1,   3, 148, 168, 169, 217 }, { 0,   0,   4, 619, 639,   0,   0 } } },
		{   0,   1,   2, { { 1,   1,   2, 127, 147,   0,   0 }, { 0,   0,   1,   1,  21, 271, 311 } } },
		{   0,   1,   1, { { 1,   1,   2, 106, 126, 312, 360 }, { 0,   0,   1, 640, 660,   0,   0 } } },
		{   1,   1,   3, { { 1,   0,   3, 661, 681,   0,   0 }, { 0,   1,   2, 535, 555, 218, 270 } } },
		{   1,   1,   2, { { 1,   0,   3, 556, 575, 361, 409 }, { 0,   1,   2, 682, 702,   0,   0 } } },
		{   0,   0,   0, { { 1,   0,   0, 757, 777,   0,   0 }, { 0,   1,   0, 736, 756,   0,   0 } } },
		{   1,   0,   0, { { 1,   1,   0, 799, 819,   0,   0 }, { 0,   0,   0, 841, 861,   0,   0 } } },
		{   0,   1,   0, { { 1,   1,   0, 778, 798,   0,   0 }, { 0,   0,   0, 820, 840,   0,   0 } } },
		{   1,   1,   0, { { 1,   0,   0, 883, 903,   0,   0 }, { 0,   1,   0, 862, 882,   0,   0 } } },
		{  -1,   0,   0, { { 0,   0,   0,   0,   0,   0,   0 }, { 0,   0,   0,   0,   0,   0,   0 } } }
	};

	uint16 oldPosition = _vm->_state->getBallPosition();
	uint16 oldLeverLeft = _vm->_state->getBallLeverLeft();
	uint16 oldLeverRight = _vm->_state->getBallLeverRight();

	// Toggle lever position
	_vm->_state->setVar(var, !_vm->_state->getVar(var));

	uint16 newLeverLeft = _vm->_state->getBallLeverLeft();
	uint16 newLeverRight = _vm->_state->getBallLeverRight();

	const Move *move = nullptr;
	for (uint i = _vm->_state->getBallDoorOpen() ? 0 : 1; i < ARRAYSIZE(moves); i++)
		if (moves[i].oldBallPosition == oldPosition
				&& moves[i].oldLeft == oldLeverLeft
				&& moves[i].oldRight == oldLeverRight) {
			move = &moves[i];
			break;
		}

	if (!move)
		error("Unable to find move with old levers l:%d r:%d p:%d", oldLeverLeft, oldLeverRight, oldPosition);

	const NewPosition *position = nullptr;
	for (uint i = 0; i < ARRAYSIZE(move->p); i++)
		if (move->p[i].newLeft == newLeverLeft
				&& move->p[i].newRight == newLeverRight) {
			position = &move->p[i];
			break;
		}

	if (!position)
		error("Unable to find position with levers l:%d r:%d", newLeverLeft, newLeverRight);

	_vm->_sound->playEffect(789, 50);
	_drawForVarHelper(35, position->movieStart, position->movieEnd);

	if (position->newBallPosition != oldPosition) {
		uint16 sound;
		if (position->newBallPosition == 0) {
			sound = 792;
		} else if (position->newBallPosition == 1 || position->newBallPosition == 4) {
			sound = 790;
		} else {
			sound = 791;
		}

		_vm->_sound->playEffect(sound, 50);

		if (position->movieBallStart != 0) {
			_drawForVarHelper(35, position->movieBallStart, position->movieBallEnd);
		}
	}

	_vm->_state->setBallPosition(position->newBallPosition);
	_vm->_state->setBallFrame(_vm->_state->getVar(35));
}

void Puzzles::tesla(int16 movie, int16 var, int16 move) {
	uint16 node = _vm->_state->getLocationNode();

	int16 movieStart = 0;
	switch (node) {
	case 114:
		movieStart = 0;
		break;
	case 116:
		movieStart = 320;
		break;
	case 118:
		movieStart = 240;
		break;
	case 120:
		movieStart = 160;
		break;
	case 122:
		movieStart = 80;
		break;
	}

	_vm->_state->setTeslaMovieStart(movieStart);

	uint16 position = movieStart + _vm->_state->getVar(var);

	if (position > 400)
		position -= 400;

	_vm->_state->setVar(32, node % 100);
	_vm->_state->setVar(33, node % 100 + 10000);

	if (movie) {
		_vm->_sound->playEffect(1243, 100);
		_vm->_state->setMovieSynchronized(true);
		_vm->playSimpleMovie(movie);
	}

	if (move) {
		uint16 sound = _vm->_rnd->getRandomNumberRng(1244, 1245);
		_vm->_sound->playEffect(sound, 100);
	}

	if (move > 0) {
		_drawForVarHelper(var - 303, position + 1, position + 19);
		position += 20;
	} else if (move < 0) {
		if (position == 1)
			position = 401;

		_drawForVarHelper(var - 303, position - 1, position - 19);
		position -= 20;
	}

	if (position < 1)
		position = 381;
	else if (position > 400)
		position = 1;

	_vm->_state->setVar(var - 303, position);

	int16 absPosition = position - movieStart;

	if (absPosition < 1)
		absPosition += 400;

	_vm->_state->setVar(var, absPosition);

	bool puzzleSolved = _vm->_state->getTeslaTopAligned() == 1
			&& _vm->_state->getTeslaMiddleAligned() == 1
			&& _vm->_state->getTeslaBottomAligned() == 1;

	_vm->_state->setTeslaAllAligned(puzzleSolved);
}

void Puzzles::resonanceRingControl() {
	static const uint16 frames[] = { 0, 24, 1, 5, 10, 15, 0, 0, 0 };

	uint16 startPos = _vm->_state->getVar(29);
	uint16 destPos = _vm->_state->getVar(27);

	int16 startFrame = frames[startPos] - 27;
	int16 destFrame = frames[destPos];

	// Choose the shortest direction
	for (int16 i = destFrame - startFrame; abs(i) > 14; i -= 27)
		startFrame += 27;

	// Play the movie, taking care of the limit case
	if (destFrame >= startFrame) {
		if (startFrame < 1) {
			_drawForVarHelper(28, startFrame + 27, 27);
			_drawForVarHelper(28, 1, destFrame);
			return;
		}
	} else {
		if (startFrame > 27) {
			_drawForVarHelper(28, startFrame - 27, 1);
			_drawForVarHelper(28, 27, destFrame);
			return;
		}
	}
	if (startFrame)
		_drawForVarHelper(28, startFrame, destFrame);
}

void Puzzles::resonanceRingsLaunchBall() {
	struct TrackFrames {
		uint16 ringFrame;
		uint16 var;
		uint16 num;
		uint16 shatterStartFrame;
		uint16 shatterEndFrame;
	};

	static const TrackFrames tracks[] = {
		{ 38, 436, 1, 182, 190 },
		{ 74, 434, 2, 194, 214 },
		{ 104, 437, 3, 215, 224 },
		{ 138, 435, 4, 225, 234 },
		{ 166, 438, 5, 235, 244 },
		{ 0, 0, 0, 0, 0 }
	};

	struct LightFrames {
		uint16 startFrame;
		uint16 endFrame;
		uint16 num;
	};

	static const LightFrames lights[] = {
		{ 26, 44, 1 },
		{ 66, 85, 2 },
		{ 89, 118, 3 },
		{ 126, 150, 4 },
		{ 154, 180, 5 }
	};

	bool ballShattered = false;
	bool lastIsOnLightButton = false;
	int32 lightStatus = 0;
	uint part = 0;
	uint16 buttonVar = 0;
	int32 ballMoviePlaying;
	int32 boardMoviePlaying;

	do {
		_vm->processInput(false);
		_vm->drawFrame();

		ballMoviePlaying = _vm->_state->getVar(27);
		boardMoviePlaying = _vm->_state->getVar(34);

		if (ballMoviePlaying && tracks[part].ringFrame) {
			int32 currentFrame = _vm->_state->getVar(30);

			if (!ballShattered && currentFrame >= tracks[part].ringFrame) {
				int32 value = _vm->_state->getVar(tracks[part].var);

				if (value == tracks[part].num) {
					// Correct ring order, go to next track part
					part++;
				} else {
					// Incorrect ring order, shatter ball
					ballShattered = true;
					_vm->_sound->playEffect(1010, 50);

					_vm->_state->setVar(28, tracks[part].shatterStartFrame);
					_vm->_state->setVar(29, tracks[part].shatterEndFrame);
					_vm->_state->setVar(31, tracks[part].shatterStartFrame);
				}
			}
		}

		bool isOnLightButton = false;

		const LightFrames *frames = 0;
		int32 currentLightFrame = _vm->_state->getVar(33);

		// Look is the mini ball is on a light button
		for (uint j = 0; j < ARRAYSIZE(lights); j++)
			if (currentLightFrame >= lights[j].startFrame && currentLightFrame <= lights[j].endFrame) {
				frames = &lights[j];
				break;
			}

		// If ball on light button, turn it off
		if (frames) {
			for (uint j = 0; j < 5; j++) {
				int32 ringValue = _vm->_state->getVar(434 + j);
				if (ringValue == frames->num)
					_vm->_state->setVar(38 + j, true);
			}

			isOnLightButton = true;
			buttonVar = 438 + frames->num;
		}

		// Restore previous light value
		if (lastIsOnLightButton != isOnLightButton) {
			lastIsOnLightButton = isOnLightButton;
			if (isOnLightButton) {
				lightStatus = _vm->_state->getVar(buttonVar);
				_vm->_state->setVar(buttonVar, 0);
			} else {
				_vm->_state->setVar(buttonVar, lightStatus);

				for (uint j = 0; j < 5; j++)
					_vm->_state->setVar(38 + j, false);
			}

			_vm->_ambient->playCurrentNode(100, 2);
		}
	} while ((ballMoviePlaying || boardMoviePlaying) && !_vm->shouldQuit());

	_vm->_state->setResonanceRingsSolved(!ballShattered);
}

void Puzzles::resonanceRingsLights() {
	// Turn off all lights
	for (uint i = 0; i < 5; i++)
		_vm->_state->setVar(439 + i, false);

	// For each button / ring value
	for (uint i = 0; i < 5; i++) {
		// For each light
		for (uint j = 0; j < 5; j++) {
			// Ring selector value
			uint32 ringValue = _vm->_state->getVar(434 + j);
			if (ringValue == i + 1) {
				// Button state
				uint32 buttonState = _vm->_state->getVar(43 + i);
				if (buttonState) {
					uint32 oldValue = _vm->_state->getVar(444 + i);
					_vm->_state->setVar(439 + i, oldValue);
					_vm->_state->setVar(38 + j, true);
				} else {
					_vm->_state->setVar(38 + j, false);
				}
			}
		}
	}

	_vm->_ambient->playCurrentNode(100, 2);
}

void Puzzles::pinball(int16 var) {
	static const byte remainingPegsFrames[] = { 2, 15, 25, 32 };

	static const PegCombination leftPegs[] = {
		{ 10101, { 0, 1, 0, 0, 0 }, {   0,   0,   0 }, 300 },
		{ 10102, { 1, 1, 0, 0, 0 }, {  49,   0,   0 }, 310 },
		{ 10103, { 0, 1, 0, 1, 0 }, { 200,   0,   0 }, 310 },
		{ 10104, { 0, 1, 1, 0, 0 }, { 150,   0,   0 }, 305 },
		{ 10105, { 0, 1, 0, 0, 1 }, { 250,   0,   0 }, 305 },
		{ 10106, { 1, 1, 0, 1, 0 }, {  49, 205,   0 }, 310 },
		{ 10107, { 1, 1, 1, 0, 0 }, {  49, 155,   0 }, 309 },
		{ 10108, { 1, 1, 0, 0, 1 }, {  49, 253,   0 }, 310 },
		{ 10109, { 0, 1, 1, 1, 0 }, { 150, 205,   0 }, 310 },
		{ 10110, { 0, 1, 0, 1, 1 }, { 199, 254,   0 }, 309 },
		{ 10111, { 0, 1, 1, 0, 1 }, { 150, 254,   0 }, 309 },
		{ 10112, { 1, 1, 1, 1, 0 }, {  49, 155, 210 }, 315 },
		{ 10113, { 1, 1, 0, 1, 1 }, {  49, 205, 260 }, 315 },
		{ 10114, { 1, 1, 1, 0, 1 }, {  49, 155, 260 }, 315 },
		{ 10115, { 0, 1, 1, 1, 1 }, { 150, 205, 260 }, 315 }
	};

	static const PegCombination rightPegs[] = {
		{ 10201, { 0, 0, 0, 0, 0 }, {   0,   0,   0 }, 300 },
		{ 10202, { 1, 0, 0, 0, 0 }, { 250,   0,   0 }, 305 },
		{ 10203, { 0, 1, 0, 0, 0 }, { 200,   0,   0 }, 305 },
		{ 10204, { 0, 0, 1, 0, 0 }, { 150,   0,   0 }, 305 },
		{ 10205, { 0, 0, 0, 1, 0 }, { 100,   0,   0 }, 305 },
		{ 10206, { 0, 0, 0, 0, 1 }, {  50,   0,   0 }, 305 },
		{ 10207, { 1, 1, 0, 0, 0 }, { 200, 255,   0 }, 305 },
		{ 10208, { 1, 0, 1, 0, 0 }, { 150, 255,   0 }, 310 },
		{ 10209, { 1, 0, 0, 1, 0 }, { 100, 255,   0 }, 310 },
		{ 10210, { 1, 0, 0, 0, 1 }, {  50, 255,   0 }, 310 },
		{ 10211, { 0, 1, 1, 0, 0 }, { 150, 205,   0 }, 310 },
		{ 10212, { 0, 1, 0, 1, 0 }, { 100, 205,   0 }, 310 },
		{ 10213, { 0, 1, 0, 0, 1 }, {  50, 205,   0 }, 310 },
		{ 10214, { 0, 0, 1, 1, 0 }, { 100, 155,   0 }, 310 },
		{ 10215, { 0, 0, 1, 0, 1 }, {  50, 155,   0 }, 210 },
		{ 10216, { 0, 0, 0, 1, 1 }, {  50, 105,   0 }, 310 },
		{ 10217, { 1, 1, 1, 0, 0 }, { 150, 205, 260 }, 315 },
		{ 10218, { 1, 1, 0, 1, 0 }, { 100, 205, 260 }, 315 },
		{ 10219, { 1, 1, 0, 0, 1 }, {  50, 205, 260 }, 312 },
		{ 10220, { 1, 0, 1, 1, 0 }, { 100, 155, 260 }, 314 },
		{ 10221, { 1, 0, 1, 0, 1 }, {  50, 155, 259 }, 315 },
		{ 10222, { 1, 0, 0, 1, 1 }, {  50, 105, 260 }, 315 },
		{ 10223, { 0, 1, 1, 1, 0 }, { 100, 155, 210 }, 315 },
		{ 10224, { 0, 1, 1, 0, 1 }, {  50, 155, 210 }, 315 },
		{ 10225, { 0, 1, 0, 1, 1 }, {  50, 105, 210 }, 315 },
		{ 10226, { 0, 0, 1, 1, 1 }, {  50, 105, 155 }, 315 }
	};

	struct BallJump {
		int16 positionLeft;
		int16 positionRight;
		int16 filter;
		int16 startFrame;
		int16 endFrame;
		int16 sound;
		int16 targetLeftFrame;
		int16 tragetRightFrame;
		int16 type;
	};

	static const BallJump jumps[] = {
		{   0, 450,  1,   16,   28, 1021, 250, 550, 0 },
		{   0, 450, -1,   29,   41, 1021, 500, 550, 3 },
		{   0, 200,  0,   42,   57, 1023, 300, 500, 0 },
		{   0, 200, -1,   58,   74, 1023, 550, 500, 3 },
		{   0, 250,  1,   75,   90, 1023, 350, 550, 0 },
		{   0, 250, -1,   91,  106, 1023, 500, 550, 3 },
		{   0, 300,  0,  107,  119, 1021, 400, 500, 0 },
		{   0, 300, -1,  120,  132, 1021, 550, 500, 3 },
		{   0, 400,  0,  133,  165, 1022, 500, 500, 2 },
		{   0, 400,  1, 1039, 1071, 1022, 550, 500, 2 },
		{   0, 350,  0,  166,  198, 1022, 500, 550, 2 },
		{   0, 350,  1, 1072, 1109, 1022, 550, 550, 2 },
		{ 250,   0,  1,  801,  815, 1021, 550, 450, 0 },
		{ 250,   0, -1,  816,  827, 1021, 550, 500, 4 },
		{ 300,   0,  0,  828,  845, 1023, 500, 200, 0 },
		{ 300,   0, -1,  846,  858, 1023, 500, 550, 3 },
		{ 350,   0,  1,  859,  876, 1023, 550, 250, 0 },
		{ 350,   0, -1,    0,    0,    0, 550, 500, 1 },
		{ 400,   0,  0,  893,  907, 1021, 500, 300, 0 },
		{ 400,   0,  1, 1267, 1278, 1023, 500, 550, 3 },
		{ 200,   0,  1,  908,  940, 1022, 500, 550, 2 },
		{ 200,   0,  0,  974, 1006, 1022, 500, 500, 2 },
		{ 450,   0,  1,  941,  973, 1022, 550, 550, 2 },
		{ 450,   0,  0, 1007, 1038, 1022, 550, 500, 2 }
	};

	struct BallExpireFrames {
		uint16 leftPosition;
		uint16 rightPosition;
		uint16 startFrame;
		uint16 endFrame;
	};

	static const BallExpireFrames ballExpireFrames[] = {
		{ 200, 200, 1105, 1131 },
		{ 250, 250, 1132, 1158 },
		{ 300, 300, 1159, 1185 },
		{ 350, 350, 1186, 1212 },
		{ 400, 400, 1213, 1239 },
		{ 450, 450, 1240, 1266 }
	};

	// Toggle peg state
	if (var > 0) {
		int32 value = _vm->_state->getVar(var);
		if (value) {
			_vm->_state->setVar(var, 0);
		} else {
			_vm->_state->setVar(var, 1);

			// Play the "peg clicks into spot sound"
			if (!_vm->_sound->isPlaying(1024)) {
				_vm->_sound->playEffect(1024, 100);
			}
		}
	}

	// Remaining pegs movie
	uint32 pegs = _vm->_state->getPinballRemainingPegs();
	uint32 frame = remainingPegsFrames[pegs];
	_vm->_state->setVar(33, frame);

	// Choose pegs movie according to peg combination
	const PegCombination *leftComb = _pinballFindCombination(461, leftPegs, ARRAYSIZE(leftPegs));
	if (!leftComb)
		error("Unable to find correct left pegs combination");
	_vm->_state->setVar(31, leftComb->movie - 10100);

	const PegCombination *rightComb = _pinballFindCombination(466, rightPegs, ARRAYSIZE(rightPegs));
	if (!rightComb)
		error("Unable to find correct right pegs combination");
	_vm->_state->setVar(32, rightComb->movie - 10200);

	if (var >= 0)
		return;

	_vm->_state->setWaterEffectRunning(false);

	// Remove the default panel movies
	_vm->removeMovie(10116);
	_vm->removeMovie(10227);

	// Set up left panel movie with the correct combination
	_vm->_state->setMoviePreloadToMemory(true);
	_vm->_state->setMovieScriptDriven(true);
	_vm->_state->setMovieNextFrameGetVar(31);
	_vm->loadMovie(leftComb->movie, 1, false, true);
	_vm->_state->setVar(31, 2);

	// Set up right panel movie with the correct combination
	_vm->_state->setMoviePreloadToMemory(true);
	_vm->_state->setMovieScriptDriven(true);
	_vm->_state->setMovieNextFrameGetVar(32);
	_vm->loadMovie(rightComb->movie, 1, false, true);
	_vm->_state->setVar(32, 2);

	// Launch sound
	_vm->_sound->playEffect(1021, 50);
	_drawForVarHelper(-34, 2, 15);
	_drawXTicks(30);

	int32 leftSideFrame = 250;
	int32 rightSideFrame = 500;
	_vm->_state->setVar(34, 250);
	_vm->_state->setVar(35, 500);
	int32 leftPanelFrame = 2;
	int32 rightPanelFrame = 2;
	int32 ballOnLeftSide = 1;
	int32 ballOnRightSide = 0;
	int32 ballShouldExpire = 0;
	int32 ballCrashed = 0;
	int32 leftToRightJumpCountDown = 0;
	int32 rightToLeftJumpCountdown = 0;
	int32 ballJumpedFromLeftSide = 1;
	int32 ballJumpedFromRightSide = 0;
	int32 jumpType = -1;

	while (1) {
		_drawXTicks(1);

		bool shouldRotate;
		if (leftToRightJumpCountDown >= 3 || rightToLeftJumpCountdown >= 3) {
			shouldRotate = false;
			_vm->_sound->stopEffect(1025, 7);
		} else {
			shouldRotate = true;
			_vm->_sound->playEffectLooping(1025, 50, ballOnLeftSide != 0 ? 150 : 210, 95);
		}

		if (ballOnLeftSide && shouldRotate) {
			++leftSideFrame;

			if (ballJumpedFromLeftSide) {
				if (leftSideFrame >= 500)
					leftSideFrame = 200;
			} else {
				if (leftSideFrame >= 800)
					leftSideFrame = 500;
			}

			_vm->_state->setVar(34, leftSideFrame);
		}

		if (ballOnRightSide && shouldRotate) {
			rightSideFrame++;

			if (ballJumpedFromRightSide) {
				if (rightSideFrame >= 500)
					rightSideFrame = 200;
			} else {
				if (rightSideFrame >= 800)
					rightSideFrame = 500;
			}

			_vm->_state->setVar(35, rightSideFrame);
		}

		if (ballOnLeftSide) {
			leftPanelFrame++;
			_vm->_state->setVar(31, leftPanelFrame);

			for (uint i = 0; i < 3; i++) {
				if (leftComb->pegFrames[i] == leftPanelFrame) {
					_vm->_sound->playEffect(1027, 50);
					leftToRightJumpCountDown = 5;
				}
			}

			if (leftPanelFrame == leftComb->expireFrame) {
				ballShouldExpire = 1;
				ballOnLeftSide = 0;
			}
		}

		if (ballOnRightSide) {
			rightPanelFrame++;
			_vm->_state->setVar(32, rightPanelFrame);

			for (uint i = 0; i < 3; i++) {
				if (rightComb->pegFrames[i] == rightPanelFrame) {
					_vm->_sound->playEffect(1027, 50);
					rightToLeftJumpCountdown = 5;
				}
			}

			if (rightPanelFrame == rightComb->expireFrame) {
				ballShouldExpire = 1;
				ballOnRightSide = 0;
			}
		}

		bool ballShouldJump = false;
		if (leftToRightJumpCountDown) {
			--leftToRightJumpCountDown;
			if (!leftToRightJumpCountDown) {
				ballOnLeftSide = 0;
				ballShouldJump = true;
			}
		}

		if (rightToLeftJumpCountdown) {
			--rightToLeftJumpCountdown;
			if (!rightToLeftJumpCountdown) {
				ballOnRightSide = 0;
				ballShouldJump = true;
			}
		}

		if (ballShouldJump) {
			_vm->_sound->stopEffect(1025, 7);
			_drawXTicks(30);

			int32 jumpPositionLeft = 50 * ((leftSideFrame + 25) / 50);
			int32 jumpPositionRight = 50 * ((rightSideFrame + 25) / 50);

			const BallJump *jump = 0;

			for (uint i = 0; i < ARRAYSIZE(jumps); i++) {
				int32 filter = jumps[i].filter;
				if (filter != -1) {
					if (filter) {
						if (!(jumpPositionLeft % 100))
							continue;
					} else {
						if (jumpPositionLeft % 100)
							continue;
					}
				}

				if (abs(jumps[i].positionRight - jumpPositionRight) < 10) {
					ballOnRightSide = 0;
					ballOnLeftSide = 1;
					ballJumpedFromRightSide = 0;
					ballJumpedFromLeftSide = 1;
					jump = &jumps[i];
					break;
				}
			}

			for (uint i = 0; i < ARRAYSIZE(jumps); i++) {
				int32 filter = jumps[i].filter;
				if (filter != -1) {
					if (filter) {
						if (!(jumpPositionRight % 100))
							continue;
					} else {
						if (jumpPositionRight % 100)
							continue;
					}
				}

				if (abs(jumps[i].positionLeft - jumpPositionLeft) < 10) {
					ballOnLeftSide = 0;
					ballOnRightSide = 1;
					ballJumpedFromRightSide = 1;
					ballJumpedFromLeftSide = 0;
					jump = &jumps[i];
					break;
				}
			}

			if (!jump)
				error("Bad orb jump combo %d %d", jumpPositionLeft, jumpPositionRight);

			jumpType = jump->type;

			int32 sound = jump->sound;
			if (sound)
				_vm->_sound->playEffect(sound, 50);

			int32 jumpStartFrame = jump->startFrame;
			if (jumpStartFrame)
				_drawForVarHelper(-34, jumpStartFrame, jump->endFrame);

			if (jumpType == 3) {
				_drawXTicks(6);
				_vm->_sound->playEffect(1028, 50);
			} else if (jumpType == 1 || jumpType == 4) {
				_vm->_state->setVar(26, jumpType);
				_vm->_state->setWaterEffectRunning(true);
				_vm->_sound->stopEffect(1025, 7);
				return;
			}

			leftSideFrame = jump->targetLeftFrame;
			rightSideFrame = jump->tragetRightFrame;
			_vm->_state->setVar(34, leftSideFrame);
			_vm->_state->setVar(35, rightSideFrame);

			if (jumpType >= 2)
				ballCrashed = 1;

			_drawXTicks(30);
		}

		if (ballShouldExpire) {
			leftSideFrame = 50 * ((leftSideFrame + 25) / 50);
			rightSideFrame = 50 * ((rightSideFrame + 25) / 50);

			if (leftSideFrame == 500)
				leftSideFrame = 200;

			if (rightSideFrame == 500)
				rightSideFrame = 200;

			_vm->_sound->stopEffect(1025, 7);
			_vm->_sound->playEffectFadeInOut(1005, 65, 0, 0, 5, 60, 20);
			_drawXTicks(55);
			_vm->_sound->playEffect(1010, 50);

			for (uint i = 0; i < ARRAYSIZE(ballExpireFrames); i++) {
				if (ballJumpedFromLeftSide && ballExpireFrames[i].leftPosition == leftSideFrame) {
					_drawForVarHelper(34, ballExpireFrames[i].startFrame, ballExpireFrames[i].endFrame);
					break;
				}

				if (ballJumpedFromRightSide && ballExpireFrames[i].rightPosition == rightSideFrame) {
					_drawForVarHelper(35, ballExpireFrames[i].startFrame, ballExpireFrames[i].endFrame);
					break;
				}
			}

			_drawXTicks(15);
			break;
		}

		if (ballCrashed)
			break;
	}

	if (ballCrashed || ballShouldExpire) {
		if (leftSideFrame < 500)
			leftSideFrame += 300;
		if (rightSideFrame < 500)
			rightSideFrame += 300;

		int32 crashedLeftFrame = ((((leftSideFrame + 25) / 50) >> 4) & 1) != 0 ? 550 : 500;
		int32 crashedRightFrame = ((((rightSideFrame + 25) / 50) >> 4) & 1) != 0 ? 550 : 500;

		while (1) {
			bool moviePlaying = false;
			if ((leftComb->movie != 10101 || leftPanelFrame > 2)
					&& leftPanelFrame != leftComb->expireFrame) {

				if (leftToRightJumpCountDown) {
					--leftToRightJumpCountDown;
				}
				if (!leftToRightJumpCountDown) {
					_vm->_state->setVar(34, crashedLeftFrame);
					crashedLeftFrame++;
				}

				_vm->_state->setVar(31, leftPanelFrame);

				++leftPanelFrame;
				leftSideFrame = leftPanelFrame;

				for (uint i = 0; i < 3; i++) {
					if (leftComb->pegFrames[i] == leftSideFrame) {
						_vm->_sound->playEffect(1027, 50);
						leftToRightJumpCountDown = 5;
					}
				}

				moviePlaying = true;
			}

			if (!moviePlaying) {
				if ((rightComb->movie != 10201 || rightPanelFrame > 2)
					&& rightPanelFrame != rightComb->expireFrame) {

					if (rightToLeftJumpCountdown) {
						--rightToLeftJumpCountdown;
					}
					if (!rightToLeftJumpCountdown) {
						_vm->_state->setVar(35, crashedRightFrame);
						crashedRightFrame++;
					}

					_vm->_state->setVar(32, rightPanelFrame);

					++rightPanelFrame;
					rightSideFrame = rightPanelFrame;

					for (uint i = 0; i < 3; i++) {
						if (rightComb->pegFrames[i] == rightSideFrame) {
							_vm->_sound->playEffect(1027, 50);
							rightToLeftJumpCountdown = 5;
						}
					}

					moviePlaying = true;
				}
			}

			_drawXTicks(1);

			if (!moviePlaying) {
				_vm->_state->setVar(26, jumpType);
				_vm->_state->setVar(93, 1);
				_vm->_sound->stopEffect(1025, 7);
				return;
			}

			_vm->_sound->playEffectLooping(1025, 50);
		}
	}
}

const Puzzles::PegCombination *Puzzles::_pinballFindCombination(uint16 var, const PegCombination pegs[], uint16 size) {
	const PegCombination *combination = 0;

	for (uint i = 0; i < size; i++) {
		bool good = true;
		for (uint j = 0; j < 5; j++) {
			bool setPeg = _vm->_state->getVar(var + j);
			bool targetPeg = pegs[i].pegs[j];
			if (setPeg != targetPeg)
				good = false;
		}

		if (good) {
			combination = &pegs[i];
			break;
		}
	}

	return combination;
}

void Puzzles::weightDrag(uint16 var, uint16 movie) {
	if (var >= 429 && var <= 432) {
		movie = _vm->_state->getVar(var);
		_vm->_state->setVar(var, 0);
		var = movie;
	}

	uint16 sound = 0;
	if (var) {
		switch (var) {
		case 423:
			movie = 1022;
			sound = 921;
			break;
		case 425:
			movie = 1023;
			sound = 921;
			break;
		case 424:
			movie = 1024;
			sound = 922;
			break;
		case 427:
			movie = 1025;
			sound = 922;
			break;
		case 426:
			movie = 1020;
			sound = 920;
			break;
		case 428:
			movie = 1021;
			sound = 920;
			break;
		default:
			break;
		}

		_vm->_state->setDraggedWeight(var);
		_vm->dragItem(var, movie, 1, 2, 26);
		_vm->_sound->playEffect(sound, 25);
	}

	for (uint i = 0; i < 4; i++) {
		int32 value = _vm->_state->getVar(429 + i);
		uint16 frame = 0;
		switch (value) {
		case 423:
		case 425:
			frame = 2;
			break;
		case 424:
		case 427:
			frame = 3;
			break;
		case 426:
		case 428:
			frame = 1;
			break;
		default:
			break;
		}
		_vm->_state->setVar(28 + i, frame);
		_vm->_state->setVar(32 + i, frame != 0);
	}
}

void Puzzles::journalSaavedro(int16 move) {
	uint16 chapter = _vm->_state->getJournalSaavedroChapter();
	int16 page = _vm->_state->getJournalSaavedroPageInChapter();

	if (!_journalSaavedroHasChapter(chapter))
		chapter = _journalSaavedroNextChapter(chapter, true);

	if (move > 0) {
		// Go to the next available page
		int16 pageCount = _journalSaavedroPageCount(chapter);
		page++;

		if (page == pageCount) {
			chapter = _journalSaavedroNextChapter(chapter, true);
			page = 0;
		}

		_vm->_state->setJournalSaavedroChapter(chapter);
		_vm->_state->setJournalSaavedroPageInChapter(page);
	} else if (move < 0) {
		// Go to the previous available page
		page--;

		if (page < 0) {
			chapter = _journalSaavedroNextChapter(chapter, false);
			page = _journalSaavedroPageCount(chapter) - 1;
		}

		_vm->_state->setJournalSaavedroChapter(chapter);
		_vm->_state->setJournalSaavedroPageInChapter(page);
	} else {
		// Display current page
		int16 chapterStartNode = _journalSaavedroGetNode(chapter);
		int16 closed = 0;
		int16 opened = 0;
		int16 lastPage = 0;

		if (chapter > 0) {
			opened = 1;
			if (chapter == 21)
				lastPage = _journalSaavedroLastPageLastChapterValue();
			else
				lastPage = 1;

		} else {
			closed = 1;
		}

		uint16 nodeRight;
		uint16 nodeLeft;
		if (page || !chapter) {
			nodeRight = chapterStartNode + page;
			nodeLeft = chapterStartNode + page;
		} else {
			nodeRight = chapterStartNode + page;
			uint16 chapterLeft = _journalSaavedroNextChapter(chapter, false);
			if (chapterLeft > 0)
				nodeLeft = _journalSaavedroGetNode(chapterLeft + 1);
			else
				nodeLeft = 201;
		}

		_vm->_state->setJournalSaavedroClosed(closed);
		_vm->_state->setJournalSaavedroOpen(opened);
		_vm->_state->setJournalSaavedroLastPage(lastPage);

		_vm->loadNodeFrame(nodeRight);

		// Does the left page need to be loaded from a different node?
		if (nodeLeft != nodeRight) {
			ResourceDescription jpegDesc = _vm->getFileDescription("", nodeLeft, 0, Archive::kFrame);

			if (!jpegDesc.isValid())
				error("Frame %d does not exist", nodeLeft);

			Graphics::Surface *bitmap = Myst3Engine::decodeJpeg(&jpegDesc);

			// Copy the left half of the node to a new surface
			Graphics::Surface *leftBitmap = new Graphics::Surface();
			leftBitmap->create(bitmap->w / 2, bitmap->h, Texture::getRGBAPixelFormat());

			for (uint i = 0; i < bitmap->h; i++) {
				memcpy(leftBitmap->getBasePtr(0, i),
						bitmap->getBasePtr(0, i),
						leftBitmap->w * 4);
			}

			bitmap->free();
			delete bitmap;

			// Create a spotitem covering the left half of the screen
			// to display the left page
			SpotItemFace *leftPage = _vm->addMenuSpotItem(999, 1, Common::Rect(0, 0, leftBitmap->w, leftBitmap->h));

			leftPage->updateData(leftBitmap);

			leftBitmap->free();
			delete leftBitmap;
		}
	}
}

int16 Puzzles::_journalSaavedroLastPageLastChapterValue() {
	// The scripts just expect different values ...
	if (_vm->getPlatform() == Common::kPlatformXbox) {
		return 0;
	} else {
		return 2;
	}
}

uint16 Puzzles::_journalSaavedroGetNode(uint16 chapter) {
	ResourceDescription desc = _vm->getFileDescription("", 1200, 0, Archive::kNumMetadata);

	if (!desc.isValid())
		error("Node 1200 does not exist");

	return desc.getMiscData(chapter) + 199;
}

uint16 Puzzles::_journalSaavedroPageCount(uint16 chapter) {
	uint16 chapterStartNode = _journalSaavedroGetNode(chapter);
	if (chapter != 21)
		return _journalSaavedroGetNode(chapter + 1) - chapterStartNode;
	else
		return 1;
}

bool Puzzles::_journalSaavedroHasChapter(uint16 chapter) {
	return _vm->_state->getVar(285 + chapter) != 0;
}

uint16 Puzzles::_journalSaavedroNextChapter(uint16 chapter, bool forward) {
	do {
		if (forward)
			chapter++;
		else
			chapter--;
	} while (!_journalSaavedroHasChapter(chapter));

	return chapter;
}

void Puzzles::journalAtrus(uint16 node, uint16 var) {
	uint numPages = 0;

	while (_vm->getFileDescription("", node++, 0, Archive::kFrame).isValid())
		numPages++;

	_vm->_state->setVar(var, numPages - 1);
}

void Puzzles::symbolCodesInit(uint16 var, uint16 posX, uint16 posY) {
	struct Point {
		uint16 x;
		uint16 y;
	};

	struct CodeData {
		uint16 node;
		uint16 movie;
		bool flag;
		Point coords[20];
	};

	static const CodeData codes[] = {
	{
		144, 10144, 0,
		{
			{ 296, 120 }, { 312, 128 }, { 296, 144 }, { 296, 128 }, { 312, 120 },
			{ 328, 120 }, { 312, 144 }, { 312, 128 }, { 296, 136 }, { 312, 144 },
			{ 296, 160 }, { 296, 144 }, { 312, 136 }, { 328, 144 }, { 312, 160 },
			{ 312, 144 }, { 296, 112 }, { 328, 120 }, { 296, 160 }, { 288, 120 }
		}
		}, {
			244, 10244, 1,
			{
				{ 288, 16 }, { 336, 32 }, { 294, 72 }, { 280, 24 }, { 336, 16 },
				{ 376, 24 }, { 336, 72 }, { 328, 32 }, { 288, 64 }, { 336, 80 },
				{ 288, 120 }, { 280, 72 }, { 336, 64 }, { 384, 72 }, { 336, 120 },
				{ 328, 80 }, { 288, 0 }, { 384, 24 }, { 288, 120 }, { 264, 24 }
			}
		}, {
			148, 10148, 0,
			{
				{ 280, 24 }, { 304, 32 }, { 288, 48 }, { 280, 24 }, { 304, 24 },
				{ 320, 32 }, { 304, 48 }, { 296, 32 }, { 288, 40 }, { 304, 48 },
				{ 280, 64 }, { 280, 48 }, { 304, 48 }, { 320, 48 }, { 304, 64 },
				{ 296, 48 }, { 280, 16 }, { 320, 24 }, { 280, 64 }, { 272, 24 }
			}
		}, {
			248, 10248, 1,
			{
				{ 280, 48 }, { 320, 56 }, { 287, 88 }, { 272, 56 }, { 320, 48 },
				{ 360, 56 }, { 328, 96 }, { 312, 56 }, { 288, 88 }, { 320, 96 },
				{ 280, 128 }, { 271, 96 }, { 328, 88 }, { 360, 96 }, { 320, 128 },
				{ 312, 96 }, { 280, 32 }, { 360, 48 }, { 280, 128 }, { 264, 48 }
			}
		}, {
			348, 10348, 1,
			{
				{ 336, 24 }, { 376, 32 }, { 336, 80 }, { 328, 32 }, { 376, 24 },
				{ 424, 32 }, { 384, 80 }, { 368, 40 }, { 336, 72 }, { 376, 80 },
				{ 336, 120 }, { 328, 80 }, { 384, 72 }, { 424, 80 }, { 376, 120 },
				{ 368, 80 }, { 328, 8 }, { 424, 32 }, { 328, 128 }, { 312, 32 }
			}
		}, {
			448, 10448, 1,
			{
				{ 224, 32 }, { 264, 40 }, { 224, 80 }, { 208, 40 }, { 264, 32 },
				{ 304, 40 }, { 270, 88 }, { 256, 40 }, { 224, 72 }, { 264, 88 },
				{ 224, 128 }, { 208, 88 }, { 272, 72 }, { 312, 88 }, { 264, 128 },
				{ 256, 88 }, { 216, 16 }, { 312, 40 }, { 216, 128 }, { 200, 40 }
			}
		}
	};

	uint16 node = _vm->_state->getLocationNode();

	const CodeData *code = 0;
	for (uint i = 0; i < ARRAYSIZE(codes); i++)
		if (codes[i].node == node) {
			code = &codes[i];
			break;
		}

	if (!code)
		error("Unable to find puzzle data for node %d", node);

	int32 value = _vm->_state->getVar(var);

	for (uint i = 0; i < 20; i++) {
		if (code->flag || value & (1 << i)) {
			_vm->_state->setMoviePreloadToMemory(true);
			_vm->_state->setMovieScriptDriven(true);
			_vm->_state->setMovieOverridePosition(true);
			_vm->_state->setMovieOverridePosU(posX + code->coords[i].x);
			_vm->_state->setMovieOverridePosV(posY + code->coords[i].y);
			_vm->_state->setMovieConditionBit(i + 1);
			_vm->loadMovie(code->movie + i * 1000, var, false, true);
		}
	}
}

void Puzzles::symbolCodesClick(int16 var) {
	// Toggle clicked symbol element
	if (var > 0) {
		int32 value = _vm->_state->getVar(var);
		value ^= 1 << _vm->_state->getHotspotActiveRect();
		_vm->_state->setVar(var, value);
	}

	// Check puzzle with one symbol solution
	static const SymbolCodeSolution smallSolution = { 330080, 53575, 241719, 116411 };
	if (_vm->_state->getSymbolCode1AllSolved()) {
		bool code2Solved = _symbolCodesCheckSolution(490, smallSolution);
		_vm->_state->setSymbolCode2Solved(code2Solved);
	}

	// Check puzzle with 3 symbols solution
	static const SymbolCodeSolution solutions[] = {
		{ 208172, 131196, 252945, 788771 },
		{ 431060, 418863, 558738, 653337 },
		{ 472588, 199440, 155951, 597954 }
	};


	_vm->_state->setSymbolCode1CurrentSolved(false);

	for (uint i = 1; i <= ARRAYSIZE(solutions); i++) {
		int32 solutionsFound = _symbolCodesFound();

		// Symbol already found, don't allow it another time
		if (solutionsFound & (1 << i))
			continue;

		if (_symbolCodesCheckSolution(498, solutions[i - 1])) {
			_vm->_state->setSymbolCode1TopSolved(i);
			_vm->_state->setSymbolCode1CurrentSolved(true);
		}

		if (_symbolCodesCheckSolution(503, solutions[i - 1])) {
			_vm->_state->setSymbolCode1LeftSolved(i);
			_vm->_state->setSymbolCode1CurrentSolved(true);
		}

		if (_symbolCodesCheckSolution(508, solutions[i - 1])) {
			_vm->_state->setSymbolCode1RightSolved(i);
			_vm->_state->setSymbolCode1CurrentSolved(true);
		}
	}

	bool allSolved = _symbolCodesFound() == 14;
	_vm->_state->setSymbolCode1AllSolved(allSolved);
}

bool Puzzles::_symbolCodesCheckSolution(uint16 var, const SymbolCodeSolution &solution) {
	bool solved = true;

	for (uint i = 0; i < ARRAYSIZE(solution); i++) {
		int32 value = _vm->_state->getVar(var + i);
		if (value != solution[i]) {
			solved = false;
			break;
		}
	}

	return solved;
}

int32 Puzzles::_symbolCodesFound() {
	int32 top = _vm->_state->getSymbolCode1TopSolved();
	int32 left = _vm->_state->getSymbolCode1LeftSolved();
	int32 right = _vm->_state->getSymbolCode1RightSolved();

	return (1 << top) | (1 << left) | (1 << right);
}

void Puzzles::railRoadSwitchs() {
	uint16 index = _vm->_state->getHotspotActiveRect();
	uint16 startFrame = _vm->_state->getVar(449 + index);
	uint16 endFrame;

	switch (startFrame) {
	case 1:
		endFrame = 4;
		break;
	case 4:
		endFrame = 7;
		break;
	case 7:
		endFrame = 10;
		break;
	case 10:
		endFrame = 12;
		break;
	default:
		error("Bad railroad switchs start value %d", startFrame);
		return;
	}

	_drawForVarHelper(28 + index, startFrame, endFrame);

	if (endFrame == 12)
		endFrame = 1;

	_vm->_state->setVar(28 + index, endFrame);
	_vm->_state->setVar(449 + index, endFrame);
}

void Puzzles::rollercoaster() {
	static const uint8 map1[][8] = {
		{ 3, 9, 9, 0, 7, 9, 9, 4 },
		{ 2, 6, 0, 9, 9, 9, 1, 9 },
		{ 6, 9, 4, 9, 2, 9, 0, 9 },
		{ 4, 9, 6, 9, 0, 9, 2, 9 },
		{ 9, 4, 7, 9, 1, 9, 9, 2 },
		{ 2, 9, 0, 9, 7, 9, 9, 4 },
		{ 6, 9, 9, 7, 9, 9, 0, 3 },
		{ 4, 9, 7, 6, 0, 9, 3, 2 },
		{ 9, 5, 9, 9, 6, 1, 4, 9 }
	};

	static const uint8 map2[][8] = {
		{   0,  0,  26, 57,  40,  0,   0,  0 },
		{ 100,  0,  36, 67,  50, 41,  12,  0 },
		{   0,  0,   0,  0,  60, 51,  22,  0 },
		{  14, 25,  56, 87,  70,  0, 103,  0 },
		{  24, 35,  66, 97,  80, 71,  42, 13 },
		{  34,  0, 101,  0,  90, 81,  52, 23 },
		{  44, 55,  86,  0,   0,  0,   0,  0 },
		{  54, 65,  96,  0, 102,  0,  72, 43 },
		{  64,  0,   0,  0,   0,  0,  82, 53 }
	};

	int32 entryPoint = _vm->_state->getVar(26);
	int32 movie = 0;
	int32 exitPoint = 0;

	if (_vm->_state->getVar(38 + entryPoint - 100)) {
		_vm->_state->setVar(42, 0);
		_vm->_state->setVar(26, 0);
		return;
	}

	_vm->_state->setVar(38 + entryPoint - 100, 1);

	switch (entryPoint) {
	case 100:
		_vm->_state->setVar(42, 0);
		_vm->_state->setVar(26, 1);
		return;
	case 101:
		movie = 12007;
		exitPoint = 93;
		break;
	case 102:
		movie = 14007;
		exitPoint = 75;
		break;
	case 103:
		movie = 16007;
		exitPoint = 17;
		break;
	default:
		_vm->_state->setVar(42, 0);
		_vm->_state->setVar(26, 0);
		return;
	}

	int32 recursion = 20;
	while (1) {
		int32 switchIndex = exitPoint / 10 - 1;
		int32 switchFrame = _vm->_state->getVar(449 + switchIndex);
		int32 switchPosition = 2 * (switchFrame - 1) / 3;

		int32 direction = map1[switchIndex][(exitPoint % 10 - switchPosition) & 7];

		if (direction != 9)
			exitPoint = map2[switchIndex][(switchPosition + direction) & 7];
		else
			exitPoint = 0;

		if (!recursion)
			break;

		recursion--;

		if (exitPoint <= 0 || exitPoint >= 100) {
			_vm->_state->setVar(42, exitPoint);
			_vm->_state->setVar(26, movie);
			return;
		}
	}

	_vm->_state->setVar(42, 0);
	_vm->_state->setVar(26, movie);
}

void Puzzles::mainMenu(uint16 action) {
	_vm->setMenuAction(action);
}

static void copySurfaceRect(Graphics::Surface *dest, const Common::Point &destPoint, const Graphics::Surface *src) {
	for (uint16 i = 0; i < src->h; i++)
		memcpy(dest->getBasePtr(destPoint.x, i + destPoint.y), src->getBasePtr(0, i), src->pitch);
}

void Puzzles::projectorLoadBitmap(uint16 bitmap) {
	assert(_vm->_projectorBackground == 0 && "Previous background not yet used.");

	// This surface is freed by the destructor of the movie that uses it
	_vm->_projectorBackground = new Graphics::Surface();
	_vm->_projectorBackground->create(1024, 1024, Texture::getRGBAPixelFormat());

	ResourceDescription movieDesc = _vm->getFileDescription("", bitmap, 0, Archive::kStillMovie);

	if (!movieDesc.isValid())
		error("Movie %d does not exist", bitmap);

	// Rebuild the complete background image from the frames of the bink movie
	Common::SeekableReadStream *movieStream = movieDesc.getData();
	Video::BinkDecoder bink;
	bink.setDefaultHighColorFormat(Texture::getRGBAPixelFormat());
	bink.loadStream(movieStream);
	bink.start();

	for (uint i = 0; i < 1024; i += 256) {
		for (uint j = 0; j < 1024; j += 256) {
			const Graphics::Surface *frame = bink.decodeNextFrame();
			copySurfaceRect(_vm->_projectorBackground, Common::Point(j, i), frame);
		}
	}
}

void Puzzles::projectorAddSpotItem(uint16 bitmap, uint16 x, uint16 y) {
	assert(_vm->_projectorBackground != 0 && "Projector background already used.");

	// Nothing to do if the spotitem is not enabled
	if (!_vm->_state->getVar(26))
		return;

	ResourceDescription movieDesc = _vm->getFileDescription("", bitmap, 0, Archive::kStillMovie);

	if (!movieDesc.isValid())
		error("Movie %d does not exist", bitmap);

	// Rebuild the complete background image from the frames of the bink movie
	Common::SeekableReadStream *movieStream = movieDesc.getData();
	Video::BinkDecoder bink;
	bink.setDefaultHighColorFormat(Texture::getRGBAPixelFormat());
	bink.loadStream(movieStream);
	bink.start();

	const Graphics::Surface *frame = bink.decodeNextFrame();
	copySurfaceRect(_vm->_projectorBackground, Common::Point(x, y), frame);
}

void Puzzles::projectorUpdateCoordinates() {
	int16 x = CLIP<int16>(_vm->_state->getProjectorX(), 840, 9400);
	int16 y = CLIP<int16>(_vm->_state->getProjectorY(), 840, 9400);
	int16 zoom = CLIP<int16>(_vm->_state->getProjectorZoom(), 1280, 5120);
	int16 blur = CLIP<int16>(_vm->_state->getProjectorBlur(), 400, 2470);

	int16 halfZoom = zoom / 2;
	if (x - halfZoom < 0)
		x = halfZoom;
	if (x + halfZoom > 10240)
		x = 10240 - halfZoom;
	if (y - halfZoom < 0)
		y = halfZoom;
	if (y + halfZoom > 10240)
		y = 10240 - halfZoom;

	int16 angleXOffset = _vm->_state->getProjectorAngleXOffset();
	int16 angleYOffset = _vm->_state->getProjectorAngleYOffset();
	int16 angleZoomOffset = _vm->_state->getProjectorAngleZoomOffset();
	int16 angleBlurOffset = _vm->_state->getProjectorAngleBlurOffset();

	int16 angleX = (angleXOffset + 200 * (5 * x - 4200) / 8560) % 1000;
	int16 angleY = (angleYOffset + 200 * (5 * y - 4200) / 8560) % 1000;
	int16 angleZoom = (angleZoomOffset + 200 * (5 * zoom - 6400) / 3840) % 1000;
	int16 angleBlur = (angleBlurOffset + 200 * (5 * blur - 2000) / 2070) % 1000;

	_vm->_state->setProjectorAngleX(angleX);
	_vm->_state->setProjectorAngleY(angleY);
	_vm->_state->setProjectorAngleZoom(angleZoom);
	_vm->_state->setProjectorAngleBlur(angleBlur);

	_vm->_state->setProjectorX(x);
	_vm->_state->setProjectorY(y);
	_vm->_state->setProjectorZoom(zoom);
	_vm->_state->setProjectorBlur(blur);
}

void Puzzles::settingsSave() {
	ConfMan.flushToDisk();
}

void Puzzles::updateSoundScriptTimer() {
	int frequency = 15 * ConfMan.getInt("music_frequency") / 100;
	if (_vm->_state->getSoundScriptsPaused()) {
		_vm->_state->setSoundScriptsTimer(60 * (20 - frequency));
	} else {
		_vm->_state->setSoundScriptsTimer(60 * (frequency + 5));
	}
}

void Puzzles::checkCanSave() {
	// There is no reason to forbid saving games with ScummVM,
	// since there is no notion of memory card, free blocks and such.
	_vm->_state->setStateCanSave(true);
}

} // End of namespace Myst3
