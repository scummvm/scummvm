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

#include "common/macresman.h"

#include "common/str-enc.h"
#include "common/stream.h"
#include "fool/fool.h"
#include "fool/toolbox.h"
#include "fool/zbasic.h"

namespace Fool {

void ZBasic::loadProgram(const Common::Path &path) {
	Common::MacResManager resman;
	if (!resman.open(path)) {
		warning("ZBasic::loadProgram: unable to open %s", path.toString(':').c_str());
		return;
	}
	Common::SeekableReadStream *scot = resman.getResource(MKTAG('S', 'C', 'O', 'T'), 1);
	if (!scot) {
		warning("ZBasic::loadProgram: could not find SCOT chunk");
		return;
	}

	Common::Array<ZBasicDatum> dataTable;
	Common::Array<ZBasicDatum> stringTable;
	bool hasStringTable = false;
	while (scot->pos() < scot->size()) {
		uint32 offset = scot->pos();
		uint8 opcode = scot->readByte();
		if (opcode == 0x80) {
			hasStringTable = true;
			break;
		}
		switch (opcode) {
		case kDatumNULL:
			dataTable.push_back(ZBasicDatum::newNull(offset));
			break;
		case kDatumINT:
			{
				uint8 size = scot->readByte();
				if (size == 0) {
					dataTable.push_back(ZBasicDatum::newInt(offset, 0));
				} else if (size == 2) {
					dataTable.push_back(ZBasicDatum::newInt(offset, scot->readSint16BE()));
				} else {
					warning("ZBasic::loadProgram: unexpected size for int %d", size);
					scot->skip(size);
				}
			}
			break;
		case kDatumDBLINT:
			{
				uint8 size = scot->readByte();
				if (size == 0) {
					dataTable.push_back(ZBasicDatum::newDblInt(offset, 0));
				} else if (size == 2) {
					dataTable.push_back(ZBasicDatum::newDblInt(offset, scot->readSint32BE()));
				} else {
					warning("ZBasic::loadProgram: unexpected size for dblint %d", size);
					scot->skip(size);
				}
			}
			break;
		case kDatumSTR:
			{
				uint8 size = scot->readByte();
				char *buf = new char[size];
				scot->read(buf, size);
				dataTable.push_back(ZBasicDatum::newStr(offset, Common::convertToU32String(buf, Common::kMacRoman)));
				delete[] buf;
			}
			break;
		case kDatumBCD:
			{
				scot->seek(-1);
				dataTable.push_back(ZBasicDatum::newBcd(offset, ZBasicBCD::read(scot)));
			}
			break;
		default:
			break;
		}
	}

	delete scot;

}

int16 ZBasic::instr(int16 expression, const Common::U32String &string1, const Common::U32String &string2) {
	uint32 result =  string1.find(string2, (uint32)MAX<int16>(expression - 1, 1));
	if (result == Common::U32String::npos)
		return 0;
	return (int16)(result + 1);
}

bool ZBasic::maybe() {
	return g_engine->getRandomNumber(1) == 1;
}

uint32 ZBasic::mem(int16 index) {
	if (index == -1)
		return 32*1024*1024;
	return 0;
}

int16 ZBasic::rndInt(int16 max) {
	if (max == 0)
		return 0;
	max = (int16)abs(MAX<int16>(max, -32767));
	return (int16)(g_engine->getRandomNumber(max - 1) + 1);
}


} // namespace Fool
