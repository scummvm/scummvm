/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Simon debug functions
#include "stdafx.h"
#include "simon.h"

#ifdef SIMONDEBUG
#define SIMON2
#define SIMON2WIN

static const char * const opcode_name_table[256] = {
	/* 0 */
	"|INV_COND",
	"IJ|PTRA_PARENT_IS",
	"IJ|PTRA_PARENT_ISNOT",
	NULL,
	/* 4 */
	NULL,
	"IJ|PARENT_IS_1",
	"IJ|PARENT_ISNOT_1",
	"IIJ|PARENT_IS",
	/* 8 */
	NULL,
	NULL,
	NULL,
	"VJ|IS_ZERO",
	/* 12 */
	"VJ|ISNOT_ZERO",
	"VWJ|IS_EQ",
	"VWJ|IS_NEQ",
	"VWJ|IS_LE",
	/* 16 */
	"VWJ|IS_GE",
	"VVJ|IS_EQF",
	"VVJ|IS_NEQF",
	"VVJ|IS_LEF",
	/* 20 */
	"VVJ|IS_GEF",
	NULL,
	NULL,
	"WJ|UNK23",
	/* 24 */
	NULL,
	"IJ|HAS_CHILD_1",
	"IJ|HAS_CHILD_2",
	"IWJ|ITEM_UNK3_IS",
	/* 28 */
	"IBJ|CHILD_HAS_FLAG",
	NULL,
	NULL,
	"I|SET_NO_PARENT",
	/* 32 */
	NULL,
	"II|SET_PARENT",
	NULL,
	NULL,
	/* 36 */
	"VV|MOVE",
	NULL,
	NULL,
	NULL,
	/* 40 */
	NULL,
	"V|ZERO",
	"VW|SET",
	"VW|ADD",
	/* 44 */
	"VW|SUB",
	"VV|ADDF",
	"VV|SUBF",
	"VW|MUL",
	/* 48 */
	"VW|DIV",
	"VV|MULF",
	"VV|DIVF",
	"VW|MOD",
	/* 52 */
	"VV|MODF",
	"VW|RANDOM",
	NULL,
	"I|SET_A_PARENT",
	/* 56 */
	"IB|SET_CHILD2_BIT",
	"IB|CLEAR_CHILD2_BIT",
	"II|MAKE_SIBLING",
	"I|INC_UNK3",
	/* 60 */
	"I|DEC_UNK3",
	"IW|SET_UNK3",
	"V|SHOW_INT",
	"T|SHOW_STRING_NL",
	/* 64 */
	"T|SHOW_STRING",
	"WWWWWB|ADD_HITAREA",
	"BT|SET_ITEM_NAME",
#if defined SIMON1WIN || defined SIMON2
	"BTw|SET_ITEM_DESC",
#endif
#ifdef SIMON1DOS
	"BT|SET_ITEM_DESC",
#endif
	/* 68 */
	"x|HALT",
	"x|RET1",
	"V|SHOW_STRING_AR3",
	"W|START_SUB",
	/* 72 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 76 */
	"WW|ADD_TIMEOUT",
	"J|IS_M1_EMPTY",
	"J|IS_M3_EMPTY",
	"ITJ|CHILD_FR2_IS",
	/* 80 */
	"IIJ|IS_ITEM_EQ",
	NULL,
	"B|UNK82",
	"|RETM10",
	/* 84 */
	NULL,
	NULL,
	NULL,
	"W|UNK87",
	/* 88 */
	"|OR_SCRIPT_WORD_10",
	"|AND_SCRIPT_WORD_10",
	"IB|SET_M_TO_PARENT",
	"IB|SET_M_TO_SIBLING",
	/* 92 */
	"IB|SET_M_TO_CHILD",
	NULL,
	NULL,
	NULL,
	/* 96 */
	"WB|UNK96",
	"W|LOAD_VGA",
#ifdef SIMON2
	"WWBWWW|START_VGA",
#else
	"WBWWW|START_VGA",
#endif
#ifdef SIMON2
	"WW|KILL_THREAD",
#else
	"W|KILL_THREAD",
#endif
	/* 100 */
	"|VGA_RESET",
	"BWWWWWW|UNK101",
	"B|UNK102",
	"|UNK103",
	/* 104 */
	"B|UNK104",
	NULL,
	NULL,
	"WWWWWIW|ADD_ITEM_HITAREA",
	/* 108 */
	"W|DEL_HITAREA",
	"W|CLEAR_HITAREA_0x40",
	"W|SET_HITAREA_0x40",
	"WWW|SET_HITAREA_XY",
	/* 112 */
	NULL,
	NULL,
	"IB|UNK114",
	"IBJ|HAS_FLAG",
	/* 116 */
	"IB|SET_FLAG",
	"IB|CLEAR_FLAG",
	NULL,
	"W|WAIT_VGA",
	/* 120 */
	"W|UNK120",
	"BI|SET_VGA_ITEM",
	NULL,
	NULL,
	/* 124 */
	NULL,
	"IJ|IS_SIBLING_WITH_A",
	"IBB|UNK126",
	"WW|UNK127",
	/* 128 */
	"W|GET_DUMMY_WORD",
	"W|GET_WORD_COND_TRUE",
	"Bww|UNK131",
	NULL,	/* opcode 131 doesn't exist */
	/* 132 */
	"|SAVE_GAME",
	"|LOAD_GAME",
	"|DUMMYPROC_134",
	"|QUIT_IF_USER_PRESSES_Y",
	/* 136 */
	"IV|GET_ITEM_UNK3",
	"B|UNK137",
	"|VGA_POINTER_OP_4",
	"II|SET_PARENT_SPECIAL",
	/* 140 */
	"|DEL_TE_AND_ADD_ONE",
	"BI|SET_M1_OR_M3",
	"WJ|IS_HITAREA_0x40_CLEAR",
	"I|START_ITEM_SUB",
	/* 144 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 148 */
	NULL,
	NULL,
	NULL,
	"BI|SET_ARRAY6_TO",
	/* 152 */
	"BB|SET_M1_M3_TO_ARRAY6",
	"B|SET_BIT",
	"B|CLEAR_BIT",
	"BJ|IS_BIT_CLEAR",
	/* 156 */
	"BJ|IS_BIT_SET",
	"IBB|GET_ITEM_PROP",
	"IBW|SET_ITEM_PROP",
	NULL,
	/* 160 */
	"B|UNK160",
	"BWBW|SETUP_TEXT",
#if defined SIMON1WIN || defined SIMON2
	"BBTW|PRINT_STR",
#endif
#ifdef SIMON1DOS
	"BBT|PRINT_STR",
#endif
	"W|SOUND_1",
	/* 164 */
	"|UNK164",
	"IWWJ|ITEM_UNK1_UNK2_IS",
	"B|SET_BIT2",
	"B|CLEAR_BIT2",
	/* 168 */
	"BJ|IS_BIT2_CLEAR",
	"BJ|IS_BIT2_SET",
	NULL,
	NULL,
	/* 172 */
	NULL,
	NULL,
	NULL,
	"|VGA_POINTER_OP_1",
	/* 176 */
	"|VGA_POINTER_OP_2",
	"BBI|UNK177",
	"WWBB|PATHFIND",
	"BBB|UNK179",
	/* 180 */
	"|FORCE_UNLOCK",
	"|FORCE_LOCK",
	"|READ_VGARES_328",
	"|READ_VGARES_23",
	/* 184 */
	"W|CLEAR_VGAPOINTER_ENTRY",
	"W|DUMMY_185",
	"|VGA_POINTER_OP_3",
	"|FADE_TO_BLACK",
#ifdef SIMON2
	/* 188 */
	"BSJ|STRING2_IS",
	"|UNK189",
	"B|UNK190",
#endif
};

