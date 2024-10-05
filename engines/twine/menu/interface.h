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

#ifndef TWINE_INTERFACE_H
#define TWINE_INTERFACE_H

#include "common/scummsys.h"
#include "common/rect.h"

namespace Graphics {
class ManagedSurface;
}

namespace TwinE {

class TwinEEngine;

class Interface {
private:
	TwinEEngine *_engine;
	Common::Rect _memoClip;

public:
	Interface(TwinEEngine *engine);
	Common::Rect _clip { 0, 0, 0, 0 };
	bool _animateTexture = false; // lba2: AnimateTexture

	/**
	 * Draw button line
	 * @param startWidth width value where the line starts
	 * @param startHeight height value where the line starts
	 * @param endWidth width value where the line ends
	 * @param endHeight height value where the line ends
	 * @param lineColor line color in the current palette
	 */
	bool drawLine(int32 startWidth, int32 startHeight, int32 endWidth, int32 endHeight, uint8 lineColor);

	/**
	 * Blit button box from working buffer to front buffer
	 * @param source source screen buffer, in this case working buffer
	 * @param dest destination screen buffer, in this case front buffer
	 */
	void blitBox(const Common::Rect &rect, const Graphics::ManagedSurface &source, Graphics::ManagedSurface &dest);
	/**
	 * Draws inside buttons transparent area
	 * @param colorAdj index to adjust the transparent box color
	 */
	void shadeBox(const Common::Rect &rect, int32 colorAdj);

	void box(const Common::Rect &rect, uint8 colorIndex);

	bool setClip(const Common::Rect &rect);
	void memoClip(); // saveTextWindow
	void restoreClip(); // loadSavedTextWindow
	void unsetClip();
};

} // namespace TwinE

#endif
