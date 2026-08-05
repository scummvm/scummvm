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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_MODAL_CONSTANTS_H
#define RIPPER_MODAL_CONSTANTS_H

namespace Ripper {

static const uint kModalSkinFrameCount = 15;
static const uint kWacModalSkinFrameCount = 16;
static const uint kModalScrollStartFrame = 9;
static const uint kModalScrollEndFrame = 10;
static const uint kModalScrollTrackFrame = 14;
static const uint kModalTitleResourceId = 0x42;
static const uint kModalMaximumRows = 10;
static const int kModalHeadingTopPadding = 17;
static const int kModalBottomPadding = 5;
static const int kModalLeftPadding = 5;
static const int kModalRightPadding = 5;
static const int kModalScrollStartInset = 16;
static const int kModalScrollEndInset = 4;
static const int kModalScrollEdgeInset = 4;
static const int kModalTextHorizontalInset = 2;
static const int kModalTextVerticalInset = 1;
static const uint kModalWidth = 300;
static const int kModalRowHeight = 14;
static const byte kModalBackgroundColor = 253;
static const byte kModalHeadingColor = 255;
static const byte kModalTitleColor = 254;
static const byte kModalTextColor = 4;
static const byte kModalSelectedBackgroundColor = 248;
static const uint kBinaryPromptYesResourceId = 0x3c;
static const uint kBinaryPromptNoResourceId = 0x3d;
static const uint kBinaryPromptOptionCount = 2;
static const int kWacModalHeadingTopPadding = 20;
static const int kWacModalBottomPadding = 6;
static const int kWacModalLeftPadding = 5;
static const int kWacModalRightPadding = 20;
static const int kWacModalTitleTopInset = 1;
static const byte kWacModalBackgroundColor = 4;
static const byte kWacModalTitleColor = 248;
static const byte kWacModalTextColor = 251;
static const uint kPrimaryScrollSkinFrameCount = 4;
static const int kPrimaryRowHeight = 9;
static const int kPrimaryScrollGap = 5;
static const byte kPrimaryTextColor = 254;
static const uint kModalCursor = 16;
static const int kTextEntryLeft = 228;
static const int kTextEntryTop = 312;
static const int kTextEntryWidth = 194;
static const int kTextEntryHeight = 20;
static const int kTextEntryPadding = 5;
static const byte kSceneEntryTextColor = 251;
static const uint32 kTextEntryCaretBlinkMillis = 500;

} // End of namespace Ripper

#endif // RIPPER_MODAL_CONSTANTS_H
