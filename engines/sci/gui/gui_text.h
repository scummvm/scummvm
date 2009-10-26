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

#ifndef SCI_GUI_TEXT_H
#define SCI_GUI_TEXT_H

namespace Sci {

#define SCI_TEXT_ALIGNMENT_RIGHT -1
#define SCI_TEXT_ALIGNMENT_CENTER 1
#define SCI_TEXT_ALIGNMENT_LEFT	0

class SciGuiScreen;
class SciGuiFont;
class SciGuiText {
public:
	SciGuiText(SciGuiScreen *screen);
	~SciGuiText();

private:
	void init(void);

	SciGuiScreen *_screen;

	int _codeFontsCount;
	GuiResourceId *_codeFonts;
	int _codeColorsCount;
	uint16 *_codeColors;
};

} // End of namespace Sci

#endif
