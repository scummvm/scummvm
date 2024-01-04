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

#include "dgds/scene.h"

namespace Dgds {

Scene::Scene() : _magic(0) {
}

bool Scene::isVersionOver(const char *version) const {
	return strncmp(_version.c_str(), version, _version.size()) > 0;
}

bool Scene::isVersionUnder(const char *version) const {
	return strncmp(_version.c_str(), version, _version.size()) < 0;
}


bool Scene::readStruct1List(Common::SeekableReadStream *s, Common::Array<SceneStruct1> &list) const {
	list.resize(s->readUint16LE());
	for (SceneStruct1 &dst : list) {
		dst.val1 = s->readUint16LE();
		dst.flags = s->readUint16LE();
		dst.val3 = s->readUint16LE();
	}
	return !s->err();
}


bool Scene::readStruct2(Common::SeekableReadStream *s, SceneStruct2 &dst) const {
	dst.rect.x = s->readUint16LE();
	dst.rect.y = s->readUint16LE();
	dst.rect.width = s->readUint16LE();
	dst.rect.height = s->readUint16LE();
	dst.field1_0x8 = s->readUint16LE();
	dst.field2_0xa = s->readUint16LE();
	readStruct1List(s, dst.struct1List);
	readStruct5List(s, dst.struct5List1);
	readStruct5List(s, dst.struct5List2);
	readStruct5List(s, dst.struct5List3);
	return !s->err();
}


bool Scene::readStruct2List(Common::SeekableReadStream *s, Common::Array<SceneStruct2> &list) const {
	list.resize(s->readUint16LE());
	for (SceneStruct2 &dst : list) {
		readStruct2(s, dst);
	}
	return !s->err();
}


bool Scene::readStruct2ExtendedList(Common::SeekableReadStream *s, Common::Array<SceneStruct2_Extended> &list) const {
	list.resize(s->readUint16LE());
	for (SceneStruct2_Extended &dst : list) {
		readStruct2(s, dst);
	}
	for (SceneStruct2_Extended &dst : list) {
		dst.field11_0x26 = s->readUint16LE();
		dst.field12_0x28 = s->readUint16LE();
		dst.field14_0x2c = s->readUint16LE();
		if (!isVersionUnder(" 1.211"))
			dst.field13_0x2a = s->readUint16LE() & 0xfffe;
		if (!isVersionUnder(" 1.204")) {
			dst.field10_0x24 = s->readUint16LE();
			readStruct5List(s, dst.struct5List5);
			readStruct5List(s, dst.struct5List6);
		}
	}
	return !s->err();

}


bool Scene::readStruct3List(Common::SeekableReadStream *s, Common::Array<SceneStruct3> &list) const {
	list.resize(s->readUint16LE());
	for (SceneStruct3 &dst : list) {
		dst.val1 = s->readUint16LE();
		dst.val2 = s->readUint16LE();
	}
	return !s->err();
}


bool Scene::readStruct4List(Common::SeekableReadStream *s, Common::Array<SceneStruct4> &list) const {
	list.resize(s->readUint16LE());
	for (SceneStruct4 &dst : list) {
		if (!isVersionOver(" 1.205")) {
			dst.val2 = s->readUint16LE();
			dst.val1 = s->readUint16LE();
			dst.val2 += s->readUint16LE();
		} else {
			dst.val1 = s->readUint16LE();
			dst.val2 = s->readUint16LE();
		}
		readStruct5List(s, dst.struct5List);
	}
	return !s->err();
}


bool Scene::readStruct5List(Common::SeekableReadStream *s, Common::Array<SceneStruct5> &list) const {
	list.resize(s->readUint16LE());
	for (SceneStruct5 &dst : list) {
		readStruct1List(s, dst.struct1List);
		dst.val = s->readUint16LE();
		int nvals = s->readUint16LE();
		for (int i = 0; i < nvals / 2; i++) {
			dst.uintList.push_back(s->readUint16LE());
		}
	}

	return !s->err();
}


bool Scene::readDialogueList(Common::SeekableReadStream *s, Common::Array<Dialogue> &list) const {
	list.resize(s->readUint16LE());
	for (Dialogue &dst : list) {
		dst.num = s->readUint16LE();
		dst.rect.x = s->readUint16LE();
		dst.rect.y = s->readUint16LE();
		dst.rect.width = s->readUint16LE();
		dst.rect.height = s->readUint16LE();
		dst.bgColor = s->readUint16LE();
		dst.fontColor = s->readUint16LE(); // 0 = black, 0xf = white
		if (isVersionUnder(" 1.209")) {
			dst.field7_0xe = dst.bgColor;
			dst.field8_0x10 = dst.fontColor;
		} else {
			dst.field7_0xe = s->readUint16LE();
			dst.field8_0x10 = s->readUint16LE();
		}
		dst.fontSize = s->readUint16LE(); // 01 = 8x8, 02 = 6x6, 03 = 4x5
		if (isVersionUnder(" 1.210")) {
			dst.flags = s->readUint16LE();
		} else {
			// Game reads a 32 bit int but then truncates anyway..
			// probably never used the full thing.
			dst.flags = (s->readUint32LE() & 0xffff);
		}
		
		dst.frametype = s->readUint16LE(); // 01 =simple frame, 02 = with title w/ text before :, 03 = baloon, 04 = eliptical
		dst.field12_0x1a = s->readUint16LE();
		if (isVersionOver(" 1.207")) {
			dst.maybeNextDialogNum = s->readUint16LE();
		}
		
		uint16 nbytes = s->readUint16LE();
		if (nbytes > 0) {
			dst.str = s->readString('\0', nbytes);
		} else {
			dst.str.clear();
		}
		readDialogSubstringList(s, dst.subStrings);
		
		if (isVersionUnder(" 1.209") && !dst.subStrings.empty()) {
			if (dst.fontColor == 0)
				dst.field8_0x10 = 4;
			else if (dst.fontColor == 0xff)
				dst.fontColor = 7;
			else
				dst.fontColor = dst.fontColor ^ 8;
		}
	}

	return !s->err();
}


bool Scene::readStruct7List(Common::SeekableReadStream *s, Common::Array<SceneStruct7> &list) const {
	list.resize(s->readUint16LE());
	for (SceneStruct7 &dst : list) {
		dst.val = s->readUint16LE();
		readStruct1List(s, dst.struct1List);
		readStruct5List(s, dst.struct5List);
	}

	return !s->err();
}


bool Scene::readDialogSubstringList(Common::SeekableReadStream *s, Common::Array<DialogueSubstring> &list) const {
	list.resize(s->readUint16LE());
	
	if (!list.empty())
		list[0].val = 1;
	
	for (DialogueSubstring &dst : list) {
		dst.strOff1 = s->readUint16LE();
		dst.strOff2 = s->readUint16LE();
		readStruct5List(s, dst.struct5List);
	}

	return !s->err();
}


SDSScene::SDSScene() : _num(-1) {
}

bool SDSScene::parse(Common::SeekableReadStream *stream) {
	_magic = stream->readUint32LE();
	_version = stream->readString();
	if (isVersionOver(" 1.211")) {
	//if (isVersionOver(" 1.216")) { // HoC
	//if (isVersionOver(" 1.224")) { // Beamish
		error("Unsupported scene version '%s'", _version.c_str());
	}
	_num = stream->readUint16LE();
	readStruct5List(stream, _struct5List1);
	readStruct5List(stream, _struct5List2);
	if (isVersionOver(" 1.206")) {
		readStruct5List(stream, _struct5List3);
	}
	readStruct5List(stream, _struct5List4);
	_field6_0x14 = stream->readUint16LE();
	_adsFile = stream->readString();
	readStruct2List(stream, _struct2List);
	readStruct4List(stream, _struct4List1);
	if (isVersionOver(" 1.205")) {
		readStruct4List(stream, _struct4List2);
	}
	readDialogueList(stream, _dialogues);
	if (isVersionOver(" 1.203")) {
		readStruct7List(stream, _struct7List);
	}

	return !stream->err();
}


GDSScene::GDSScene() {
}


bool GDSScene::parseInf(Common::SeekableReadStream *s) {
	_magic = s->readUint32LE();
	_version = s->readString();
	return !s->err();
}


bool GDSScene::parse(Common::SeekableReadStream *stream) {
	readStruct5List(stream, _struct5List1);
	readStruct5List(stream, _struct5List2);
	if (isVersionOver(" 1.206"))
		readStruct5List(stream, _struct5List3);
	readStruct5List(stream, _struct5List4);
	if (isVersionOver(" 1.208"))
		readStruct5List(stream, _struct5List5);
	Common::Array<struct SceneStruct1> struct1List;
	readStruct1List(stream, struct1List);
	Common::Array<struct SceneStruct3> struct3List;
	_iconFile = stream->readString();
	readStruct3List(stream, struct3List);
	readStruct2ExtendedList(stream, _struct2ExtList);
	readStruct4List(stream, _struct4List2);
	if (isVersionOver(" 1.205"))
		readStruct4List(stream, _struct4List1);
	
	return !stream->err();
}


} // End of namespace Dgds

