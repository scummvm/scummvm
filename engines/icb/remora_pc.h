/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_REMORA_PC_H_INCLUDED
#define ICB_REMORA_PC_H_INCLUDED

namespace ICB {

#define REMORA_MAX_YS 1024
#define REMORA_BYTES_PER_PIXEL 4

#define REMORA_SCREEN_ORIGIN_X (0)
#define REMORA_SCREEN_ORIGIN_Y (0)

// Brightness defaults.
#define REMORA_BRIGHT_MIN 150
#define REMORA_BRIGHT_HALF 200
#define REMORA_BRIGHT_FULL 255

// This controls thickness of barriers.
#define REMORA_LINE_FUZZ 3

// Some text parameters.
#define REMORA_TEXT_LEFT_MARGIN 125
#define REMORA_TEXT_RIGHT_MARGIN 480
#define REMORA_TEXT_CENTRE ((REMORA_TEXT_RIGHT_MARGIN - REMORA_TEXT_LEFT_MARGIN) / 2 + REMORA_TEXT_LEFT_MARGIN)
#define REMORA_TEXT_TITLE_Y 90
#define REMORA_TEXT_TOP_MARGIN 115
#define REMORA_TEXT_BOTTOM_MARGIN 400
#define REMORA_TEXT_TAB_ONE 20
#define REMORA_DISPLAY_WIDTH 340
#define REMORA_SPEECH_X_POSITION (SCREEN_WIDTH / 2)
#define REMORA_SPEECH_Y_POSITION 10
#define REMORA_TEXT_CLIP_TOP 116
#define REMORA_TEXT_CLIP_BOTTOM 357

// This defines the maximum height and width for a remora text picture.
#define REMORA_MAX_PICTURE_HEIGHT 280
#define REMORA_MAX_PICTURE_WIDTH 420

// These position the email-waiting message on the screen.
#define REMORA_EMAIL_WAITING_X 400
#define REMORA_EMAIL_WAITING_Y 380

// This defines the size and speed of the motion scan beam.
#define REMORA_SCAN_SPEED 4
#define REMORA_SCAN_WIDTH 45
#define REMORA_SCAN_LENGTH ((float)REMORA_SCREEN_HEIGHT)

#define REMORA_SCREEN_CENTRE_X (REMORA_SCREEN_WIDTH / 2)
#define REMORA_SCREEN_CENTRE_Z (REMORA_SCREEN_HEIGHT / 2)

// Some values for the progress bar.
#define REMORA_PROGRESS_BAR_WIDTH 200
#define REMORA_PROGRESS_BAR_HEIGHT 10
#define REMORA_PROGRESS_BAR_X (320 - (REMORA_PROGRESS_BAR_WIDTH / 2))
#define REMORA_PROGRESS_BAR_Y 231

// Size and positioning of the background grid.
#define REMORA_GRID_SIZE 40
#define REMORA_GRID_X 39
#define REMORA_GRID_Y 36

// These are for the text scrolling.
#define REMORA_SCROLL_AMOUNT 2 // Pixels.

// These are the values that the PC borrows to emulate the PSX.
#define REMORA_PC_DISPLAYED_TEXT_ROWS 12
#define REMORA_PC_TEXT_ROW_SPACING 0
#define REMORA_PC_CHARACTER_SPACING 0

// PC uses these values to emulate PSX text formatting.
#define REMORA_PSXFORMAT_DISPLAYED_ROWS 10 // Get this from remora_psx.h ( had to add 1 for some reason).
#define REMORA_PSXFORMAT_ROW_SPACING 16    // Guess this.
#define REMORA_PSXFORMAT_CHAR_SPACING 1    // Guess this.

// These set a size for the screen drawing area, which used to come from a
// sprite but now doesn't.
#define REMORA_DRAWABLE_SCREEN_LEFT 20
#define REMORA_DRAWABLE_SCREEN_TOP 20
#define REMORA_DRAWABLE_SCREEN_WIDTH 600
#define REMORA_DRAWABLE_SCREEN_HEIGHT 440

// These are the names of the bitmaps used for marking when text can scroll.
#define REMORA_BITMAP_MORE_UP "remora_more_up"
#define REMORA_BITMAP_MORE_DOWN "remora_more_down"

// This defines the size of the symbols that get draw on the scan modes.
#define REMORA_SYMBOL_SIZE_NORMAL 3
#define REMORA_SYMBOL_SIZE_BIG 6

// This defines a safe border for drawing the symbols (which works because the casing covers the gap).
#define REMORA_SYMBOL_BORDER 20

#define REMORA_NUM_COLOUR_SCHEMES 3
#define REMORA_COLOURS_PER_SCHEME 25
extern const uint8 pnRemoraColour[REMORA_NUM_COLOUR_SCHEMES][REMORA_COLOURS_PER_SCHEME][3];
enum { CI_RED = 0, CI_GREEN, CI_BLUE };

// These are drawing points for drawing the animating pulse blip and some definitions.
#define REMORA_PULSE_X 430
#define REMORA_PULSE_Y 105
#define REMORA_PULSE_WIDTH 40
#define REMORA_PULSE_POINTS 13
#define REMORA_MAX_HEALTH 3
#define REMORA_HIGHLIGHT_LENGTH 10

#define REMORA_IRPULSE_X 435
#define REMORA_IRPULSE_Y 95
#define REMORA_IR_LINK_WIDTH 35
#define REMORA_IR_LINK_POINTS 9

const int32 pnPulsePoints[REMORA_PULSE_POINTS][2] = {{0, 0}, {3, 0}, {6, 0}, {9, -5}, {12, -10}, {15, -5}, {18, 0}, {21, 5}, {24, 10}, {27, 5}, {30, 0}, {33, 0}, {36, 0}};

const int32 pnIRLinkPoints[REMORA_IR_LINK_POINTS][2] = {{0, 0}, {5, 0}, {10, 0}, {15, 0}, {20, 0}, {25, 0}, {25, 10}, {30, 10}, {35, 10}};

const int32 pnIRLinkPointsR[REMORA_IR_LINK_POINTS][2] = {{0, 10}, {5, 10}, {10, 10}, {10, 20}, {15, 20}, {20, 20}, {25, 20}, {30, 20}, {35, 20}};

inline uint32 GetPen(uint8 nPalette, int eCI, uint8 nAlpha = 255) {
	return ((uint32)((nAlpha << 24) | (pnRemoraColour[nPalette][eCI][CI_RED] << 16) | (pnRemoraColour[nPalette][eCI][CI_GREEN] << 8) | pnRemoraColour[nPalette][eCI][CI_BLUE]));
}

inline const char *_remora::MakeRemoraGraphicsPath(const char *pcBitmapName) const {
	static char pcRemoraGraphicsPath[MAXLEN_URL];

	sprintf(pcRemoraGraphicsPath, REMORA_GRAPHICS_PATH);
	strcat(pcRemoraGraphicsPath, pcBitmapName);
	strcat(pcRemoraGraphicsPath, ".");
	strcat(pcRemoraGraphicsPath, PX_BITMAP_EXT);

	return (pcRemoraGraphicsPath);
}

_remora::~_remora() {
	if (m_pDisplayBuffer) {
		delete[] m_pDisplayBuffer;
		m_pDisplayBuffer = NULL;
	}
}

inline bool8 _remora::TestOutcode(uint32 nOutcode, Outcode eCodeToTest) const { return ((bool8)(nOutcode & eCodeToTest)); }

} // End of namespace ICB

#endif // #if !defined( REMORA_PC_H_INCLUDED )
