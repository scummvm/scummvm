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
#ifndef SLUDGE_STATUSBA_H
#define SLUDGE_STATUSBA_H

#include "common/str.h"

namespace Sludge {

struct StatusBar {
	Common::String text;
	StatusBar *next;
};

struct StatusStuff {
	StatusBar *firstStatusBar;
	uint16 alignStatus;
	int litStatus;
	int statusX, statusY;
	int statusR, statusG, statusB;
	int statusLR, statusLG, statusLB;
};

void initStatusBar();

void setStatusBar(Common::String &txt);
void clearStatusBar();
void addStatusBar();
void killLastStatus();
void statusBarColour(byte r, byte g, byte b);
void statusBarLitColour(byte r, byte g, byte b);
void setLitStatus(int i);
const Common::String statusBarText();
void positionStatus(int, int);
void drawStatusBar();

// Load and save
bool loadStatusBars(Common::SeekableReadStream *stream);
void saveStatusBars(Common::WriteStream *stream);

// For freezing
void restoreBarStuff(StatusStuff  *here);
StatusStuff  *copyStatusBarStuff(StatusStuff  *here);

} // End of namespace Sludge

#endif
