/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
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
#include "simon/simon.h"
#include "simon/intern.h"
#include "simon/debug.h"
#include "simon/vga.h"

#if defined (WIN32) && !defined(_WIN32_WCE)
#include <sys/stat.h>
#endif

namespace Simon {

byte *SimonEngine::dumpOpcode(byte *p) {
	byte opcode;
	const char *s, *st;

	opcode = *p++;
	if (opcode == 255)
		return NULL;
	if (_game & GF_SIMON2 && _game & GF_TALKIE) {
		st = s = simon2talkie_opcode_name_table[opcode];
	} else if (_game & GF_TALKIE) {
		st = s = simon1talkie_opcode_name_table[opcode];
	} else if (_game & GF_SIMON2) {
		st = s = simon2dos_opcode_name_table[opcode];
	} else {
		st = s = simon1dos_opcode_name_table[opcode];
	}
	if (s == NULL) {
		error("INVALID OPCODE %d", opcode);
		return NULL;
	}
	while (*st != '|')
		st++;
	fprintf(_dump_file, "%s ", st + 1);

	for (;;) {
		switch (*s++) {
		case 'x':
			fprintf(_dump_file, "\n");
			return NULL;
		case '|':
			fprintf(_dump_file, "\n");
			return p;
		case 'B':{
				byte b = *p++;
				if (b == 255)
					fprintf(_dump_file, "[%d] ", *p++);
				else
					fprintf(_dump_file, "%d ", b);
				break;
			}
		case 'V':{
				byte b = *p++;
				if (b == 255)
					fprintf(_dump_file, "[[%d]] ", *p++);
				else
					fprintf(_dump_file, "[%d] ", b);
				break;
			}

		case 'W':{
				int n = (int16)((p[0] << 8) | p[1]);
				p += 2;
				if (n >= 30000 && n < 30512)
					fprintf(_dump_file, "[%d] ", n - 30000);
				else
					fprintf(_dump_file, "%d ", n);
				break;
			}

		case 'w':{
				int n = (int16)((p[0] << 8) | p[1]);
				p += 2;
				fprintf(_dump_file, "%d ", n);
				break;
			}

		case 'I':{
				int n = (int16)((p[0] << 8) | p[1]);;
				p += 2;
				if (n == -1)
					fprintf(_dump_file, "ITEM_M1 ");
				else if (n == -3)
					fprintf(_dump_file, "ITEM_M3 ");
				else if (n == -5)
					fprintf(_dump_file, "ITEM_1 ");
				else if (n == -7)
					fprintf(_dump_file, "ITEM_0 ");
				else if (n == -9)
					fprintf(_dump_file, "ITEM_A_PARENT ");
				else
					fprintf(_dump_file, "<%d> ", n);
				break;
			}

		case 'J':{
				fprintf(_dump_file, "-> ");
			}
			break;

		case 'T':{
				uint n = ((p[0] << 8) | p[1]);
				p += 2;
				if (n != 0xFFFF)
					fprintf(_dump_file, "\"%s\"(%d) ", getStringPtrByID(n), n);
				else
					fprintf(_dump_file, "NULL_STRING ");
			}
			break;
		}
	}
}

void SimonEngine::dumpSubroutineLine(SubroutineLine *sl, Subroutine *sub) {
	byte *p;

	printf("; ****\n");

	p = (byte *)sl + SUBROUTINE_LINE_SMALL_SIZE;
	if (sub->id == 0) {
		fprintf(_dump_file, "; cond_a=%d, cond_b=%d, cond_c=%d\n", sl->cond_a, sl->cond_b, sl->cond_c);
		p = (byte *)sl + SUBROUTINE_LINE_BIG_SIZE;
	}

	for (;;) {
		p = dumpOpcode(p);
		if (p == NULL)
			break;
	}
}

void SimonEngine::dumpSubroutine(Subroutine *sub) {
	SubroutineLine *sl;

	fprintf(_dump_file, "\n******************************************\n;Subroutine, ID=%d:\nSUB_%d:\n", sub->id, sub->id);
	sl = (SubroutineLine *)((byte *)sub + sub->first);
	for (; (byte *)sl != (byte *)sub; sl = (SubroutineLine *)((byte *)sub + sl->next)) {
		dumpSubroutineLine(sl, sub);
	}
	fprintf(_dump_file, "\nEND ******************************************\n");
	fflush(_dump_file);
}

void SimonEngine::dumpSubroutines() {
	Subroutine *sub = _subroutine_list;
	for (; sub; sub = sub->next) {
		dumpSubroutine(sub);
	}
}

void SimonEngine::dump_video_script(byte *src, bool one_opcode_only) {
	uint opcode;
	const char *str, *strn;

	do {
		if (!(_game & GF_SIMON2)) {
			opcode = READ_BE_UINT16(src);
			src += 2;
		} else {
			opcode = *src++;
		}

		if (opcode >= gss->NUM_VIDEO_OP_CODES) {
			error("Invalid opcode %x\n", opcode);
			return;
		}

		if (!(_game & GF_SIMON2)) {
			strn = str = simon1_video_opcode_name_table[opcode];
		} else {
			strn = str = simon2_video_opcode_name_table[opcode];
		}
		
		while (*strn != '|')
			strn++;
		fprintf(_dump_file, "%.2d: %s ", opcode, strn + 1);

		for (; *str != '|'; str++) {
			switch (*str) {
			case 'x':
				fprintf(_dump_file, "\n");
				return;
			case 'b':
				fprintf(_dump_file, "%d ", *src++);
				break;
			case 'd':
				fprintf(_dump_file, "%d ", READ_BE_UINT16(src));
				src += 2;
				break;
			case 'v':
				fprintf(_dump_file, "[%d] ", READ_BE_UINT16(src));
				src += 2;
				break;
			case 'i':
				fprintf(_dump_file, "%d ", (int16)READ_BE_UINT16(src));
				src += 2;
				break;
			case 'q':
				while (READ_BE_UINT16(src) != 999) {
					fprintf(_dump_file, "(%d,%d) ", READ_BE_UINT16(src),
									READ_BE_UINT16(src + 2));
					src += 4;
				}
				src++;
				break;
			default:
				error("Invalid fmt string '%c' in decompile VGA", *str);
			}
		}

		fprintf(_dump_file, "\n");
	} while (!one_opcode_only);
}

void SimonEngine::dump_vga_file(byte *vga) {
	{
		byte *pp;
		byte *p;
		int count;

		pp = vga;
		p = pp + READ_BE_UINT16(&((VgaFile1Header *) pp)->hdr2_start);
		count = READ_BE_UINT16(&((VgaFile1Header2 *) p)->id_count);
		p = pp + READ_BE_UINT16(&((VgaFile1Header2 *) p)->id_table);
		while (--count >= 0) {
			int id = READ_BE_UINT16(&((VgaFile1Struct0x6 *) p)->id);

			dump_vga_script_always(vga + READ_BE_UINT16(&((VgaFile1Struct0x6 *) p)->script_offs), id / 100, id);
			p += sizeof(VgaFile1Struct0x6);
		}
	}

	{
		byte *bb, *b;
		int c;

		bb = vga;
		b = bb + READ_BE_UINT16(&((VgaFile1Header *) bb)->hdr2_start);
		c = READ_BE_UINT16(&((VgaFile1Header2 *) b)->unk1);
		b = bb + READ_BE_UINT16(&((VgaFile1Header2 *) b)->unk2_offs);

		while (--c >= 0) {
			int id = READ_BE_UINT16(&((VgaFile1Struct0x8 *) b)->id);

			dump_vga_script_always(vga + READ_BE_UINT16(&((VgaFile1Struct0x8 *) b)->script_offs), id / 100, id);
			b += sizeof(VgaFile1Struct0x8);
		}
	}
}

const byte bmp_hdr[] = {
	0x42, 0x4D,
	0x9E, 0x14, 0x00, 0x00,				/* offset 2, file size */
	0x00, 0x00, 0x00, 0x00,
	0x36, 0x04, 0x00, 0x00,
	0x28, 0x00, 0x00, 0x00,

	0x3C, 0x00, 0x00, 0x00,				/* image width */
	0x46, 0x00, 0x00, 0x00,				/* image height */
	0x01, 0x00, 0x08, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,

	0x00, 0x01, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00,
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

	*(uint32 *)(my_hdr + 2) = w * h + 1024 + sizeof(bmp_hdr);
	*(uint32 *)(my_hdr + 18) = w;
	*(uint32 *)(my_hdr + 22) = h;


	fwrite(my_hdr, 1, sizeof(my_hdr), out);

	for (i = 0; i != 256; i++, palette++) {
		byte color[4];
		color[0] = (byte)(*palette >> 16);
		color[1] = (byte)(*palette >> 8);
		color[2] = (byte)(*palette);
		color[3] = 0;
		fwrite(color, 1, 4, out);
	}

	while (--h >= 0) {
		fwrite(bytes + h * ((w + 3) & ~3), ((w + 3) & ~3), 1, out);
	}

	fclose(out);
}

void dump_bitmap(const char *filename, byte *offs, int w, int h, int flags, const byte *palette,
								 byte base)
{
	/* allocate */
	byte *b = (byte *)malloc(w * h);
	int i, j;

	VC10_state state;

	state.depack_cont = -0x80;
	state.depack_src = offs;
	state.dh = h;
	state.y_skip = 0;

	for (i = 0; i != w; i += 2) {
		byte *c = vc_10_depack_column(&state);
		for (j = 0; j != h; j++) {
			byte pix = c[j];
			b[j * w + i] = (pix >> 4) | base;
			b[j * w + i + 1] = (pix & 0xF) | base;

		}
	}

	dump_bmp(filename, w, h, b, (const uint32 *)palette);
	free(b);
}

void SimonEngine::dump_single_bitmap(int file, int image, byte *offs, int w, int h, byte base) {
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

void pal_load(byte *pal, const byte *vga1, int a, int b) {
	uint num = (a == 0) ? 0x20 : 0x10;
	byte *palptr;
	const byte *src;
	
	palptr = (byte *)&pal[a << 4];
	src = vga1 + 6 + b * 96;
	
	do {
		palptr[0] = src[0] << 2;
		palptr[1] = src[1] << 2;
		palptr[2] = src[2] << 2;
		palptr[3] = 0;

		palptr += 4;
		src += 3;
	} while (--num);
}

void SimonEngine::dump_vga_bitmaps(byte *vga, byte *vga1, int res) {
/* Only supported for win32 atm. mkdir doesn't work otherwise. */
#if defined (WIN32) && !defined(_WIN32_WCE)

	int i;
	uint32 offs;
	byte *p2;
	byte pal[768];

	{
		memset(pal, 0, sizeof(pal));
		pal_load(pal, vga1, 2, 0);
		pal_load(pal, vga1, 3, 1);
		pal_load(pal, vga1, 4, 2);
		pal_load(pal, vga1, 5, 3);
	}

	{
		char buf[255], buf2[255];
		sprintf(buf, "bmp_%d", res);
		mkdir(buf2);
	}

	int width, height, flags;
	
	i = 538;

	for(i = 1; ; i++) {
		p2 = vga + i * 8;
		offs = TO_BE_32(*(uint32 *)p2);

		/* try to detect end of images.
		 * assume the end when offset >= 200kb */
		if (offs >= 200*1024)
			return;
		
		width = TO_BE_16(*(uint16 *)(p2 + 6));
		height = p2[5];
		flags = p2[4];

		fprintf(_dump_file, "Image %d. Width=%d, Height=%d, Flags=0x%X\n", i, width, height, flags);
		fflush(_dump_file);

		/* dump bitmap */
		{
			char buf[255];
			sprintf(buf, "bmp_%d\\%d.bmp", res, i);

			dump_bitmap(buf, vga + offs, width, height, flags, pal, 0);
		}
	}
#endif
}

void SimonEngine::dump_vga_script_always(byte *ptr, uint res, uint sprite_id) {
	fprintf(_dump_file, "; address=%x, vgafile=%d  vgasprite=%d\n",
					ptr - _vga_buffer_pointers[res].vgaFile1, res, sprite_id);
	dump_video_script(ptr, false);
	fprintf(_dump_file, "; end\n");
}

void SimonEngine::dump_vga_script(byte *ptr, uint res, uint sprite_id) {
	dump_vga_script_always(ptr, res, sprite_id);
}

} // End of namespace Simon
