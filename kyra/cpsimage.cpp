/* ScummVM - Kyrandia Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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

#include "stdafx.h"
#include "resource.h"
#include "codecs.h"

#include "common/stream.h"

namespace Kyra {

	struct CPSResource {
		uint32 size;
		uint16 width;
	};

	static const CPSResource CPSResourceTable[] = {
		{ 64000, 320 }, { 7740, 180 }, { 46080, 320 }, { 0, 0 }
	};
	
	static int16 getWidthFromCPSRes(uint32 size) {
		int16 c = 0;
	
		for (; CPSResourceTable[c].size; ++c) {
			if (CPSResourceTable[c].size == size)
				return CPSResourceTable[c].width;
		}
	
		return -1;
	}

	CPSImage::CPSImage(uint8* buffer, uint32 size) {
		if (!buffer) {
			error("resource created without data");
		}
		Common::MemoryReadStream bufferstream(buffer, size);
		
		// reads in the Header
		_cpsHeader._filesize = bufferstream.readUint16LE() + 2;
		_cpsHeader._format = bufferstream.readUint16LE();
		_cpsHeader._imagesize = bufferstream.readUint16LE();
		_cpsHeader._pal = bufferstream.readUint32LE();
		
		// lets check a bit
		if(_cpsHeader._pal == 0x3000000) {
			warning("CPS images with a palette aren't supported");
			
			// skip 768 bytes
			// if this was a compressed palette you should have strange graphics
			bufferstream.seek(bufferstream.pos() + 768);
		}
		
		_image = new uint8[_cpsHeader._imagesize];
		assert(_image);
		
		uint8* imagebuffer = &buffer[bufferstream.pos()];
		assert(imagebuffer);
		
		if(_cpsHeader._format == 4) {
			Compression::decode80(imagebuffer, _image);
		} else if(_cpsHeader._format == 3) {
			Compression::decode3(imagebuffer, _image, _cpsHeader._imagesize);
		} else {
			error("unknown CPS format %d", _cpsHeader._format);
		}
		
		int16 width = getWidthFromCPSRes(_cpsHeader._imagesize);
		
		if(width == -1) {
			warning("unknown CPS width(imagesize: %d)", _cpsHeader._imagesize);
			delete [] buffer;
			return;
		}
		
		_width = (uint16)width;
		_height = _cpsHeader._imagesize / _width;
		
		_transparency = -1;
		
		delete [] buffer;
	}
	
	CPSImage::~CPSImage() {
		delete [] _image;
		delete _ownPalette;
	}
	
	void CPSImage::drawToPlane(uint8* plane, uint16 planepitch, uint16 planeheight, uint16 x, uint16 y) {
		if (_transparency == -1) {
			// 'fast' blitting
			
			uint8* src = _image;
			uint8* dst = &plane[y * planepitch + x];
			uint32 copysize = planepitch - x;
			
			if (copysize > _width)
				copysize = _width;
			
			for (uint16 y_ = 0; y_ < _height && y + y_ < planeheight; ++y_) {
				memcpy(dst, src, copysize * sizeof(uint8));
				dst += planepitch;
				src += _width;
			}
			
		} else {
			// oh no! we have transparency so we have a very slow copy :/
			uint8* src = _image;
			uint8* dst = &plane[y * planepitch + x];
			
			for (uint16 yadd = 0; yadd < _height; ++yadd) {
				for (uint16 xadd = 0; xadd < _width; ++xadd) {
					if (*src == _transparency) {
						++dst;
						++src;
					} else {
						*dst++ = *src++;
					}
				}
				
				dst += planepitch - _width;
			}
		}
	}
	
	void CPSImage::drawToPlane(uint8* plane, uint16 planepitch, uint16 planeheight, uint16 x, uint16 y,
								uint16 srcx, uint16 srcy, uint16 srcwidth, uint16 srcheight) {
		if (_transparency == -1) {
			// 'fast' blitting
			
			uint8* src = &_image[srcy * _width + srcx];
			uint8* dst = &plane[y * planepitch + x];
			uint32 copysize = planepitch - x;
			
			if (copysize > srcwidth)
				copysize = srcwidth;
			
			for (uint16 y_ = 0; y_ < srcheight && y + y_ < planeheight; ++y_) {
				memcpy(dst, src, copysize * sizeof(uint8));
				dst += planepitch;
				src += _width;
			}
			
		} else {
			// oh no! we have transparency so we have a very slow copy :/
			// blit it without transparency
			uint8* src = &_image[srcy * _width + srcx];
			uint8* dst = &plane[y * planepitch + x];
			
			for (uint16 yadd = 0; yadd < _height; ++yadd) {
				for (uint16 xadd = 0; xadd < _width; ++xadd) {
					if (*src == _transparency) {
						++dst;
						++src;
					} else {
						*dst++ = *src++;
					}
				}
				
				dst += planepitch - _width;
				src += _width - srcwidth;
			}
		}
	}
} // end of namespace Kyra

