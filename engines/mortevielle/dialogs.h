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

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#ifndef MORTEVIELLE_ALERT_H
#define MORTEVIELLE_ALERT_H

#include "common/rect.h"
#include "common/str.h"

namespace Mortevielle {
class MortevielleEngine;

static const int NUM_LINES = 7;
const int kMaxHotspots = 14;

struct Hotspot {
	Common::Rect _rect;
	bool _enabled;
};

class DialogManager {
private:
	MortevielleEngine *_vm;

	void decodeAlertDetails(Common::String inputStr, int &choiceNumb, int &lineNumb, int &col, Common::String &choiceStr, Common::String &choiceListStr);
	void setPosition(int ji, int coldep, int esp);
	void drawAlertBox(int firstLine, int lineNum, int width);
	void setButtonText(Common::String c, int coldep, int nbcase, Common::String *str, int esp);
public:
	DialogManager(MortevielleEngine *vm);

	int show(const Common::String &msg);
	void drawF3F8();
	void checkForF8(int SpeechNum, bool drawFrame2Fl);
	int waitForF3F8();
	void displayIntroScreen(bool drawFrame2Fl);
	void displayIntroFrame2();
	bool showKnowledgeCheck();
};

} // End of namespace Mortevielle
#endif
