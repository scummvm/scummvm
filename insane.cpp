/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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

#if !defined(macintosh) && !defined(__MORPHOS__)
#define NEED_SDL_HEADERS
#endif


#include "stdafx.h"
#include "scumm.h"

#define SWAP2(a) ((((a)>>24)&0xFF) | (((a)>>8)&0xFF00) | (((a)<<8)&0xFF0000) | (((a)<<24)&0xFF000000))
#define MAX_STREAMER 10

byte * SmushPlayer::loadTres()
{
	byte buf[100];
	FILE * f_tres;
	uint32 tmp, l;

	sprintf((char *)buf, "%sVIDEO/%sTXT.TRS", (char *)sm->_gameDataPath, 
			(char *)sm->_exe_name);
	f_tres = fopen((char *)buf, "rb");
	
	if (f_tres == NULL) {
		sprintf((char *)buf, "%svideo/%stxt.trs", (char *)sm->_gameDataPath,
				(char *)sm->_exe_name);
		f_tres = fopen((char *)buf, "rb");
		if (f_tres == NULL) 
			return NULL;
	}

	if (fread(&tmp, 4, 1, f_tres) != 1) // read tag
		error("error while reading TRES");
	
	tmp = READ_BE_UINT32(&tmp); 
	if (tmp == 'ETRS')
	{
		fread(&tmp, 4, 1, f_tres); // read length
		tmp = READ_BE_UINT32(&tmp);
		tmp -= 8;
		_buffer_tres = (byte*)malloc (tmp + 1);
		fread(_buffer_tres, 8, 1, f_tres); // skip 8 bytes
		fread(_buffer_tres, tmp, 1, f_tres);
		for (l = 0; l < tmp; l++) 
			*(_buffer_tres + l) ^= 0xcc;
		_buffer_tres[tmp] = 0;
	}
	fclose (f_tres);

	return _buffer_tres;
}

void SmushPlayer::loadFonts()
{
	byte buf[100];
	FILE * f_tres;
	uint32 tmp;
	int l;
	byte * buffer = NULL;

	for (l = 0; l < SP_MAX_FONTS; l++)
	{
		_fonts [l] = NULL;
		sprintf((char *)buf, "%sVIDEO/FONT%d.NUT", (char *)sm->_gameDataPath, l);
		f_tres = fopen((char *)buf, "rb");

		if (f_tres == NULL) {
			sprintf((char *)buf, "%svideo/font%d.nut", (char *)sm->_gameDataPath, l);
			f_tres = fopen((char *)buf, "rb");
			if (f_tres == NULL) 
				continue;
		}

		fread(&tmp, 4, 1, f_tres); // read tag
		tmp = READ_BE_UINT32(&tmp);
		if (tmp == 'ANIM') {
			fread(&tmp, 4, 1, f_tres); // read length
			tmp = READ_BE_UINT32(&tmp);
			buffer = (byte *)malloc(tmp);
			fread(buffer, tmp, 1, f_tres);
			_fonts[l] = buffer;
		}
		fclose (f_tres);
	}
}

byte * SmushPlayer::getStringTRES(int32 number)
{
	byte * txt = NULL;
	uint32 l, i, j;

	for (l = 0;; l++) {
		char t = *(_buffer_tres + l);
		if (t == 0)
			break;
		if (t == '#') {
			byte buf[10];
			strncpy ((char*)buf, (char*)_buffer_tres + l + 1, 9);
			buf[9] = 0;
			if (strcmp ((char*)buf, "define a ") == 0) {
				l += 10;
				for (i = 0; i < 5; i++) {
					buf[i] = *(_buffer_tres + l + i);
					if (buf[i] == 0x0d)
					{
						buf[i] = 0;
						l += 2;
						break;
					}
				}
				if (atol((char*)buf) == number) {
					for (j = 0; j < 200; j++) {
						t = *(_buffer_tres + l + j + i);
						if ((t == 0) || (t == '#'))
							break;
					}
					txt = (byte *)malloc(j + 1);
					strncpy((char*)txt, (char*)_buffer_tres + l + i, j);
					txt[j] = 0;
					return txt;
				}
			}
		}
	}

	return txt;
}

void SmushPlayer::drawStringTRES(uint32 x, uint32 y, byte * txt)
{
	char buf[4];
	uint32 c_line = 0, l = 0, i, tmp_x;
	uint8 c_font = 0, c_color = 0;

	if ((txt[l] == '^') && (txt[l + 1] == 'f')) {
		buf[0] = txt[l + 2];
		buf[1] = txt[l + 3];
		buf[2] = 0;
		l += 4;
		c_font = atoi(buf);
		if (c_font >= SP_MAX_FONTS)
			error ("SP: number font out of range");
	}
	if ((txt[l] == '^') && (txt[l + 1] == 'c')) {
		buf[0] = txt[l + 2];
		buf[1] = txt[l + 3];
		buf[2] = txt[l + 4];
		buf[3] = 0;
		l += 5;
		c_color = atoi(buf);
	}

	for (;;) {
		tmp_x = x;
		for (i = 0;; i++) {
			if (txt[l + i] == 0)
				goto exit_loop;
			if (txt[l + i] == 0x0d) {
				l += i + 2;
				break;
			}
			drawCharTRES (&tmp_x, y, c_line, c_font, c_color, txt[l + i]);
			
			// this is hack
			if (y + c_line * 0xe > 170) continue;
			
			if (tmp_x > 320) {
				tmp_x = 0;
				c_line++;
			}
		}
		c_line++;
	}
exit_loop: ;

}

void codec44_depack(byte *dst, byte *src, uint32 len)
{
	byte val;
	uint16 size_line;
	uint16 num;

	do {
		size_line = READ_LE_UINT16(src);
		src += 2;
		len -= 2;

		for (; size_line != 0;) {
			num = *src++;
			val = *src++;
			memset(dst, val, num);
			dst += num;
			len -= 2;
			size_line -= 2;
			if (size_line == 0) break;

			num = READ_LE_UINT16(src) + 1;
			src += 2;
			memcpy(dst, src, num);
			dst += num;
			src += num;
			len -= num + 2;
			size_line -= num + 2;

		}
		dst--;

	} while (len > 1);
}

