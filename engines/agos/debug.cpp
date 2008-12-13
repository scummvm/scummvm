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
 */

// AGOS debug functions


#include "agos/debug.h"
#include "agos/agos.h"
#include "agos/intern.h"
#include "agos/vga.h"

namespace AGOS {

const byte *AGOSEngine::dumpOpcode(const byte *p) {
	uint opcode;
	const char *s, *st;

	if (getGameType() == GType_ELVIRA1) {
		opcode = READ_BE_UINT16(p);
		p += 2;
		if (opcode == 10000)
			return NULL;
	} else {
		opcode = *p++;
		if (opcode == 255)
			return NULL;
	}

	if (getGameType() == GType_PP) {
		st = s = puzzlepack_opcodeNameTable[opcode];
	} else if (getGameType() == GType_FF) {
		st = s = feeblefiles_opcodeNameTable[opcode];
	} else if (getGameType() == GType_SIMON2 && getFeatures() & GF_TALKIE) {
		st = s = simon2talkie_opcodeNameTable[opcode];
	} else if (getFeatures() & GF_TALKIE) {
		st = s = simon1talkie_opcodeNameTable[opcode];
	} else if (getGameType() == GType_SIMON2) {
		st = s = simon2dos_opcodeNameTable[opcode];
	} else if (getGameType() == GType_SIMON1) {
		st = s = simon1dos_opcodeNameTable[opcode];
	} else if (getGameType() == GType_WW) {
		st = s = waxworks_opcodeNameTable[opcode];
	} else if (getGameType() == GType_ELVIRA2) {
		st = s = elvira2_opcodeNameTable[opcode];
	} else {
		st = s = elvira1_opcodeNameTable[opcode];
	}

	if (s == NULL) {
		error("dumpOpcode: INVALID OPCODE %d", opcode);
	}

	while (*st != '|')
		st++;
	printf("%s ", st + 1);

	for (;;) {
		switch (*s++) {
		case 'x':
			printf("\n");
			return NULL;
		case '|':
			printf("\n");
			return p;
		case 'B':{
				byte b = *p++;
				if (b == 255)
					printf("[%d] ", *p++);
				else
					printf("%d ", b);
				break;
			}
		case 'V':{
				byte b = *p++;
				if (b == 255)
					printf("[[%d]] ", *p++);
				else
					printf("[%d] ", b);
				break;
			}

		case 'W':{
				int n = (int16)READ_BE_UINT16(p);
				p += 2;
				if (getGameType() == GType_PP) {
					if (n >= 60000 && n < 62048)
						printf("[%d] ", n - 60000);
					else
						printf("%d ", n);

				} else {
					if (n >= 30000 && n < 30512)
						printf("[%d] ", n - 30000);
					else
						printf("%d ", n);
				}
				break;
			}

		case 'w':{
				int n = (int16)READ_BE_UINT16(p);
				p += 2;
				printf("%d ", n);
				break;
			}

		case 'I':{
				int n = (int16)READ_BE_UINT16(p);
				p += 2;
				if (n == -1)
					printf("SUBJECT_ITEM ");
				else if (n == -3)
					printf("OBJECT_ITEM ");
				else if (n == -5)
					printf("ME_ITEM ");
				else if (n == -7)
					printf("ACTOR_ITEM ");
				else if (n == -9)
					printf("ITEM_A_PARENT ");
				else
					printf("<%d> ", n);
				break;
			}

		case 'J':{
				printf("-> ");
			}
			break;

		case 'T':{
				uint n = READ_BE_UINT16(p);
				p += 2;
				if (n != 0xFFFF)
					printf("\"%s\"(%d) ", getStringPtrByID(n), n);
				else
					printf("NULL_STRING ");
			}
			break;
		}
	}
}

void AGOSEngine::dumpSubroutineLine(SubroutineLine *sl, Subroutine *sub) {
	const byte *p;

	printf("; ****\n");

	p = (byte *)sl + SUBROUTINE_LINE_SMALL_SIZE;
	if (sub->id == 0) {
		printf("; verb=%d, noun1=%d, noun2=%d\n", sl->verb, sl->noun1, sl->noun2);
		p = (byte *)sl + SUBROUTINE_LINE_BIG_SIZE;
	}

	for (;;) {
		p = dumpOpcode(p);
		if (p == NULL)
			break;
	}
}

void AGOSEngine::dumpSubroutine(Subroutine *sub) {
	SubroutineLine *sl;

	printf("\n******************************************\n;Subroutine, ID=%d:\nSUB_%d:\n", sub->id, sub->id);
	sl = (SubroutineLine *)((byte *)sub + sub->first);
	for (; (byte *)sl != (byte *)sub; sl = (SubroutineLine *)((byte *)sub + sl->next)) {
		dumpSubroutineLine(sl, sub);
	}
	printf("\nEND ******************************************\n");
}

void AGOSEngine::dumpSubroutines() {
	Subroutine *sub = _subroutineList;
	for (; sub; sub = sub->next) {
		dumpSubroutine(sub);
	}
}

void AGOSEngine::dumpAllSubroutines() {
	for (int i = 0; i < 65536; i++) {
		Subroutine *sub = getSubroutineByID(i);
		if (sub != NULL) {
			dumpSubroutine(sub);
		}
	}
}

void AGOSEngine::dumpVideoScript(const byte *src, bool one_opcode_only) {
	uint opcode;
	const char *str, *strn;

	do {
		if (getGameType() == GType_SIMON2 || getGameType() == GType_FF || getGameType() == GType_PP) {
			opcode = *src++;
		} else {
			opcode = READ_BE_UINT16(src);
			src += 2;
		}

		if (opcode >= _numVideoOpcodes) {
			error("Invalid opcode %x", opcode);
		}

		if (getGameType() == GType_FF || getGameType() == GType_PP) {
			strn = str = feeblefiles_videoOpcodeNameTable[opcode];
		} else if (getGameType() == GType_SIMON2) {
			strn = str = simon2_videoOpcodeNameTable[opcode];
		} else if (getGameType() == GType_SIMON1) {
			strn = str = simon1_videoOpcodeNameTable[opcode];
		} else if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
			strn = str = ww_videoOpcodeNameTable[opcode];
		} else {
			strn = str = elvira1_videoOpcodeNameTable[opcode];
		}

		if (strn == NULL) {
			error("dumpVideoScript: INVALID OPCODE %d", opcode);
		}

		while (*strn != '|')
			strn++;
		printf("%.2d: %s ", opcode, strn + 1);

		int end = (getGameType() == GType_FF || getGameType() == GType_PP) ? 9999 : 999;
		for (; *str != '|'; str++) {
			switch (*str) {
			case 'x':
				printf("\n");
				return;
			case 'b':
				printf("%d ", *src++);
				break;
			case 'd':
				printf("%d ", (int16)readUint16Wrapper(src));
				src += 2;
				break;
			case 'v':
				printf("[%d] ", readUint16Wrapper(src));
				src += 2;
				break;
			case 'i':
				printf("%d ", (int16)readUint16Wrapper(src));
				src += 2;
				break;
			case 'j':
				printf("-> ");
				break;
			case 'q':
				while (readUint16Wrapper(src) != end) {
					printf("(%d,%d) ", readUint16Wrapper(src),
									readUint16Wrapper(src + 2));
					src += 4;
				}
				src += 2;
				break;
			default:
				error("Invalid fmt string '%c' in decompile VGA", *str);
			}
		}

		printf("\n");
	} while (!one_opcode_only);
}

