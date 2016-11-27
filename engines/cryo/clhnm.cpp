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

#include "cryo/cryolib.h"

namespace Cryo {

static bool safe_palette = false;
static bool use_mono = false;

void CLHNM_Desentrelace320(byte *frame_buffer, byte *final_buffer, uint16 height);

void CLHNM_DecompLempelZiv(byte *buffer, byte *output) {
	byte *inp = buffer;
	byte *out = output;

	unsigned int queue = 0;
	int qpos = -1;

	//TODO: fix for BE
#define GetBit() ( 1 & ( (qpos >= 0) ? (queue >> qpos--) : (queue = *(unsigned int*)((inp += 4) - 4)) >> ((qpos = 30) + 1) ) )

	for (;;) {
		if (GetBit()) {
			*out++ = *inp++;
		} else {
			int l, o;
			if (GetBit()) {
				l = *inp & 7;
				o = *(uint16 *)inp >> 3;
				inp += 2;
				o -= 8192;
				if (!l)
					l = *inp++;
				if (!l)
					break;
			} else {
				l = GetBit() * 2 + GetBit();
				o = *(inp++) - 256;
			}
			l += 2;
			while (l--) {
				*out = *(out + o);
				out++;
			}
		}
	}

#undef GetBit

	return;
}

void CLHNM_DecompUBA(byte *output, byte *curr_buffer, byte *prev_buffer,
                     byte *input, int width, char flags) {
	unsigned int code;
	byte mode, count, color;
	uint16 offs;
	byte *ref;
	byte *out_start = output;
	byte swap;
	int shft1, shft2;
//	return;
	if ((flags & 1) == 0) {
		//HNM4 classic
		int twolinesabove = -(width * 2);
		for (;;) {
			code = PLE32(input) & 0xFFFFFF; //input++;
			count = code & 0x1F;
			if (count) {
				input += 3;
				mode = (code >> 5) & 0xF;
				offs = code >> 9;
				//
				swap = mode >> 3;
				ref = ((mode & 1) ? prev_buffer : curr_buffer) + (output - out_start) + (offs * 2) - 32768;
				if (mode & 2) {
					//                  ref += twolinesabove;
					shft1 = twolinesabove + 1;
					shft2 = 0;
					//swap ^= 1;
				} else {
					shft1 = 0;
					shft2 = 1;
				}
				while (count--) {
					byte b0 = ref[shft1];
					byte b1 = ref[shft2];
					output[swap] = b0;
					output[swap ^ 1] = b1;
					output += 2;
					ref += (mode & 4) ? -2 : 2;
				}
			} else {
				input++;
				mode = code & 0xFF; // bits 0..4 are zero
				switch (mode) {
				case 0:
					*(output++) = *(input++);
					*(output++) = *(input++);
					break;
				case 0x20:
					output += 2 * *(input++);
					break;
				case 0x40:
					output += 2 * (code >> 8);
					input += 2;
					break;
				case 0x60:
					count = *(input++);
					color = *(input++);
					while (count--) {
						*(output++) = color;
						*(output++) = color;
					}
					break;
				default:
					return;
				}
			}
		}
	} else {
		assert(0);
		//HNM4 hires
		for (;;) {
			code = PLE32(input) & 0xFFFFFF;
			input++;
			count = code & 0x3F;
			if (count) {
				mode = (code >> 5) & 0xF;
				offs = code >> 9;
				//
			} else {
				mode = code & 0xFF; // bits 0..5 are zero
				switch (mode) {
				case 0x00:
					output += *input++;
					break;
				case 0x40:
					*output++ = *input++;
					*(output++ + width) = *input++;
					break;
				case 0x80:
					output += width;
					break;
				default:
					return;
				}
			}
		}
	}
}

void CLHNM_Done() {
}

hnm_t *CLHNM_New(int preload_size) {
	hnm_t *hnm = (hnm_t *)malloc(sizeof(*hnm));

	hnm->_frameNum = 0;
	hnm->ff_4 = 0;
	hnm->_file = nullptr;
	hnm->tmpBuffer[0] = nullptr;
	hnm->tmpBuffer[1] = nullptr;
	hnm->finalBuffer = nullptr;
	hnm->_readBuffer = nullptr;
	hnm->ff_896 = 0;
	hnm->_totalRead = 0;
	for (int i = 0; i < 256; i++) {
		hnm->_palette[i].a = 0;
		hnm->_palette[i].r = 0;
		hnm->_palette[i].g = 0;
		hnm->_palette[i].b = 0;
	}

	return hnm;
}

void CLHNM_Dispose(hnm_t *hnm) {
	free(hnm);
}


void CLHNM_SetFile(hnm_t *hnm, file_t *file) {
	hnm->_file = file;
}

void CLHNM_SetFinalBuffer(hnm_t *hnm, byte *buffer) {
	hnm->finalBuffer = buffer;
}

void CLHNM_AllocMemory(hnm_t *hnm) {
	hnm->tmpBuffer[0] = (byte *)malloc(hnm->_header._bufferSize + 2);

	if (!hnm->tmpBuffer[0])
		return;

	hnm->tmpBuffer[1] = (byte *)malloc(hnm->_header._bufferSize + 2);

	if (!hnm->tmpBuffer[1]) {
		free(hnm->tmpBuffer[0]);
		hnm->tmpBuffer[0] = nullptr;
		return;
	}

	hnm->_readBuffer = (byte *)malloc(hnm->_header._bufferSize + 2);
	if (!hnm->_readBuffer) {
		free(hnm->tmpBuffer[0]);
		hnm->tmpBuffer[0] = nullptr;
		free(hnm->tmpBuffer[1]);
		hnm->tmpBuffer[1] = nullptr;
	}
}

void CLHNM_DeallocMemory(hnm_t *hnm) {
	free(hnm->tmpBuffer[0]);
	free(hnm->tmpBuffer[1]);
	free(hnm->_readBuffer);

	hnm->tmpBuffer[0] = nullptr;
	hnm->tmpBuffer[1] = nullptr;
	hnm->_readBuffer = nullptr;
}

void CLHNM_GiveTime(hnm_t *hnm) {
}

void CLHNM_CanLoop(hnm_t *hnm, bool canLoop) {
	hnm->_canLoop = canLoop;
}

void CLHNM_SelectBuffers(hnm_t *hnm) {
	if (hnm->_frameNum % 2) {
		hnm->_newFrameBuffer = hnm->tmpBuffer[1];
		hnm->_oldFrameBuffer = hnm->tmpBuffer[0];
	} else {
		hnm->_newFrameBuffer = hnm->tmpBuffer[0];
		hnm->_oldFrameBuffer = hnm->tmpBuffer[1];
	}
}

void CLHNM_ChangePalette(hnm_t *hnm) {
	int16 mincolor, maxcolor;
	uint16 fst, cnt;
	byte *pal;
	color_t *color;
	CLPalette_GetLastPalette(hnm->_palette);
	pal = hnm->_dataPtr;
	if (*(uint16 *)pal == 0xFFFF)
		return;
	mincolor = 255;
	maxcolor = 0;
	do {
		fst = *pal++;
		cnt = *pal++;
		if (cnt == 0)
			cnt = 256;
		debug("hnm: setting palette, fst = %d, cnt = %d, last = %d", fst, cnt, fst + cnt - 1);
		assert(fst + cnt <= 256);
		if (mincolor > fst)
			mincolor = fst;
		if (maxcolor < fst + cnt)
			maxcolor = fst + cnt;
		color = hnm->_palette + fst;
		if (safe_palette) {
			while (cnt--) {
				byte r = *pal++;
				byte g = *pal++;
				byte b = *pal++;
				int16 rr = r << 10;
				int16 gg = g << 10;
				int16 bb = b << 10;
				if (color->r != rr || color->g != gg || color->b != bb)
					CLBlitter_OneBlackFlash();
				color->r = rr;
				color->g = gg;
				color->b = bb;
				color++;
			}
		} else {
			while (cnt--) {
				byte r = *pal++;
				byte g = *pal++;
				byte b = *pal++;
				color->r = r << 10;
				color->g = g << 10;
				color->b = b << 10;
				color++;
			}
		}

	} while (*(uint16 *)pal != 0xFFFF);
#if 0
	if (preserve_color0) {
		hnm->palette[0].r = 0;
		hnm->palette[0].g = 0;
		hnm->palette[0].b = 0;
	}
#endif
//	CLBlitter_Send2ScreenNextCopy(hnm->palette, mincolor, maxcolor - mincolor);
	CLBlitter_Send2ScreenNextCopy(hnm->_palette, 0, 256);
}

void CLHNM_Desentrelace(hnm_t *hnm) {
	switch (hnm->_header._width) {
	case 320:
		CLHNM_Desentrelace320(hnm->_newFrameBuffer, hnm->finalBuffer, hnm->_header._height);
		break;
//	case 480:
//		CLHNM_Desentrelace480(hnm->new_frame_buffer, hnm->final_buffer, hnm->header.height);
//		break;
	default:
		error("CLHNM_Desentrelace - Unexpected width");
	}
}

void CLHNM_TryRead(hnm_t *hnm, int size) {
	hnm->_file->read(hnm->_readBuffer, size);
}

bool CLHNM_LoadFrame(hnm_t *hnm) {
	int chunk;
	CLHNM_TryRead(hnm, 4);
	chunk = *(int *)hnm->_readBuffer;
	chunk = LE32(chunk);
	chunk &= 0xFFFFFF;  // upper bit - keyframe mark?
	if (!chunk)
		return false;

	if (chunk - 4 > hnm->_header._bufferSize)
		error("CLHNM_LoadFrame - Chunk size");

	CLHNM_TryRead(hnm, chunk - 4);
	hnm->_dataPtr = hnm->_readBuffer;
	hnm->_totalRead += chunk;
	return true;
}

void CLHNM_SoundMono(bool isMono) {
	use_mono = isMono;
}

void CLHNM_ReadHeader(hnm_t *hnm) {
	int32 size = sizeof(hnm->_header);
	hnm->_file->read(&hnm->_header, size);

	hnm->_header._width = LE16(hnm->_header._width);
	hnm->_header._height = LE16(hnm->_header._height);
	hnm->_header._unusedFileSize = LE32(hnm->_header._unusedFileSize);
	hnm->_header._numbFrame = LE32(hnm->_header._numbFrame);
	hnm->_header._unusedTableOffset = LE32(hnm->_header._unusedTableOffset);
	hnm->_header._unusedSpeed = LE16(hnm->_header._unusedSpeed);
	hnm->_header._unusedMaxBuffer = LE16(hnm->_header._unusedMaxBuffer);
	hnm->_header._bufferSize = LE32(hnm->_header._bufferSize);
	hnm->_header._unusedUnknown = LE16(hnm->_header._unusedUnknown);
	hnm->_header._bufferSize += 4096; //TODO: checkme
}

int16 CLHNM_GetVersion(hnm_t *hnm) {
	if (hnm->_header._signature == BE32('HNM4'))
		return 4;
	return -1;
}

int CLHNM_GetFrameNum(hnm_t *hnm) {
	return hnm->_frameNum;
}

void CLHNM_Desentrelace320(byte *frame_buffer, byte *final_buffer, uint16 height) {
	unsigned int *input = (unsigned int *)frame_buffer;
	unsigned int *line0 = (unsigned int *)final_buffer;
	unsigned int *line1 = (unsigned int *)(final_buffer + 320);
	int count = (height) / 2;
	while (count--) {
		int16 i;
		for (i = 0; i < 320 / 4; i++) {
			unsigned int p0 = *input++;
			unsigned int p4 = *input++;
#if 0
			*line0++ = ((p4 & 0xFF00) >> 8) | ((p4 & 0xFF000000) >> 16) | ((p0 & 0xFF00) << 8) | (p0 & 0xFF000000);
//			*line0++ = (p0 & 0xFF000000) | ((p0 & 0xFF00) << 8) | ((p4 & 0xFF000000) >> 16) | ((p4 & 0xFF00) >> 8);
			*line1++ = ((p0 & 0xFF0000) << 8) | ((p0 & 0xFF) << 16) | ((p4 & 0xFF0000) >> 8) | (p4 & 0xFF);
#else
			*line0++ = (p0 & 0xFF) | ((p0 & 0xFF0000) >> 8) | ((p4 & 0xFF) << 16) | ((p4 & 0xFF0000) << 8);
			*line1++ = ((p0 & 0xFF00) >> 8) | ((p0 & 0xFF000000) >> 16) | ((p4 & 0xFF00) << 8) | (p4 & 0xFF000000);
#endif
		}
		line0 += 320 / 4;
		line1 += 320 / 4;
	}
}

} // End of namespace Cryo
