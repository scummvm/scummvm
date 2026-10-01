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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCUMM_INSANE_MORTIMER_TEXT_H
#define SCUMM_INSANE_MORTIMER_TEXT_H

#include "common/hashmap.h"
#include "graphics/font.h"
#include "scumm/insane/mortimer/movie.h"

namespace Scumm {

class MortimerFont : public Graphics::Font {
public:
	bool load(const char *path);
	bool load(Common::SeekableReadStream &stream);
	int getFontHeight() const override { return _height; }
	int getMaxCharWidth() const override { return _width; }
	int getCharWidth(uint32 chr) const override;
	void drawChar(Graphics::Surface *surface, uint32 chr, int x, int y, uint32 color) const override;

private:
	Common::Array<MortimerSprite> _glyphs;
	int _height = 0, _width = 0;
};

class MortimerSubtitles {
public:
	bool load(const char *textPath, const char *fontPath, const char *infoFontPath);
	bool loadText(Common::SeekableReadStream &stream);
	void draw(Graphics::Surface &screen, const Common::Array<MortimerMovie::Caption> &captions, bool enabled,
		const MortimerMovieContext *context = nullptr) const;

private:
	struct Entry {
		Common::String text;
		byte color = 1;
		bool information = false;
		bool conditional = false;
	};

	MortimerFont _font, _infoFont;
	Common::HashMap<uint16, Entry> _entries;
};

} // End of namespace Scumm

#endif