void AGOSEngine::dumpVgaScript(const byte *ptr, uint res, uint sprite_id) {
	dumpVgaScriptAlways(ptr, res, sprite_id);
}

void AGOSEngine::dumpVgaScriptAlways(const byte *ptr, uint res, uint sprite_id) {
	printf("; address=%x, vgafile=%d  vgasprite=%d\n",
					(unsigned int)(ptr - _vgaBufferPointers[res].vgaFile1), res, sprite_id);
	dumpVideoScript(ptr, false);
	printf("; end\n");
}

void AGOSEngine_Feeble::dumpVgaFile(const byte *vga) {
	const byte *pp;
	const byte *p;
	int count;

	pp = vga;
	p = pp + READ_LE_UINT16(pp + 2);
	count = READ_LE_UINT16(&((const VgaFileHeader2_Feeble *) p)->animationCount);
	p = pp + READ_LE_UINT16(&((const VgaFileHeader2_Feeble *) p)->animationTable);

	while (--count >= 0) {
		int id = READ_LE_UINT16(&((const AnimationHeader_Feeble *) p)->id);

		dumpVgaScriptAlways(vga + READ_LE_UINT16(&((const AnimationHeader_Feeble *) p)->scriptOffs), id / 100, id);
		p += sizeof(AnimationHeader_Feeble);
	}

	pp = vga;
	p = pp + READ_LE_UINT16(pp + 2);
	count = READ_LE_UINT16(&((const VgaFileHeader2_Feeble *) p)->imageCount);
	p = pp + READ_LE_UINT16(&((const VgaFileHeader2_Feeble *) p)->imageTable);

	while (--count >= 0) {
		int id = READ_LE_UINT16(&((const ImageHeader_Feeble *) p)->id);

		dumpVgaScriptAlways(vga + READ_LE_UINT16(&((const ImageHeader_Feeble *) p)->scriptOffs), id / 100, id);
		p += sizeof(ImageHeader_Feeble);
	}
}

