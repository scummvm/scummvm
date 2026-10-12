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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef ULTIMA2_GFX_TEXT_H
#define ULTIMA2_GFX_TEXT_H

#include "graphics/fonts/dosfont.h"
#include "graphics/views/gfx_surface.h"

namespace Ultima {
namespace Ultima2 {
namespace Gfx {

/**
 * The kinds of text that the Ultima II Upgrade shows in different colors. Each
 * is a number the game gives its text drawing, which the graphics driver then
 * turns into a color. They're named for what they were used for, but are only
 * ever different colors in the EGA graphics modes
 */
enum TextColor {
	TEXT_NORMAL = 0,		// Most text
	TEXT_TITLE = 1,			// The name of the game on the title screen
	TEXT_NAME = 2,			// The character's name and the credits
	TEXT_HEADING = 3,		// Headings, and things chosen when creating a character
	TEXT_HIGHLIGHT = 4,		// An empty hit points or food value
	TEXT_LIST = 5,			// What's readied, and a name being typed in
	TEXT_NUMBER = 6,		// Numbers on the character's status screen
	TEXT_REVERSE = 7		// Reverse video, which is drawn as the background color instead
};

/**
 * Returns the palette color for a type of text, which depends on the graphics
 * mode. Without the upgrade's style of text, all text is white
 */
extern byte textColor(TextColor color);

/**
 * Switches reverse video on or off for a surface drawing text
 */
extern void setReverseText(Graphics::Views::GfxSurface &s, bool on);

/**
 * The font the game's text is drawn with, which shows letters in capitals
 * unless the upgrade's mixed case text is on
 */
class TextFont : public Graphics::DosFont {
public:
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;
};

} // namespace Gfx
} // namespace Ultima2
} // namespace Ultima

#endif