void SmushPlayer::drawCharTRES(uint32 * x, uint32 y, uint32 c_line, uint8 c_font, uint8 color, uint8 txt)
{
	byte * font = _fonts[c_font];
	uint32 offset = 0, t_offset = 0, l, width, height, length = 0;
	
	if (font == NULL)
		return;
	
	if (READ_BE_UINT32(font) != 'AHDR')
		return;
	
	offset = READ_BE_UINT32(font + 4) + 8;
	for (l = 0; l <= txt; l++) {
		if (READ_BE_UINT32(font + offset) == 'FRME') {
			offset += 8;
			if (READ_BE_UINT32(font + offset) == 'FOBJ') {
				t_offset = offset + 8;
				length = READ_BE_UINT32(font + offset + 4) - 0xe;
				offset += READ_BE_UINT32(font + offset + 4) + 8;
			}
			else
				return;
		}
		else
			return;
	}

	byte * dst = (byte*)malloc (1000);
	byte * src = (byte*)(font + t_offset + 0x0e);

	memset (dst, 0, 1000);
	
	codec44_depack (dst, src, length);

	width = *(uint16*)(font + t_offset + 6);
	height = *(uint16*)(font + t_offset + 8);

	y += c_line * height;
	for (uint32 ty = 0; ty < height; ty++) {
		for (uint32 tx = 0; tx < width; tx++) {
			byte pixel = *(dst + ty * width + tx);
			if (pixel != 0) {
				if (color == 0) {
					if (pixel == 0x01)
						pixel = 0xf;
				}
				else {
					if (pixel == 0x01)
						pixel = color;
				}
				if (pixel == 0xff)
					pixel = 0x0;
				*(_renderBitmap + ((ty + y) * 320 + *x + tx)) = pixel;
			}
		}
	}
	*x += width;
	free (dst);
}

void invalidblock(uint32 tag)
{
	error("Encountered invalid block %c%c%c%c", tag >> 24, tag >> 16, tag >> 8, tag);
}

int _frameChanged;
int _mixer_num;

uint32 SmushPlayer::nextBE32()
{
	uint32 a = *((uint32 *)_cur);
	_cur += sizeof(uint32);

	return SWAP2(a);
}

void SmushPlayer::fileRead(void *mem, int len)
{
	if (fread(mem, len, 1, _in) != 1)
		error("EOF while reading");

}

uint32 SmushPlayer::fileReadBE32()
{
	uint32 number;

	fileRead(&number, sizeof(number));
	return SWAP2(number);
}

uint32 SmushPlayer::fileReadLE32()
{
	uint32 number;

	fileRead(&number, sizeof(number));
	return number;
}

void SmushPlayer::openFile(byte *fileName)
{
	byte buf[100];

	sprintf((char *)buf, "%sVIDEO/%s", (char *)sm->_gameDataPath, (char *)fileName);
	_in = fopen((char *)buf, "rb");

	if (_in == NULL) {
		sprintf((char *)buf, "%svideo/%s", (char *)sm->_gameDataPath, (char *)fileName);
		_in = fopen((char *)buf, "rb");

	}
}

void SmushPlayer::nextBlock()
{
	_blockTag = fileReadBE32();
	_blockSize = fileReadBE32();

	if (_block != NULL)
		free(_block);

	_block = (byte *)malloc(_blockSize);

	if (_block == NULL)
		error("cannot allocate memory");

	fileRead(_block, _blockSize);
}

bool SmushPlayer::parseTag()
{
	switch (nextBlock(), _blockTag) {

	case 'AHDR':
		parseAHDR();
		break;

	case 'FRME':
		parseFRME();
		break;

	default:
		invalidblock(_blockTag);
	}

	return true;
}



void SmushPlayer::parseAHDR()
{
	memcpy(_fluPalette, _block + 6, 0x300);
	_paletteChanged = true;

//  printf("parse AHDR\n");
}


