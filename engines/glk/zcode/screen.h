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

#ifndef GLK_ZCODE_FONTS
#define GLK_ZCODE_FONTS

#include "glk/screen.h"

namespace Glk {
namespace ZCode {

/**
 * Derived screen class that adds in the Infocom character graphics font
 */
class FrotzScreen : public Glk::Screen {
private:
	/**
	 * Handles loading fonts for V6 games
	 */
	void loadVersion6Fonts(Common::Archive *archive);

	/**
	 * Handles loading the character graphics and runic fonts
	 */
	void loadExtraFonts(Common::Archive *archive);
protected:
	/**
	 * Load the fonts
	 */
	void loadFonts(Common::Archive *archive) override;
public:
	/**
	 * Constructor
	 */
	FrotzScreen();
};

} // End of namespace ZCode
} // End of namespace Glk

#endif
