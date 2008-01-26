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

#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include "bits.h"
#include "colormap.h"

#include <ronin/ta.h>

class TextureManager {
 public:
	class Texture {
	public:
		Texture(uint16 *tex, unsigned int fsz, int width, int height, int cnt);
		void setTexture(int n, const uint8 *data, const CMap *cmap);
		void setup(int n, struct polygon_list &list, float &uscale, float &vscale) {
			list.mode2 = _mode2;
			list.texture = _texture + TA_TEXTUREMODE_ADDRESS(n*_framesize);
			uscale = _uscale;
			vscale = _vscale;
		}
		uint16 *getMemory() const { return _data; }
		unsigned int getFrameSize() const { return _framesize; }
		uint16 getCnt() const { return _cnt; }
	private:
		static void setTextureTwiddled(uint16 *tex, const uint8 *data,
					       const CMap *cmap, int sz, int extra=0);
		unsigned int _mode2, _texture;
		float _uscale, _vscale;
		uint16 *_data;
		uint16 _width, _height, _cnt;
		unsigned int _framesize;
	};

	TextureManager() : _freeListHead(NULL) {}
	Texture *allocateTexture(int width, int height,  int cnt);
	void freeTexture(Texture *t);
	void initTextures();

 private:
	struct FreeListNode {
		struct FreeListNode *next;
		unsigned int size;
	};
	FreeListNode *_freeListHead;
	void *allocateTexMem(unsigned int size);
	void freeTexMem(void *mem, unsigned int size);
};

#endif