byte *SimonState::dumpOpcode(byte *p) {
	byte opcode;	
	const char *s, *st;

	opcode = *p++;
	if (opcode == 255)
		return NULL;
	st = s = opcode_name_table[opcode];
	if (s == NULL) {
		error("INVALID OPCODE %d\n", opcode);
		return NULL;
	}
	while (*st != '|') st++;
	fprintf(_dump_file,"%s ", st+1);

	for(;;) {
		switch(*s++) {
		case 'x':
			fprintf(_dump_file,"\n");
			return NULL;
		case '|':
			fprintf(_dump_file,"\n");
			return p;
		case 'B': {
			byte b = *p++;
			if (b==255)
				fprintf(_dump_file,"[%d] ", *p++);
			else
				fprintf(_dump_file,"%d ", b);
			break;
		}
		case 'V': {
			byte b = *p++;
			if (b==255)
				fprintf(_dump_file,"[[%d]] ", *p++);
			else
				fprintf(_dump_file,"[%d] ", b);
			break;
		}

		case 'W': {
			int n = (int16)((p[0]<<8)|p[1]);
			p+=2;
			if (n>=30000 && n<30512)
				fprintf(_dump_file,"[%d] ", n - 30000);
			else
				fprintf(_dump_file,"%d ", n);
			break;
		}

		case 'w': {
			int n = (int16)((p[0]<<8)|p[1]);
			p+=2;
			fprintf(_dump_file,"%d ", n);
			break;
		}

		case 'I': {
			int n = (int16)((p[0]<<8)|p[1]);;
			p+=2;
			if (n == -1)
				fprintf(_dump_file,"ITEM_M1 ");
			else if (n == -3)
				fprintf(_dump_file,"ITEM_M3 ");
			else if (n == -5)
				fprintf(_dump_file,"ITEM_1 ");
			else if (n == -7)
				fprintf(_dump_file,"ITEM_0 ");
			else if (n == -9)
				fprintf(_dump_file,"ITEM_A_PARENT ");
			else
				fprintf(_dump_file,"<%d> ", n);
			break;
		}
		case 'J': {
			fprintf(_dump_file,"-> ");
		} break;


		case 'T': {
			uint n = ((p[0]<<8)|p[1]);
			p+=2;
			if (n != 0xFFFF)
				fprintf(_dump_file,"\"%s\"(%d) ", getStringPtrByID(n), n);
			else
				fprintf(_dump_file,"NULL_STRING ");
		} break;
		}
	}
}

