/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "graphics/fontman.h"
//#include "gui/consolefont.h"

namespace GUI {
	extern const Graphics::NewFont g_consolefont;
};

namespace Graphics {

const ScummFont g_scummfont;
extern const NewFont g_sysfont;


DECLARE_SINGLETON(FontManager);

FontManager::FontManager() {
}

//const Font *FontManager::getFontByName(const Common::String &name) const {
//}

const Font *FontManager::getFontByUsage(FontUsage usage) const {
	switch (usage) {
	case kOSDFont:
		return &g_scummfont;
	case kConsoleFont:
		return &GUI::g_consolefont;
	case kGUIFont:
		return &g_sysfont;
	}
	return 0;
}

} // End of namespace Graphics
