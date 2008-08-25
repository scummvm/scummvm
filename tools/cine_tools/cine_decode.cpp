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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * This is a utility to decode Delphine's Cinematique engine's script files.
 * The script files can be recognized from their extension .prc or .rel.
 * Should at least work with Future Wars and Operation Stealth.
 *
 * Note that this isn't polished code so caveat emptor.
 *
 * TODO: Unify opcode namings between ScummVM's Cine engine and this utility.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "typedefs.h"
#include "endian.h"

static const uint8 *_baseScriptData, *_scriptData;

static uint8 fetchScriptByte() {
	return *_scriptData++;
}

static uint16 fetchScriptWord() {
	uint16 w = READ_BE_UINT16(_scriptData);
	_scriptData += 2;
	return w;
}

static const char *fetchScriptString() {
	const char *str = (const char *)_scriptData;
	_scriptData += strlen(str) + 1;
	return str;
}

//typedef void (*DecodeOpcode)();

static const char *objectField[] = {
	"<null>",
	"x",
	"y",
	"mask",
	"frame",
	"costume",
	"part"
};

static bool decodeOpcode(int isV2) {
	uint8 opcode;
	unsigned int addr = _scriptData - _baseScriptData;
	
	opcode = fetchScriptByte();
	if (opcode == 0) {
		printf("[%04X] (FF) ", addr);
	} else {
		--opcode;
		printf("[%04X] (%02X) ", addr, opcode);
		switch (opcode) {
		case 0x00: {
				uint8 num = fetchScriptByte();
				uint8 field = fetchScriptByte();
				int16 value = fetchScriptWord();
				if (field >= 7) return false;
				printf("SET object[%d].%s, %d", num, objectField[field], value);
			}
			break;
		case 0x01: {
				uint8 num = fetchScriptByte();
				uint8 field = fetchScriptByte();
				uint8 var = fetchScriptByte();
				if (field >= 7) return false;
				printf("LOAD localvars[%d], object[%d].%s", var, num, objectField[field]);
			}		
			break;
		case 0x02: {
				uint8 num = fetchScriptByte();
				uint8 field = fetchScriptByte();
				int16 value = fetchScriptWord();
				if (field >= 7) return false;
				printf("ADD object[%d].%s, %d", num, objectField[field], value);
			}
			break;
		case 0x03: {
				uint8 num = fetchScriptByte();
				uint8 field = fetchScriptByte();
				int16 value = fetchScriptWord();
				if (field >= 7) return false;
				printf("SUB object[%d].%s, %d", num, objectField[field], value);
			}
			break;
		case 0x06: {
				uint8 num = fetchScriptByte();
				uint8 field = fetchScriptByte();
				int16 value = fetchScriptWord();
				if (field >= 7) return false;
				printf("CMP object[%d].%s, %d", num, objectField[field], value);
			}
			break;
		case 0x07: {
				uint8 num = fetchScriptByte();
				int16 x = fetchScriptWord();
				int16 y = fetchScriptWord();
				int16 mask = fetchScriptWord();
				int16 frame = fetchScriptWord();
				printf("SETUP object[%d] x=%d y=%d mask=%d frame=%d", num, x, y, mask, frame);
			}
			break;
		case 0x08: {
				uint8 num = fetchScriptByte();
				int16 x = fetchScriptWord();
				int16 y = fetchScriptWord();
				int16 count = fetchScriptWord();
				int16 index = fetchScriptWord();
				printf("CHECK_COLLISION objects[%d] x=%d y=%d count=%d index=%d", num, x, y, count, index);
			}
			break;
		case 0x09: {
				uint8 var = fetchScriptByte();
				uint8 type = fetchScriptByte();
				if (type == 0) {
					int16 value = fetchScriptWord();
					printf("SET localvars[%d], %d", var, value);
				} else {
					uint8 num = fetchScriptByte();
				 	switch (type) {
					case 1:
						printf("localvars[%d] = localvars[%d]", var, num);
						break;
					case 2:
						printf("localvars[%d] = globalvars[%d]", var, num);
						break;
					case 3:
						printf("localvars[%d] = mouse.x", var);
						break;
					case 4:
						printf("localvars[%d] = mouse.y", var);
						break;
					case 5:
						printf("localvars[%d] = random(%d)", var, num);
						break;
					case 8:
						printf("localvars[%d] = datas[%d].packed_size", var, num);
						break;
					case 9:
						printf("localvars[%d] = datas[%d].unpacked_size", var, num);
						break;
					default:
						return false;
						break;
					}
				}
			}
			break;
		case 0x0A: {
				uint8 var = fetchScriptByte();
				uint8 type = fetchScriptByte();
				if (type == 0) {
					int16 value = fetchScriptWord();
					printf("ADD localvars[%d], %d", var, value);
				} else {
					uint8 var2 = fetchScriptByte();
					printf("ADD localvars[%d], localvars[%d]", var, var2);
				}
			}
			break;
		case 0x0B: {
				uint8 var = fetchScriptByte();
				uint8 type = fetchScriptByte();
				if (type == 0) {
					int16 value = fetchScriptWord();
					printf("SUB localvars[%d], %d", var, value);
				} else {
					uint8 var2 = fetchScriptByte();
					printf("SUB localvars[%d], localvars[%d]", var, var2);
				}
			}
			break;
		case 0x0C: {
				uint8 var = fetchScriptByte();
				uint8 type = fetchScriptByte();
				if (type == 0) {
					int16 value = fetchScriptWord();
					printf("MUL localvars[%d], %d", var, value);
				} else {
					uint8 var2 = fetchScriptByte();
					printf("MUL localvars[%d], localvars[%d]", var, var2);
				}
			}
			break;
		case 0x0D: {
				uint8 var = fetchScriptByte();
				uint8 type = fetchScriptByte();
				if (type == 0) {
					int16 value = fetchScriptWord();
					printf("DIV localvars[%d], %d", var, value);
				} else {
					uint8 var2 = fetchScriptByte();
					printf("DIV localvars[%d], localvars[%d]", var, var2);
				}
			}
			break;
		case 0x0E: {
				uint8 var = fetchScriptByte();
				uint8 type = fetchScriptByte();
				if (type == 0) {
					int16 value = fetchScriptWord();
					printf("CMP localvars[%d], %d", var, value);
				} else {
					uint8 var2 = fetchScriptByte();
					if (type == 1) {
						printf("CMP localvars[%d], localvars[%d]", var, var2);
					} else if (type == 2) {
						printf("CMP localvars[%d], globalvars[%d]", var, var2);
					} else {
						return false;
					}
				}
			}
			break;
		case 0x0F: {
				uint8 num = fetchScriptByte();
				uint8 field = fetchScriptByte();
				int16 var = fetchScriptByte();				
				if (field >= 7) return false;
				printf("SET object[%d].%s, localvars[%d]", num, objectField[field], var);
			}
			break;
		case 0x13: {
				uint8 num = fetchScriptByte();
				printf("ADD_OVERLAY object=%d, type=0", num);
			}
			break;
		case 0x14: {
				uint8 num = fetchScriptByte();
				printf("REMOVE_OVERLAY object=%d, type=0", num);
			}
			break;
		case 0x15: {
				uint8 num = fetchScriptByte();
				printf("ADD_OBJECT_TO_BACKGROUND_LIST objects[%d]", num);
			}
			break;
		case 0x16: {
				uint8 num = fetchScriptByte();
				printf("ADD_OVERLAY object=%d, type=1", num);
			}
			break;
		case 0x17: {
				uint8 num = fetchScriptByte();
				printf("REMOVE_OVERLAY object=%d, type=1", num);
			}
			break;
		case 0x18: {
				uint8 num = fetchScriptByte();
				printf("ADD_OVERLAY object=%d, type=4", num);
			}
			break;
		case 0x19: {
				uint8 num = fetchScriptByte();
				printf("REMOVE_OVERLAY object=%d, type=4", num);
			}
			break;
		case 0x1A: {
				uint8 num = fetchScriptByte();
				printf("ADD_SPRITE_TO_BACKGROUND_LIST objects[%d], type=4", num);
			}
			break;
		case 0x1B: {
				printf("CLEAR_BACKGROUND_LIST");
			}
			break;
		case 0x1D: {
				uint8 num = fetchScriptByte();
				printf("LABEL_%d:", num);
			}
			break;
		case 0x1E: {
				uint8 num = fetchScriptByte();
				printf("JUMP LABEL_%d", num);
			}
			break;
		case 0x1F: {
				uint8 num = fetchScriptByte();
				printf("JUMP G LABEL_%d", num);
			}
			break;
		case 0x20: {
				uint8 num = fetchScriptByte();
				printf("JUMP GEQ LABEL_%d", num);
			}
			break;
		case 0x21: {
				uint8 num = fetchScriptByte();
				printf("JUMP L LABEL_%d", num);
			}
			break;
		case 0x22: {
				uint8 num = fetchScriptByte();
				printf("JUMP LEQ LABEL_%d", num);
			}
			break;
		case 0x23: {
				uint8 num = fetchScriptByte();
				printf("JUMP EQ LABEL_%d", num);
			}
			break;
		case 0x24: {
				uint8 num = fetchScriptByte();
				printf("JUMP NOT_EQ LABEL_%d", num);
			}
			break;
		case 0x26: {
				uint8 var = fetchScriptByte();
				uint8 num = fetchScriptByte();
				printf("--localvars[%d]; JUMP GEQ LABEL_%d", var, num);
			}
			break;
		case 0x31: {
				uint8 num = fetchScriptByte();
				printf("START_GLOBAL_SCRIPT num=%d", num);
			}
			break;
		case 0x32: {
				uint8 num = fetchScriptByte();
				printf("END_GLOBAL_SCRIPT num=%d", num);
			}
			break;
		case 0x3B: {
				const char *name = fetchScriptString();
				printf("LOAD_ANIM name='%s'", name);
			}
			break;
		case 0x3C: {
				const char *name = fetchScriptString();
				printf("LOAD_BACKGROUND_IMAGE name='%s'", name);
			}
			break;
		case 0x3D: {
				const char *name = fetchScriptString();
				printf("LOAD_CT name='%s'", name);
			}
			break;
		case 0x3F: {
				const char *name = fetchScriptString();
				printf("OPEN_RESOURCE_FILE name='%s'", name);
			}
			break;
		case 0x40: {
				printf("CLOSE_RESOURCE_FILE");
			}
			break;
		case 0x41: {
				static const char *dataType[] = { "prc", "rel", "obj", "msg" };
				uint8 type = fetchScriptByte();
				const char *name = fetchScriptString();
				if (type >= 4) return false;
				printf("SET_LOADING_DATA type='%s', name='%s'", dataType[type], name);
			}
			break;
		case 0x42: {
				printf("LOAD_PENDING_DATA");
			}
			break;
		case 0x45: {
				printf("FADE_IN");
			}
			break;
		case 0x46: {
				printf("FADE_OUT");
			}
			break;
		case 0x47: {
				uint8 start = fetchScriptByte();
				uint8 count = fetchScriptByte();
				int16 r = fetchScriptWord();
				int16 g = fetchScriptWord();
				int16 b = fetchScriptWord();
				printf("TRANSFORM_PALETTE start=%d, count=%d, delta_red=%d, delta_green=%d, delta_blue=%d", start, count, r, g, b);
			}
			break;
		case 0x49: {
				uint8 color = fetchScriptByte();
				printf("SET_MENU_COLOR_1 %d", color);
			}
			break;
		case 0x4A: {
				uint8 firstColor = fetchScriptByte();
				uint8 endColor = fetchScriptByte();
				uint8 type = fetchScriptByte();
				printf("ROTATE_PALETTE first_color=%d, end_color=%d, type=%d", firstColor, endColor, type);
			}
			break;			
		case 0x4F: {
				printf("BREAK");
			}
			break;
		case 0x50: {
				printf("END");
				_baseScriptData = 0;
			}
			break;
		case 0x51: {
				uint8 num = fetchScriptByte();
				int16 x = fetchScriptWord();
				int16 y = fetchScriptWord();
				int16 width = fetchScriptWord();
				int16 color = fetchScriptWord();
				printf("ADD_MESSAGE num=%d x=%d y=%d width=%d color=%d", num, x, y, width, color);
			}
			break;		
		case 0x52: {
				uint8 var = fetchScriptByte();
				uint8 type = fetchScriptByte();
				if (type == 0) {
					int16 value = fetchScriptWord();
					printf("SET globalvars[%d], %d", var, value);
				} else {
					uint8 var2 = fetchScriptByte();
					if (type == 1) {
						printf("SET globalvars[%d], localvars[%d]", var, var2);
					} else {
						printf("SET globalvars[%d], globalvars[%d]", var, var2);
					}
				}
				break;
			}
		case 0x53: {
				uint8 var = fetchScriptByte();
				uint8 type = fetchScriptByte();
				if (type == 0) {
					int16 value = fetchScriptWord();
					printf("CMP globalvars[%d], %d", var, value);
				} else {
					uint8 var2 = fetchScriptByte();
					printf("CMP globalvars[%d], localvars[%d]", var, var2);
				}
			}
			break;
		case 0x59: {
				const char *str = fetchScriptString();
				printf("/* %s */", str);
			}
			break;
		case 0x5A: {
				uint8 start = fetchScriptByte();
				uint8 count = fetchScriptByte();
				printf("FREE_FRAMES start=%d, count=%d", start, count);
			}
			break;
		case 0x5B: {
				printf("UNLOAD_OVERLAYS");
			}
			break;
		case 0x65: {
				printf("CLEAR_ZONE_DATA");
			}
			break;
		case 0x66: {
				uint8 num = fetchScriptByte();
				int16 value = fetchScriptWord();
				printf("SET zones[%d], %d", num, value);
			}
			break;			
		case 0x68: {
				uint8 color = fetchScriptByte();
				printf("SET_MENU_COLOR_2 %d", color);
			}
			break;		
		case 0x69: {
				printf("ENABLE_PLAYER_INPUT");
			}
			break;
		case 0x6A: {
				printf("DISABLE_PLAYER_INPUT");
			}
			break;
		case 0x6B: {
				uint8 num = fetchScriptByte();
				printf("SET_CURRENT_DISK num=%d", num);
			}
			break;
		case 0x6D: {
				const char *name = fetchScriptString();
				printf("LOAD_MUSIC name='%s'", name);
			}
			break;
		case 0x6E: {
				printf("PLAY_MUSIC");
			}
			break;
		case 0x6F: {
				printf("FADE_OUT_MUSIC");
			}
			break;
		case 0x70: {
				printf("STOP_MUSIC");
			}
			break;
		case 0x77: 
		case 0x78: {
				uint8 frame = fetchScriptByte();
				uint8 channel = fetchScriptByte();
				int16 freq = fetchScriptWord();
				uint8 repeat = fetchScriptByte();
				int16 volume = fetchScriptWord();
				int16 size = fetchScriptWord();
				printf("PLAY_SOUND frame=%d channel=%d freq=%d repeat=%d volume=%d size=%d", frame, channel, freq, repeat, volume, size);
			}
			break;
		case 0x79: {
				uint8 param = fetchScriptByte();
				printf("DISABLE_SYSTEM_MENU flag=%d", param);
			}
			break;
		case 0x7A: {
				uint8 num = fetchScriptByte();
				printf("ADD_OVERLAY object=%d, type=5", num);
			}
			break;
		case 0x7B: {
				uint8 num = fetchScriptByte();
				printf("REMOVE_OVERLAY object=%d, type=5", num);
			}
			break;
		case 0x7F:
			assert(isV2); {
				uint8 a = fetchScriptByte();
				uint8 b = fetchScriptByte();
				uint8 c = fetchScriptByte();
				uint8 d = fetchScriptByte();
				int16 e = fetchScriptWord();
				int16 f = fetchScriptWord();
				int16 g = fetchScriptWord();
				printf("ADD_ANIM_SEQUENCE %d %d %d %d %d %d %d", a, b, c, d, e, f, g);
			}			
			break;
		case 0x80:
			assert(isV2); {
				uint8 a = fetchScriptByte();
				uint8 b = fetchScriptByte();
				printf("REMOVE_ANIM_SEQUENCE %d %d", a, b);
			}
			break;
		case 0x82:
			assert(isV2); {
				uint8 a = fetchScriptByte();
				uint8 b = fetchScriptByte();
				int16 c = fetchScriptWord();
				int16 d = fetchScriptWord();
				uint8 e = fetchScriptByte();
				printf("OP_0x82 %d %d %d %d %d", a, b, c, d, e);
			}
			break;			
		case 0x83:
			assert(isV2); {
				uint8 a = fetchScriptByte();
				uint8 b = fetchScriptByte();
				printf("IS_ANIM_SEQUENCE_PLAYING %d %d", a, b);				
			}
			break;
		case 0x84:
			assert(isV2); {
				uint8 num = fetchScriptByte();
				printf("JUMP G LABEL_%d", num);
			}
			break;
		case 0x85:
			assert(isV2); {
				uint8 num = fetchScriptByte();
				printf("JUMP GEQ LABEL_%d", num);	
			}
			break;	
		case 0x86:
			assert(isV2); {
				uint8 num = fetchScriptByte();
				printf("JUMP L LABEL_%d", num);
			}
			break;		
		case 0x88:
			assert(isV2); {
				uint8 num = fetchScriptByte();
				printf("JUMP EQ LABEL_%d", num);
			}
			break;
		case 0x89:
			assert(isV2); {
				uint8 num = fetchScriptByte();
				printf("JUMP NOT_EQ LABEL_%d", num);
			}
			break;
		case 0x8B:
			assert(isV2); {
				uint8 num = fetchScriptByte();
				printf("START_OBJECT_SCRIPT num=%d", num);
			}
			break;
		case 0x8C:
			assert(isV2); {
				uint8 num = fetchScriptByte();
				printf("STOP_OBJECT_SCRIPT num=%d", num);
			}
			break;
		case 0x8D:
			assert(isV2); {
				int16 a = fetchScriptWord();
				int16 b = fetchScriptWord();
				int16 c = fetchScriptWord();
				int16 d = fetchScriptWord();
				int16 e = fetchScriptWord();
				int16 f = fetchScriptWord();
				int16 g = fetchScriptWord();
				int16 h = fetchScriptWord();
				printf("IS_OBJECT_INTERSECTING_OBJECT %d %d %d %d %d %d %d %d", a, b, c, d, e, f, g, h);
			}
			break;
		case 0x8E:
			assert(isV2); {
				uint8 num = fetchScriptByte();
				const char *name = fetchScriptString();
				printf("ADD_BACKGROUND name='%s' num=%d", name, num);
			}
			break;
		case 0x8F:
			assert(isV2); {
				uint8 num = fetchScriptByte();
				printf("REMOVE_BACKGROUND num=%d", num);
			}
			break;
		case 0x90:
			assert(isV2); {
				uint8 pos = fetchScriptByte();
				const char *name = fetchScriptString();
				printf("LOAD_DATA_ABSOLUTE name='%s' pos=%d", name, pos);
			}
			break;
		case 0x91:
			assert(isV2); {
				uint8 num = fetchScriptByte();
				printf("SET_CURRENT_BACKGROUND num=%d", num);
			}
			break;
		case 0x9A:
			assert(isV2); {				
				uint8 a = fetchScriptByte();
				printf("OP_0x9A %d", a);
			}
			break;
		case 0x9D:
			assert(isV2); {
				uint8 num = fetchScriptByte();
				printf("SET_NEXT_BACKGROUND %d", num);
			}
			break;
		case 0x9E:
			assert(isV2); {
				uint8 type = fetchScriptByte();
				if (type == 0) {
					int16 value = fetchScriptWord();
					printf("SET_SCROLLING_BACKGROUND %d", value);
				} else {
					uint8 var = fetchScriptByte();
					printf("SET_SCROLLING_BACKGROUND localvars[%d]", var);
				}
			}
			break;
		case 0xA0:
			assert(isV2); {
				int16 a = fetchScriptWord();
				int16 b = fetchScriptWord();
				printf("OP_0xA0 %d %d", a, b);
			}
			break;
		case 0xA1:
			assert(isV2); {
				int16 a = fetchScriptWord();
				int16 b = fetchScriptWord();
				printf("OP_0xA1 %d %d", a, b);
			}
			break;
		case 0xA2:
			assert(isV2); {
				int16 a = fetchScriptWord();
				int16 b = fetchScriptWord();
				printf("OP_0xA2 %d %d", a, b);
			}
			break;
		case 0xA3:
			assert(isV2); {
				int16 a = fetchScriptWord();
				int16 b = fetchScriptWord();
				printf("OP_0xA3 %d %d", a, b);
			}
			break;
		case 0xA4:
			assert(isV2); {
				uint8 num = fetchScriptByte();
				printf("ADD_OVERLAY object=%d, type=22", num);
			}
			break;
		case 0xA5:
			assert(isV2); {
				uint8 num = fetchScriptByte();
				printf("REMOVE_OVERLAY object=%d, type=22", num);
			}
			break;
		default:
			printf("Unhandled script opcode 0x%X", opcode);
			return false;
		}
	}
	printf("\n");
	return true;
}