void SimonState::dumpSubroutineLine(SubroutineLine *sl, Subroutine *sub) {
	byte *p;
	
	
	printf("; ****\n");

	p = (byte*)sl + SUBROUTINE_LINE_SMALL_SIZE;
	if (sub->id == 0) {
		fprintf(_dump_file,"; cond_a=%d, cond_b=%d, cond_c=%d\n", sl->cond_a, sl->cond_b, sl->cond_c);
		p = (byte*)sl + SUBROUTINE_LINE_BIG_SIZE;
	}

	for(;;) {
		p = dumpOpcode(p);
		if (p==NULL)
			break;
	}
}

void SimonState::dumpSubroutine(Subroutine *sub) {
	SubroutineLine *sl;

	fprintf(_dump_file,"\n******************************************\n;Subroutine, ID=%d:\nSUB_%d:\n", sub->id, sub->id);
	sl = (SubroutineLine*) ((byte*)sub + sub->first);
	for(;(byte*)sl != (byte*)sub; sl = (SubroutineLine*) ((byte*)sub + sl->next) ) {
		dumpSubroutineLine(sl, sub);
	}
	fprintf(_dump_file,"\nEND ******************************************\n");
	fflush(_dump_file);
}

void SimonState::dumpSubroutines() {
	Subroutine *sub = _subroutine_list;
	for(;sub;sub = sub->next) {
		dumpSubroutine(sub);
	}
}

