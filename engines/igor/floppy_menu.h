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

#ifndef IGOR_FLOPPY_MENU_H
#define IGOR_FLOPPY_MENU_H

namespace Igor {

// Geometry and colors of the panel shared by the floppy options menu and the copy protection.
static const int kFloppyMenuX = 53;
static const int kFloppyMenuY = 19;
static const int kFloppyMenuWidth = 214;
static const int kFloppyMenuHeight = 106;
static const int kFloppyMenuSize = 320 * kFloppyMenuHeight;
static const int kFloppyButtonX = 55;
static const int kFloppyButtonY = 21;
static const int kFloppyButtonWidth = 42;
static const int kFloppyButtonHeight = 13;
static const int kFloppyControlPanelFirstRow = 36;
static const int kFloppyControlPanelRows = 85;
static const int kFloppyCaptionY = 22;
static const int kFloppyCaptionLastRow = 34;
static const int kFloppyLastFadedColor = 248;
static const int kFloppyFadeSteps = 30;
static const int kFloppyFadeCount = 3 * (kFloppyLastFadedColor + 1);

enum {
	kFloppyColorCaption = 0xF9,
	kFloppyColorText = 0xFF,
	kFloppyColorFrameLight = 0xFA,
	kFloppyColorFrameDark = 0xFC,
	kFloppyColorButtonLight = 0xFE,
	kFloppyColorButtonDark = 0xFD
};

} // End of namespace Igor

#endif
