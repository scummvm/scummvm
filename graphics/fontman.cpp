/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 */

#include "graphics/fontman.h"
//#include "gui/consolefont.h"

DECLARE_SINGLETON(Graphics::FontManager);

namespace Graphics {

#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
const ScummFont g_scummfont;
extern const NewFont g_sysfont;
extern const NewFont g_sysfont_big;
extern const NewFont g_consolefont;

FontManager::FontManager() {
}

#else
const ScummFont *g_scummfont;
extern const NewFont *g_sysfont;
extern const NewFont *g_sysfont_big;
extern const NewFont *g_consolefont;

static bool g_initialized = false;
void initfonts() {
	if (!g_initialized) {
		// FIXME : this need to be freed
		g_initialized = true;
		g_scummfont = new ScummFont;
		INIT_FONT(g_sysfont)
		INIT_FONT(g_sysfont_big)
		INIT_FONT(g_consolefont)
	}
}

FontManager::FontManager() {
	initfonts();
}
#endif

const Font *FontManager::getFontByName(const Common::String &name) const {
	if (!_fontMap.contains(name))
		return 0;
	return _fontMap[name];
}

const Font *FontManager::getFontByUsage(FontUsage usage) const {
	switch (usage) {
#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
	case kOSDFont:
		return &g_scummfont;
	case kConsoleFont:
		return &g_consolefont;
	case kGUIFont:
		return &g_sysfont;
	case kBigGUIFont:
		return &g_sysfont_big;
#else
	case kOSDFont:
		return g_scummfont;
	case kConsoleFont:
		return g_consolefont;
	case kGUIFont:
		return g_sysfont;
	case kBigGUIFont:
		return g_sysfont_big;
#endif
	}

	return 0;
}

} // End of namespace Graphics