const char * const video_opcode_name_table[] = {
	/* 0 */
	"x|RET",
	"ddd|DUMMY",
	"d|CALL",
	"ddddd|NEW_THREAD",
	/* 4 */
	"ddd|DUMMY_2",
	"vd|SKIP_IF_NEQ",
	"d|SKIP_IFN_SIB_WITH_A",
	"d|SKIP_IF_SIB_WITH_A",
	/* 8 */
	"dd|SKIP_IF_PARENT_IS",
	"dd|SKIP_IF_UNK3_IS",
#ifdef SIMON2
	"ddddb|DRAW",
#else
	"ddddd|DRAW",
#endif
	"|CLEAR_PATHFIND_ARRAY",
	/* 12 */
#ifdef SIMON2
	"b|DELAY",
#else
	"d|DELAY",
#endif
	"d|OFFSET_X",
	"d|OFFSET_Y",
	"d|IDENT_WAKEUP",
	/* 16 */
	"d|IDENT_SLEEP",
	"dq|SET_PATHFIND_ITEM",
	"i|JUMP_REL",
	"|CHAIN_TO",
	/* 20 */
	"dd|SET_CODE_WORD",
	"i|JUMP_IF_CODE_WORD",
	"dd|SET_PAL",
	"d|SET_PRI",
	/* 24 */
	"diid|SET_IMG_XY",
	"x|HALT_THREAD",
	"ddddd|SET_WINDOW",
	"|RESET",
	/* 28 */
	"dddd|DUMMY_3",
	"|STOP_ALL_SOUNDS",
	"d|SET_BASE_DELAY",
	"d|SET_PALETTE_MODE",
	/* 32 */
	"vv|COPY_VAR",
	"|FORCE_UNLOCK",
	"|FORCE_LOCK",
	"dd|DUMMY_4",
	/* 36 */
	"dd|SAVELOAD_THING",
	"v|OFFSET_Y_F",
	"v|SKIP_IF_VAR_ZERO",
	"vd|SET_VAR",
	/* 40 */
	"vd|ADD_VAR",
	"vd|SUB_VAR",
	"vd|SLEEP_UNTIL_SET",
	"d|SKIP_IF_BIT_CLEAR",
	/* 44 */
	"d|SKIP_IF_BIT_SET",	
	"v|SET_X_F",
	"v|SET_Y_F",
	"vv|ADD_VAR_F",
	/* 48 */
	"|VC_48",
	"d|SET_BIT",
	"d|CLEAR_BIT",
	"d|CLEAR_HITAREA_BIT_0x40",
	/* 52 */
	"d|VC_52",
	"dd|DUMMY_5",
	"ddd|DUMMY_6",
	"ddd|OFFSET_HIT_AREA",
	/* 56 */
#ifdef SIMON2
	"i|SLEEP_EX",
#else
	"|DUMMY_7",
#endif
	"|DUMMY_8",
	"|DUMMY_9",
#ifdef SIMON2
	"ddd|KILL_MULTI_THREAD",
#else
	"|SKIP_IF_SOUND??",
#endif
	/* 60 */
#ifdef SIMON2
	"dd|KILL_THREAD",
#else
	"d|KILL_THREAD",
#endif
	"ddd|INIT_SPRITE",
	"|PALETTE_THING",
	"|PALETTE_THING_2",
#ifdef SIMON2
	/* 64 */
	"|UNK64",
	"|UNK65",
	"|UNK66",
	"|UNK67",
	/* 68 */
	"|UNK68",
	"dd|UNK69",
	"dd|UNK70",
	"|UNK71",
	/* 72 */
	"dd|UNK72",
	"bb|UNK73",
	"bb|UNK74",
#endif
};

void SimonState::dump_video_script(byte *src, bool one_opcode_only) {
	uint opcode;
	const char *str, *strn;

	do {
		if (!(_game & GAME_SIMON2)) {
			opcode = READ_BE_UINT16_UNALIGNED(src);
			src+=2;
		} else {
			opcode = *src++;
		}

		if (opcode >= gss->NUM_VIDEO_OP_CODES) {
			error("Invalid opcode %x\n", opcode);
			return;
		}

		strn = str = video_opcode_name_table[opcode];
		while (*strn != '|') strn++;
		fprintf(_dump_file,"%.2d: %s ", opcode, strn + 1);

		for (;*str != '|';str++) {
			switch(*str) {
			case 'x': fprintf(_dump_file,"\n"); return;
			case 'b':	fprintf(_dump_file,"%d ", *src++); break;
			case 'd':	fprintf(_dump_file,"%d ", READ_BE_UINT16_UNALIGNED(src)); src+=2; break;
			case 'v':	fprintf(_dump_file,"[%d] ", READ_BE_UINT16_UNALIGNED(src)); src+=2; break;
			case 'i':	fprintf(_dump_file,"%d ", (int16)READ_BE_UINT16_UNALIGNED(src)); src+=2; break;
			case 'q': 
				while (READ_BE_UINT16_UNALIGNED(src) != 999) {
					fprintf(_dump_file,"(%d,%d) ", READ_BE_UINT16_UNALIGNED(src), READ_BE_UINT16_UNALIGNED(src+2));
					src += 4;
				}
				src++;
				break;
			default:
				error("Invalid fmt string '%c' in decompile VGA", *str);
			}
		}

		fprintf(_dump_file,"\n");
	} while(!one_opcode_only);
}

