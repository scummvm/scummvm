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
#include "bladerunner/savefile.h"
#include "bladerunner/vqa_player.h"

namespace BladeRunner {

Scores::Scores(BladeRunnerEngine *vm) {
	_vm = vm;
	reset();
}

Scores::~Scores() {
}

bool Scores::isOpen() const {
	return _isOpen;
}

void Scores::tick() {
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
