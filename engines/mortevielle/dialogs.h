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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#ifndef MORTEVIELLE_ALERT_H
#define MORTEVIELLE_ALERT_H

#include "common/str.h"

namespace Mortevielle {

static const int NUM_LINES = 7;
const int kMaxRect = 14;

class Alert {
private:
	static void decodeAlertDetails(Common::String inputStr, int &choiceNumb, int &lineNumb, int &col, Common::String &choiceStr, Common::String &choiceListStr);
	static void setPosition(int ji, int coldep, int esp);
	static void drawAlertBox(int lidep, int nli, int tx);
	static void setButtonText(Common::String c, int coldep, int nbcase, Common::String *str, int esp);
public:
	static int show(const Common::String &s, int n);
};

class KnowledgeCheck {
public:
	static bool show();
};

class f3f8 {
public:
	static void draw();
	static void checkForF8(int SpeechNum, bool drawAni50Fl);
	static int waitForF3F8();
	static void aff50(bool drawAni50Fl);
	static void ani50();
};

} // End of namespace Mortevielle
#endif
