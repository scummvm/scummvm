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

#include "ngi/ngi.h"
#include "ngi/font.h"
#include "ngi/objects.h"
#include "ngi/gfx.h"
#include "common/formats/xmlparser.h"

namespace NGI {

class BitmapFont::Loader : public Common::XMLParser {
public:
	Loader(BitmapFont &font, const Common::Path &path) : _font(font), _directory(path.getParent()) {}

private:
	BitmapFont &_font;
	Common::Path _directory;

	CUSTOM_XML_PARSER(Loader) {
		XML_KEY(font)
			XML_KEY(info)
				XML_PROP(face, false)
				XML_PROP(size, false)
				XML_PROP(bold, false)
				XML_PROP(italic, false)
				XML_PROP(charset, false)
				XML_PROP(unicode, false)
				XML_PROP(stretchH, false)
				XML_PROP(smooth, false)
				XML_PROP(aa, false)
				XML_PROP(padding, false)
				XML_PROP(spacing, false)
				XML_PROP(outline, false)
				XML_PROP(add_spacing, false)
				XML_PROP(noScale, false)
			KEY_END()
			XML_KEY(common)
				XML_PROP(lineHeight, false)
				XML_PROP(base, false)
				XML_PROP(scaleW, false)
				XML_PROP(scaleH, false)
				XML_PROP(pages, false)
				XML_PROP(packed, false)
				XML_PROP(alphaChnl, false)
				XML_PROP(redChnl, false)
				XML_PROP(greenChnl, false)
				XML_PROP(blueChnl, false)
			KEY_END()
			XML_KEY(pages)
				XML_KEY(page)
					XML_PROP(id, true)
					XML_PROP(file, true)
				KEY_END()
			KEY_END()
			XML_KEY(chars)
				XML_PROP(count, false)
				XML_KEY(char)
					XML_PROP(id, true)
					XML_PROP(x, true)
					XML_PROP(y, true)
					XML_PROP(width, true)
					XML_PROP(height, true)
					XML_PROP(xoffset, true)
					XML_PROP(yoffset, true)
					XML_PROP(xadvance, true)
					XML_PROP(page, true)
					XML_PROP(chnl, false)
				KEY_END()
			KEY_END()
			XML_KEY(kernings)
				XML_PROP(count, false)
				XML_KEY(kerning)
					XML_PROP(first, true)
					XML_PROP(second, true)
					XML_PROP(amount, true)
				KEY_END()
			KEY_END()
		KEY_END()
	} PARSER_END()

	bool parserCallback_font(ParserNode *) { return true; }
	bool parserCallback_pages(ParserNode *) { return true; }
	bool parserCallback_chars(ParserNode *) { return true; }
	bool parserCallback_kernings(ParserNode *) { return true; }

	bool parserCallback_info(ParserNode *node) {
		_font._outline = atoi(node->values.getValOrDefault("outline").c_str());
		_font._extraLineSpacing = atoi(node->values.getValOrDefault("add_spacing").c_str());
		int x = 0, y = 0;
		if (node->values.contains("spacing") && !parseIntegerKey(node->values["spacing"], 2, &x, &y))
			return parserError("Invalid bitmap font spacing");
		_font._spacing = Common::Point(x, y);
		return true;
	}

	bool parserCallback_common(ParserNode *node) {
		_font._lineHeight = atoi(node->values.getValOrDefault("lineHeight").c_str());
		return true;
	}

	bool parserCallback_page(ParserNode *node) {
		if (atoi(node->values["id"].c_str()) != (int)_font._pages.size())
			return parserError("Invalid bitmap font page index");
		Common::SharedPtr<Texture> page(new Texture());
		page->loadPNG(_directory.join(node->values["file"]), Dims());
		if (page->getDimensions().x == 0)
			return parserError("Missing bitmap font page");
		_font._pages.push_back(page);
		return true;
	}

