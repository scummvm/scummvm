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

#ifndef DARKSEED_GAMEFONT_H
#define DARKSEED_GAMEFONT_H

#include "graphics/font.h"
#include "darkseed/nsp.h"
#include "graphics/surface.h"

namespace Darkseed {

class GameFont : public Graphics::Font {
private:
	Nsp _letters;
	int _maxWidth = 0;

public:
	GameFont();
	bool load();

	int getFontHeight() const override;
	int getMaxCharWidth() const override;
	int getCharWidth(uint32 chr) const override;
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;

private:
	const Sprite *getCharacterSprite(char c) const;
};

} // namespace Darkseed

#endif // DARKSEED_GAMEFONT_H
