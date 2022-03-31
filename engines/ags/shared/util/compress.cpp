/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ags/shared/util/compress.h"
#include "ags/shared/ac/common.h"   // quit, update_polled_stuff
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/util/file.h"
#include "ags/shared/util/lzw.h"
#include "ags/shared/util/memory_stream.h"
#include "ags/globals.h"
#if AGS_PLATFORM_ENDIAN_BIG
#include "ags/shared/util/bbop.h"
#endif

namespace AGS3 {

using namespace AGS::Shared;

//-----------------------------------------------------------------------------
// RLE
//-----------------------------------------------------------------------------

static void cpackbitl(const uint8_t *line, size_t size, Stream *out) {
	size_t cnt = 0;               // bytes encoded

	while (cnt < size) {
		// note that the algorithm below requires signed operations
		int i = cnt;
		int j = i + 1;
		int jmax = i + 126;
		if ((size_t)jmax >= size)
			jmax = size - 1;

		if (i == (int)size - 1) {        //................last byte alone
			out->WriteInt8(0);
			out->WriteInt8(line[i]);
			cnt++;

		} else if (line[i] == line[j]) {    //....run
			while ((j < jmax) && (line[j] == line[j + 1]))
				j++;

			out->WriteInt8(i - j);
			out->WriteInt8(line[i]);
			cnt += j - i + 1;

		} else {                    //.............................sequence
			while ((j < jmax) && (line[j] != line[j + 1]))
				j++;

			out->WriteInt8(j - i);
			out->Write(line + i, j - i + 1);
			cnt += j - i + 1;

		}
	} // end while
}

static void cpackbitl16(const uint16_t *line, size_t size, Stream *out) {
	size_t cnt = 0;               // bytes encoded

	while (cnt < size) {
		// note that the algorithm below requires signed operations
		int i = cnt;
		int j = i + 1;
		int jmax = i + 126;
		if ((size_t)jmax >= size)
			jmax = size - 1;

		if (i == (int)size - 1) {        //................last byte alone
			out->WriteInt8(0);
			out->WriteInt16(line[i]);
			cnt++;

		} else if (line[i] == line[j]) {    //....run
			while ((j < jmax) && (line[j] == line[j + 1]))
				j++;

			out->WriteInt8(i - j);
			out->WriteInt16(line[i]);
			cnt += j - i + 1;

		} else {                    //.............................sequence
			while ((j < jmax) && (line[j] != line[j + 1]))
				j++;

			out->WriteInt8(j - i);
			out->WriteArray(line + i, j - i + 1, 2);
			cnt += j - i + 1;

		}
	} // end while
}

static void cpackbitl32(const uint32_t *line, size_t size, Stream *out) {
	size_t cnt = 0;               // bytes encoded

	while (cnt < size) {
		// note that the algorithm below requires signed operations
		int i = cnt;
		int j = i + 1;
		int jmax = i + 126;
		if ((size_t)jmax >= size)
			jmax = size - 1;

		if (i == (int)size - 1) {        //................last byte alone
			out->WriteInt8(0);
			out->WriteInt32(line[i]);
			cnt++;

		} else if (line[i] == line[j]) {    //....run
			while ((j < jmax) && (line[j] == line[j + 1]))
				j++;

			out->WriteInt8(i - j);
			out->WriteInt32(line[i]);
			cnt += j - i + 1;

		} else {                    //.............................sequence
			while ((j < jmax) && (line[j] != line[j + 1]))
				j++;

			out->WriteInt8(j - i);
			out->WriteArray(line + i, j - i + 1, 4);
			cnt += j - i + 1;

		}
	} // end while
}

static int cunpackbitl(uint8_t *line, size_t size, Stream *in) {
	size_t n = 0;                  // number of bytes decoded

	while (n < size) {
		int ix = in->ReadByte();     // get index byte
		if (in->HasErrors())
			break;

		char cx = ix;
		if (cx == -128)
			cx = 0;

		if (cx < 0) {                //.............run
			int i = 1 - cx;
			char ch = in->ReadInt8();
			while (i--) {
				// test for buffer overflow
				if (n >= size)
					return -1;

				line[n++] = ch;
			}
		} else {                     //.....................seq
			int i = cx + 1;
			while (i--) {
				// test for buffer overflow
				if (n >= size)
					return -1;

				line[n++] = in->ReadByte();
			}
		}
	}

	return in->HasErrors() ? -1 : 0;
}

static int cunpackbitl16(uint16_t *line, size_t size, Stream *in) {
	size_t n = 0;                  // number of bytes decoded

	while (n < size) {
		int ix = in->ReadByte();     // get index byte
		if (in->HasErrors())
			break;

		char cx = ix;
		if (cx == -128)
			cx = 0;

		if (cx < 0) {                //.............run
			int i = 1 - cx;
			unsigned short ch = in->ReadInt16();
			while (i--) {
				// test for buffer overflow
				if (n >= size)
					return -1;

				line[n++] = ch;
			}
		} else {                     //.....................seq
			int i = cx + 1;
			while (i--) {
				// test for buffer overflow
				if (n >= size)
					return -1;

				line[n++] = in->ReadInt16();
			}
		}
	}

	return in->HasErrors() ? -1 : 0;
}

static int cunpackbitl32(uint32_t *line, size_t size, Stream *in) {
	size_t n = 0;                  // number of bytes decoded

	while (n < size) {
		int ix = in->ReadByte();     // get index byte
		if (in->HasErrors())
			break;

		char cx = ix;
		if (cx == -128)
			cx = 0;

		if (cx < 0) {                //.............run
			int i = 1 - cx;
			unsigned int ch = in->ReadInt32();
			while (i--) {
				// test for buffer overflow
				if (n >= size)
					return -1;

				line[n++] = ch;
			}
		} else {                     //.....................seq
			int i = cx + 1;
			while (i--) {
				// test for buffer overflow
				if (n >= size)
					return -1;

				line[n++] = (unsigned int)in->ReadInt32();
			}
		}
	}

	return in->HasErrors() ? -1 : 0;
}

void rle_compress(const uint8_t *data, size_t data_sz, int image_bpp, Stream *out) {
	switch (image_bpp) {
	case 1: cpackbitl(data, data_sz, out); break;
	case 2: cpackbitl16(reinterpret_cast<const uint16_t *>(data), data_sz / sizeof(uint16_t), out); break;
	case 4: cpackbitl32(reinterpret_cast<const uint32_t *>(data), data_sz / sizeof(uint32_t), out); break;
	default: assert(0); break;
	}
}

void rle_decompress(uint8_t *data, size_t data_sz, int image_bpp, Stream *in) {
	switch (image_bpp) {
	case 1: cunpackbitl(data, data_sz, in); break;
	case 2: cunpackbitl16(reinterpret_cast<uint16_t *>(data), data_sz / sizeof(uint16_t), in); break;
	case 4: cunpackbitl32(reinterpret_cast<uint32_t *>(data), data_sz / sizeof(uint32_t), in); break;
	default: assert(0); break;
	}
}

void save_rle_bitmap8(Stream *out, const Bitmap *bmp, const RGB(*pal)[256]) {
	assert(bmp->GetBPP() == 1);
	out->WriteInt16(static_cast<uint16_t>(bmp->GetWidth()));
	out->WriteInt16(static_cast<uint16_t>(bmp->GetHeight()));
	// Pack the pixels
	cpackbitl(bmp->GetData(), bmp->GetWidth() * bmp->GetHeight(), out);
	// Save palette
	if (!pal) { // if no pal, write dummy palette, because we have to
		out->WriteByteCount(0, 256 * 3);
		return;
	}
	const RGB *ppal = *pal;
	for (int i = 0; i < 256; ++i) {
		out->WriteInt8(ppal[i].r);
		out->WriteInt8(ppal[i].g);
		out->WriteInt8(ppal[i].b);
	}
}

Shared::Bitmap *load_rle_bitmap8(Stream *in, RGB(*pal)[256]) {
	int w = in->ReadInt16();
	int h = in->ReadInt16();
	Bitmap *bmp = BitmapHelper::CreateBitmap(w, h, 8);
	if (!bmp) return nullptr;
	// Unpack the pixels
	cunpackbitl(bmp->GetDataForWriting(), w * h, in);
	// Load or skip the palette
	if (!pal) {
		in->Seek(3 * 256);
		return bmp;
	}
	RGB *ppal = *pal;
	for (int i = 0; i < 256; ++i) {
		ppal[i].r = in->ReadInt8();
		ppal[i].g = in->ReadInt8();
		ppal[i].b = in->ReadInt8();
	}
	return bmp;
}

//-----------------------------------------------------------------------------
// LZW
//-----------------------------------------------------------------------------

void save_lzw(Stream *out, const Bitmap *bmpp, const RGB(*pal)[256]) {
	// First write original bitmap's info and data into the memory buffer
	// NOTE: we must do this purely for backward compatibility with old room formats:
	// because they also included bmp width and height into compressed data!
	std::vector<uint8_t> membuf;
	{
		MemoryStream memws(membuf, kStream_Write);
		int w = bmpp->GetWidth(), h = bmpp->GetHeight(), bpp = bmpp->GetBPP();
		memws.WriteInt32(w * bpp); // stride
		memws.WriteInt32(h);
		switch (bpp) {
		case 1: memws.Write(bmpp->GetData(), w * h * bpp); break;
		case 2: memws.WriteArrayOfInt16(reinterpret_cast<const int16_t *>(bmpp->GetData()), w *h); break;
		case 4: memws.WriteArrayOfInt32(reinterpret_cast<const int32_t *>(bmpp->GetData()), w *h); break;
		default: assert(0); break;
		}
	}

	// Open same buffer for reading, and begin writing compressed data into the output
	MemoryStream mem_in(membuf);
	// NOTE: old format saves full RGB struct here (4 bytes, including the filler)
	if (pal)
		out->WriteArray(*pal, sizeof(RGB), 256);
	else
		out->WriteByteCount(0, sizeof(RGB) * 256);
	out->WriteInt32((uint32_t)mem_in.GetLength());

	// reserve space for compressed size
	soff_t cmpsz_at = out->GetPosition();
	out->WriteInt32(0);
	lzwcompress(&mem_in, out);
	soff_t toret = out->GetPosition();
	out->Seek(cmpsz_at, kSeekBegin);
	soff_t compressed_sz = (toret - cmpsz_at) - sizeof(uint32_t);
	out->WriteInt32(compressed_sz); // write compressed size
	// seek back to the end of the output stream
	out->Seek(toret, kSeekBegin);
}

void load_lzw(Stream *in, Bitmap **dst_bmp, int dst_bpp, RGB(*pal)[256]) {
	soff_t        uncompsiz;
	int *loptr;
	unsigned char *membuffer;
	int           arin;

	// NOTE: old format saves full RGB struct here (4 bytes, including the filler)
	if (pal)
		in->Read(*pal, sizeof(RGB) * 256);
	else
		in->Seek(sizeof(RGB) * 256);
	_G(maxsize) = in->ReadInt32();
	uncompsiz = in->ReadInt32();

	uncompsiz += in->GetPosition();
	_G(outbytes) = 0; _G(putbytes) = 0;

	update_polled_stuff_if_runtime();
	membuffer = lzwexpand_to_mem(in);
	update_polled_stuff_if_runtime();

	loptr = (int *)&membuffer[0];
	membuffer += 8;
#if AGS_PLATFORM_ENDIAN_BIG
	loptr[0] = BBOp::SwapBytesInt32(loptr[0]);
	loptr[1] = BBOp::SwapBytesInt32(loptr[1]);
	int bitmapNumPixels = loptr[0] * loptr[1] / dst_bpp;
	switch (dst_bpp) // bytes per pixel!
	{
	case 1:
	{
		// all done
		break;
	}
	case 2:
	{
		short *sp = (short *)membuffer;
		for (int i = 0; i < bitmapNumPixels; ++i) {
			sp[i] = BBOp::SwapBytesInt16(sp[i]);
}
		// all done
		break;
	}
	case 4:
	{
		int *ip = (int *)membuffer;
		for (int i = 0; i < bitmapNumPixels; ++i) {
			ip[i] = BBOp::SwapBytesInt32(ip[i]);
		}
		// all done
		break;
	}
  }
#endif // AGS_PLATFORM_ENDIAN_BIG

	update_polled_stuff_if_runtime();

	Bitmap *bmm = BitmapHelper::CreateBitmap((loptr[0] / dst_bpp), loptr[1], dst_bpp * 8);
	if (bmm == nullptr)
		quit("load_room: not enough memory to load room background");

	update_polled_stuff_if_runtime();

	for (arin = 0; arin < loptr[1]; arin++)
		memcpy(&bmm->GetScanLineForWriting(arin)[0], &membuffer[arin * loptr[0]], loptr[0]);

	update_polled_stuff_if_runtime();

	free(membuffer - 8);

	if (in->GetPosition() != uncompsiz)
		in->Seek(uncompsiz, kSeekBegin);

	update_polled_stuff_if_runtime();

	*dst_bmp = bmm;
}

} // namespace AGS3
