/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef PACKFILE_H
#define PACKFILE_H

namespace Common {
class SeekableReadStream;
}

namespace Grim {

class PackFile : public Common::SeekableReadStream {
public:
	PackFile(Common::SeekableReadStream *data);
	~PackFile();

	bool err() const;
	void clearErr();
	uint32 read(void *dataPtr, uint32 dataSize);
	bool eos() const;
	int32 pos() const;
	int32 size() const;
	bool seek(int32 offset, int whence = SEEK_SET);

private:
	Common::SeekableReadStream *_orgStream;
	int32 _offset, _size;

	uint16 *_codeTable;
	const uint32 _kCodeTableSize;

	void createCodeTable(uint32 key);
	void decode(uint8 *data, uint32 size, uint32 start_point);

	static const uint32 _knownOffsets[5];
};

} // end of namespace Grim

#endif