	bool parserCallback_char(ParserNode *node) {
		const int id = atoi(node->values["id"].c_str());
		if (_font._glyphs.contains(id))
			return true;

		Glyph &g = _font._glyphs[id];
		g.page = atoi(node->values["page"].c_str());
		if (g.page < 0 || (uint)g.page >= _font._pages.size())
			return parserError("Unknown bitmap font page");
		const Dims size = _font._pages[g.page]->getDimensions();
		int x = atoi(node->values["x"].c_str()), y = atoi(node->values["y"].c_str());
		int w = atoi(node->values["width"].c_str()), h = atoi(node->values["height"].c_str());
		if (x < 0 || y < 0 || w < 0 || h < 0 || x > size.x - w || y > size.y - h)
			return parserError("Glyph outside bitmap font page");
		g.source = Common::Rect(x, y, x + w, y + h);
		g.offset = Common::Point(atoi(node->values["xoffset"].c_str()), atoi(node->values["yoffset"].c_str()));
		g.advance = atoi(node->values["xadvance"].c_str());
		if (!g.advance)
			g.advance = w;
		_font._maxWidth = MAX(_font._maxWidth, _font.getCharWidth(id));
		return true;
	}

	bool parserCallback_kerning(ParserNode *node) {
		const byte first = atoi(node->values["first"].c_str());
		const byte second = atoi(node->values["second"].c_str());
		_font._kerning[first * 256 + second] = atoi(node->values["amount"].c_str());
		return true;
	}
};

bool BitmapFont::load(const Common::Path &path) {
	BitmapFont font;
	Loader loader(font, path);
	if (!loader.loadFile(path) || !loader.parse() || font._glyphs.empty()) {
		warning("Could not load bitmap font %s", path.toString().c_str());
		return false;
	}
	*this = Common::move(font);
	return true;
}

const BitmapFont::Glyph &BitmapFont::glyph(uint32 chr) const {
	return _glyphs.getValOrDefault(chr, _glyphs.getValOrDefault('*'));
}

int BitmapFont::getCharWidth(uint32 chr) const {
	const Glyph &g = glyph(chr);
	return g.advance + g.offset.x + 2 * _outline + _spacing.x;
}

int BitmapFont::getKerningOffset(uint32 left, uint32 right) const {
	return _kerning.getValOrDefault(left * 256 + right);
}

Common::Rect BitmapFont::getBoundingBox(uint32 chr) const {
	const Glyph &g = glyph(chr);
	return Common::Rect(g.offset, g.source.width(), g.source.height());
}

void BitmapFont::drawChar(Graphics::Surface *target, uint32 chr, int x, int y, uint32 color) const {
	Graphics::ManagedSurface view;
	view.surfacePtr()->init(target->w, target->h, target->pitch, target->getPixels(), target->format);
	drawChar(&view, chr, x, y, color);
}

void BitmapFont::drawChar(Graphics::ManagedSurface *target, uint32 chr, int x, int y, uint32 color) const {
	const Glyph &g = glyph(chr);
	if (!g.source.isEmpty())
		_pages[g.page]->blitTo(*target, g.source, Common::Point(x, y) + g.offset);
}

int BitmapFont::lineHeight(const Common::String &line) const {
	int height = 0;
	for (const char c : line)
		height = MAX<int>(height, getBoundingBox((byte)c).bottom + 2 * _outline + _spacing.y);
	return height;
}

void BitmapFont::drawCentered(Graphics::ManagedSurface &target, const Common::Rect &bounds, const Common::String &text) const {
	Common::Array<Common::String> lines;
	uint start = 0;
	int width = 0;
	for (uint i = 0; i < text.size(); ++i) {
		if (text[i] == '%' && i + 1 < text.size() && text[i + 1] == 'n') {
			lines.push_back(text.substr(start, i - start));
			++i;
			start = i + 1;
			width = 0;
			continue;
		}
		width += getCharWidth((byte)text[i]);
		if (width >= bounds.width()) {
			// DivideString breaks at a preceding space and resumes measuring after the next letter.
			for (uint j = i; j > start; --j) {
				if (text[j - 1] == ' ' && text[j] != 'n') {
					lines.push_back(text.substr(start, j - 1 - start));
					start = j;
					i = j;
					width = 0;
					break;
				}
			}
		}
	}
	lines.push_back(text.substr(start));
	int totalHeight = 0;
	for (const Common::String &line : lines)
		totalHeight += lineHeight(line);
	float y = bounds.top + (bounds.height() - totalHeight) * 0.5f;
	for (const Common::String &line : lines) {
		const int x = (int)roundf(bounds.left + (bounds.width() - getStringWidth(line)) * 0.5f);
		const int top = (int)roundf(y - lineHeight(line) * 0.5f);
		drawString(&target, line, x, top, target.w - x, 0, Graphics::kTextAlignLeft, 0, false, true);
		y += (float)totalHeight / lines.size();
		if (line == " ")
			y += _extraLineSpacing;
	}
}

} // End of namespace NGI
