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

#include "bladerunner/ui/scores.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/font.h"
#include "bladerunner/savefile.h"
#include "bladerunner/scene.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/vqa_player.h"

#include "common/keyboard.h"

namespace BladeRunner {

Scores::Scores(BladeRunnerEngine *vm) {
	_vm = vm;

	_font = nullptr;
	_txtScorers = nullptr;

	reset();
}

Scores::~Scores() {
}

void Scores::open() {
	if (!_vm->openArchive("MODE.MIX")) {
		return;
	}

	_vqaPlayer = new VQAPlayer(_vm, &_vm->_surfaceBack, "SCORE.VQA");

	if (!_vqaPlayer->open()) {
		return;
	}

	_vqaPlayer->setLoop(1, -1, 0, nullptr, nullptr);

	// TODO: Freeze game time

	_txtScorers = new TextResource(_vm);
	_txtScorers->open("SCORERS");

	_font = new Font(_vm);
	_font->open("TAHOMA24.FON", 640, 480, -1, 0, 0);
	_font->setSpacing(1, 0);

	fill();

	_isOpen = true;
	_isLoaded = false;
}

bool Scores::isOpen() const {
	return _isOpen;
}

void Scores::close() {
	_isOpen = false;

	delete _font;
	_font = nullptr;

	delete _txtScorers;
	_txtScorers = nullptr;

	if (_vqaPlayer) {
		_vqaPlayer->close();
		delete _vqaPlayer;
		_vqaPlayer = nullptr;
	}

	_vm->closeArchive("MODE.MIX");

	// TODO: Unfreeze game time
	_vm->_scene->resume();
}

void Scores::set(int index, int value) {
	if (value > _scores[index]) {
		_scores[index] = value;
	}

	_lastScoreId = index;
	_lastScoreValue = value;
}

void Scores::handleKeyDown(const Common::KeyState &kbd) {
	close();
}

int Scores::handleMouseUp(int x, int y) {
	if (_isLoaded) {
		close();
	}

	_isLoaded = false;

	return false;
}

int Scores::handleMouseDown(int x, int y) {
	_isLoaded = true;

	return false;
}

void Scores::tick() {
	if (!_vm->_gameIsRunning) {
		return;
	}

	int frame = _vqaPlayer->update();
	assert(frame >= -1);

	// vqaPlayer renders to _surfaceBack
	blit(_vm->_surfaceBack, _vm->_surfaceFront);

	_vm->_surfaceFront.hLine(200, 139, 400, 0x3e0);
	_vm->_surfaceFront.hLine(200, 347, 400, 0x1f);

	_font->draw(_txtScorers->getText(7), _vm->_surfaceFront, 200, 114);

	int y = 140;

	for (int i = 0; i < 7; i++) {
		_font->draw(_txtScorers->getText(_scorers[i]), _vm->_surfaceFront, 220, y);
		_font->drawNumber(_scores[_scorers[i]], _vm->_surfaceFront, 360, y);

		y += 26;
	}

	_font->draw(_txtScorers->getText(8), _vm->_surfaceFront, 200, 322);
	_font->draw(_txtScorers->getText(_lastScoreId), _vm->_surfaceFront, 220, 348);
	_font->drawNumber(_lastScoreValue, _vm->_surfaceFront, 360, 348);

	_vm->blitToScreen(_vm->_surfaceFront);
}

void Scores::fill() {
	for (int i = 0; i < 7; i++) {
		_scorers[i] = i;
	}

	// Network sorting using Bose-Nelson Algorithm
	const byte network[32] = { // Bose-Nelson
		1,2, 3,4, 5,6,
		0,2, 3,5, 4,6,
		0,1, 4,5, 2,6,
		0,4, 1,5,
		0,3, 2,5,
		1,3, 2,4,
		2,3
	};

	for (int i = 0; i < 32; i += 2) {
		int i1 = network[i], i2 = network[i + 1];
		if (_scores[_scorers[i1]] < _scores[_scorers[i2]]) {
			SWAP(_scorers[i1], _scorers[i2]);
		}
	}
}

void Scores::reset() {
	_isOpen = false;
	_isLoaded = false;
	_vqaPlayer = nullptr;

	for (int i = 0; i < 7; i++) {
		_scores[i] = -80;
		_scorers[i] = 0;
	}

	_lastScoreId = 0;
	_lastScoreValue = 0;
}

void Scores::save(SaveFileWriteStream &f) {
	for (int i = 0; i < 7; i++) {
		f.writeInt(_scores[i]);
	}

	f.writeInt(_lastScoreId);
	f.writeInt(_lastScoreValue);
}

void Scores::load(SaveFileReadStream &f) {
	for (int i = 0; i < 7; i++) {
		_scores[i] = f.readInt();
	}

	_lastScoreId = f.readInt();
	_lastScoreValue = f.readInt();
}

} // End of namespace BladeRunner