typedef void (*DecodeFile)(int isV2, const uint8 *data, int dataSize);

/* .prc procedure */
static void decodePrc(int isV2, const uint8 *data, int dataSize) {
	int i, num;
	struct {
		uint16 size;
	} hdr[256];

	num = READ_BE_UINT16(data); data += 2;
	if (num >= 256) {
		printf("// Number of scripts (%d) in PRC-file is too large (i.e. >= 256)\n\n", num);
		return;
	}

	for (i = 0; i < num; ++i) {
		hdr[i].size = READ_BE_UINT16(data); data += 2;
	}
	for (i = 0; i < num; ++i) {
		if (hdr[i].size != 0) {
			printf("\n");
			printf("// script.prc %d size 0x%X\n", i, hdr[i].size);
			_baseScriptData = _scriptData = data; data += hdr[i].size;
			bool status = true;
			while (_baseScriptData && _scriptData < data && status) {
				status = decodeOpcode(isV2);
			}
			if (!status) {
				printf("// Error handling opcode\n\n");
				return;
			}
			printf("\n");
		}
	}
}

/* .rel relation */
static void decodeRel(int isV2, const uint8 *data, int dataSize) {
	int i, num;
	struct {
		uint16 size;
		uint16 param1;
		uint16 index;
		uint16 param2;
	} hdr[256];

	num = READ_BE_UINT16(data); data += 2;
	if (num >= 256) {
		printf("// Number of scripts (%d) in REL-file is too large (i.e. >= 256)\n\n", num);
		return;
	}	
	for (i = 0; i < num; ++i) {
		hdr[i].size = READ_BE_UINT16(data); data += 2;
		hdr[i].param1 = READ_BE_UINT16(data); data += 2;
		hdr[i].index = READ_BE_UINT16(data); data += 2;
		hdr[i].param2 = READ_BE_UINT16(data); data += 2;
	}
	for (i = 0; i < num; ++i) {
		if (hdr[i].size != 0) {
			printf("\n");
			printf("// script.rel %d size 0x%X param1 %d index %d param2 %d\n", i, hdr[i].size, hdr[i].param1, hdr[i].index, hdr[i].param2);
			_baseScriptData = _scriptData = data; data += hdr[i].size;
			bool status = true;
			while (_baseScriptData && _scriptData < data && status) {
				status = decodeOpcode(isV2);
			}
			if (!status) {
				printf("// Error handling opcode\n\n");
				return;
			}
			printf("\n");
		}
	}
}

