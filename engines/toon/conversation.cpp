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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, MojoTouch has
 * exclusively licensed this code on March 23th, 2024, to be used in
 * closed-source products.
 * Therefore, any contributions (commits) to it will also be dual-licensed.
 *
 */

#include "toon/conversation.h"

namespace Toon {

void Conversation::save(Common::WriteStream *stream, int16 *conversationDataBase) {
	stream->writeSint32BE(_enable);
	for (int32 i = 0; i < 10; i++) {
		stream->writeSint32BE(state[i]._data2);
		stream->writeSint16BE(state[i]._data3);
		stream->writeSint32BE((int16 *)state[i]._data4 - conversationDataBase);
	}
}

void Conversation::load(Common::ReadStream *stream, int16 *conversationDataBase) {
	_enable = stream->readSint32BE();
	for (int32 i = 0; i < 10; i++) {
		state[i]._data2 = stream->readSint32BE();
		state[i]._data3 = stream->readSint16BE();
		state[i]._data4 = conversationDataBase + stream->readSint32BE();
	}
}

}
