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
#ifndef PELROCK_GRAPHICS_H
#define PELROCK_GRAPHICS_H

#include "common/array.h"
#include "common/str-array.h"
#include "common/scummsys.h"

#include "graphics/font.h"
#include "graphics/screen.h"

namespace Pelrock {

class GraphicsManager {
public:
	GraphicsManager();
	~GraphicsManager();

	Common::Point showOverlay(int height, byte *buf);
	byte *grabBackgroundSlice(byte *buf, int x, int y, int w, int h);
	void putBackgroundSlice(byte *buf, int x, int y, int w, int h, byte *slice);
	void fadeToBlack(int stepSize);
	void fadePaletteToTarget(byte *targetPalette, int stepSize);
	void clearScreen();
	void drawColoredText(Graphics::ManagedSurface *screen, const Common::String &text, int x, int y, int w, byte &defaultColor, Graphics::Font *font);
	void drawColoredText(byte *buf, const Common::String &text, int x, int y, int w, byte &defaultColor, Graphics::Font *font);
	void drawColoredTexts(Graphics::ManagedSurface *surface, const Common::StringArray &text, int x, int y, int w, int yPadding, Graphics::Font *font);
	void drawColoredTexts(byte *buf, const Common::StringArray &text, int x, int y, int w, int yPadding, Graphics::Font *font);
};

} // End of namespace Pelrock
#endif // PELROCK_GRAPHICS_H