void SmushPlayer::parseIACT()
{
	unsigned int pos, bpos, tag, sublen, subpos, trk, idx, flags;
	bool new_mixer = false;
	byte *buf;

	flags = SoundMixer::FLAG_AUTOFREE;

	pos = 0;
	pos += 6;

	trk = READ_LE_UINT32(_cur + pos);	/* FIXME: is this correct ? */
	pos += 4;

	for (idx = 0; idx < MAX_STREAMER; idx++) {
		if (_imusTrk[idx] == trk)
			break;
	}

	if (idx == MAX_STREAMER) {
		for (idx = 0; idx < MAX_STREAMER; idx++) {
			if (_imusTrk[idx] == 0 && g_mixer->_channels[idx] == NULL) {
				_imusTrk[idx] = trk;
				_imusSize[idx] = 0;
				new_mixer = true;
				break;
			}
		}
	}

	if (idx == MAX_STREAMER) {
		warning("iMUS table full");
		return;
	}

	pos += 8;											/* FIXME: what are these ? */

	while (pos < _frmeSize) {

		if (_imusSize[idx] == 0) {
			tag = READ_BE_UINT32(_cur + pos);
			pos += 4;
			if (tag != 'iMUS')
				error("trk %d: iMUS tag not found", trk);
			_imusSize[idx] = READ_BE_UINT32(_cur + pos);
			pos += 4;
		}
		if (_imusSubSize[idx] == 0) {
			_imusSubTag[idx] = READ_BE_UINT32(_cur + pos);
			pos += 4;
			_imusSubSize[idx] = READ_BE_UINT32(_cur + pos);
			pos += 4;
			_imusSize[idx] -= 8;
			debug(3, "trk %d: tag '%4s' size %x", trk, _cur + pos - 8, _imusSubSize[idx]);
		}

		sublen = _imusSubSize[idx] < (_frmeSize - pos) ? _imusSubSize[idx] : (_frmeSize - pos);

		switch (_imusSubTag[idx]) {
		case 'MAP ':
			tag = READ_BE_UINT32(_cur + pos);
			if (tag != 'FRMT')
				error("trk %d: no FRMT section");
			_imusCodec[idx] = READ_BE_UINT32(_cur + pos + 16);
			_imusRate[idx] = READ_BE_UINT32(_cur + pos + 20);
			_imusChan[idx] = READ_BE_UINT32(_cur + pos + 24);
			_imusPos[idx] = 0;
			break;
		case 'DATA':
			switch (_imusCodec[idx]) {
			case 8:
				if (_imusChan[idx] == 2)
					flags |= SoundMixer::FLAG_STEREO;
				flags |= SoundMixer::FLAG_UNSIGNED;
				buf = (byte *)malloc(sublen);
				memcpy(buf, _cur + pos, sublen);
				bpos = sublen;
				break;
			case 12:
				if (_imusChan[idx] == 2)
					flags |= SoundMixer::FLAG_STEREO;
				flags |= SoundMixer::FLAG_16BITS;
				buf = (byte *)malloc(2 * sublen);

				bpos = 0;
				subpos = 0;

				while (subpos < sublen) {

					while (_imusPos[idx] < 3 && subpos < sublen) {
						_imusData[idx][_imusPos[idx]] = _cur[pos + subpos];
						_imusPos[idx]++;
						subpos++;
					}

					if (_imusPos[idx] == 3) {
						uint32 temp;

						temp = (_imusData[idx][1] & 0x0f) << 8;
						temp = (temp | _imusData[idx][0]) << 4;
						temp -= 0x8000;

						buf[bpos++] = (byte)((temp >> 8) & 0xff);
						buf[bpos++] = (byte)(temp & 0xff);

						temp = (_imusData[idx][1] & 0xf0) << 4;
						temp = (temp | _imusData[idx][2]) << 4;
						temp -= 0x8000;

						buf[bpos++] = (byte)((temp >> 8) & 0xff);
						buf[bpos++] = (byte)(temp & 0xff);
						_imusPos[idx] = 0;
					}
				}
				break;
			default:
				error("trk %d: unknown iMUS codec %d", trk, _imusCodec[idx]);
			}

			debug(3, "trk %d: iMUSE play part, len 0x%x rate %d remain 0x%x",
						trk, bpos, _imusRate[idx], _imusSubSize[idx]);

			if (new_mixer) {
				g_mixer->play_stream(NULL, idx, buf, bpos, _imusRate[idx], flags);
			} else {
				g_mixer->append(idx, buf, bpos, _imusRate[idx], flags);
			}

			/* FIXME: append with re-used idx may cause problems
			   with signed/unsigned issues */

			break;
		default:
			error("trk %d: unknown tag inside iMUS %08x [%c%c%c%c]",
						trk, _imusSubTag[idx], _imusSubTag[idx] >> 24,
						_imusSubTag[idx] >> 16, _imusSubTag[idx] >> 8, _imusSubTag[idx]);
		}

		_imusSubSize[idx] -= sublen;
		_imusSize[idx] -= sublen;
		pos += sublen;

		if (_imusSubSize[idx] == 0 && _imusSubTag[idx] == 'DATA') {
			_imusTrk[idx] = 0;
			return;
		}
	}
}

void SmushPlayer::parseNPAL()
{
	memcpy(_fluPalette, _cur, 0x300);

	_paletteChanged = true;
}



void codec1(CodecData * cd)
{
	uint y = cd->y;
	byte *src = cd->src;
	byte *dest = cd->out;
	uint h = cd->h;

	if (!h || !cd->w)
		return;

	dest += cd->y * cd->pitch;

	do {
		byte color;
		uint len, num;
		uint x;

		if ((uint) y >= (uint) cd->outheight) {
			src += *(uint16 *)(src) + 2;
			continue;
		}

		len = cd->w;
		x = cd->x;
		src += 2;

		do {
			byte code = *src++;

			num = (code >> 1) + 1;
			if (num > len)
				num = len;
			len -= num;

			if (code & 1) {
				color = *src++;
//        if ((color = *src++)!=0) {
				do {
					if ((uint) x < (uint) cd->outwidth)
						dest[x] = color;
				} while (++x, --num);
//        } else {
//          x += num;
//        }
			} else {
				do {
					color = *src++;
					if ( /*(color=*src++) != 0 && */ (uint) x < (uint) cd->outwidth)
						dest[x] = color;
				} while (++x, --num);
			}
		} while (len);
	} while (dest += cd->pitch, y++, --h);
}



void codec37_bompdepack(byte *dst, byte *src, int len)
{
	byte code;
	byte color;
	int num;

	do {
		code = *src++;
		if (code & 1) {
			num = (code >> 1) + 1;
			color = *src++;
			memset(dst, color, num);
			dst += num;
		} else {
			num = (code >> 1) + 1;
			memcpy(dst, src, num);
			dst += num;
			src += num;
		}
	} while (len -= num);
}

