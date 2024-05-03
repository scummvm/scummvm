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

#ifndef BACKENDS_IMGUI_FONTS_H
#define BACKENDS_IMGUI_FONTS_H

#include "backends/imgui/imgui.h"

namespace ImGui {

/**
 * Loads and adds a TTF font file from the common fonts archive to ImGui .
 *
 * @param filename          The name of the font to load.
 * @param size_pixels       The size of the font in pixels.
 * @param font_cfg_template Configuration of the font.
 * @param glyph_ranges      Glyph ranges array terminated by 0, you need to make sure that your array persist up until the
 *                          atlas is build (when calling GetTexData*** or Build()). We only copy the pointer, not the data.
 * @return 0 in case loading fails, otherwise a pointer to the Font object.
 */
ImFont *addTTFFontFromArchive(const char *filename, float size_pixels, const ImFontConfig *font_cfg_template = NULL, const ImWchar *glyph_ranges = NULL);
}

#endif