void AGOSEngine_Simon1::dumpVgaFile(const byte *vga) {
	const byte *pp;
	const byte *p;
	int count;

	pp = vga;
	p = pp + READ_BE_UINT16(pp + 4);
	count = READ_BE_UINT16(&((const VgaFileHeader2_Common *) p)->animationCount);
	p = pp + READ_BE_UINT16(&((const VgaFileHeader2_Common *) p)->animationTable);

	while (--count >= 0) {
		int id = READ_BE_UINT16(&((const AnimationHeader_Simon *) p)->id);

		dumpVgaScriptAlways(vga + READ_BE_UINT16(&((const AnimationHeader_Simon *) p)->scriptOffs), id / 100, id);
		p += sizeof(AnimationHeader_Simon);
	}

	pp = vga;
	p = pp + READ_BE_UINT16(pp + 4);
	count = READ_BE_UINT16(&((const VgaFileHeader2_Common *) p)->imageCount);
	p = pp + READ_BE_UINT16(&((const VgaFileHeader2_Common *) p)->imageTable);

	while (--count >= 0) {
		int id = READ_BE_UINT16(&((const ImageHeader_Simon *) p)->id);

		dumpVgaScriptAlways(vga + READ_BE_UINT16(&((const ImageHeader_Simon *) p)->scriptOffs), id / 100, id);
		p += sizeof(ImageHeader_Simon);
	}
}

void AGOSEngine::dumpVgaFile(const byte *vga) {
	const byte *pp;
	const byte *p;
	int count;

	pp = vga;
	p = pp + READ_BE_UINT16(pp + 10) + 20;
	count = READ_BE_UINT16(&((const VgaFileHeader2_Common *) p)->animationCount);
	p = pp + READ_BE_UINT16(&((const VgaFileHeader2_Common *) p)->animationTable);

	while (--count >= 0) {
		int id = READ_BE_UINT16(&((const AnimationHeader_WW *) p)->id);

		dumpVgaScriptAlways(vga + READ_BE_UINT16(&((const AnimationHeader_WW *) p)->scriptOffs), id / 100, id);
		p += sizeof(AnimationHeader_WW);
	}

	pp = vga;
	p = pp + READ_BE_UINT16(pp + 10) + 20;
	count = READ_BE_UINT16(&((const VgaFileHeader2_Common *) p)->imageCount);
	p = pp + READ_BE_UINT16(&((const VgaFileHeader2_Common *) p)->imageTable);

	while (--count >= 0) {
		int id = READ_BE_UINT16(&((const ImageHeader_WW *) p)->id);

		dumpVgaScriptAlways(vga + READ_BE_UINT16(&((const ImageHeader_WW *) p)->scriptOffs), id / 100, id);
		p += sizeof(ImageHeader_WW);
	}
}

