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
 * $URL$
 * $Id$
 *
 */

#ifndef GROOVIE_ROQ_H
#define GROOVIE_ROQ_H

#include "groovie/player.h"

namespace Groovie {

class GroovieEngine;

struct ROQBlockHeader {
	uint16 type;
	uint32 size;
	uint16 param;
};

class ROQPlayer : public VideoPlayer {
public:
	ROQPlayer(GroovieEngine *vm);
	~ROQPlayer();

protected:
	uint16 loadInternal();
	bool playFrameInternal();

private:
	bool readBlockHeader(ROQBlockHeader &blockHeader);

	bool processBlock();
	bool processBlockInfo(ROQBlockHeader &blockHeader);
	bool processBlockQuadCodebook(ROQBlockHeader &blockHeader);
	bool processBlockQuadVector(ROQBlockHeader &blockHeader);
	bool processBlockQuadVectorSub(ROQBlockHeader &blockHeader);
	bool processBlockStill(ROQBlockHeader &blockHeader);
	bool processBlockSoundMono(ROQBlockHeader &blockHeader);
	bool processBlockSoundStereo(ROQBlockHeader &blockHeader);
	bool processBlockAudioContainer(ROQBlockHeader &blockHeader);

	uint16 _num2blocks;
	uint16 _num4blocks;
};

} // End of Groovie namespace

#endif // GROOVIE_ROQ_H