void SimonState::dump_vga_file(byte *vga) {
	{
		byte *pp;
		byte *p;
		int count;

		pp = vga;
		p = pp + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header*)pp)->hdr2_start);
		count = READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2*)p)->id_count);
		p = pp + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2*)p)->id_table);
		while (--count >= 0) {
			int id = READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x6*)p)->id);

			dump_vga_script_always(vga + READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x6*)p)->script_offs), id/100, id);
			p += sizeof(VgaFile1Struct0x6);
		}
	}

	{
		byte *bb, *b;
		int c;

		bb = vga;
		b = bb + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header*)bb)->hdr2_start);
		c = READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2*)b)->unk1);
		b = bb + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2*)b)->unk2_offs);

		while (--c >= 0) {
			int id = READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x8*)b)->id);

			dump_vga_script_always(vga + READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x8*)b)->script_offs), id/100, id);
			b += sizeof(VgaFile1Struct0x8);
		}
	}
}



const byte bmp_hdr[] = {
0x42,0x4D,
0x9E,0x14,0x00,0x00, /* offset 2, file size */
0x00,0x00,0x00,0x00,
0x36,0x04,0x00,0x00,
0x28,0x00,0x00,0x00,

0x3C,0x00,0x00,0x00, /* image width */
0x46,0x00,0x00,0x00, /* image height */
0x01,0x00,0x08,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,

0x00,0x01,0x00,0x00,
0x00,0x01,0x00,0x00,
};

void dump_bmp(const char *filename, int w, int h, const byte *bytes, const uint32 *palette) {
	FILE *out = fopen(filename, "wb");
	byte my_hdr[sizeof(bmp_hdr)];
	int i;

	if (out == NULL) {
		printf("DUMP ERROR\n");
		return;
	}

	memcpy(my_hdr, bmp_hdr, sizeof(bmp_hdr));

	*(uint32*)(my_hdr + 2) = w*h + 1024 + sizeof(bmp_hdr);
	*(uint32*)(my_hdr + 18) = w;
	*(uint32*)(my_hdr + 22) = h;

	
	fwrite(my_hdr, 1, sizeof(my_hdr), out);
	
	for(i=0; i!=256; i++,palette++) {
		byte color[4];
		color[0] = (byte)(*palette >> 16);
		color[1] = (byte)(*palette >> 8);
		color[2] = (byte)(*palette);
		color[3] = 0;
		fwrite(color, 1, 4, out);
	}

	while (--h >= 0) {
		fwrite(bytes + h * ((w+3)&~3), ((w+3)&~3), 1, out);
	}

	fclose(out);
}

void dump_bitmap(const char *filename, byte *offs, int w, int h, int flags, const byte *palette, byte base) {
	/* allocate */
	byte *b = (byte*)malloc(w*h);
	int i,j;

	VC10_state state;

	state.depack_cont = -0x80;
	state.depack_src = offs;
	state.dh = h;
	state.y_skip = 0;

	for(i=0; i!=w; i+=2) {
		byte *c = vc_10_depack_column(&state);
		for(j=0;j!=h;j++) {
			byte pix = c[j];
			b[j*w+i] = (pix>>4)|base;
			b[j*w+i+1] = (pix&0xF)|base;

		}
	}

	dump_bmp(filename, w, h, b, (uint32*)palette);
	free(b);
}

void SimonState::dump_single_bitmap(int file, int image, byte *offs, int w, int h, byte base) {
/* Only supported for win32 atm. mkdir doesn't work otherwise. */
#if defined (WIN32) && !defined(_WIN32_WCE)
	char buf[255], buf2[255];
	struct stat statbuf;

	sprintf(buf, "bmp_%d\\%d.bmp", file, image);

	if (stat(buf, &statbuf) == 0)
		return;

	sprintf(buf2, "bmp_%d", file);
	mkdir(buf2);

	dump_bitmap(buf, offs, w, h, 0, _palette, base);
#endif
}

void SimonState::dump_vga_script_always(byte *ptr, uint res, uint sprite_id) {
	fprintf(_dump_file,"; address=%x, vgafile=%d  vgasprite=%d\n", 
		ptr - _vga_buffer_pointers[res].vgaFile1, res, sprite_id);
	dump_video_script(ptr, false);
	fprintf(_dump_file,"; end\n");
}

void SimonState::dump_vga_script(byte *ptr, uint res, uint sprite_id) {
	dump_Vga_script_always(ptr,res,sprite_id);
}


#endif