static const byte bmp_hdr[] = {
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

void dumpBMP(const char *filename, int w, int h, const byte *bytes, const uint32 *palette) {
	Common::DumpFile out;
	byte my_hdr[sizeof(bmp_hdr)];
	int i;

	out.open(filename);
	if (!out.isOpen())
		return;

	memcpy(my_hdr, bmp_hdr, sizeof(bmp_hdr));

	*(uint32 *)(my_hdr + 2) = w * h + 1024 + sizeof(bmp_hdr);
	*(uint32 *)(my_hdr + 18) = w;
	*(uint32 *)(my_hdr + 22) = h;


	out.write(my_hdr, sizeof(my_hdr));

	for (i = 0; i != 256; i++, palette++) {
		byte color[4];
		color[0] = (byte)(*palette >> 16);
		color[1] = (byte)(*palette >> 8);
		color[2] = (byte)(*palette);
		color[3] = 0;
		out.write(color, 4);
	}

	while (--h >= 0) {
		out.write(bytes + h * ((w + 3) & ~3), ((w + 3) & ~3));
	}
}

void AGOSEngine::dumpBitmap(const char *filename, const byte *offs, int w, int h, int flags, const byte *palette,
								 byte base) {

	if (getGameType() != GType_FF && getGameType() != GType_PP)
		w *= 16;

	/* allocate */
	byte *b = (byte *)malloc(w * h);
	int i, j;

	VC10_state state;

	state.depack_cont = -0x80;
	state.srcPtr = offs;
	state.dh = h;
	state.y_skip = 0;

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		for (i = 0; i != w; i++) {
			byte *c = vc10_depackColumn(&state);
			for (j = 0; j != h; j++) {
				b[j * w + i] = c[j];
			}
		}
	} else {
		for (i = 0; i != w; i += 2) {
			byte *c = vc10_depackColumn(&state);
			for (j = 0; j != h; j++) {
				byte pix = c[j];
				b[j * w + i] = (pix >> 4) | base;
				b[j * w + i + 1] = (pix & 0xF) | base;
			}
		}
	}

	dumpBMP(filename, w, h, b, (const uint32 *)palette);
	free(b);
}

void AGOSEngine::dumpSingleBitmap(int file, int image, const byte *offs, int w, int h, byte base) {
	char buf[40];

	sprintf(buf, "dumps/File%d_Image%d.bmp", file, image);

	if (Common::File::exists(buf))
		return;

	dumpBitmap(buf, offs, w, h, 0, _displayPalette, base);
}

void palLoad(byte *pal, const byte *vga1, int a, int b) {
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

void AGOSEngine::dumpVgaBitmaps(const byte *vga, byte *vga1, int res) {
	int i;
	uint32 offs;
	const byte *p2;
	byte pal[768];

	memset(pal, 0, sizeof(pal));
	palLoad(pal, vga1, 2, 0);
	palLoad(pal, vga1, 3, 1);
	palLoad(pal, vga1, 4, 2);
	palLoad(pal, vga1, 5, 3);

	int width, height, flags;

	for (i = 1; ; i++) {
		p2 = vga + i * 8;
		offs = readUint32Wrapper(p2);

		/* try to detect end of images.
		 * assume the end when offset >= 200kb */
		if (offs >= 204800)
			return;

		if (getGameType() == GType_FF || getGameType() == GType_PP) {
			width = READ_LE_UINT16(p2 + 6);
			height = READ_LE_UINT16(p2 + 4) & 0x7FFF;
			flags = p2[5];
		} else {
			width = READ_BE_UINT16(p2 + 6) / 16;
			height = p2[5];
			flags = p2[4];
		}

		printf("Image %d. Width=%d, Height=%d, Flags=0x%X\n", i, width, height, flags);

		/* dump bitmap */
		char buf[40];
		sprintf(buf, "dumps/Res%d_Image%d.bmp", res, i);

		dumpBitmap(buf, vga + offs, width, height, flags, pal, 0);
	}
}

} // End of namespace AGOS
