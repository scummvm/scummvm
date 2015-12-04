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

#ifndef ILLUSIONS_GAMESTATE_H
#define ILLUSIONS_GAMESTATE_H

#include "common/file.h"
#include "common/memstream.h"

namespace Illusions {

class GameState {
public:
	GameState();
	virtual ~GameState();
	bool readState(uint32 &sceneId, uint32 &threadId);
	void writeState(uint32 sceneId, uint32 threadId);
	void read(Common::ReadStream *in);
	void write(Common::WriteStream *out);
	void deleteReadStream();
protected:
	uint32 _writeBufferSize;
	byte *_writeBuffer;
	Common::SeekableReadStream *_readStream;
	Common::WriteStream *newWriteStream();
	uint32 calcWriteBufferSize();
	virtual uint32 calcWriteBufferSizeInternal() = 0;
	virtual bool readStateInternal(Common::ReadStream *in) = 0;
	virtual void writeStateInternal(Common::WriteStream *out) = 0;
};

} // End of namespace Illusions

#endif // ILLUSIONS_GAMESTATE_H