void codec37_proc4(byte *dst, byte *src, int next_offs, int bw, int bh, int pitch, int16 *table)
{
	byte code, *tmp;
	int i;
	uint32 t;

	if (pitch != 320) {
		warning("invalid pitch");
		return;
	}

	do {
		i = bw;
		do {
			code = *src++;
			if (code == 0xFD) {
				t = src[0];
				t += (t << 8) + (t << 16) + (t << 24);
				*(uint32 *)(dst + 0) = t;
				*(uint32 *)(dst + 320) = t;
				*(uint32 *)(dst + 320 * 2) = t;
				*(uint32 *)(dst + 320 * 3) = t;
				src += 1;
				dst += 4;
			} else if (code == 0xFE) {
				t = src[0];
				t += (t << 8) + (t << 16) + (t << 24);
				*(uint32 *)(dst + 0) = t;
				t = src[1];
				t += (t << 8) + (t << 16) + (t << 24);
				*(uint32 *)(dst + 320) = t;
				t = src[2];
				t += (t << 8) + (t << 16) + (t << 24);
				*(uint32 *)(dst + 320 * 2) = t;
				t = src[3];
				t += (t << 8) + (t << 16) + (t << 24);
				*(uint32 *)(dst + 320 * 3) = t;
				src += 4;
				dst += 4;
			} else if (code == 0xFF) {
				*(uint32 *)(dst + 0) = ((uint32 *)src)[0];
				*(uint32 *)(dst + 320) = ((uint32 *)src)[1];
				*(uint32 *)(dst + 320 * 2) = ((uint32 *)src)[2];
				*(uint32 *)(dst + 320 * 3) = ((uint32 *)src)[3];
				src += 16;
				dst += 4;
			} else if (code == 0x00) {
				uint16 count = src[0] + 1;
				src += 1;
				for (uint16 l = 0; l < count; l++) {
					tmp = dst + next_offs;
					*(uint32 *)(dst + 0) = *(uint32 *)(tmp);
					*(uint32 *)(dst + 320) = *(uint32 *)(tmp + 320);
					*(uint32 *)(dst + 320 * 2) = *(uint32 *)(tmp + 320 * 2);
					*(uint32 *)(dst + 320 * 3) = *(uint32 *)(tmp + 320 * 3);
					dst += 4;
					i--;
					if (i == 0) {
						i = bw;
						dst += 320 * 4 - 320;
						bh--;
					}
				}
				i++;
			} else {
				tmp = dst + table[code] + next_offs;
				*(uint32 *)(dst + 0) = *(uint32 *)(tmp);
				*(uint32 *)(dst + 320) = *(uint32 *)(tmp + 320);
				*(uint32 *)(dst + 320 * 2) = *(uint32 *)(tmp + 320 * 2);
				*(uint32 *)(dst + 320 * 3) = *(uint32 *)(tmp + 320 * 3);
				dst += 4;
			}
			if (i <= 0)
				break;
			if (bh <= 0)
				break;
		} while (--i);
		dst += 320 * 4 - 320;
		if (bh <= 0)
			break;
	} while (--bh);
}


void codec37_proc5(int game, byte *dst, byte *src, int next_offs, int bw, int bh,
									 int pitch, int16 *table)
{
	byte code, *tmp;
	int i;
	uint32 t;

	if (pitch != 320) {
		warning("invalid pitch");
		return;
	}

	do {
		i = bw;
		do {
			code = *src++;

			// FIXME: Full Throttle has different FD and FEs?
			if ((game == GID_DIG) && (code == 0xFD)) {
				t = src[0];
				t += (t << 8) + (t << 16) + (t << 24);
				*(uint32 *)(dst + 0) = t;
				*(uint32 *)(dst + 320) = t;
				*(uint32 *)(dst + 320 * 2) = t;
				*(uint32 *)(dst + 320 * 3) = t;
				src += 1;
				dst += 4;
			} else if ((game == GID_DIG) && (code == 0xFE)) {
				t = src[0];
				t += (t << 8) + (t << 16) + (t << 24);
				*(uint32 *)(dst + 0) = t;
				t = src[1];
				t += (t << 8) + (t << 16) + (t << 24);
				*(uint32 *)(dst + 320) = t;
				t = src[2];
				t += (t << 8) + (t << 16) + (t << 24);
				*(uint32 *)(dst + 320 * 2) = t;
				t = src[3];
				t += (t << 8) + (t << 16) + (t << 24);
				*(uint32 *)(dst + 320 * 3) = t;
				src += 4;
				dst += 4;
			} else if (code == 0xFF) {
				*(uint32 *)(dst + 0) = ((uint32 *)src)[0];
				*(uint32 *)(dst + 320) = ((uint32 *)src)[1];
				*(uint32 *)(dst + 320 * 2) = ((uint32 *)src)[2];
				*(uint32 *)(dst + 320 * 3) = ((uint32 *)src)[3];
				src += 16;
				dst += 4;
			} else {
				tmp = dst + table[code] + next_offs;
				*(uint32 *)(dst + 0) = *(uint32 *)(tmp);
				*(uint32 *)(dst + 320) = *(uint32 *)(tmp + 320);
				*(uint32 *)(dst + 320 * 2) = *(uint32 *)(tmp + 320 * 2);
				*(uint32 *)(dst + 320 * 3) = *(uint32 *)(tmp + 320 * 3);
				dst += 4;
			}
		} while (--i);
		dst += 320 * 4 - 320;
	} while (--bh);
}


