/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
 * Copyright (C) 2005 Joost Peters PSP Backend
 * Copyright (C) 2005 Thomas Mayer PSP Backend
 * Copyright (C) 2005 Paolo Costabel PSP Backend
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
 *
 */

#include <pspgu.h>
#include "common/stdafx.h"
#include "common/scummsys.h"

#include "common/rect.h"
#include "osys_psp.h"

#define min(a,b) (a > b ? b : a)

class OSystem_PSP_GU : public OSystem_PSP
{
public:
	struct Vertex
	{
		float u,v;
		float x,y,z;
	};

	OSystem_PSP_GU();
	~OSystem_PSP_GU();
	void updateScreen();
	void initSize(uint width, uint height, int overlayScale);
	int getDefaultGraphicsMode() const;
	bool setGraphicsMode(int mode);
	bool setGraphicsMode(const char *name);
	int getGraphicsMode() const;
	void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int cursorTargetScale);
	void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) ;
	void setPalette(const byte *colors, uint start, uint num);
	bool pollEvent(Event &event);
	int _graphicMode;
	struct Vertex *_vertices;
	unsigned short* _clut;
	unsigned short* _kbdClut;
	bool _keyboardVisible;
	int _keySelected;
	int _keyboardMode;
};

