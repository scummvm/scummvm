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

#ifndef SCUMM_FILE_NES_H
#define SCUMM_FILE_NES_H

#include "scumm/file.h"
#include "common/array.h"

namespace Scumm {

class ScummNESFile : public BaseScummFile {
public:
	enum ROMset {
		kROMsetUSA,
		kROMsetEurope,
		kROMsetSweden,
		kROMsetFrance,
		kROMsetGermany,
		kROMsetSpain,
		kROMsetItaly,
		kROMsetNum
	};

	struct Resource;
	struct ResourceGroup;
	struct LFLEntry;
	struct LFL;

	enum ResType {
		NES_UNKNOWN,
		NES_GLOBDATA,
		NES_ROOM,
		NES_SCRIPT,
		NES_SOUND,
		NES_COSTUME,
		NES_ROOMGFX,
		NES_COSTUMEGFX,
		NES_SPRPALS,
		NES_SPRDESC,
		NES_SPRLENS,
		NES_SPROFFS,
		NES_SPRDATA,
		NES_CHARSET,
		NES_PREPLIST,
		NES_TITLES,
		NES_TITLE2_SPARKLECHR,
		NES_TITLE2_SPARKLEPAL,
	};


	struct NESTitleScreen {
		uint16 unk1;
		uint16 unk2;
		byte numberOfTiles;
		Common::Array<byte> gfx;
		uint16 unk3;
		byte unk4;
		byte width;
		byte height;
		Common::Array<byte> nametable;
		uint16 unk5;
		byte unk6;
		byte attrWidth;
		byte attrHeight;
		Common::Array<byte> attributes;
		byte stepNum;
		Common::Array<byte> palette;
		byte endOfData;
	};

	bool decodeTitleScreen(uint titleIndex, NESTitleScreen &outTitle);
	bool readTitle2SparkleChr(Common::Array<byte> &outChr);
	bool readTitle2SparklePalette(Common::Array<byte> &outPalette);


private:
	Common::SeekableReadStream *_stream;
	ROMset _ROMset;
	byte *_buf;

	bool generateIndex();
	bool generateResource(int res);
	uint16 extractResource(Common::WriteStream *out, const Resource *res, ResType type);

	byte fileReadByte();
	void decodeTitleRLE(Common::Array<byte> &dst, uint32 expectedSize);
	uint16 fileReadUint16LE();

public:
	ScummNESFile();

	bool open(const Common::Path &filename) override;
	bool openSubFile(const Common::Path &filename) override;

	void close() override;
	bool eos() const override { return _stream->eos(); }
	int64 pos() const override { return _stream->pos(); }
	int64 size() const override { return _stream->size(); }
	bool seek(int64 offs, int whence = SEEK_SET) override { return _stream->seek(offs, whence); }
	uint32 read(void *dataPtr, uint32 dataSize) override;
};

} // End of namespace Scumm

#endif