// this table is the same in FT and Dig
static const int8 maketable_bytes[] = {
	0, 0, 1, 0, 2, 0, 3, 0, 5, 0, 8, 0, 13, 0, 21, 0,
	-1, 0, -2, 0, -3, 0, -5, 0, -8, 0, -13, 0, -17, 0, -21, 0,
	0, 1, 1, 1, 2, 1, 3, 1, 5, 1, 8, 1, 13, 1, 21, 1,
	-1, 1, -2, 1, -3, 1, -5, 1, -8, 1, -13, 1, -17, 1, -21, 1,
	0, 2, 1, 2, 2, 2, 3, 2, 5, 2, 8, 2, 13, 2, 21, 2,
	-1, 2, -2, 2, -3, 2, -5, 2, -8, 2, -13, 2, -17, 2, -21, 2,
	0, 3, 1, 3, 2, 3, 3, 3, 5, 3, 8, 3, 13, 3, 21, 3,
	-1, 3, -2, 3, -3, 3, -5, 3, -8, 3, -13, 3, -17, 3, -21, 3,
	0, 5, 1, 5, 2, 5, 3, 5, 5, 5, 8, 5, 13, 5, 21, 5,
	-1, 5, -2, 5, -3, 5, -5, 5, -8, 5, -13, 5, -17, 5, -21, 5,
	0, 8, 1, 8, 2, 8, 3, 8, 5, 8, 8, 8, 13, 8, 21, 8,
	-1, 8, -2, 8, -3, 8, -5, 8, -8, 8, -13, 8, -17, 8, -21, 8,
	0, 13, 1, 13, 2, 13, 3, 13, 5, 13, 8, 13, 13, 13, 21, 13,
	-1, 13, -2, 13, -3, 13, -5, 13, -8, 13, -13, 13, -17, 13, -21, 13,
	0, 21, 1, 21, 2, 21, 3, 21, 5, 21, 8, 21, 13, 21, 21, 21,
	-1, 21, -2, 21, -3, 21, -5, 21, -8, 21, -13, 21, -17, 21, -21, 21,
	0, -1, 1, -1, 2, -1, 3, -1, 5, -1, 8, -1, 13, -1, 21, -1,
	-1, -1, -2, -1, -3, -1, -5, -1, -8, -1, -13, -1, -17, -1, -21, -1,
	0, -2, 1, -2, 2, -2, 3, -2, 5, -2, 8, -2, 13, -2, 21, -2,
	-1, -2, -2, -2, -3, -2, -5, -2, -8, -2, -13, -2, -17, -2, -21, -2,
	0, -3, 1, -3, 2, -3, 3, -3, 5, -3, 8, -3, 13, -3, 21, -3,
	-1, -3, -2, -3, -3, -3, -5, -3, -8, -3, -13, -3, -17, -3, -21, -3,
	0, -5, 1, -5, 2, -5, 3, -5, 5, -5, 8, -5, 13, -5, 21, -5,
	-1, -5, -2, -5, -3, -5, -5, -5, -8, -5, -13, -5, -17, -5, -21, -5,
	0, -8, 1, -8, 2, -8, 3, -8, 5, -8, 8, -8, 13, -8, 21, -8,
	-1, -8, -2, -8, -3, -8, -5, -8, -8, -8, -13, -8, -17, -8, -21, -8,
	0, -13, 1, -13, 2, -13, 3, -13, 5, -13, 8, -13, 13, -13, 21, -13,
	-1, -13, -2, -13, -3, -13, -5, -13, -8, -13, -13, -13, -17, -13, -21, -13,
	0, -17, 1, -17, 2, -17, 3, -17, 5, -17, 8, -17, 13, -17, 21, -17,
	-1, -17, -2, -17, -3, -17, -5, -17, -8, -17, -13, -17, -17, -17, -21, -17,
	0, -21, 1, -21, 2, -21, 3, -21, 5, -21, 8, -21, 13, -21, 21, -21,
	-1, -21, -2, -21, -3, -21, -5, -21, -8, -21, -13, -21, -17, -21, 0, 0,
	-8, -29, 8, -29, -18, -25, 17, -25, 0, -23, -6, -22, 6, -22, -13, -19,
	12, -19, 0, -18, 25, -18, -25, -17, -5, -17, 5, -17, -10, -15, 10, -15,
	0, -14, -4, -13, 4, -13, 19, -13, -19, -12, -8, -11, -2, -11, 0, -11,
	2, -11, 8, -11, -15, -10, -4, -10, 4, -10, 15, -10, -6, -9, -1, -9,
	1, -9, 6, -9, -29, -8, -11, -8, -8, -8, -3, -8, 3, -8, 8, -8,
	11, -8, 29, -8, -5, -7, -2, -7, 0, -7, 2, -7, 5, -7, -22, -6,
	-9, -6, -6, -6, -3, -6, -1, -6, 1, -6, 3, -6, 6, -6, 9, -6,
	22, -6, -17, -5, -7, -5, -4, -5, -2, -5, 0, -5, 2, -5, 4, -5,
	7, -5, 17, -5, -13, -4, -10, -4, -5, -4, -3, -4, -1, -4, 0, -4,
	1, -4, 3, -4, 5, -4, 10, -4, 13, -4, -8, -3, -6, -3, -4, -3,
	-3, -3, -2, -3, -1, -3, 0, -3, 1, -3, 2, -3, 4, -3, 6, -3,
	8, -3, -11, -2, -7, -2, -5, -2, -3, -2, -2, -2, -1, -2, 0, -2,
	1, -2, 2, -2, 3, -2, 5, -2, 7, -2, 11, -2, -9, -1, -6, -1,
	-4, -1, -3, -1, -2, -1, -1, -1, 0, -1, 1, -1, 2, -1, 3, -1,
	4, -1, 6, -1, 9, -1, -31, 0, -23, 0, -18, 0, -14, 0, -11, 0,
	-7, 0, -5, 0, -4, 0, -3, 0, -2, 0, -1, 0, 0, -31, 1, 0,
	2, 0, 3, 0, 4, 0, 5, 0, 7, 0, 11, 0, 14, 0, 18, 0,
	23, 0, 31, 0, -9, 1, -6, 1, -4, 1, -3, 1, -2, 1, -1, 1,
	0, 1, 1, 1, 2, 1, 3, 1, 4, 1, 6, 1, 9, 1, -11, 2,
	-7, 2, -5, 2, -3, 2, -2, 2, -1, 2, 0, 2, 1, 2, 2, 2,
	3, 2, 5, 2, 7, 2, 11, 2, -8, 3, -6, 3, -4, 3, -2, 3,
	-1, 3, 0, 3, 1, 3, 2, 3, 3, 3, 4, 3, 6, 3, 8, 3,
	-13, 4, -10, 4, -5, 4, -3, 4, -1, 4, 0, 4, 1, 4, 3, 4,
	5, 4, 10, 4, 13, 4, -17, 5, -7, 5, -4, 5, -2, 5, 0, 5,
	2, 5, 4, 5, 7, 5, 17, 5, -22, 6, -9, 6, -6, 6, -3, 6,
	-1, 6, 1, 6, 3, 6, 6, 6, 9, 6, 22, 6, -5, 7, -2, 7,
	0, 7, 2, 7, 5, 7, -29, 8, -11, 8, -8, 8, -3, 8, 3, 8,
	8, 8, 11, 8, 29, 8, -6, 9, -1, 9, 1, 9, 6, 9, -15, 10,
	-4, 10, 4, 10, 15, 10, -8, 11, -2, 11, 0, 11, 2, 11, 8, 11,
	19, 12, -19, 13, -4, 13, 4, 13, 0, 14, -10, 15, 10, 15, -5, 17,
	5, 17, 25, 17, -25, 18, 0, 18, -12, 19, 13, 19, -6, 22, 6, 22,
	0, 23, -17, 25, 18, 25, -8, 29, 8, 29, 0, 31, 0, 0, -6, -22,
	6, -22, -13, -19, 12, -19, 0, -18, -5, -17, 5, -17, -10, -15, 10, -15,
	0, -14, -4, -13, 4, -13, 19, -13, -19, -12, -8, -11, -2, -11, 0, -11,
	2, -11, 8, -11, -15, -10, -4, -10, 4, -10, 15, -10, -6, -9, -1, -9,
	1, -9, 6, -9, -11, -8, -8, -8, -3, -8, 0, -8, 3, -8, 8, -8,
	11, -8, -5, -7, -2, -7, 0, -7, 2, -7, 5, -7, -22, -6, -9, -6,
	-6, -6, -3, -6, -1, -6, 1, -6, 3, -6, 6, -6, 9, -6, 22, -6,
	-17, -5, -7, -5, -4, -5, -2, -5, -1, -5, 0, -5, 1, -5, 2, -5,
	4, -5, 7, -5, 17, -5, -13, -4, -10, -4, -5, -4, -3, -4, -2, -4,
	-1, -4, 0, -4, 1, -4, 2, -4, 3, -4, 5, -4, 10, -4, 13, -4,
	-8, -3, -6, -3, -4, -3, -3, -3, -2, -3, -1, -3, 0, -3, 1, -3,
	2, -3, 3, -3, 4, -3, 6, -3, 8, -3, -11, -2, -7, -2, -5, -2,
	-4, -2, -3, -2, -2, -2, -1, -2, 0, -2, 1, -2, 2, -2, 3, -2,
	4, -2, 5, -2, 7, -2, 11, -2, -9, -1, -6, -1, -5, -1, -4, -1,
	-3, -1, -2, -1, -1, -1, 0, -1, 1, -1, 2, -1, 3, -1, 4, -1,
	5, -1, 6, -1, 9, -1, -23, 0, -18, 0, -14, 0, -11, 0, -7, 0,
	-5, 0, -4, 0, -3, 0, -2, 0, -1, 0, 0, -23, 1, 0, 2, 0,
	3, 0, 4, 0, 5, 0, 7, 0, 11, 0, 14, 0, 18, 0, 23, 0,
	-9, 1, -6, 1, -5, 1, -4, 1, -3, 1, -2, 1, -1, 1, 0, 1,
	1, 1, 2, 1, 3, 1, 4, 1, 5, 1, 6, 1, 9, 1, -11, 2,
	-7, 2, -5, 2, -4, 2, -3, 2, -2, 2, -1, 2, 0, 2, 1, 2,
	2, 2, 3, 2, 4, 2, 5, 2, 7, 2, 11, 2, -8, 3, -6, 3,
	-4, 3, -3, 3, -2, 3, -1, 3, 0, 3, 1, 3, 2, 3, 3, 3,
	4, 3, 6, 3, 8, 3, -13, 4, -10, 4, -5, 4, -3, 4, -2, 4,
	-1, 4, 0, 4, 1, 4, 2, 4, 3, 4, 5, 4, 10, 4, 13, 4,
	-17, 5, -7, 5, -4, 5, -2, 5, -1, 5, 0, 5, 1, 5, 2, 5,
	4, 5, 7, 5, 17, 5, -22, 6, -9, 6, -6, 6, -3, 6, -1, 6,
	1, 6, 3, 6, 6, 6, 9, 6, 22, 6, -5, 7, -2, 7, 0, 7,
	2, 7, 5, 7, -11, 8, -8, 8, -3, 8, 0, 8, 3, 8, 8, 8,
	11, 8, -6, 9, -1, 9, 1, 9, 6, 9, -15, 10, -4, 10, 4, 10,
	15, 10, -8, 11, -2, 11, 0, 11, 2, 11, 8, 11, 19, 12, -19, 13,
	-4, 13, 4, 13, 0, 14, -10, 15, 10, 15, -5, 17, 5, 17, 0, 18,
	-12, 19, 13, 19, -6, 22, 6, 22, 0, 23,
};

