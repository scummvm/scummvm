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

#ifndef CREATE_NANCY_FILE_H
#define CREATE_NANCY_FILE_H

#include <stdio.h>
#include <stdlib.h>

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "types.h"

enum AccessMode {
	kFileReadMode = 1,
	kFileWriteMode = 2
};

class File {
private:
	FILE *_f;
	const byte *_memPtr;
	size_t _offset, _size;

public:
	File() : _f(nullptr), _memPtr(nullptr), _offset(0), _size(0) {}

	bool open(const char *filename, AccessMode mode = kFileReadMode);
	bool open(const byte *data, uint size);

	void close();

    uint pos() const;
    uint size() const;
    bool eof() const;

	int seek(int offset, int whence = SEEK_SET);
	void skip(int offset);
	long read(void *buffer, size_t len);
	byte readByte();

	void write(const void *buffer, size_t len);
	void writeByte(byte v);
	void writeByte(byte v, int len);
	void writeUint16(uint16 v);
	void writeUint32(uint v);
	void writeString(const char *msg);
};

template <class T>
void writeToFile(File &file, T &obj) {
    file.write(&obj, sizeof(obj));
}

template<class T>
void writeToFile(File &file, const Common::Array<T> &obj) {
    file.writeUint16(obj.size());
    for (uint i = 0; i < obj.size(); ++i) {
        writeToFile(file, obj[i]);
    }
}

template<>
void writeToFile(File &file, const Common::Array<const char *> &obj);
template<>
void writeToFile(File &file, const EventFlagDescription &obj);
template<>
void writeToFile(File &file, const SceneChangeDescription &obj);
template<>
void writeToFile(File &file, const ConditionalDialogue &obj);
template<>
void writeToFile(File &file, const GoodbyeSceneChange &obj);
template<>
void writeToFile(File &file, const Goodbye &obj);
template<>
void writeToFile(File &file, const Hint &obj);

void writeMultilangArray(File &file, const Common::Array<Common::Array<const char *>> &array);

#endif // CREATE_NANCY_FILE_H