int showUsage() {
	printf("USAGE: cine_decode [type] [version] [filename]\n" \
		"type: -prc or -rel (Use -prc for *.prc-files, -rel for *.rel-files)\n" \
		"version: -v1 or -v2 (Use -v1 for Future Wars, -v2 for Operation Stealth)\n" \
		"filename: The name of the script file to decode\n");
	return -1;
}

int main(int argc, char *argv[]) {
	if (argc == 4) {
		FILE *fp;
		uint8 *data;
		int dataSize, isV2;
		DecodeFile decodeFile;
		
		if (strcmp(argv[1], "-prc") == 0) {
			decodeFile = decodePrc;
		}  else if (strcmp(argv[1], "-rel") == 0) {
			decodeFile = decodeRel;
		} else {
			return showUsage();
		}
		
		if (strcmp(argv[2], "-v1") == 0) {
			isV2 = 0;
		}  else if (strcmp(argv[2], "-v2") == 0) {
			isV2 = 1;
		} else {
			return showUsage();
		}
		
		fp = fopen(argv[3], "rb");
		if (fp) {
			fseek(fp, 0, SEEK_END);
			dataSize = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			data = (uint8 *)malloc(dataSize);
			if (data) {
				fread(data, dataSize, 1, fp);
				(*decodeFile)(isV2, data, dataSize);
				free(data);
			}
			fclose(fp);
		} else {
			printf("Couldn't open script file '%s'\n", argv[3]);
			return -1;
		}
	} else {
		return showUsage();
	}
	return 0;
}
