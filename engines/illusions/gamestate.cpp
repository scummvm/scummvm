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

#include "illusions/gamestate.h"

namespace Illusions {

GameState::GameState()
	: _writeBufferSize(0), _writeBuffer(0), _readStream(0) {
}

GameState::~GameState() {
	free(_writeBuffer);
}

bool GameState::readState(uint32 &sceneId, uint32 &threadId) {
	sceneId = _readStream->readUint32LE();
	threadId = _readStream->readUint32LE();
	return readStateInternal(_readStream);
}

void GameState::writeState(uint32 sceneId, uint32 threadId) {
	Common::WriteStream *writeStream = newWriteStream();
	writeStream->writeUint32LE(sceneId);
	writeStream->writeUint32LE(threadId);
	writeStateInternal(writeStream);
	delete writeStream;
}

void GameState::read(Common::ReadStream *in) {
	uint32 size = in->readUint32LE();
	_readStream = in->readStream(size);
}

void GameState::write(Common::WriteStream *out) {
	out->writeUint32LE(_writeBufferSize);
	out->write(_writeBuffer, _writeBufferSize);
}

void GameState::deleteReadStream() {
	delete _readStream;
	_readStream = 0;
}

Common::WriteStream *GameState::newWriteStream() {
	if (_writeBufferSize == 0 || !_writeBuffer) {
		_writeBufferSize = calcWriteBufferSize();
		_writeBuffer = (byte*)malloc(_writeBufferSize);
	}
	return new Common::MemoryWriteStream(_writeBuffer, _writeBufferSize);
}

uint32 GameState::calcWriteBufferSize() {
	return calcWriteBufferSizeInternal() + 4 + 4;
}

} // End of namespace Illusions
