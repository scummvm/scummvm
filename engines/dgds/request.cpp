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

#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"

#include "dgds/includes.h"
#include "dgds/request.h"
#include "dgds/resource.h"

namespace Dgds {

Request::Request(ResourceManager *resman) : DgdsParser(resman) {
}


bool Request::parseGADChunk(RequestData &data, DgdsChunk &chunk, int num) {
	Common::SeekableReadStream *str = chunk._stream;

	uint16 numGadgets = str->readUint16LE();
	data._gadgets.resize(numGadgets);
	for (Common::SharedPtr<Gadget> &gptr : data._gadgets) {
		uint16 vals[12];
		for (int i = 0; i < 12; i++)
			vals[i] = str->readUint16LE();

		uint16 gadgetTypeFlag = vals[5];
		if (num == 0 || num == vals[0]) {
			if (gadgetTypeFlag == 1)
				gptr.reset(new Gadget1());
			else if (gadgetTypeFlag == 2)
				gptr.reset(new Gadget2());
			else if (gadgetTypeFlag == 8)
				gptr.reset(new Gadget8());
			else
				gptr.reset(new Gadget());
		}

		gptr->_gadgetNo = vals[0];
		gptr->_x = vals[1];
		gptr->_y = vals[2];
		gptr->_width = vals[3];
		gptr->_height = vals[4];
		gptr->_gadgetType = vals[5];
		gptr->_flags2 = vals[6];
		gptr->_flags3 = vals[7];
		gptr->_field14_0x20 = vals[8];
		gptr->_field15_0x22 = vals[9];
		gptr->_field15_0x22 = vals[10];
		gptr->_field16_0x24 = vals[11];
		gptr->_parentX = data._x;
		gptr->_parentY = data._y;

		uint16 type1 = str->readUint16LE();
		if (type1 == 1) {
			Common::String s = str->readString();
			if (gptr)
				gptr->_sval1S = s;
		} else {
			uint16 i = str->readUint16LE();
			if (gptr)
				gptr->_sval1I = i;
		}

		uint16 type2 = str->readUint16LE();
		if (type2 == 1) {
			Common::String s = str->readString();
			if (gptr)
				gptr->_sval2S = s;
		} else {
			uint16 i = str->readUint16LE();
			if (gptr)
				gptr->_sval2I = i;
		}

		// FIXME: Where is this used?
		/*uint16 type3 = */str->readUint16LE();

		// TODO: In each of these cases, work out the true offsets to these fields.
		// and if they are shared between gadget types.
		switch (gadgetTypeFlag) {
		case 1: {
			uint16 i1 = str->readUint16LE();
			uint16 i2 = str->readUint16LE();
			if (gptr) {
				Gadget1 *g1 = static_cast<Gadget1 *>(gptr.get());
				// TODO: These fields might are actually shared with other gadget types?
				g1->_gadget1_i1 = i1;
				g1->_gadget1_i2 = i2;
			}
			break;
		}
		case 2: {
			uint16 i1 = str->readUint16LE();
			uint16 i2 = str->readUint16LE();
			uint16 i3 = str->readUint16LE();
			uint16 i4 = str->readUint16LE();
			if (gptr) {
				Gadget2 *g2 = static_cast<Gadget2 *>(gptr.get());
				g2->_gadget2_i1 = i1;
				g2->_gadget2_i2 = i2;
				g2->_gadget2_i3 = i3;
				g2->_gadget2_i4 = i4;
			}
			break;
		}
		case 4: {
			Common::String s = str->readString();
			if (gptr)
				gptr->_sval3 = s;
			break;
		}
		case 8: {
			uint16 i1 = str->readUint16LE();
			uint16 i2 = str->readUint16LE();
			if (gptr) {
				Gadget8 *g8 = static_cast<Gadget8 *>(gptr.get());
				g8->_gadget8_i1 = i1;
				g8->_gadget8_i2 = i2;
			}
			break;
		}
		default:
			break;
		}
	}
	return str->err();
}


bool Request::parseREQChunk(RequestData &data, DgdsChunk &chunk, int num) {
	Common::SeekableReadStream *str = chunk._stream;

	uint16 fileNum = str->readUint16LE();

	if (num != 0 && num != fileNum)
		return true;

	data._fileNum = fileNum;
	data._x = str->readUint16LE();
	data._y = str->readUint16LE();
	for (int i = 0; i < 5; i++)
		data._vals[i] = str->readUint16LE();

	uint16 numStruct1 = str->readUint16LE();
	data._struct1List.resize(numStruct1);
	for (int i = 0; i < numStruct1; i++) {
		RequestStruct1 &dst = data._struct1List[i];
		for (int j = 0; j < 4; j++) {
			dst._vals[j] = str->readUint16LE();
		}
		dst._str = str->readString();
	}

	uint16 numStruct2 = str->readUint16LE();
	data._struct2List.resize(numStruct2);
	for (int i = 0; i < numStruct2; i++) {
		RequestStruct2 &dst = data._struct2List[i];
		for (int j = 0; j < 6; j++) {
			dst._vals[j] = str->readUint16LE();
		}
	}

	return str->err();
}


bool Request::handleChunk(DgdsChunk &chunk, ParserData *data) {
	RequestData &rdata = *(RequestData *)data;
	int num = -1;

	if (chunk._id == ID_REQ)
		parseREQChunk(rdata, chunk, num);
	else if (chunk._id == ID_GAD)
		parseGADChunk(rdata, chunk, num);

	return chunk._stream->err();
}


} // End of namespace Dgds