void codec37_maketable(PersistentCodecData37 * pcd, int pitch, byte idx)
{
	int i, j;

	if (pcd->table_last_pitch == pitch && pcd->table_last_flags == idx)
		return;

	pcd->table_last_pitch = pitch;
	pcd->table_last_flags = idx;

	assert(idx * 255 + 254 < (int)(sizeof(maketable_bytes) / 2));

	for (i = 0; i < 255; i++) {
		j = i + idx * 255;
		pcd->table1[i] = maketable_bytes[j * 2 + 1] * pitch + maketable_bytes[j * 2];
	}
}



int codec37(int game, CodecData * cd, PersistentCodecData37 * pcd)
{
	int width_in_blocks, height_in_blocks;
	int src_pitch;
	byte *curbuf;
	uint size;
	bool result = false;

	_frameChanged = 1;

	width_in_blocks = (cd->w + 3) >> 2;
	height_in_blocks = (cd->h + 3) >> 2;
	src_pitch = width_in_blocks * 4;

	codec37_maketable(pcd, src_pitch, cd->src[1]);

	switch (cd->src[0]) {
	case 0:{
			curbuf = pcd->deltaBufs[pcd->curtable];
			memset(pcd->deltaBuf, 0, curbuf - pcd->deltaBuf);
			size = *(uint32 *)(cd->src + 4);
			memset(curbuf + size, 0, pcd->deltaBuf + pcd->deltaSize - curbuf - size);
			memcpy(curbuf, cd->src + 16, size);
			break;
		}

	case 2:{
			size = *(uint32 *)(cd->src + 4);
			curbuf = pcd->deltaBufs[pcd->curtable];
			if (size == 64000)
				codec37_bompdepack(curbuf, cd->src + 16, size);
			else
				return (1);

			memset(pcd->deltaBuf, 0, curbuf - pcd->deltaBuf);
			memset(curbuf + size, 0, pcd->deltaBuf + pcd->deltaSize - curbuf - size);
			break;
		}

	case 3:{
			uint16 number = *(uint16 *)(cd->src + 2);

			if (number && pcd->flags + 1 != number)
				break;

			if (number & 1 && cd->src[12] & 1 && cd->flags & 0x10) {
				_frameChanged = 0;
				result = true;
				break;
			}

			if ((number & 1) || !(cd->src[12] & 1)) {
				pcd->curtable ^= 1;
			}

			codec37_proc5(game, pcd->deltaBufs[pcd->curtable], cd->src + 16,
										pcd->deltaBufs[pcd->curtable ^ 1] -
										pcd->deltaBufs[pcd->curtable], width_in_blocks,
										height_in_blocks, src_pitch, pcd->table1);
			break;

		}
	case 4:{
			uint16 number = *(uint16 *)(cd->src + 2);

			if (number && pcd->flags + 1 != number)
				break;

			if (number & 1 && cd->src[12] & 1 && cd->flags & 0x10) {
				_frameChanged = 0;
				result = true;
				break;
			}

			if ((number & 1) || !(cd->src[12] & 1)) {
				pcd->curtable ^= 1;
			}

			codec37_proc4(pcd->deltaBufs[pcd->curtable], cd->src + 16,
										pcd->deltaBufs[pcd->curtable ^ 1] -
										pcd->deltaBufs[pcd->curtable], width_in_blocks,
										height_in_blocks, src_pitch, pcd->table1);
			break;
		}

	case 1:
		warning("code %d", cd->src[0]);
		return (1);

	default:
		error("codec37 default case");
	}

	pcd->flags = *(uint16 *)(cd->src + 2);

	if (result) {
		pcd->curtable ^= 1;
	} else {
		memcpy(cd->out, pcd->deltaBufs[pcd->curtable], 320 * 200);
	}

	return (_frameChanged);
}

