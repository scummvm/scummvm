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

#include "scumm/insane/mortimer/text.h"

#include "common/file.h"
#include "common/tokenizer.h"
#include "common/util.h"
#include "graphics/surface.h"

namespace Scumm {

bool MortimerFont::load(const char *path) {
	Common::File file;
	return file.open(Common::Path(path)) && load(file);
}

bool MortimerFont::load(Common::SeekableReadStream &stream) {
	_width = _height = 0;
	if (!loadMortimerSprites(stream, _glyphs) || _glyphs.size() < 126)
		return false;
	for (uint i = 0; i < _glyphs.size(); ++i) {
		_width = MAX(_width, _glyphs[i].width);
		_height = MAX(_height, _glyphs[i].height);
	}
	return true;
}

int MortimerFont::getCharWidth(uint32 chr) const {
	return chr < _glyphs.size() ? _glyphs[chr].width : 0;
}

void MortimerFont::drawChar(Graphics::Surface *surface, uint32 chr, int x, int y, uint32 color) const {
	if (chr >= _glyphs.size())
		return;
	const MortimerSprite &glyph = _glyphs[chr];
	x += glyph.x;
	y += glyph.y;
	Common::Rect clip(x, y, x + glyph.width, y + glyph.height);
	clip.clip(Common::Rect(surface->w, surface->h));
	if (clip.isEmpty())
		return;
	for (int row = clip.top; row < clip.bottom; ++row) {
		byte *dst = (byte *)surface->getBasePtr(0, row);
		for (int col = clip.left; col < clip.right; ++col) {
			const uint offset = (row - y) * glyph.width + col - x;
			const byte pixel = glyph.pixels[offset];
			if (!glyph.mask.empty()) {
				// Mortimer's codec 44 shifts the whole glyph palette, including shading.
				if (glyph.mask[offset])
					dst[col] = byte(pixel + color - 1);
			} else if (pixel) {
				dst[col] = pixel;
			}
		}
	}
}

bool MortimerSubtitles::load(const char *textPath, const char *fontPath, const char *infoFontPath) {
	Common::File file;
	return file.open(Common::Path(textPath)) && loadText(file) && _font.load(fontPath) && _infoFont.load(infoFontPath);
}

bool MortimerSubtitles::loadText(Common::SeekableReadStream &stream) {
	_entries.clear();
	int id = -1;
	while (!stream.eos() && !stream.err()) {
		Common::String line = stream.readLine();
		line.trim();
		if (line.empty())
			continue;
		if (line[0] == 0x1a)
			break;
		if (line[0] == '#') {
			// Labels may contain spaces; the numeric ID is the final token.
			Common::StringTokenizer tokens(line);
			Common::String token;
			while (!tokens.empty())
				token = tokens.nextToken();
			char *end;
			const long value = strtol(token.c_str(), &end, 10);
			if (end == token.c_str() || *end || value < 0 || value > 65535)
				return false;
			id = value;
		} else if (id >= 0 && (line.hasPrefix("//") || line.hasPrefix("^c"))) {
			if (line.hasPrefix("//"))
				line = line.substr(2);
			Entry &entry = _entries[id];
			if (entry.text.empty() && line.hasPrefix("!")) {
				entry.conditional = true;
				line = line.substr(1);
			}
			if (entry.text.empty() && line.hasPrefix("$")) {
				entry.information = true;
				line = line.substr(1);
			}
			if (line.size() >= 5 && line.hasPrefix("^c") &&
				Common::isDigit(line[2]) && Common::isDigit(line[3]) && Common::isDigit(line[4])) {
				entry.color = atoi(line.substr(2, 3).c_str());
				line = line.substr(5);
			}
			if (!entry.text.empty())
				entry.text += '\n';
			entry.text += line;
		}
	}
	return !stream.err() && !_entries.empty();
}

Common::String MortimerSubtitles::text(uint16 id) const {
	const auto found = _entries.find(id);
	return found == _entries.end() ? Common::String() : found->_value.text;
}

void drawMortimerMessage(Graphics::Surface &screen, const MortimerFont &font, const Common::String &text, byte color) {
	Common::Array<Common::String> lines;
	const int width = font.wordWrapText(text, screen.w - 40, lines, 0, Graphics::kWordWrapOnExplicitNewLines);
	const int lineHeight = font.getFontHeight() + 1;
	const int height = lines.size() * font.getFontHeight() + 1;
	const int x = (screen.w - width) / 2, y = (screen.h - height) / 2;
	Common::Rect box(x - 10, y - 10, x + width + 10, y + height + 10);
	screen.fillRect(box, 251);
	for (int i = 0; i < 3; ++i) {
		screen.hLine(box.left, box.top, box.right - 1, 250);
		screen.vLine(box.left, box.top, box.bottom - 1, 250);
		screen.hLine(box.left, box.bottom - 1, box.right - 1, 252);
		screen.vLine(box.right - 1, box.top, box.bottom - 1, 252);
		box.grow(-1);
	}
	for (uint i = 0; i < lines.size(); ++i)
		font.drawString(&screen, lines[i], x, y + i * lineHeight, width, color, Graphics::kTextAlignCenter, 0, false);
}

void MortimerSubtitles::drawScore(Graphics::Surface &screen, uint32 score) const {
	drawMortimerMessage(screen, _infoFont, Common::String::format(text(3).c_str(), (long)score), 231);
}

void MortimerSubtitles::draw(Graphics::Surface &screen, const Common::Array<MortimerMovie::Caption> &captions, bool enabled,
		const MortimerMovieContext *context) const {
	for (uint i = 0; i < captions.size(); ++i) {
		const MortimerMovie::Caption &caption = captions[i];
		const auto found = _entries.find(caption.id);
		if (found == _entries.end())
			continue;
		const Entry &entry = found->_value;
		if (entry.conditional && context && ((caption.id < 1000 && context->riddle != caption.id) ||
			(caption.id / 1000 == 3 && context->girl) || (caption.id / 1000 == 4 && !context->girl)))
			continue;
		if ((!enabled && !entry.information) || entry.text.empty())
			continue;
		const MortimerFont &font = entry.information ? _infoFont : _font;
		Common::Array<Common::String> lines;
		const int width = font.wordWrapText(entry.text, screen.w, lines);
		const int lineHeight = font.getFontHeight() + 1;
		const int height = lines.size() * lineHeight - 1;
		const int x = CLIP<int>(caption.x, width / 2, screen.w - (width + 1) / 2);
		int y = caption.y - (entry.information ? height / 2 : 0);
		y = MAX(0, MIN<int>(y, screen.h - height));
		for (uint j = 0; j < lines.size(); ++j) {
			const int lineWidth = font.getStringWidth(lines[j]);
			font.drawString(&screen, lines[j], x - lineWidth / 2, y + j * lineHeight, lineWidth, entry.color,
				Graphics::kTextAlignLeft, 0, false);
		}
	}
}

} // End of namespace Scumm
