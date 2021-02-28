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

#ifndef TRECISION_VIDEO_H
#define TRECISION_VIDEO_H

#include "graphics/pixelformat.h"

namespace Trecision {
class TrecisionEngine;

class GraphicsManager {
private:
	TrecisionEngine *_vm;

	Graphics::PixelFormat _screenFormat;

public :
	GraphicsManager(TrecisionEngine *vm);
	~GraphicsManager();
	
	bool _linearMode;
	bool _locked;
	uint16 _pitch;

	uint16 *_screenPtr;

	void lock();
	void unlock();
	void clearScreen();
	void showScreen(int px, int py, int dx, int dy);

	void vCopy(uint32 Sco, uint16 *Src, uint32 Len);
	void BCopy(uint32 Sco, uint8 *Src, uint32 Len);
	void DCopy(uint32 Sco, uint8 *Src, uint32 Len);

	uint16 palTo16bit(uint8 r, uint8 g, uint8 b);
	void updatePixelFormat(uint16 *p, uint32 len);
	uint16 restorePixelFormat(uint16 t);
	void color2RGB(uint16 a, uint8 *r, uint8 *g, uint8 *b);
	uint16 RGB2Color(uint8 r, uint8 g, uint8 b);

}; // end of class

} // end of namespace
#endif