void codec37_init(PersistentCodecData37 * pcd, int width, int height)
{
	pcd->width = width;
	pcd->height = height;
	pcd->deltaSize = width * height * 2 + 0x3E00 + 0xBA00;
	pcd->deltaBuf = (byte *)calloc(pcd->deltaSize, 1);
	pcd->deltaBufs[0] = pcd->deltaBuf + 0x3E00;
	pcd->deltaBufs[1] = pcd->deltaBuf + width * height + 0xBA00;
	pcd->curtable = 0;
	pcd->table1 = (int16 *)calloc(255, sizeof(uint16));
}

void SmushPlayer::parseFOBJ()
{
	byte codec;
	CodecData cd;

	cd.out = _renderBitmap;
	cd.pitch = cd.outwidth = 320;
	cd.outheight = 200;
	cd.y = 0;
	cd.x = 0;
	cd.src = _cur + 0xE;
	cd.w = *(uint16 *)(_cur + 6);
	cd.h = *(uint16 *)(_cur + 8);
	cd.flags = 0;

	codec = _cur[0];

	switch (codec) {
	case 1:
		codec1(&cd);
		break;
	case 37:
		_frameChanged = codec37(sm->_gameId, &cd, &pcd37);
		break;
	default:
		error("invalid codec %d", codec);
	}
}

