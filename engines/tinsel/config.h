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
 * $URL$
 * $Id$
 *
 */

#ifndef TINSEL_CONFIG_H
#define TINSEL_CONFIG_H

#include "tinsel/dw.h"

namespace Tinsel {

// double click timer initial value
enum {
	DOUBLE_CLICK_TIME	= 6,	// 6 @ 18Hz = .33 sec
	DEFTEXTSPEED		= 0
};

extern int dclickSpeed;
extern int volMusic;
extern int volSound;
extern int volVoice;
extern int speedText;
extern int bSubtitles;
extern int bSwapButtons;
extern LANGUAGE g_language;
extern int bAmerica;

void WriteConfig(void);
void ReadConfig(void);

extern bool isJapanMode();

} // End of namespace Tinsel

#endif
