/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#include "lure/debug-input.h"
#include "lure/luredefs.h"
#include "lure/events.h"
#include "lure/surface.h"
#include "lure/screen.h"

#ifdef LURE_DEBUG

namespace Lure {

bool get_string(char *buffer, uint32 maxSize, bool isNumeric, uint16 x, uint16 y) {
	Events &e = Events::getReference();
	buffer[0] = '\0';

	// Create surface for holding entered text
	Surface *s = new Surface((maxSize + 1) * FONT_WIDTH, FONT_HEIGHT);

	bool abortFlag = false;
	bool refreshFlag = true;

	while (!e.quitFlag && !abortFlag) {
		// Check for refreshing display of text
		if (refreshFlag) {
			uint16 strWidth = Surface::textWidth(buffer);
			s->empty();
			s->writeString(0, 0, buffer, false, DIALOG_TEXT_COLOUR);
			s->writeChar(strWidth, 0, '_', false, DIALOG_TEXT_COLOUR);
			s->copyToScreen(x, y);

			refreshFlag = false;
		}

		if (e.pollEvent()) {
			if (e.type() == OSystem::EVENT_KEYDOWN) {
				char ch = e.event().kbd.ascii;
				uint16 keycode = e.event().kbd.keycode;

				if ((ch == 13) || (keycode == 0x10f))
					break;
				else if (ch == 27) 
					abortFlag = true;
				else if (ch == 8) {
					if (*buffer != '\0') {
						*((char *) buffer + strlen(buffer) - 1) = '\0';
						refreshFlag = true;
					}
				} else if ((ch >= ' ') && (strlen(buffer) < maxSize)) {
					if (((ch >= '0') && (ch <= '9')) || !isNumeric) {
						char *p = buffer + strlen(buffer);
						*p++ = ch;
						*p++ = '\0';
						refreshFlag = true;
					}
				}
			}
		}
	}

	delete s;
	if (e.quitFlag) abortFlag = true;
	return !abortFlag;
}

bool input_integer(Common::String desc, uint32 &value)
{
	const int MAX_SIZE = 5;
	char buffer[MAX_SIZE + 1];

	uint16 width = DIALOG_EDGE_SIZE + Surface::textWidth(desc.c_str()) + FONT_WIDTH;
	uint16 totalWidth = width + FONT_WIDTH * (MAX_SIZE + 1) + DIALOG_EDGE_SIZE;
	uint16 totalHeight = FONT_HEIGHT + DIALOG_EDGE_SIZE * 2;

	Surface *s = new Surface(totalWidth, totalHeight);
	s->createDialog(true);
	s->writeString(DIALOG_EDGE_SIZE + 3, DIALOG_EDGE_SIZE, desc, false);

	uint16 xs = (FULL_SCREEN_WIDTH-totalWidth) / 2;
	uint16 ys = (FULL_SCREEN_HEIGHT-totalHeight) / 2;
	s->copyToScreen(xs, ys);

	bool result = get_string(&buffer[0], MAX_SIZE, true, xs+width, ys+DIALOG_EDGE_SIZE);
	Screen::getReference().update();
	if (!result || (buffer[0] == '\0')) 
		return false;

	value = atoi(buffer);
	return true;
}

bool input_string(Common::String desc, char *buffer, uint32 maxSize)
{
	uint16 width = Surface::textWidth(desc.c_str());
	if (width < FONT_WIDTH * maxSize) width = FONT_WIDTH * maxSize;

	Surface *s = new Surface(width + 2 * DIALOG_EDGE_SIZE, 2 * FONT_HEIGHT + 2 * DIALOG_EDGE_SIZE);
	s->createDialog();
	s->writeString(DIALOG_EDGE_SIZE, DIALOG_EDGE_SIZE, desc, false, DIALOG_TEXT_COLOUR);

	uint16 xs = (FULL_SCREEN_WIDTH-s->width()) / 2;
	uint16 ys = (FULL_SCREEN_HEIGHT-s->height()) / 2;

	s->copyToScreen(xs, ys);
	bool result = get_string(buffer, maxSize, true, xs + width, ys + DIALOG_EDGE_SIZE);

	Screen::getReference().update();
	return result;
}

} // end of namespace Lure

#endif
