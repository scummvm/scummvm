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

#ifndef DGDS_SCENE_H
#define DGDS_SCENE_H

#include "common/stream.h"
#include "common/array.h"

namespace Dgds {

struct SceneStruct1 {
    uint16 val1;
    uint16 flags; /* eg, see usage in FUN_1f1a_2106 */
    uint16 val3;
};

struct Rect {
    int x;
    int y;
    int width;
    int height;
};

struct SceneStruct2 {
    struct Rect rect;
    uint16 field1_0x8;
    uint16 field2_0xa;
    Common::Array<struct SceneStruct1> struct1List;
    Common::Array<struct SceneStruct5> struct5List1;
    Common::Array<struct SceneStruct5> struct5List2;
    Common::Array<struct SceneStruct5> struct5List3;
};

struct SceneStruct5 {
    Common::Array<struct SceneStruct1> struct1List;
    Common::Array<uint16> uintList;
    uint16 val;
};

struct SceneStruct2_Extended {
    struct Rect rect;
    uint16 field1_0x8;
    uint16 field2_0xa;
    Common::Array<struct SceneStruct1> struct1List;
    Common::Array<struct SceneStruct5> struct5List1;
    Common::Array<struct SceneStruct5> struct5List2;
    Common::Array<struct SceneStruct5> struct5List3;
    //struct SceneStruct2 *next;
    Common::Array<struct SceneStruct5> field8_0x1c; /* this field on are only in Extended (GDS) version */
    Common::Array<struct SceneStruct5> field9_0x20;
    uint16 field10_0x24;
    uint16 field11_0x26;
    uint16 field12_0x28;
    uint16 field13_0x2a;
    uint16 field14_0x2c;
};

struct SceneStruct3 {
    uint16 val1;
    uint16 val2;
    uint16 val3; /* Not set in loader? */
};

struct SceneStruct4 {
    uint16 val1;
    uint16 val2;
    Common::Array<struct SceneStruct5> struct5List;
};

struct Dialogue {
    uint16 num;
    Rect rect;
    uint16 bgColor;
    uint16 fontColor;
    uint16 field7_0xe;
    uint16 field8_0x10;
    uint16 fontSize;
    uint32 flags; // includes justify 
    uint16 frametype;
    uint16 field12_0x1a;
    uint16 field13_0x1c;
	Common::Array<struct DialogueSubstring> subStrings;
    uint16 field15_0x22;
    Common::String str;
    uint16  field18_0x28;
};

struct SceneStruct7 {
    uint16 val;
    int16 field1_0x2;
    Common::Array<struct SceneStruct1> struct1List;
    Common::Array<struct SceneStruct5> struct5List;
};

struct DialogueSubstring {
    uint16 strOff1;  // The game initializes these to pointers, but let's be a bit nicer.
    uint16 strOff2;
    byte unk[8]; /* Not initialized in loader */
    Common::Array<struct SceneStruct5> struct5List;
    uint val; /* First entry initialized to 1 in loader */
};


/**
 * A scene is described by an SDS file, which points to the ADS script to load
 * and holds the dialogue info.
 */
class Scene {
public:
	Scene();

	bool parseSDS(Common::SeekableReadStream *s);
	bool isVersionOver(const char *version);
	bool isVersionUnder(const char *version);

	const Common::Array<struct Dialogue> &getLines() const { return _dialogues; }

private:
	bool readStruct1List(Common::SeekableReadStream *s, Common::Array<SceneStruct1> &list);
	bool readStruct2List(Common::SeekableReadStream *s, Common::Array<SceneStruct2> &list);
	bool readStruct4List(Common::SeekableReadStream *s, Common::Array<SceneStruct4> &list);
	bool readStruct5List(Common::SeekableReadStream *s, Common::Array<SceneStruct5> &list);
	bool readDialogueList(Common::SeekableReadStream *s, Common::Array<Dialogue> &list);
	bool readStruct7List(Common::SeekableReadStream *s, Common::Array<SceneStruct7> &list);
	bool readDialogSubstringList(Common::SeekableReadStream *s, Common::Array<DialogueSubstring> &list);

	uint32 _magic;
	Common::String _version;

	int _num;
    Common::Array<struct SceneStruct5> _struct5List1;
    Common::Array<struct SceneStruct5> _struct5List2;
    Common::Array<struct SceneStruct5> _struct5List3;
    Common::Array<struct SceneStruct5> _struct5List4;
    uint _field5_0x12;
    uint _field6_0x14;
    Common::String _adsFile;
    uint _field8_0x23;
    Common::Array<struct SceneStruct2> _struct2List;
    Common::Array<struct SceneStruct4> _struct4List1;
    Common::Array<struct SceneStruct4> _struct4List2;
    uint _field12_0x2b;
    Common::Array<struct Dialogue> _dialogues;
    Common::Array<struct SceneStruct7> _struct7List;
    uint _field15_0x33;
};

} // End of namespace Dgds

#endif // DGDS_SCENE_H
