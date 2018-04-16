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

#include "bbvs/minigames/minigame.h"
#include "common/savefile.h"

namespace Bbvs {

Minigame::Minigame(BbvsEngine *vm)
	: _vm(vm), _spriteModule(nullptr) {

	memset(_hiScoreTable, 0, sizeof(_hiScoreTable));
	_gameState = 0;
	_gameTicks = 0;
	_gameResult = false;
	_gameDone = false;
	_fromMainGame = false;
	_backgroundSpriteIndex = 0;
	_titleScreenSpriteIndex = 0;
	_numbersAnim = nullptr;
}

Minigame::~Minigame() {
}

int Minigame::drawNumber(DrawList &drawList, int number, int x, int y) {
	int digits = 1, rightX = x;

	for (int mag = 10; number / mag != 0; mag *= 10)
		++digits;

	rightX = x + digits * 10;
	x = rightX;

	while (digits--) {
		const int n = number % 10;
		x -= 10;
		drawList.add(_numbersAnim->frameIndices[n], x, y, 2000);
		number /= 10;
	}

	return rightX;
}

void Minigame::playSound(uint index, bool loop) {
	if (index > 0)
		_vm->_sound->playSound(index - 1, loop);
}

void Minigame::stopSound(uint index) {
	if (index > 0)
		_vm->_sound->stopSound(index - 1);
}

bool Minigame::isSoundPlaying(uint index) {
	return index > 0 && _vm->_sound->isSoundPlaying(index - 1);
}

bool Minigame::isAnySoundPlaying(const uint *indices, uint count) {
	for (uint i = 0; i < count; ++i)
		if (isSoundPlaying(indices[i]))
			return true;
	return false;
}

void Minigame::saveHiscore(int minigameNum, int score) {
	Common::String filename = _vm->getTargetName() + "-highscore.dat";
	Common::OutSaveFile *file = g_system->getSavefileManager()->openForSaving(filename);
	if (file) {
		// Reserve a byte for future usage (rarely a bad idea, you never know...)
		file->writeByte(0);
		_hiScoreTable[minigameNum] = score;
		for (int i = 0; i < kMinigameCount; ++i)
			file->writeUint32LE(_hiScoreTable[i]);
		delete file;
	}
}

int Minigame::loadHiscore(int minigameNum) {
	int score = 0;
	Common::String filename = _vm->getTargetName() + "-highscore.dat";
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(filename);
	if (file) {
		file->readByte();
		for (int i = 0; i < kMinigameCount; ++i)
			_hiScoreTable[i] = file->readUint32LE();
		delete file;
		score = _hiScoreTable[minigameNum];
	}
	return score;
}

} // End of namespace Bbvs
