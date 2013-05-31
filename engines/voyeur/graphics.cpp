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

#include "voyeur/graphics.h"
#include "voyeur/game.h"
#include "voyeur/voyeur.h"
#include "engines/util.h"
#include "graphics/surface.h"

namespace Voyeur {

GraphicsManager::GraphicsManager() {
	_SVGAPage = 0;
	_SVGAMode = 0;
	_SVGAReset = 0;
	_screenOffset = 0;
	_palFlag = false;
	_MCGAMode = false;
	_saveBack = false;
	_clipPtr = NULL;
}

void GraphicsManager::sInitGraphics() {
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT, false);
}

void GraphicsManager::addFadeInt() {
	IntNode &node = _vm->_eventsManager._fadeIntNode;
	node._intFunc = fadeIntFunc;
	node._flags = 0;
	node._curTime = 0;
	node._timeReset = 1;

	_vm->_intPtr.addIntNode(&node);
}

void GraphicsManager::vInitColor() {
	_vm->_eventsManager._fadeIntNode._intFunc = vDoFadeInt;
	_vm->_eventsManager._cycleIntNode._intFunc = vDoCycleInt;
	// TODO: more
}

void GraphicsManager::fadeIntFunc() {

}

void GraphicsManager::vDoFadeInt() {
	
}

void GraphicsManager::vDoCycleInt() {

}

void GraphicsManager::setupMCGASaveRect(ViewPortResource *viewPort) {
	_MCGAMode = true;

	if (viewPort->_activePage) {
		viewPort->_activePage->_flags |= 1;
		Common::Rect *clipRect = _clipPtr;
		_clipPtr = &viewPort->_clipRect;

		sDrawPic(viewPort->_activePage, viewPort->_picResource, Common::Point(), NULL);

		_clipPtr = clipRect;
	}

	viewPort->_field42 = -1;
}

void GraphicsManager::addRectOptSaveRect(ViewPortResource *viewPort, void *v2, void *v3) {

}

void GraphicsManager::restoreMCGASaveRect(ViewPortResource *viewPort) {

}

void GraphicsManager::addRectNoSaveBack(ViewPortResource *viewPort, void *v2, void *v3) {

}

