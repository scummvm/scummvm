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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef VOYEUR_GRAPHICS_H
#define VOYEUR_GRAPHICS_H

//#include "voyeur/files.h"
#include "voyeur/game.h"
#include "common/scummsys.h"
#include "common/array.h"
#include "graphics/surface.h"

namespace Voyeur {

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define PALETTE_COUNT 256
#define PALETTE_SIZE (256 * 3)

class VoyeurEngine;
class GraphicsManager;
class PictureResource;
class ViewPortResource;

typedef void (GraphicsManager::*GraphicMethodPtr)(); 
typedef void (GraphicsManager::*ViewPortSetupPtr)(ViewPortResource *);
typedef void (GraphicsManager::*ViewPortAddPtr)(ViewPortResource *, void *v2, void *v3);
typedef void (GraphicsManager::*ViewPortRestorePtr)(ViewPortResource *);

class GraphicsManager {
public:
	VoyeurEngine *_vm;
	bool _palFlag;
	byte _VGAColors[PALETTE_SIZE];
	Common::Array<byte *> _colorChain;
	PictureResource *_backgroundPage;
	int _SVGAPage;
	int _SVGAMode;
	ViewPortResource *_vPort;
private:
	static void fadeIntFunc();
	static void vDoFadeInt();
	static void vDoCycleInt();

	void addIntNode(IntNode *node);
public:
	GraphicsManager();
	void setVm(VoyeurEngine *vm) { _vm = vm; }
	void sInitGraphics();

	void vInitColor();
	void addFadeInt();

	void setupMCGASaveRect(ViewPortResource *viewPort);
	void addRectOptSaveRect(ViewPortResource *viewPort, void *v2, void *v3);	
	void restoreMCGASaveRect(ViewPortResource *viewPort);
	void addRectNoSaveBack(ViewPortResource *viewPort, void *v2, void *v3);
};

} // End of namespace Voyeur

#endif /* VOYEUR_GRAPHICS_H */
