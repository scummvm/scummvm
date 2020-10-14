/** @file interface.h
	@brief
	This file contains in-game interface routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef INTERFACE_H
#define INTERFACE_H

#include "sys.h"
#include "main.h"

/** Screen top limit to display the texts */
#define SCREEN_TEXTLIMIT_TOP	0
/** Screen left limit to display the texts */
#define SCREEN_TEXTLIMIT_LEFT	0
/** Screen right limit to display the texts */
#define SCREEN_TEXTLIMIT_RIGHT	SCREEN_WIDTH-1
/** Screen bottom limit to display the texts */
#define SCREEN_TEXTLIMIT_BOTTOM	SCREEN_HEIGHT-1

int32 textWindowTop;
int32 textWindowTopSave;
int32 textWindowLeft;
int32 textWindowLeftSave;
int32 textWindowRight;
int32 textWindowRightSave;
int32 textWindowBottom;
int32 textWindowBottomSave;

/** Draw button line
	@param startWidth width value where the line starts
	@param startHeight height value where the line starts
	@param endWidth width value where the line ends
	@param endHeight height value where the line ends
	@param lineColor line color in the current palette */
void drawLine(int32 startWidth, int32 startHeight, int32 endWidth, int32 endHeight, int32 lineColor);

/** Blit button box from working buffer to front buffer
	@param left start width to draw the button
	@param top start height to draw the button
	@param right end width to draw the button
	@param bottom end height to draw the button
	@source source screen buffer, in this case working buffer
	@param leftDest start width to draw the button in destination buffer
	@param topDest start height to draw the button in destination buffer
	@dest destination screen buffer, in this case front buffer */
void blitBox(int32 left, int32 top, int32 right, int32 bottom, int8 *source, int32 leftDest, int32 topDest, int8 *dest);

/** Draws inside buttons transparent area
	@param left start width to draw the button
	@param top start height to draw the button
	@param right end width to draw the button
	@param bottom end height to draw the button
	@param colorAdj index to adjust the transparent box color */
void drawTransparentBox(int32 left, int32 top, int32 right, int32 bottom, int32 colorAdj);

void drawSplittedBox(int32 left, int32 top, int32 right, int32 bottom, uint8 e);

void setClip(int32 left, int32 top, int32 right, int32 bottom);
void saveClip(); // saveTextWindow
void loadClip(); // loadSavedTextWindow
void resetClip();

#endif
