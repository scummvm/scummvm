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

#include "graphics/cursorman.h"

#include "prince/prince.h"
#include "prince/cursor.h"
#include "prince/debugger.h"
#include "prince/script.h"

#include "common/debug.h"

namespace Prince {

Cursor::Cursor() : _surface(nullptr) {
}

Cursor::~Cursor() {
	if (_surface != nullptr) {
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}
}

bool Cursor::loadStream(Common::SeekableReadStream &stream) {
	stream.skip(4);
	uint16 width = stream.readUint16LE();
	uint16 height = stream.readUint16LE();

	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	for (int h = 0; h < height; h++) {
		stream.read(_surface->getBasePtr(0, h), width);
	}
	return true;
}

void PrinceEngine::changeCursor(uint16 curId) {
	_debugger->_cursorNr = curId;
	_mouseFlag = curId;
	_flags->setFlagValue(Flags::MOUSEENABLED, curId);

	const Graphics::Surface *curSurface = nullptr;

	switch (curId) {
	default:
		error("Unknown cursor Id: %d", curId);
	case 0:
		CursorMan.showMouse(false);
		_optionsFlag = 0;
		_selectedMob = -1;
		return;
	case 1:
		curSurface = _cursor1->getSurface();
		break;
	case 2:
		curSurface = _cursor2;
		break;
	case 3:
		curSurface = _cursor3->getSurface();
		Common::Point mousePos = _system->getEventManager()->getMousePos();
		mousePos.x = CLIP(mousePos.x, (int16) 315, (int16) 639);
		mousePos.y = CLIP(mousePos.y, (int16) 0, (int16) 170);
		_system->warpMouse(mousePos.x, mousePos.y);
		break;
	}

	CursorMan.replaceCursorPalette(_roomBmp->getPalette(), 0, 255);
	CursorMan.replaceCursor(
		curSurface->getBasePtr(0, 0),
		curSurface->w, curSurface->h,
		0, 0,
		255, false,
		&curSurface->format
	);
	CursorMan.showMouse(true);
}

void PrinceEngine::makeInvCursor(int itemNr) {
	const Graphics::Surface *cur1Surface = _cursor1->getSurface();
	int cur1W = cur1Surface->w;
	int cur1H = cur1Surface->h;
	const Common::Rect cur1Rect(0, 0, cur1W, cur1H);

	const Graphics::Surface *itemSurface = _allInvList[itemNr].getSurface();
	int itemW = itemSurface->w;
	int itemH = itemSurface->h;

	int cur2W = cur1W + itemW / 2;
	int cur2H = cur1H + itemH / 2;

	if (_cursor2 != nullptr) {
		_cursor2->free();
		delete _cursor2;
	}
	_cursor2 = new Graphics::Surface();
	_cursor2->create(cur2W, cur2H, Graphics::PixelFormat::createFormatCLUT8());
	Common::Rect cur2Rect(0, 0, cur2W, cur2H);
	_cursor2->fillRect(cur2Rect, 255);
	_cursor2->copyRectToSurface(*cur1Surface, 0, 0, cur1Rect);

	const byte *src1 = (const byte *)itemSurface->getBasePtr(0, 0);
	byte *dst1 = (byte *)_cursor2->getBasePtr(cur1W, cur1H);

	if (itemH % 2) {
		itemH--;
	}
	if (itemW % 2) {
		itemW--;
	}

	for (int y = 0; y < itemH; y++) {
		const byte *src2 = src1;
		byte *dst2 = dst1;
		if (y % 2 == 0) {
			for (int x = 0; x < itemW; x++, src2++) {
				if (x % 2 == 0) {
					if (*src2) {
						*dst2 = *src2;
					} else {
						*dst2 = 255;
					}
					dst2++;
				}
			}
			dst1 += _cursor2->pitch;
		}
		src1 += itemSurface->pitch;
	}
}

} // End of namespace Prince
