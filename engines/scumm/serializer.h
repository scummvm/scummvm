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



#ifndef SCUMM_SERIALIZER_H
#define SCUMM_SERIALIZER_H


#include "common/serializer.h"


namespace Scumm {

class Serializer : public Common::Serializer {
public:
	Serializer(Common::SeekableReadStream *stream, Common::SeekableWriteStream *writeStream)
		: Common::Serializer(stream, writeStream), _seekableSaveStr(writeStream) {
	}
	~Serializer() override {
	}
	int64 pos() const {
		return _loadStream ? _loadStream->pos() : (_saveStream ? _saveStream->pos() : 0);
	}
	int seek(int64 offset, int whence = 0) {
		return _loadStream ? _loadStream->seek(offset, whence) : (_seekableSaveStr ? _seekableSaveStr->seek(offset, whence) : -1);
	}
	int64 size() const {
		return _loadStream ? _loadStream->size() : (_seekableSaveStr ? _seekableSaveStr->size() : 0);
	}

private:
	Common::SeekableWriteStream *_seekableSaveStr;
};


} // End of namespace Audio

#endif // #ifndef SCUMM_CDDA_H
