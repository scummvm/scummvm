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

#ifndef M4_DIALOGS_H
#define M4_DIALOGS_H

#include "m4/m4.h"
#include "m4/viewmgr.h"
#include "common/list.h"

namespace M4 {

class DialogLine {
public:
	char data[100];
	uint8 xp;
	bool underline;
	bool barLine;

	DialogLine() { data[0] = '\0'; xp = 0; underline = barLine = false; }
};

class Dialog: public View {
private:
	Common::Array<DialogLine> _lines;
	int _widthChars;
	int _dialogWidth;
	int _dialogIndex;
	Common::Point _askPosition;
	RGBList *_palette;
	int _lineX;
	int _widthX;
	bool _commandCase;

	void initDialog();
	void incLine();
	bool matchCommand(const char *s1, const char *s2);
	void writeChars(const char *line);
	void addLine(const char *line, bool underlineP = false);
	void appendText(const char *line);
	void addBarLine();
	void getVocab(int vocabId, char **line);
	bool handleNounSuffix(char *destP, int nounNum, const char *srcP);
	void draw();
public:
	Dialog(M4Engine *vm, const char *msgData, const char *title = NULL);
	Dialog(M4Engine *vm, int widthChars, const char **descEntries);
	virtual ~Dialog();

	static void display(M4Engine *vm, int widthChars, const char **descEntries);

	bool onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents);
};

} // End of namespace M4

#endif
