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
 * This is a utility for extracting needed resource data from different language
 * version of the Mortevielle executable files into a new file mort.dat - this
 * is required for the ScummVM Mortevielle module to work properly
 */

#define VERSION_MAJOR 1
#define VERSION_MINOR 2

enum AccessMode {
	kFileReadMode = 1,
	kFileWriteMode = 2
};

enum DataType {
	kStaticStrings = 0,
	kGameStrings = 1,
	kEncryptionArrays = 2
};

#define MORT_DAT "mort.dat"

class File {
private:
	FILE *f;
public:
	bool open(const char *filename, AccessMode mode = kFileReadMode);
	void close();
	int seek(int32 offset, int whence = SEEK_SET);
	uint32 pos();
	long read(void *buffer, int len);
	void write(const void *buffer, int len);

	byte readByte();
	uint16 readWord();
	uint32 readLong();
	void writeByte(byte v);
	void writeWord(uint16 v);
	void writeLong(uint32 v);
	void writeString(const char *s);
};

File outputFile, mortCom;

