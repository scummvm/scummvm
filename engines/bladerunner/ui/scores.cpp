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
#include "bladerunner/text_resource.h"
#include "bladerunner/vqa_player.h"

#include "common/keyboard.h"

namespace BladeRunner {

Scores::Scores(BladeRunnerEngine *vm) {
	_vm = vm;

	_txtScorers = new TextResource(_vm);
	_font = new Font(_vm);

	reset();
}

Scores::~Scores() {
	delete _font;
	delete _txtScorers;
}

void Scores::open() {
	if (!_vm->openArchive("MODE.MIX")) {
		return;
	}

	_vqaPlayer = new VQAPlayer(_vm, &_vm->_surfaceBack);

	if (!_vqaPlayer->open("SCORE.VQA")) {
		return;
	}

	_vqaPlayer->setLoop(1, -1, 0, nullptr, nullptr);

	// TODO: Freeze game time

	_txtScorers->open("SCORERS");
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
}

void Scores::fill() {
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