void SmushPlayer::parsePSAD()		// FIXME: Needs to append to
{																//      a sound buffer
	unsigned int pos, sublen, tag, idx, trk;
	bool new_mixer = false;
	byte *buf;
	pos = 0;

	trk = READ_LE_UINT16(_cur + pos);	/* FIXME: is this correct ? */
	pos += 2;

	for (idx = 0; idx < MAX_STREAMER; idx++) {
		if (_psadTrk[idx] == trk)
			break;
	}

	if (idx == MAX_STREAMER) {
		for (idx = 0; idx < MAX_STREAMER; idx++) {
			if (_psadTrk[idx] == 0 && g_mixer->_channels[idx] == NULL) {
				_psadTrk[idx] = trk;
				_saudSize[idx] = 0;
				new_mixer = true;
				break;
			}
		}
	}

	if (idx == MAX_STREAMER) {
		warning("PSAD table full\n");
		return;
	}

	pos += 8;											/* FIXME: what are these ? */

	while (pos < _frmeSize) {

		if (_saudSize[idx] == 0) {
			tag = READ_BE_UINT32(_cur + pos);
			pos += 4;
			if (tag != 'SAUD')
				error("trk %d: SAUD tag not found", trk);
			_saudSize[idx] = READ_BE_UINT32(_cur + pos);
			pos += 4;
		}

		if (_saudSubSize[idx] == 0) {
			_saudSubTag[idx] = READ_BE_UINT32(_cur + pos);
			pos += 4;
			_saudSubSize[idx] = READ_BE_UINT32(_cur + pos);
			pos += 4;
			_saudSize[idx] -= 8;
			debug(3, "trk %d: tag '%4s' size %x", trk, _cur + pos - 8, _saudSubSize[idx]);
		}

		sublen = _saudSubSize[idx] < (_frmeSize - pos) ? _saudSubSize[idx] : (_frmeSize - pos);

		switch (_saudSubTag[idx]) {
		case 'STRK':
			/* FIXME: what is this stuff ? */
			_strkRate[idx] = 22050;
			break;
		case 'SDAT':
			buf = (byte *)malloc(sublen);

			memcpy(buf, _cur + pos, sublen);

			debug(3, "trk %d: SDAT part len 0x%x rate %d", trk, sublen, _strkRate[idx]);

			if (new_mixer) {
				g_mixer->play_stream(NULL, idx, buf, sublen, _strkRate[idx],
																		 SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
			} else {
				g_mixer->append(idx, buf, sublen,
																_strkRate[idx], SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
			}
			break;
		case 'SMRK':
			_psadTrk[idx] = 0;
			break;
		case 'SHDR':
			/* FIXME: what is this stuff ? */
			break;
		default:
			error("trk %d: unknown tag inside PSAD", trk);
		}
		_saudSubSize[idx] -= sublen;
		_saudSize[idx] -= sublen;
		pos += sublen;
	}
}

void SmushPlayer::parseTRES()
{
	byte * txt = getStringTRES (READ_LE_UINT16(_cur + 16));
	drawStringTRES (READ_LE_UINT16(_cur), READ_LE_UINT16(_cur + 2), txt);
	if (txt != NULL)
		free (txt);
}

void SmushPlayer::parseXPAL()
{
	int num;
	int i;

	num = *(uint16 *)(_cur + 2);
	if (num == 0 || num == 0x200) {
		if (num == 0x200)
			memcpy(_fluPalette, _cur + 0x604, 0x300);

		for (i = 0; i < 0x300; i++) {
			_fluPalMul129[i] = _fluPalette[i] * 129;
			_fluPalWords[i] = *(uint16 *)(_cur + 4 + i * 2);
		}
		return;
	}

	parseNPAL();

	for (i = 0; i < 0x300; i++) {
		_fluPalMul129[i] += _fluPalWords[i];
		_fluPalette[i] = _fluPalMul129[i] >> 7;
	}

	_paletteChanged = true;
}

void SmushPlayer::parseFRME()
{
	_cur = _block;

	do {
		_frmeTag = nextBE32();
		_frmeSize = nextBE32();

		switch (_frmeTag) {
		case 'NPAL':
			parseNPAL();
			break;
		case 'FOBJ':
			parseFOBJ();
			break;
		case 'PSAD':
			parsePSAD();
			break;
		case 'TRES':
			parseTRES();
			break;
		case 'XPAL':
			parseXPAL();
			break;
		case 'IACT':
			parseIACT();
			break;
		case 'STOR':
		case 'FTCH':
			break;

		default:
			invalidblock(_frmeTag);
		}

		_cur += (_frmeSize + 1) & ~1;
	} while (_cur + 4 < _block + _blockSize);
}

void SmushPlayer::init()
{
	_renderBitmap = sm->_videoBuffer;
	codec37_init(&pcd37, 320, 200);

	memset(_saudSize, 0, sizeof(_saudSize));
	memset(_saudSubSize, 0, sizeof(_saudSubSize));
	memset(_psadTrk, 0, sizeof(_psadTrk));

	memset(_imusSize, 0, sizeof(_imusSize));
	memset(_imusSubSize, 0, sizeof(_imusSubSize));
	memset(_imusTrk, 0, sizeof(_imusTrk));
	memset(_imusData, 0, sizeof(_imusData));
	memset(_imusPos, 0, sizeof(_imusPos));
	memset(_imusChan, 0, sizeof(_imusChan));
	
	if (sm->_gameId == GID_DIG)
	{
		for (uint8 l = 0; l < SP_MAX_FONTS; l++) {
			_fonts[l] = NULL;
		}
		_buffer_tres = NULL;
		loadTres();
		loadFonts();
	}
}

void SmushPlayer::deinit()
{
	if (sm->_gameId == GID_DIG)
	{
		if (_buffer_tres != NULL)
			free (_buffer_tres);

		for (int l = 0; l < SP_MAX_FONTS; l++) {
			if (_fonts[l] != NULL) {
				free (_fonts[l]);
				_fonts[l] = NULL;
			}
		}
	}
}

void SmushPlayer::go()
{
	while (parseTag()) {
	}
}

void SmushPlayer::setPalette()
{
	int i;
	byte palette_colors[1024];
	byte *p = palette_colors;
	byte *data = _fluPalette;

	for (i = 0; i != 256; i++, data += 3, p += 4) {
		p[0] = data[0];
		p[1] = data[1];
		p[2] = data[2];
		p[3] = 0;
	}

	sm->_system->set_palette(palette_colors, 0, 256);
}

void SmushPlayer::startVideo(short int arg, byte *videoFile)
{
	int frameIndex = 0;

	_in = NULL;
	_paletteChanged = 0;
	_block = NULL;
	_blockTag = 0;
	_blockSize = 0;
	_cur = NULL;
	_renderBitmap = NULL;
	_frameSize = 0;
	_frmeTag = 0;
	_frmeSize = 0;
	_deltaBuf = NULL;
	_deltaBufSize = 0;

	pcd37.deltaBuf = NULL;
	pcd37.deltaBufs[0] = NULL;
	pcd37.deltaBufs[1] = NULL;
	pcd37.deltaSize = 0;
	pcd37.width = 0;
	pcd37.height = 0;
	pcd37.curtable = 0;
	pcd37.unk2 = 0;
	pcd37.unk3 = 0;
	pcd37.flags = 0;
	pcd37.table1 = NULL;
	pcd37.table_last_pitch = 0;
	pcd37.table_last_flags = 0;

	init();
	openFile(videoFile);

	if (_in == NULL)
		return;

	if (fileReadBE32() != 'ANIM')
		error("file is not an anim");

	fileSize = fileReadBE32();

	sm->videoFinished = 0;
	sm->_insaneState = 1;

	do {
		_frameChanged = 1;

		if (ftell(_in) >= fileSize)
			return;
#ifdef INSANE_DEBUG
		warning("Playing frame %d", frameIndex);
#endif

		parseTag();
		frameIndex++;

		if (_paletteChanged) {
			_paletteChanged = false;
			setPalette();
			sm->setDirtyColors(0, 255);
		}

		if (_frameChanged) {
			/* FIXME: not properly implemented after switch to new gfx code */

			sm->_system->copy_rect(sm->_videoBuffer, 320, 0, 0, 320, 200);
			sm->_system->update_screen();
			sm->waitForTimer(60);

			//sm->delta = sm->_system->waitTick(sm->delta);
		}

		sm->processKbd();

	} while (!sm->videoFinished);

	deinit();

	sm->_insaneState = 0;
	sm->exitCutscene();

}
