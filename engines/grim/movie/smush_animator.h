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

#ifndef GRIM_MOVIE_SMUSH_ANIMATOR_H
#define GRIM_MOVIE_SMUSH_ANIMATOR_H

#include "common/scummsys.h"

namespace Grim {

class SmushDecoder;

class SmushAnimator {

private:
	SmushDecoder *_decoder;
	byte *_buffer;
	uint32 _bufferSize;
	byte *_block;
	int _blockStatus;
	int _frame;
	int _frameCount;
	int _blockSize;
	uint32 _tag;
	int _fileMode;
	int _flags;
	int _seeking;
	int _skipping;
	int _skipStart;
	int _skipBase;
	int _skipEnd;
	int _quit;
	int _pause;

	void readBlock(int size);
	int checkSkip();
	void decodeBlocks(int flags);
	bool moreData();

public:
	SmushAnimator(SmushDecoder *decoder);
	~SmushAnimator();

	int animatorInitialize();
	int animatorNewAnim(int flags, int skipLength, int skipUntil);
	int animatorRender(int flags);
	int animatorQuery();
	void animatorQuit();
	int animatorPause(int mode);

};

} // End of namespace Grim
#endif