void GraphicsManager::sDrawPic(PictureResource *srcPic, PictureResource *destPic, 
		const Common::Point &offset, void *v3) {
	int var4C = 0;
	int width1, width2;
	int widthDiff, widthDiff2;
	int height1;
	int srcOffset;
	int screenOffset;
	int flags1, flags2;
	PictureResource *saveddestPic = NULL;
	Common::Rect newBounds;
	Common::Rect backBounds;
	int var24;
	bool isClipped = false;
	int var52;
	int var20, var22;
	int var26;
	byte *imgData1, *imgData2;
	byte *srcP, *destP;

	if (srcPic->_flags & 0x8000) {
		srcPic = srcPic->_secondPicture;
		warning("TODO: Particularly validate 'extended' pictures");
	}
	if (destPic->_flags & 0x8000) {
		saveddestPic = destPic;
		destPic = destPic->_secondPicture;
		warning("TODO: Particularly validate 'extended' pictures");
	}

	Common::Point ofs = Common::Point(offset.x + srcPic->_bounds.left - destPic->_bounds.left, 
		offset.y + srcPic->_bounds.top - destPic->_bounds.top);
	width1 = width2 = srcPic->_bounds.width();
	height1 = srcPic->_bounds.height();
	srcOffset = 0;
	flags1 = srcPic->_flags;
	flags2 = destPic->_flags;

	if (flags1 & 1) {
		if (_clipPtr) {
			int xs = _clipPtr->left - srcPic->_bounds.left;
			int ys = _clipPtr->top - srcPic->_bounds.top;
			newBounds = Common::Rect(xs, ys, xs + _clipPtr->width(), ys + _clipPtr->height());
		} else if (saveddestPic) {
			int xs = saveddestPic->_bounds2.left - destPic->_bounds.left;
			int ys = saveddestPic->_bounds2.top - destPic->_bounds.top;
			newBounds = Common::Rect(xs, ys, xs + destPic->_bounds2.width(), ys + destPic->_bounds2.height());
		} else {
			newBounds = Common::Rect(0, 0, destPic->_bounds.width(), destPic->_bounds.height());
		}

		var24 = ofs.y - newBounds.top;
		if (var24 < 0) {
			var52 = width2;
			srcOffset -= var24 * var52;
			height1 += var24;
			ofs.y = newBounds.top;
			
			if (height1 <= 0)
				return;

			isClipped = true;
		}

		var20 = newBounds.bottom - (ofs.y + height1);
		if (var20 < 0) {
			height1 += var20;
			if (height1 <= 0)
				return;
		}

		var22 = ofs.x - newBounds.left;
		if (var22 < 0) {
			srcOffset -= var22;
			width2 += var22;
			ofs.x = newBounds.left;

			if (width2 <= 0)
				return;

			isClipped = true;
		}

		var26 = newBounds.right - (ofs.x + width2);
		if (var26 < 0) {
			width2 += var26;
			if (width2 <= 0)
				return;

			isClipped = true;
		}
	}

	screenOffset = ofs.y * destPic->_bounds.width() + ofs.x;
	widthDiff = width1 - width2;
	widthDiff2 = destPic->_bounds.width() - width2;

	if (saveddestPic) {
		error("TODO: Examine further when it's actually used");
		if (!_saveBack || ((srcPic->_flags & 0x800) != 0)) {
			// TODO
		} else if (!saveddestPic->_field86) {
			// TODO
		} else {
			int xs = ofs.x + destPic->_bounds.left;
			int ys = ofs.y + destPic->_bounds.top;
			backBounds = Common::Rect(xs, ys, xs + width2, ys + height1);

			(this->*saveddestPic->_field86)(saveddestPic, saveddestPic->_bounds.top, backBounds);
		}
	}

	if (flags1 & 0x1000) {
		imgData1 = srcPic->_imgData + (var4C << 14) + _screenOffset;
		for (uint idx = 0; idx < srcPic->_maskData; ++idx) {
			if (var4C < 4) {
				EMSMapPageHandle(srcPic->_planeSize, srcPic->_imgData[idx], var4C);
				++var4C;
			}
		}
	} else {
		imgData1 = srcPic->_imgData;
	}
	if (flags2 & 0x1000) {
		imgData2 = destPic->_imgData + (var4C << 14) + _screenOffset;
		for (uint idx = 0; idx < srcPic->_maskData; ++idx) {
			if (var4C < 4) {
				EMSMapPageHandle(destPic->_planeSize, destPic->_imgData[idx], var4C);
				++var4C;
			}
		}		
	} else {
		imgData2 = destPic->_imgData;
	}

	_SVGAPage = _SVGAReset;
	if (srcPic->_select != 0xff)
		return;

	if (srcPic->_pick == 0xff) {
		if (flags1 & 8) {
			error("TODO: sDrawPic");
		} else {
			srcP = imgData1 + srcOffset;

			if (flags2 & 8) {
				error("TODO: sDrawPic");
			} else {
				destP = imgData2 + screenOffset;

				if (flags1 & 2) {
					error("TODO: sDrawPic");
				} else {
					if (flags1 & 0x100) {
						error("TODO: sDrawPic");
					} else {
						for (int yp = 0; yp < height1; ++yp) {
							Common::copy(srcP, srcP + width2, destP);
							destP += width2 + widthDiff2;
							srcP += width2 + widthDiff;
						}
					}
				}
			}
		}
	} else {
		error("TODO: sDrawPic");
	}
}

void GraphicsManager::EMSMapPageHandle(int v1, int v2, int v3) {
	// TODO
}

} // End of namespace Voyeur
