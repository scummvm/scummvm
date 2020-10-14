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

#ifndef TWINE_INTERFACE_H
#define TWINE_INTERFACE_H

#include "common/scummsys.h"

namespace TwinE {

/** Screen top limit to display the texts */
#define SCREEN_TEXTLIMIT_TOP 0
/** Screen left limit to display the texts */
#define SCREEN_TEXTLIMIT_LEFT 0
/** Screen right limit to display the texts */
#define SCREEN_TEXTLIMIT_RIGHT SCREEN_WIDTH - 1
/** Screen bottom limit to display the texts */
#define SCREEN_TEXTLIMIT_BOTTOM SCREEN_HEIGHT - 1

class TwinEEngine;

class Interface {
private:
	TwinEEngine *_engine;
	int32 checkClipping(int32 x, int32 y);

public:
	Interface(TwinEEngine *engine);
	int32 textWindowTop = 0;
	int32 textWindowTopSave = 0;
	int32 textWindowLeft = 0;
	int32 textWindowLeftSave = 0;
	int32 textWindowRight = 0;
	int32 textWindowRightSave = 0;
	int32 textWindowBottom = 0;
	int32 textWindowBottomSave = 0;

	/**
	 * Draw button line
	 * @param startWidth width value where the line starts
	 * @param startHeight height value where the line starts
	 * @param endWidth width value where the line ends
	 * @param endHeight height value where the line ends
	 * @param lineColor line color in the current palette
	 */
	void drawLine(int32 startWidth, int32 startHeight, int32 endWidth, int32 endHeight, int32 lineColor);

	/**
	 * Blit button box from working buffer to front buffer
	 * @param left start width to draw the button
	 * @param top start height to draw the button
	 * @param right end width to draw the button
	 * @param bottom end height to draw the button
	 * @param source source screen buffer, in this case working buffer
	 * @param leftDest start width to draw the button in destination buffer
	 * @param topDest start height to draw the button in destination buffer
	 * @param dest destination screen buffer, in this case front buffer
	 */
	void blitBox(int32 left, int32 top, int32 right, int32 bottom, int8 *source, int32 leftDest, int32 topDest, int8 *dest);

	/**
	 * Draws inside buttons transparent area
	 * @param left start width to draw the button
	 * @param top start height to draw the button
	 * @param right end width to draw the button
	 * @param bottom end height to draw the button
	 * @param colorAdj index to adjust the transparent box color
	 */
	void drawTransparentBox(int32 left, int32 top, int32 right, int32 bottom, int32 colorAdj);

	void drawSplittedBox(int32 left, int32 top, int32 right, int32 bottom, uint8 e);

	void setClip(int32 left, int32 top, int32 right, int32 bottom);
	void saveClip(); // saveTextWindow
	void loadClip(); // loadSavedTextWindow
	void resetClip();
};

} // namespace TwinE

#endif
