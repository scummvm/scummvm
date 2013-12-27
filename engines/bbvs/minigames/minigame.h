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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BBVS_MINIGAMES_MINIGAME_H
#define BBVS_MINIGAMES_MINIGAME_H

#include "bbvs/bbvs.h"
#include "bbvs/graphics.h"
#include "bbvs/sound.h"
#include "bbvs/spritemodule.h"

namespace Bbvs {

struct ObjAnimation {
	int frameCount;
	const int *frameIndices;
	const int16 *frameTicks;
	const BBRect *frameRects;
};

class Minigame {
public:
	Minigame(BbvsEngine *vm);
	virtual ~Minigame();
	virtual int run(uint flags) = 0;
public:
	BbvsEngine *_vm;	
	SpriteModule *_spriteModule;
	
	int _gameState;
	int _gameTicks;
	int _gameResult;
	bool _gameDone;
	bool _fromMainGame;
	
	int _backgroundSpriteIndex, _titleScreenSpriteIndex;
	
	const ObjAnimation *_numbersAnim;
	
	int drawNumber(DrawList &drawList, int number, int x, int y);

	void playSound(uint index, bool loop = false);
	void stopSound(uint index);
	bool isSoundPlaying(uint index);
	bool isAnySoundPlaying(const uint *indices, uint count);

};

} // End of namespace Bbvs

#endif // BBVS_H
