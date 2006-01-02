/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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
 * $Header$
 *
 */

#ifndef KYRATEXT_H
#define KYRATEXT_H

namespace Kyra {
class Screen;

class TextDisplayer {
	struct TalkCoords {
		uint16 y, x, w;
	};

	enum {
		TALK_SUBSTRING_LEN = 80,
		TALK_SUBSTRING_NUM = 3
	};
public:
	TextDisplayer(Screen *screen);
	~TextDisplayer() {}

	void setTalkCoords(uint16 y);
	int getCenterStringX(const char *str, int x1, int x2);
	int getCharLength(const char *str, int len);
	int dropCRIntoString(char *str, int offs);
	char *preprocessString(const char *str);
	int buildMessageSubstrings(const char *str);
	int getWidestLineWidth(int linesCount);
	void calcWidestLineBounds(int &x1, int &x2, int w, int cx);
	void restoreTalkTextMessageBkgd(int srcPage, int dstPage);
	void printTalkTextMessage(const char *text, int x, int y, uint8 color, int srcPage, int dstPage);
	void printText(const char *str, int x, int y, uint8 c0, uint8 c1, uint8 c2);
	void printCharacterText(char *text, int8 charNum, int charX);

	uint16 _talkMessageY;
	uint16 _talkMessageH;
private:
	Screen *_screen;

	char _talkBuffer[300];
	char _talkSubstrings[TALK_SUBSTRING_LEN * TALK_SUBSTRING_NUM];
	TalkCoords _talkCoords;
	bool _talkMessagePrinted;
};
} // end of namespace Kyra

#endif

