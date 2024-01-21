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

#include "twp/font.h"
#include "twp/twp.h"
#include "twp/ggpack.h"
#include "common/str.h"
#include "graphics/opengl/system_headers.h"

namespace Twp {

enum TokenId {
	tiWhitespace,
	tiString,
	tiColor,
	tiNewLine,
	tiEnd
};

typedef struct Token {
	TokenId id;
	int startOff, endOff;
} Token;

typedef struct CharInfo {
	CodePoint chr;
	Math::Vector2d pos;
	Color color;
	Glyph glyph;
} CharInfo;

typedef struct Line {
	Common::Array<Token> tokens;
	Common::Array<CharInfo> charInfos;
} Line;

class TokenReader {
public:
	TokenReader(const Common::U32String &text);
	Common::U32String substr(Token tok);
	bool readToken(Token &token);

private:
	CodePoint readChar();
	TokenId readTokenId();

private:
	Common::U32String _text;
    size_t _off;
};

static Math::Vector2d normalize(Texture *texture, Math::Vector2d v) {
	Math::Vector2d textureSize(texture->width, texture->height);
	return Math::Vector2d(v.getX() / textureSize.getX(), v.getY() / textureSize.getY());
}

static void addGlyphQuad(Texture *texture, Common::Array<Vertex> &vertices, CharInfo info) {
	// Add a glyph quad to the vertex array

	float left = info.glyph.bounds.left;
	float top = info.glyph.bounds.bottom;
	float right = info.glyph.bounds.right;
	float bottom = info.glyph.bounds.top;

	Math::Vector2d uv1 = normalize(texture, Math::Vector2d(info.glyph.textureRect.left, info.glyph.textureRect.bottom));
	Math::Vector2d uv2 = normalize(texture, Math::Vector2d(info.glyph.textureRect.right, info.glyph.textureRect.top));

	vertices.push_back(Vertex{Math::Vector2d(info.pos.getX() + left, info.pos.getY() + top), info.color, Math::Vector2d(uv1.getX(), uv2.getY())});
	vertices.push_back(Vertex{Math::Vector2d(info.pos.getX() + right, info.pos.getY() + top), info.color, Math::Vector2d(uv2.getX(), uv2.getY())});
	vertices.push_back(Vertex{Math::Vector2d(info.pos.getX() + left, info.pos.getY() + bottom), info.color, Math::Vector2d(uv1.getX(), uv1.getY())});
	vertices.push_back(Vertex{Math::Vector2d(info.pos.getX() + left, info.pos.getY() + bottom), info.color, Math::Vector2d(uv1.getX(), uv1.getY())});
	vertices.push_back(Vertex{Math::Vector2d(info.pos.getX() + right, info.pos.getY() + top), info.color, Math::Vector2d(uv2.getX(), uv2.getY())});
	vertices.push_back(Vertex{Math::Vector2d(info.pos.getX() + right, info.pos.getY() + bottom), info.color, Math::Vector2d(uv2.getX(), uv1.getY())});
}

// Skips all characters while one char from the set `token` is found.
// Returns number of characters skipped.
static int skipWhile(const Common::U32String& s, const char *toSkip, int start = 0) {
	int result = 0;
	int len = s.size();
	while ((start + result < len) && strchr(toSkip, s[result + start]))
		result++;
	return result;
}

static int skipUntil(const Common::U32String& s, const char *until, int start = 0) {
	int result = 0;
	int len = s.size();
	while ((start + result < len) && !strchr(until, s[result + start]))
		result++;
	return result;
}

static float width(Text &text, TokenReader &reader, Token tok) {
	float result = 0;
	Common::String s = reader.substr(tok);
	for (size_t i = 0; i < s.size(); i++) {
		char c = s[i];
		result += text.getFont()->getGlyph(c).advance;
	}
	return result;
}

TokenReader::TokenReader(const Common::U32String &text) : _text(text), _off(0) {
}

Common::U32String TokenReader::substr(Token tok) {
	return _text.substr(tok.startOff, tok.endOff - tok.startOff + 1);
}

CodePoint TokenReader::readChar() {
	char result = _text[_off];
	_off++;
	return result;
}

TokenId TokenReader::readTokenId() {
	const char Whitespace[] = {' ', '\t', '\v', '\r', '\f'};
	const char Whitespace2[] = {' ', '\t', '\v', '\r', '\f', '#', '\n'};
	if (_off < _text.size()) {
		char c = readChar();
		switch (c) {
		case '\n':
			return tiNewLine;
		case '\t':
		case ' ':
			_off += skipWhile(_text, Whitespace, _off);
			return tiWhitespace;
		case '#':
			_off += 7;
			return tiColor;
		default:
			_off += skipUntil(_text, Whitespace2, _off);
			return tiString;
		}
	} else {
		return tiEnd;
	}
}

bool TokenReader::readToken(Token &token) {
	int start = _off;
	TokenId id = readTokenId();
	if (id != tiEnd) {
		token.id = id;
		token.startOff = start;
		token.endOff = _off - 1;
		return true;
	}
	return false;
}

GGFont::~GGFont() {}

void GGFont::load(const Common::String &path) {
	SpriteSheet *spritesheet = g_engine->_resManager.spriteSheet(path);
	int lineHeight = 0;
	for (auto it = spritesheet->frameTable.begin(); it != spritesheet->frameTable.end(); it++) {
		const SpriteSheetFrame &frame = it->_value;
		Glyph glyph;
		glyph.advance = MAX(frame.sourceSize.getX() - frame.spriteSourceSize.left - 4, 0.f);
		glyph.bounds = Common::Rect(Common::Point(frame.spriteSourceSize.left, frame.sourceSize.getY() - frame.spriteSourceSize.height() - frame.spriteSourceSize.top), frame.spriteSourceSize.width(), frame.spriteSourceSize.height());
		lineHeight = MAX(lineHeight, (int)frame.spriteSourceSize.top);
		glyph.textureRect = frame.frame;
		_glyphs[it->_key.asUint64()] = glyph;
	}
	_lineHeight = lineHeight;
	_name = path;
}

Glyph GGFont::getGlyph(CodePoint chr) {
	int key = (int)chr;
	if (_glyphs.contains(key)) {
		return _glyphs[key];
	}
	return _glyphs['?'];
}

BmFont::~BmFont() {}

void BmFont::load(const Common::String &name) {
	Common::String path = name + ".fnt";
	if (!g_engine->_pack.assetExists(path.c_str())) {
		path = name + "Font.fnt";
	}
	debug("Load font %s", path.c_str());
	GGPackEntryReader entry;
	if (!entry.open(g_engine->_pack, path)) {
		error("error loading font %s", path.c_str());
	}
	char tmp[80];
	while (!entry.eos()) {
		Common::String line = entry.readLine();
		if (line.hasPrefix("common")) {
			sscanf(line.c_str(), "common lineHeight=%d base=%d scaleW=%d scaleH=%d pages=%d packed=%d", &_lnHeight, &_base, &_scaleW, &_scaleH, &_pages, &_packed);
		} else if (line.hasPrefix("chars")) {
		} else if (line.hasPrefix("char")) {
			Char c;
			sscanf(line.c_str(), "char id=%d\tx=%d\ty=%d\twidth=%d\theight=%d\txoffset=%d\tyoffset=%d\txadvance=%d\tpage=%d\tchnl=%d\tletter=\"%s\"", &c.id, &c.x, &c.y, &c.w, &c.h, &c.xoff, &c.yoff, &c.xadv, &c.page, &c.chnl, tmp);
			_glyphs[c.id] = Glyph{c.xadv,
								  Common::Rect(c.xoff, _lnHeight - c.yoff - c.h, c.xoff + c.w, _lnHeight - c.yoff),
								  Common::Rect(c.x, c.y, c.x + c.w, c.y + c.h)};
		} else if (line.hasPrefix("kernings")) {
		} else if (line.hasPrefix("kerning")) {
			KerningKey key;
			int amount = 0;
			sscanf(line.c_str(), "kerning\tfirst=%d\tsecond=%d\tamount=%d", &key.first, &key.second, &amount);
			_kernings[key] = amount;
		}
	}
	_name = name;
}

Glyph BmFont::getGlyph(CodePoint chr) {
	if (_glyphs.contains(chr)) {
		return _glyphs[chr];
	}
	return _glyphs['?'];
}

float BmFont::getKerning(CodePoint prev, CodePoint next) {
	return 0.f;
}

bool operator==(const KerningKey &l, const KerningKey &r) {
	return l.first == r.first && l.second == r.second;
}

Text::Text(const Common::String &fontName, const Common::String &text, TextHAlignment hAlign, TextVAlignment vAlign, float maxWidth, Color color)
	: _font(NULL), _fontName(fontName), _texture(NULL), _txt(text), _col(color), _hAlign(hAlign), _vAlign(vAlign), _maxW(maxWidth), _dirty(true) {
	update();
}

Text::Text() {}

void Text::setFont(const Common::String &fontName) {
	_fontName = fontName;
	_dirty = true;
}

Math::Vector2d Text::getBounds() {
	update();
	return _bnds;
}

void Text::update() {
	if (_dirty) {
		_dirty = false;
		_font = g_engine->_resManager.font(_fontName);
		_texture = g_engine->_resManager.texture(_font->getName() + ".png");

		// Reset
		_vertices.clear();
		_quads.clear();
		_bnds = Math::Vector2d();
		Color color = _col;

		// split text by tokens and split tokens by lines
		Common::Array<Line> lines;
		Line line1;
		TokenReader reader(_txt);
		float x = 0;
		Token tok;
		while (reader.readToken(tok)) {
			// ignore color token width
			float w = tok.id == tiColor || tok.id == tiNewLine ? 0.f : width(*this, reader, tok);
			// new line if width > maxWidth or newline character
			if (tok.id == tiNewLine || ((_maxW > 0) && (line1.tokens.size() > 0) && (x + w > _maxW))) {
				lines.push_back(line1);
				line1.tokens.clear();
				x = 0;
			}
			if (tok.id != tiNewLine) {
				if (line1.tokens.size() != 0 || tok.id != tiWhitespace) {
					line1.tokens.push_back(tok);
					x += w;
				}
			}
		}
		lines.push_back(line1);

		// create quads for all characters
		float maxW = 0.f;
		float lineHeight = _font->getLineHeight();
		float y = -lineHeight;
		for (size_t i = 0; i < lines.size(); i++) {
			Line &line = lines[i];
			CodePoint prevChar;
			x = 0;
			for (size_t j = 0; j < line.tokens.size(); j++) {
				tok = line.tokens[j];
				if (tok.id == tiColor) {
					uint iColor;
					Common::String s = reader.substr(tok);
					sscanf(s.c_str() + 1, "%x", &iColor);
					color = Color::withAlpha(Color::rgb((int)(iColor & 0x00FFFFFF)), color.rgba.a);
				} else {
					Common::U32String s = reader.substr(tok);
					for (size_t k = 0; k < s.size(); k++) {
						CodePoint c = s[k];
						Glyph glyph = _font->getGlyph(c);
						float kern = _font->getKerning(prevChar, c);
						prevChar = c;
						line.charInfos.push_back(CharInfo{c, Math::Vector2d(x + kern, y), color, glyph});
						// self.quads.add(rect(x, y, glyph.bounds.x.float32 + glyph.bounds.w.float32, lineHeight));
						x += (float)glyph.advance;
					}
				}
			}
			_quads.push_back(Common::Rect(Common::Point(0.0f, y), x, lineHeight));
			maxW = MAX(maxW, x);
			y -= lineHeight;
		}

		// Align text
		if (_hAlign == thRight) {
			for (size_t i = 0; i < lines.size(); i++) {
				float w = maxW - _quads[i].width();
				for (size_t j = 0; j < lines[i].charInfos.size(); j++) {
					CharInfo &info = lines[i].charInfos[j];
					info.pos.setX(info.pos.getX() + w);
				}
			}
		} else if (_hAlign == thCenter) {
			for (size_t i = 0; i < lines.size(); i++) {
				float w = maxW - _quads[i].width();
				for (size_t j = 0; j < lines[i].charInfos.size(); j++) {
					CharInfo &info = lines[i].charInfos[j];
					info.pos.setX(info.pos.getX() + w / 2.f);
				}
			}
		}

		// Add the glyphs to the vertices
		for (size_t i = 0; i < lines.size(); i++) {
			for (size_t j = 0; j < lines[i].charInfos.size(); j++) {
				const CharInfo &info = lines[i].charInfos[j];
				addGlyphQuad(_texture, _vertices, info);
			}
		}

		_bnds = Math::Vector2d(maxW, lines.size() * _font->getLineHeight());
	}
}

void Text::draw(Gfx &gfx, Math::Matrix4 trsf) {
	if (_font && _txt.size() > 0) {
		update();
		gfx.drawPrimitives(GL_TRIANGLES, _vertices.begin(), _vertices.size(), trsf, _texture);
	}
}

} // namespace Twp
