/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "debug.h"
#include "colormap.h"
#include "material.h"
#include "texture_manager.h"

static int twiddletab[1024];

static void init_twiddletab()
{
  int x;
  for(x=0; x<1024; x++)
    twiddletab[x] = (x&1)|((x&2)<<1)|((x&4)<<2)|((x&8)<<3)|((x&16)<<4)|
      ((x&32)<<5)|((x&64)<<6)|((x&128)<<7)|((x&256)<<8)|((x&512)<<9);
}

TextureManager::Texture::Texture(uint16 *tex, unsigned int fsz, int width, int height, int cnt)
	: _data(tex), _width(width), _height(height), _cnt(cnt), _framesize(fsz)
{
	_texture = TA_TEXTUREMODE_ARGB1555|TA_TEXTUREMODE_ADDRESS(_data);
	_mode2 = TA_POLYMODE2_BLEND_SRC|TA_POLYMODE2_FOG_DISABLED|
		TA_POLYMODE2_TEXTURE_REPLACE|TA_POLYMODE2_BILINEAR_FILTER;
	
	switch(_width) {
	case 4:
		/* XXX */
	case 8:
		_mode2 |= TA_POLYMODE2_U_SIZE_8;
		_uscale = 1.0/8;
		break;
	case 16:
		_mode2 |= TA_POLYMODE2_U_SIZE_16;
		_uscale = 1.0/16;
		break;
	case 32:
		_mode2 |= TA_POLYMODE2_U_SIZE_32;
		_uscale = 1.0/32;
		break;
	case 64:
		_mode2 |= TA_POLYMODE2_U_SIZE_64;
		_uscale = 1.0/64;
		break;
	case 128:
		_mode2 |= TA_POLYMODE2_U_SIZE_128;
		_uscale = 1.0/128;
		break;
	case 256:
		_mode2 |= TA_POLYMODE2_U_SIZE_256;
		_uscale = 1.0/256;
		break;
	default:
		assert(0);
		break;
	}
	switch(_height) {
	case 4:
		/* XXX */
	case 8:
		_mode2 |= TA_POLYMODE2_V_SIZE_8;
		_vscale = 1.0/8;
		break;
	case 16:
		_mode2 |= TA_POLYMODE2_V_SIZE_16;
		_vscale = 1.0/16;
		break;
	case 32:
		_mode2 |= TA_POLYMODE2_V_SIZE_32;
		_vscale = 1.0/32;
		break;
	case 64:
		_mode2 |= TA_POLYMODE2_V_SIZE_64;
		_vscale = 1.0/64;
		break;
	case 128:
		_mode2 |= TA_POLYMODE2_V_SIZE_128;
		_vscale = 1.0/128;
		break;
	case 256:
		_mode2 |= TA_POLYMODE2_V_SIZE_256;
		_vscale = 1.0/256;
		break;
	default:
		assert(0);
		break;
	}
	assert((_width > 4 && _height > 4) || (_width == 4 && _height == 4));
}

void TextureManager::Texture::
setTextureTwiddled(uint16 *tex, const uint8 *data,
		   const CMap *cmap, int sz, int extra)
{
	for(int y=0; y<sz; y++, data += extra)
		for(int x=0; x<sz; x++) {
			uint8 n = *data++;
			uint16 t;
			if(!n)
				t = 0;
			else {
				uint8 *c = (uint8*)cmap->_colors+3*n;
				t = 0x8000 |
					((c[0]<<7)&0x7c00)|
					((c[1]<<2)&0x03e0)|
					((c[2]>>3)&0x001f);
			}
			*(uint16*)(((twiddletab[x]<<2)|(twiddletab[y]<<1))
				   + (char *)tex) = t;
		}
}

void TextureManager::Texture::setTexture(int n, const uint8 *data, const CMap *cmap)
{
	uint16 *tex = (uint16 *)(n * _framesize + (char *)_data);

	if(_width == _height)
		setTextureTwiddled(tex, data, cmap, _width);
	else if(_width > _height) {
		const uint8 *d2 = data;
		uint16 *t2 = tex;
		unsigned int fsz = _height * _height * 2;
		for(int x = 0; x < _width; x += _height) {
			setTextureTwiddled(t2, d2, cmap, _height, _width - _height);
			d2 += _height;
			t2 += _height * _height;
		}
	} else {
		const uint8 *d2 = data;
		uint16 *t2 = tex;
		for(int x = 0; x < _height; x += _width) {
			setTextureTwiddled(t2, d2, cmap, _width);
			d2 += _width * _width;
			t2 += _width * _width;
		}
	}
}

TextureManager::Texture *TextureManager::allocateTexture(int width, int height, int cnt)
{
	unsigned int framesize = width * height * 2;
	uint16 *tex = (uint16 *) allocateTexMem(framesize * cnt);
	return new Texture(tex, framesize, width, height, cnt);
}

void TextureManager::freeTexture(Texture *t)
{
	freeTexMem(t->getMemory(), t->getFrameSize()*t->getCnt());
	delete t;
}

void *TextureManager::allocateTexMem(unsigned int size)
{
	assert(size > 0 && !(size&7));
	// First fit
	FreeListNode *n, **link;
	for(link = &_freeListHead; (n = *link) != NULL; link = &n->next)
		if(n->size >= size)
			break;
	assert(n != NULL);
	if(n->size > size) {
		FreeListNode *n2 = (FreeListNode *)(size + (char *)n);
		n2->size = n->size - size;
		n2->next = n->next;
		*link = n2;
	} else {
		*link = n->next;
	}
	return (void *)n;
}

void TextureManager::freeTexMem(void *mem, unsigned int size)
{
	assert(size > 0 && !(size&7) && mem != NULL);

	FreeListNode *n, **link;
	for(link = &_freeListHead; (n = *link) != NULL; link = &n->next)
		if(mem == (n->size + (char *)n))
			break;
		else if(mem < n) {
			n = NULL;
			break;
		} else
			assert(mem > (n->size + (char *)n));
	if(n == NULL) {
		n = (FreeListNode *)mem;
		n->size = size;
		n->next = *link;
		*link = n;
	} else {
		n->size += size;
	}

	// Join this block with the next one if possible
	if((char *)n->next == n->size + (char *)n) {
		n->size += n->next->size;
		n->next = n->next->next;
	}

	assert(n->next == NULL || (char *)n->next > n->size + (char *)n);
}

void TextureManager::initTextures()
{
	unsigned int aperture = 0x180000;
	_freeListHead = (FreeListNode *)ta_txalloc(aperture);
	_freeListHead->next = NULL;
	_freeListHead->size = aperture;
	init_twiddletab();
}
