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

#include "director/lingo/lingo.h"

namespace Director {

static struct LingoV4Bytecode {
    const char opcode;
    const inst func;
} lingoV4[] = {
    { 0x03, Lingo::c_voidpush },
    { 0x04, Lingo::c_mul },
    { 0x05, Lingo::c_add },
    { 0x06, Lingo::c_sub },
    { 0x07, Lingo::c_div },
    { 0x08, Lingo::c_mod },
    { 0x09, Lingo::c_negate },
    { 0x0a, Lingo::c_ampersand },
    { 0x0b, Lingo::c_concat },
    { 0x0c, Lingo::c_lt },
    { 0x0d, Lingo::c_le },
    { 0x0e, Lingo::c_neq },
    { 0x0f, Lingo::c_eq },
    { 0x10, Lingo::c_gt },
    { 0x11, Lingo::c_ge },
    { 0x12, Lingo::c_and },
    { 0x13, Lingo::c_or },
    { 0x14, Lingo::c_not },
    { 0x15, Lingo::c_contains },
    { 0x16, Lingo::c_starts },
    { 0x17, Lingo::c_of },
    { 0x18, Lingo::c_hilite }, 
    { 0x19, Lingo::c_intersects },
    { 0x1a, Lingo::c_within },
    { 0x1b, Lingo::c_field },
    { 0x1c, Lingo::c_tell },
    { 0x1d, Lingo::c_telldone },
    
    { 0x41, Lingo::c_intpush },
    { 0, 0 }
};


void Lingo::addCodeV4(Common::SeekableSubReadStreamEndian &stream, ScriptType type, uint16 id) { 
	debugC(1, kDebugLingoCompile, "Add V4 bytecode for type %s with id %d", scriptType2str(type), id);

    // unk1
    for (uint32 i = 0; i < 0x10; i++) {
        stream.readByte();
    }
    uint16 code_store_offset = stream.readUint16();
    // unk2
    for (uint32 i = 0; i < 0x2e; i++) {
        stream.readByte();
    }
    uint16 functions_offset = stream.readUint16();
    // unk3
    for (uint32 i = 0; i < 0x6; i++) {
        stream.readByte();
    }
    uint16 functions_count = stream.readUint16();
    uint16 unk4 = stream.readUint16();
    uint16 unk5 = stream.readUint16();
    uint16 consts_count = stream.readUint16();
    stream.readUint16();
    uint16 consts_offset = stream.readUint16();
    stream.readUint16();
    uint16 unk6 = stream.readUint16();
    stream.readUint16();
    uint16 consts_base = stream.readUint16();
    
    // preload all the constants
    stream.seek(consts_offset);
}

}
