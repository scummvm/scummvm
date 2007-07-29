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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */
 
#ifndef KYRA_SCREEN_V2_H
#define KYRA_SCREEN_V2_H

#include "kyra/screen.h"

namespace Kyra {

class KyraEngine_v2;

class Screen_v2 : public Screen {
public:
	Screen_v2(KyraEngine_v2 *vm, OSystem *system);
	virtual ~Screen_v2();
	
	virtual void setScreenDim(int dim);
	const ScreenDim *getScreenDim(int dim);
	
	// palette handling
	void k2IntroFadeToGrey(int delay=0x54);
	
	// screen page handling
	void copyWsaRect(int x, int y, int w, int h, int dimState, int plotFunc, const uint8 *src,
					int unk1, const uint8 *unkPtr1, const uint8 *unkPtr2);
	
	// shape handling
	uint8 *getPtrToShape(uint8 *shpFile, int shape);
	const uint8 *getPtrToShape(const uint8 *shpFile, int shape);
	
	int getShapeScaledWidth(const uint8 *shpFile, int scale);
	int getShapeScaledHeight(const uint8 *shpFile, int scale);

	uint16 getShapeSize(const uint8 *shp);
	
	uint8 *makeShapeCopy(const uint8 *src, int index);
	
	void drawShape(uint8 pageNum, const uint8 *shapeData, int x, int y, int sd, int flags, ...);
	
	// rect handling
	virtual int getRectSize(int w, int h);
	
	// layer handling
	int getLayer(int x, int y);
	
	// mouse handling
	bool isMouseVisible() const;
private:
	KyraEngine_v2 *_vm;
	
	static const ScreenDim _screenDimTable[];
	static const int _screenDimTableCount;
	
	// maybe subclass screen for kyra3
	static const ScreenDim _screenDimTableK3[];
	static const int _screenDimTableCountK3;
};

} // End of namespace Kyra

#endif
