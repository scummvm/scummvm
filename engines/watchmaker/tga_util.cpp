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

#include <cstring>
#include <cstdlib>
#include "SDL_image.h"
#include "watchmaker/tga_util.h"
#include "watchmaker/types.h"
#include "watchmaker/render.h"
#include "watchmaker/utils.h"

namespace Watchmaker {

#pragma pack(1)
struct STGAHeader {
	unsigned char idLength;                                    // length of ID string
	unsigned char mapType;                                     // color map type - pres/abs/grey/col
	unsigned char imageType;                                   // image type code - u/c cm/bw/tc
	unsigned short mapOrigin;                                  // starting index of map
	unsigned short mapLength;                                  // length of map
	unsigned char mapWidth;                                    // width of map in bits
	unsigned short xOrigin;                                    // x-origin of image
	unsigned short yOrigin;                                    // y-origin of image
	unsigned short imageWidth;                                 // width of image
	unsigned short imageHeight;                                // height of image
	unsigned char pixelDepth;                                  // bits per pixel
	unsigned char imageDesc;                                   // image descriptor - alpha/ori/interleave

	STGAHeader(Common::SeekableReadStream &stream) {
		idLength = stream.readByte();
		mapType = stream.readByte();
		imageType = stream.readByte();
		mapOrigin = stream.readUint16LE();
		mapLength = stream.readUint16LE();
		mapWidth = stream.readByte();
		xOrigin = stream.readUint16LE();
		yOrigin = stream.readUint16LE();
		imageWidth = stream.readUint16LE();
		imageHeight = stream.readUint16LE();
		pixelDepth = stream.readByte();
		imageDesc = stream.readByte();
		stream.seek(SEEK_CUR, idLength);
	}
};
#pragma pack()

char TGAPal[768];//, *TGAFileMem;
unsigned int TGAFileLen; //, CurTGAFilePointer;
const PixelFormat RGBA8888(32, 8, 8, 8, 8, 24, 16, 8, 0);

/* -----------------10/06/99 16.07-------------------
 *                      SaveTga
 * --------------------------------------------------*/
unsigned char SaveTga(const char *s, unsigned char *image, unsigned short xdim, unsigned short ydim, unsigned char flag) {
//	32-bit tga header
	uint8 TgaHeader[] = {0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x10, 0x00, 0x20, 0x00};
#if 0
//	t3dU8 TgaHeader2[]={0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x18, 0x00};
	FILE *f;
	int16 i, j;
	uint8  r, g, b, a;

	if (!(f = fopen(s, "wb")))                                                                  // open destination file
		return 0;

	TgaHeader[12] = (uint8)(xdim & 0xff);                                                       // encode dimensions
	TgaHeader[13] = (uint8)(xdim >> 8) & 0xff;
	TgaHeader[14] = (uint8)(ydim & 0xff);
	TgaHeader[15] = (uint8)(ydim >> 8) & 0xff;
	fwrite(TgaHeader, 1, 18, f);                                                                // write patched header

	for (i = 0; i < ydim; i++) {
		for (j = 0; j < xdim; j++) {
			r = (uint8)image[i * xdim * 4 + j * 4 + 0];                                         // gets r,g,b components
			g = (uint8)image[i * xdim * 4 + j * 4 + 1];
			b = (uint8)image[i * xdim * 4 + j * 4 + 2];
			if (flag & TGA_READALPHABITS)
				a = (uint8)image[i * xdim * 4 + j * 4 + 3];                                     // gets alpha val
			else
				a = 0;

			fwrite(&r, 1, 1, f);                                                                // write r,g,b,a, components
			fwrite(&g, 1, 1, f);
			fwrite(&b, 1, 1, f);
			fwrite(&a, 1, 1, f);
		}
	}
	fclose(f);
#endif
	return 1;
}

/* -----------------10/11/98 16.22-------------------
 *    TGAread - reads from a preloaded file
 * --------------------------------------------------*/
unsigned int TGAread(void *ptr, unsigned int size, unsigned int n, Common::SeekableReadStream &stream) {
	unsigned int len = size * n;
#if 0
	if ((!ptr) || (!len) || (/*stream.pos() > stream.size()*/)) {
		warning("Overrun TGA parsing\n");
		return 0;
	}
#endif
	stream.read(ptr, len);

	return n;
}

/* -----------------10/11/98 14.59-------------------
 *              decode a TGA RLE line
 * --------------------------------------------------*/
static int ReadRLERow(unsigned char *p, int n, int bpp, Common::SeekableReadStream &stream) {
#define RLEBUFSIZ 512                                      // size of largest possible RLE packet

	int    inRawPacket = 0;                                    // flags processing state for RLE data
	int    inRLEPacket = 0;                                    // flags processing state for RLE data
	unsigned int packetSize = 0;                               // records current RLE packet size in bytes
	char   rleBuf[RLEBUFSIZ] = {};

	unsigned int   value = 0;
	int      i = 0;
	static char *q;

	while (n > 0) {
		if (inRLEPacket) {
			if ((int)(packetSize * bpp) > n) {
				value = n / bpp;                                        // calculate pixel count
				packetSize -= value;
				n = 0;
			} else {
				n -= packetSize * bpp;
				value = packetSize;
				packetSize = 0;
				inRLEPacket = 0;
			}
			while (value > 0) {
				*p++ = rleBuf[0];
				if (bpp > 1) *p++ = rleBuf[1];
				if (bpp > 2) *p++ = rleBuf[2];
				if (bpp > 3) *p++ = rleBuf[3];
				value--;
			}
		} else if (inRawPacket) {
			if ((int)(packetSize * bpp) > n) {
				value = n;
				packetSize -= n / bpp;
				n = 0;
			} else {
				value = packetSize * bpp;                               // calculate byte count
				n -= value;
				inRawPacket = 0;
			}
			for (i = 0; i < (int)value; ++i) *p++ = *q++;
		} else {
//          No accumulated data in buffers, so read from file
			TGAread(&packetSize, 1, 1, stream);
			if (packetSize & 0x80) {
				packetSize &= 0x7f;
				packetSize++;
				if ((int)(packetSize * bpp) > n) {
					value = n / bpp;                                       // calculate pixel count
					packetSize -= value;
					inRLEPacket = 1;
					n = 0;
				} else {
					n -= packetSize * bpp;
					value = packetSize;
				}
				if (TGAread(rleBuf, 1, bpp, stream) != (size_t)bpp) return (-1);
				while (value > 0) {
					*p++ = rleBuf[0];
					if (bpp > 1) *p++ = rleBuf[1];
					if (bpp > 2) *p++ = rleBuf[2];
					if (bpp > 3) *p++ = rleBuf[3];
					value--;
				}
			} else {
				packetSize++;
//              Maximum for packetSize is 128 so as long as RLEBUFSIZ
//              is at least 512, and bpp is not greater than 4
//              we can read in the entire raw packet with one operation.
				if (TGAread(rleBuf, bpp, packetSize, stream) != packetSize)
					return (-1);
//              But is there enough room to copy them to our line buffer?
				if ((int)(packetSize) * bpp > n) {
					value = n;                                             // number of bytes remaining
					packetSize -= n / bpp;
					inRawPacket = 1;
					n = 0;
				} else {
					value = packetSize * bpp;                              // calculate byte count
					n -= value;
				}
				for (i = 0, q = rleBuf; i < (int)value; ++i) *p++ = *q++;
			}
		}
	}
	return (0);
}

/* -----------------10/11/98 15.01-------------------
 *              Loads and decode a TGA Row
 * --------------------------------------------------*/
static bool loadTGARow(STGAHeader &th, unsigned char *buf, Common::SeekableReadStream &stream) {
	unsigned char tmp[4];
	int a, bpp, bpc;

	bpp = (th.pixelDepth + 7) >> 3;
	bpc = (th.mapWidth + 7) >> 3;

	if (buf == nullptr)
		return false;

//	if compressed
	if (th.imageType > 8)
		ReadRLERow(buf, th.imageWidth * bpp, bpp, stream);
//	if uncompressed
	else
		TGAread(buf, bpp, th.imageWidth, stream);

//	adjust pixel format
	for (a = (th.imageWidth - 1); a >= 0; a--) {
		switch (th.pixelDepth) {
		case 32:
			tmp[0] = buf[a * bpp + 3];
			tmp[1] = buf[a * bpp + 0];
			tmp[2] = buf[a * bpp + 1];
			tmp[3] = buf[a * bpp + 2];
			break;

		case 24:
			tmp[0] = 0;
			tmp[1] = buf[a * bpp + 0];
			tmp[2] = buf[a * bpp + 1];
			tmp[3] = buf[a * bpp + 2];
			break;

		case 16:
		case 15:
			tmp[0] = 0;
			tmp[1] = ((buf[a * bpp + 0] & 0x1F) << 3);
			tmp[2] = (((buf[a * bpp + 1] & 0x3) << 3) + ((buf[a * bpp + 0] & 0xE0) >> 5)) << 3;
			tmp[3] = ((buf[a * bpp + 1] & 0x7F) << 1);
			break;

		case 8:
			tmp[0] = 0;
			tmp[1] = TGAPal[ buf[a] * bpc + 0 ];
			tmp[2] = TGAPal[ buf[a] * bpc + 1 ];
			tmp[3] = TGAPal[ buf[a] * bpc + 2 ];
			break;
		}

//		stores values
		buf[a * 4 + 0] = tmp[0];
		buf[a * 4 + 1] = tmp[1];
		buf[a * 4 + 2] = tmp[2];
		buf[a * 4 + 3] = tmp[3];
	}

	return (true);
}

/* -----------------10/06/99 16.08-------------------
 *      loads tga file using d3d puxel format
 * --------------------------------------------------*/
Surface *ReadTgaImage(const char *Name, Common::SeekableReadStream *stream, PixelFormat &format, unsigned int flag) {
#if 0
	uint8 *data = new uint8[stream->size()];
	stream->read(data, stream->size());
	auto rwops = SDL_RWFromConstMem(data, stream->size());
	auto loaded = IMG_LoadTGA_RW(rwops);
	Surface *surface = new Surface;
	surface->width = loaded->w;
	surface->height = loaded->h;
	surface->sdl_surface = loaded;
	surface->data = loaded->pixels;
	surface->dataSize = surface->width*surface->height*loaded->format->BytesPerPixel;
	free(rwops);
	delete[] data;
	return surface;

	//uint32 dwWidth, dwHeight;
	uint32 *lpLP;
	uint16 *lpSP;
	byte *lpCP;
	unsigned long m;
	int s = 0, i, j;
	int alpha_shift = 0, alpha_scale = 1;
	char bAlpha = 0, bFlip = 0;
	unsigned char *TGARow = nullptr;

	if (flag & 1)                                                                                // read alpa byte?
		bAlpha = 1;
	if (flag & rSURFACEFLIP)                                                                    // flip tga orientation?
		bFlip = 1;

	if (!stream->good())
		return nullptr;

	STGAHeader th(*stream);

	// TODO: Implement
	int rBitMask = (format.rBits() << 1) - 1;
	int gBitMask = (format.gBits() << 1) - 1;
	int bBitMask = (format.bBits() << 1) - 1;
	int aBitMask = (format.aBits() << 1) - 1;
	int rgbaMask = (rBitMask << format.rShift) |
	               (gBitMask << format.gShift) |
	               (bBitMask << format.bShift) |
	               (aBitMask << format.aShift);
	//  Determine the red, green and blue masks' shift and scale.
	/*
	for (s = 0, m = rBitMask; !(m & 1);
	        s++, m >>= 1);
	int red_shift = s;
	int red_scale = 255 / (rBitMask >> s);
	for (s = 0, m = gBitMask; !(m & 1);
	        s++, m >>= 1);
	int green_shift = s;
	int green_scale = 255 / (gBitMask >> s);
	for (s = 0, m = bBitMask; !(m & 1);
	        s++, m >>= 1);
	int blue_shift = s;
	int blue_scale = 255 / (bBitMask >> s);
	if (bAlpha) {
		for (s = 0, m = rgbaMask; !(m & 1);    s++, m >>= 1);

		alpha_shift = s;
		if ((rgbaMask >> s))
			alpha_scale = 255 / (rgbaMask >> s);
		else
			alpha_scale = 255;

		if (alpha_scale == 0)
			alpha_scale = 1;
	}
	 */
	int red_scale = 0;
	int green_scale = 0;
	int blue_scale = 0;


	int rowWidth = format.bytesPerPixel * th.imageWidth;
	int dstHeight = th.imageHeight;

	Surface *dst = new Surface();
	dst->data = new byte[rowWidth * dstHeight] {};
	dst->width = rowWidth;
	dst->height = dstHeight;
	int pitch = rowWidth;

//	Each RGB bit count requires different pointers
	switch (format.bytesPerPixel * 8) {
	case 32 :
		TGARow = new byte[rowWidth] {};
		for (j = 0; j < th.imageHeight; j++) {
//				Point to next row in texture surface
			if (bFlip)
				lpLP = (uint32 *)(((char *)dst->data) + pitch * ((dstHeight - 1) - j));
			else
				lpLP = (uint32 *)(((char *)dst->data) + pitch * j);
			if (!loadTGARow(th, TGARow, *stream)) {                                                    // read a row
				DebugLogWindow("Error reading TGA row (loadTGARow).");
				return nullptr;
			}
			for (i = 0; i < th.imageWidth; i++) {
				unsigned int o_r, o_g, o_b;
				unsigned int r, g, b, a;

//				Read each value, scale it and shift it into position
				r = ((o_r = TGARow[i * 4 + 3]) / red_scale);
				g = ((o_g = TGARow[i * 4 + 2]) / green_scale);
				b = ((o_b = TGARow[i * 4 + 1]) / blue_scale);
				*lpLP = (r << format.rShift) | (g << format.gShift) | (b << format.bShift) ;//| (255 << format.aShift);
				if (bAlpha) {
					a = TGARow[i * 4 + 0] / alpha_scale;
					a = 0;
					if (r != 0 && g != 0 && b != 0)
						a = 255 / alpha_scale;
					else
						a = 0;
					*lpLP |= (a << format.aShift);
				}
				lpLP++;
			}
		}
		delete [] TGARow;
		break;
	case 16 :
		TGARow = new unsigned char[	th.imageWidth * 4]{};
		for (j = 0; j < (int)th.imageHeight; j++) {
			if (bFlip)
				lpSP = (unsigned short *)(((char *)dst->data) + pitch * ((dstHeight - 1) - j));
			else
				lpSP = (unsigned short *)(((char *)dst->data) + pitch * j);
			if (!loadTGARow(th, TGARow, *stream)) {
				DebugLogWindow("Error reading TGA row (loadTGARow).");
				return nullptr;
			}

			for (i = 0; i < (int)th.imageWidth; i++) {
				unsigned int o_r, o_g, o_b;
				unsigned int r, g, b, a;

//					Read each value, scale it and shift it into position
				r = ((o_r = TGARow[i * 4 + 3]) / red_scale);
				g = ((o_g = TGARow[i * 4 + 2]) / green_scale);
				b = ((o_b = TGARow[i * 4 + 1]) / blue_scale);
				a = 255;
				if (bAlpha) {
//						a = TGARow[i*4+0];
//						if (a==0)
					{
						a = ((o_r + o_g + o_b) / 3);
						if (a)
							a = 255;
					}
//						else
					{
//							a=a;
					}
					a /= alpha_scale;

					/*                      if (a==0)
					                        {
					                            r=255;
					                            g=b=0;
					                        }
					*/
				}
				*lpSP = (unsigned short)((r << format.rShift) | (g << format.gShift) | (b << format.bShift));// | (a << format.aShift));
				lpSP++;
			}
		}
		delete[] TGARow;
		break;
	case 8:
		TGARow = new unsigned char[th.imageWidth * 4]{};
		for (j = 0; j < (int)th.imageHeight; j++) {
			if (bFlip)
				lpCP = (unsigned char *)(((char *)dst->data) + pitch * ((dstHeight - 1) - j));
			else
				lpCP = (unsigned char *)(((char *)dst->data) + pitch * j);
			if (!loadTGARow(th, TGARow, *stream)) {
				DebugLogWindow("Error reading TGA row (loadTGARow).");
				return nullptr;
			}
			for (i = 0; i < (int)th.imageWidth; i++) {
				unsigned int r, g, b, a;
				r = TGARow[i * 4 + 3] / red_scale;
				g = TGARow[i * 4 + 2] / green_scale;
				b = TGARow[i * 4 + 1] / blue_scale;
				*lpCP = (r << format.rShift) | (g << format.gShift) | (b << format.bShift);
				if (bAlpha) {
					a = TGARow[i * 4 + 0] / alpha_scale;
					a = 0;
					if (r != 0 && g != 0 && b != 0)
						a = 255 / alpha_scale;
					*lpCP |= (a << alpha_shift);
				}
				lpCP++;
			}
		}
		delete[] TGARow;
		break;
	default:
//			This wasn't a format I recognize
		warning("Unknown pixel format (loadtex).");
		return nullptr;
	}

	return dst;
#endif
}

} // End of namespace Watchmaker
