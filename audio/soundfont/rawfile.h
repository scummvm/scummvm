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
 */
/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included VGMTrans_LICENSE.txt file
 */

#ifndef AUDIO_SOUNDFONT_RAWFILE_H
#define AUDIO_SOUNDFONT_RAWFILE_H

#include "common/stream.h"
#include "common/str.h"

class VGMFile;

class RawFile {
public:
	virtual ~RawFile() {};

	virtual size_t size() const = 0;

	bool IsValidOffset(uint32 ofs) { return ofs < size(); }

	const char *begin() const { return data(); }
	const char *end() { return data() + size(); }
	virtual const char *data() const = 0;

	virtual uint8 GetByte(size_t offset) const = 0;
	virtual uint16 GetShort(size_t offset) const = 0;
	virtual uint32 GetWord(size_t offset) const = 0;

	uint32 GetBytes(size_t offset, uint32 nCount, void *pBuffer) const;

private:
};

class MemFile : public RawFile {
private:
	Common::SeekableReadStream *_seekableReadStream;
	const byte *_data;

public:
	MemFile(const byte *data, uint32 size);
	~MemFile() override;

	const char *data() const override;

	uint8 GetByte(size_t offset) const override;
	uint16 GetShort(size_t offset) const override;
	uint32 GetWord(size_t offset) const override;

	size_t size() const override;
};

#endif // AUDIO_SOUNDFONT_RAWFILE_H
