/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SMUSH_FONT_H
#define SMUSH_FONT_H

#include "common/scummsys.h"
#include "scumm/nut_renderer.h"

namespace Scumm {

class SmushFont : public NutRenderer {
protected:
	int _nbChars;
	int16 _color;
	bool _new_colors;
	bool _original;
	

	int getStringWidth(const char *str);
	int getStringHeight(const char *str);
	int draw2byte(byte *buffer, int dst_width, int x, int y, int idx);
	int drawChar(byte *buffer, int dst_width, int x, int y, byte chr);
	void drawSubstring(const char *str, byte *buffer, int dst_width, int x, int y);

public:
	SmushFont(bool use_original_colors, bool new_colors);

	void setColor(byte c) { _color = c; }
	void drawStringAbsolute    (const char *str, byte *buffer, int dst_width, int x, int y);
	void drawStringCentered    (const char *str, byte *buffer, int dst_width, int dst_height, int x, int y);
	void drawStringWrap        (const char *str, byte *buffer, int dst_width, int dst_height, int x, int y, int left, int right);
	void drawStringWrapCentered(const char *str, byte *buffer, int dst_width, int dst_height, int x, int y, int left, int right);
};

} // End of namespace Scumm

#endif
