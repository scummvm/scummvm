/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.

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

#include "common/tokenizer.h"
#include "common/unicode-bidi.h"

#include "graphics/macgui/mactext.h"

namespace Graphics {

#define DEBUG 0

#if DEBUG
#define D(...)  debug(__VA_ARGS__)
#define DN(...)  debugN(__VA_ARGS__)
#else
#define D(...)  ((void)0)
#define DN(...) ((void)0)
#endif

MacTextCanvas::~MacTextCanvas() {
	delete _surface;
	delete _shadowSurface;

	for (auto &t : _text) {
		delete t.table;
		delete t.tableSurface;
	}
}

// Adds the given string to the end of the last line/chunk
// while observing the _canvas._maxWidth and keeping this chunk's
// formatting
void MacTextCanvas::chopChunk(const Common::U32String &str, int *curLinePtr, int indent, int maxWidth) {
	int curLine = *curLinePtr;
	int curChunk;
	MacFontRun *chunk;

	curChunk = _text[curLine].chunks.size() - 1;
	chunk = &_text[curLine].chunks[curChunk];

	// Check if there is nothing to add, then remove the last chunk
	// This happens when the previous run is finished only with
	// empty formatting, or when we were adding text for the first time
	if (chunk->text.empty() && str.empty()) {
		D(9, "** chopChunk, replaced formatting, line %d", curLine);

		_text[curLine].chunks.pop_back();

		return;
	}

	if (maxWidth == -1) {
		chunk->text += str;

		return;
	}

	Common::Array<Common::U32String> text;

	int w = getLineWidth(curLine, true);
	D(9, "** chopChunk before wrap \"%s\"", Common::toPrintable(str.encode()).c_str());

	chunk->getFont()->wordWrapText(str, maxWidth, text, w);

	if (text.size() == 0) {
		warning("chopChunk: too narrow width, >%d", maxWidth);
		chunk->text += str;
		getLineCharWidth(curLine, true);

		return;
	}

	for (int i = 0; i < (int)text.size(); i++) {
		D(9, "** chopChunk result %d \"%s\"", i, toPrintable(text[i].encode()).c_str());
	}
	chunk->text += text[0];

	// Recalc dims
	getLineWidth(curLine, true);

	D(9, "** chopChunk, subchunk: \"%s\" (%d lines, maxW: %d)", toPrintable(text[0].encode()).c_str(), text.size(), maxWidth);

	// We do not overlap, so we're done
	if (text.size() == 1)
		return;

	// Now add rest of the chunks
	MacFontRun newchunk = _text[curLine].chunks[curChunk];

	for (uint i = 1; i < text.size(); i++) {
		newchunk.text = text[i];

		curLine++;
		_text.insert_at(curLine, MacTextLine());
		_text[curLine].chunks.push_back(newchunk);
		_text[curLine].indent = indent;
		_text[curLine].firstLineIndent = 0;

		D(9, "** chopChunk, added line (firstIndent: %d): \"%s\"", _text[curLine].firstLineIndent, toPrintable(text[i].encode()).c_str());
	}

	*curLinePtr = curLine;
}

void MacTextCanvas::splitString(const Common::U32String &str, int curLine, MacFontRun &defaultFormatting) {
	D(9, "** splitString(\"%s\", %d)", toPrintable(str.encode()).c_str(), curLine);

	if (str.empty()) {
		D(9, "** splitString, empty line");
		return;
	}

	(void)splitString(str.c_str(), curLine, defaultFormatting);
}

Common::String preprocessImageExt(const char *ptr) {
	// w[idth]=WWWw  -- width in units 'w'
	// h[eight]=HHHh -- height in units 'h'
	//
	// units:
	//   % for percents of the text width  -> %
	//   em for font height as a unit      -> e
	//   px for actual pixels              -> p
	//
	// Translated into fixed format:
	// WWWWwHHHHh -- 4 fixed hex numbers followed by units

	int w = 0, h = 0;
	char wu = ' ', hu = ' ';

	enum {
		kStateNone,
		kStateW,
		kStateH,
	};

	int state = kStateNone;

	while (*ptr) {
		if (*ptr == ' ' || *ptr == '\t') {
			ptr++;
			continue;
		}

		if (*ptr == 'w' || *ptr == 'h') {
			state = *ptr == 'w' ? kStateW : kStateH;

			while (*ptr && *ptr != '=')
				ptr++;

			if (*ptr != '=') {
				warning("MacTextCanvas: Malformatted image extention: '=' expected at '%s'", ptr);
				return "";
			}
		} else if (Common::isDigit(*ptr)) {
			int num = 0;

			if (state == kStateNone) {
				warning("MacTextCanvas: Malformatted image extention: unexpected digit at '%s'", ptr);
				return "";
			}

			while (*ptr && Common::isDigit(*ptr)) {
				num *= 10;
				num += *ptr - '0';

				ptr++;
			}

			if (*ptr == 'e' || *ptr == '%' || *ptr == 'p') {
				char unit = *ptr == 'e' ? 'm' : *ptr;
				if (state == kStateW) {
					w = num;
					wu = unit;
				} else {
					h = num;
					hu = unit;
				}

				while (*ptr && *ptr != ' ' && *ptr != '\t')
					ptr++;
			} else {
				warning("MacTextCanvas: Malformatted image extention: %% or e[m] or p[x] expected at '%s'", ptr);
				return "";
			}
		} else {
			warning("MacTextCanvas: Malformatted image extention: w[idth] or h[eight] expected at '%s'", ptr);
			return "";
		}

		ptr++;
	}

	return Common::String::format("%04x%c%04x%c", w, wu, h, hu);
}

const Common::U32String::value_type *MacTextCanvas::splitString(const Common::U32String::value_type *s, int curLine, MacFontRun &defaultFormatting) {
	if (_text.empty()) {
		_text.resize(1);
		_text[0].chunks.push_back(defaultFormatting);
		D(9, "** splitString, added default formatting");
	} else {
		D(9, "** splitString, continuing, %d lines", _text.size());
	}

	_defaultFormatting = defaultFormatting;

	Common::U32String tmp;

	if (curLine == -1 || curLine >= (int)_text.size())
		curLine = _text.size() - 1;

	if (_text[curLine].chunks.empty())
		_text[curLine].chunks.push_back(_defaultFormatting);

	int curChunk = _text[curLine].chunks.size() - 1;
	MacFontRun chunk = _text[curLine].chunks[curChunk];
	int indentSize = 0;
	int firstLineIndent = 0;
	bool inTable = false;

	while (*s) {
		firstLineIndent = 0;

		tmp.clear();

		MacTextLine *curTextLine = &_text[curLine];

		while (*s) {
			bool endOfLine = false;

			// Scan till next font change or end of line
			while (*s && *s != '\001') {
				if (*s == '\r') {
					s++;
					if (*s == '\n')	// Skip whole '\r\n'
						s++;

					endOfLine = true;

					break;
				}

				// deal with single \n
				if (*s == '\n') {
					s++;

					endOfLine = true;
					break;
				}

				tmp += *s;

				s++;
			}

			if (*s == '\001')	// If it was \001, skip it
				s++;

			if (*s == '\001') { // \001\001 -> \001
				tmp += *s++;

				if (*s)	// Check we reached end of line
					continue;
			}

			D(9, "\n** splitString, chunk: \"%s\"", Common::toPrintable(tmp.encode()).c_str());

			// Okay, now we are either at the end of the line, or in the next
			// chunk definition. That means, that we have to store the previous chunk
			chopChunk(tmp, &curLine, indentSize, _maxWidth > 0 ? _maxWidth - indentSize : _maxWidth);

			curTextLine = &_text[curLine];

			firstLineIndent = curTextLine->firstLineIndent;

			tmp.clear();

			// If it is end of the line, we're done
			if (!*s) {
				D(9, "** splitString, end of line");

				break;
			}

			// get format (sync with stripFormat() )
			if (*s == '\016') {	// human-readable format
				s++;

				// First two digits is slant, third digit is Header number
				switch (*s) {
				case '+': { // \016+XXYZ  -- opening textSlant, H<Y>, indent<+Z>
					uint16 textSlant, headSize, indent;
					s++;

					s = readHex(&textSlant, s, 2);

					chunk.textSlant |= textSlant; // Setting the specified bit

					s = readHex(&headSize, s, 1);
					if (headSize >= 1 && headSize <= 6) { // set
						const float sizes[] = { 1, 2.0f, 1.41f, 1.155f, 1.0f, .894f, .816f };
						chunk.fontSize = defaultFormatting.fontSize * sizes[headSize];
					}

					s = readHex(&indent, s, 1);

					if (s)
						indentSize += indent * chunk.fontSize * 2;

					D(9, "** splitString+: fontId: %d, textSlant: %d, fontSize: %d, indent: %d",
							chunk.fontId, chunk.textSlant, chunk.fontSize,
							indent);

					break;
					}
				case '-': { // \016-XXYZ  -- closing textSlant, H<Y>, indent<+Z>
					uint16 textSlant, headSize, indent;
					s++;

					s = readHex(&textSlant, s, 2);

					chunk.textSlant &= ~textSlant; // Clearing the specified bit

					s = readHex(&headSize, s, 1);
					if (headSize == 0xf) // reset
						chunk.fontSize = _defaultFormatting.fontSize;

					s = readHex(&indent, s, 1);

					if (s)
						indentSize -= indent * chunk.fontSize * 2;

					D(9, "** splitString-: fontId: %d, textSlant: %d, fontSize: %d, indent: %d",
							chunk.fontId, chunk.textSlant, chunk.fontSize,
							indent);
					break;
					}

				case '[': { // \016[RRGGBB  -- setting color
					uint16 palinfo1, palinfo2, palinfo3;
					s++;

					s = readHex(&palinfo1, s, 4);
					s = readHex(&palinfo2, s, 4);
					s = readHex(&palinfo3, s, 4);

					chunk.palinfo1 = palinfo1;
					chunk.palinfo2 = palinfo2;
					chunk.palinfo3 = palinfo3;
					chunk.fgcolor  = _wm->findBestColor(palinfo1 & 0xff, palinfo2 & 0xff, palinfo3 & 0xff);

					D(9, "** splitString[: %08x", chunk.fgcolor);
					break;
					}

				case ']': { // \016]  -- setting default color
					s++;

					chunk.palinfo1 = _defaultFormatting.palinfo1;
					chunk.palinfo2 = _defaultFormatting.palinfo2;
					chunk.palinfo3 = _defaultFormatting.palinfo3;
					chunk.fgcolor  = _defaultFormatting.fgcolor;

					D(9, "** splitString]: %08x", chunk.fgcolor);
					break;
					}

				case '*': { // \016*XXsssssss  -- negative indent, XX size, sssss is the string
					s++;

					uint16 len;

					s = readHex(&len, s, 2);

					Common::U32String bullet = Common::U32String(s, len);

					s += len;

					firstLineIndent = -chunk.getFont()->getStringWidth(bullet);

					D(9, "** splitString*: %02x '%s' (%d)", len, bullet.encode().c_str(), firstLineIndent);
					break;
					}

				case 'i': { // \016iXXNNnnnnAAaaaaTTttt -- image, XX% width,
										//          NN, nnnn -- filename len and text
										//          AA, aaaa -- alt len and text
										//          TT, tttt -- text (tooltip) len and text
					s++;

					uint16 len;

					s = readHex(&_text[curLine].picpercent, s, 2);
					s = readHex(&len, s, 2);
					_text[curLine].picfname = Common::U32String(s, len).encode();
					s += len;

					s = readHex(&len, s, 2);
					_text[curLine].picalt = Common::U32String(s, len);
					s += len;

					s = readHex(&len, s, 2);
					_text[curLine].pictitle = Common::U32String(s, len);
					s += len;

					s = readHex(&len, s, 2);
					_text[curLine].picext = preprocessImageExt(Common::U32String(s, len).encode().c_str());
					s += len;

					D(9, "** splitString[i]: %d%% fname: '%s'  alt: '%s'  title: '%s'  ext: '%s'",
						_text[curLine].picpercent,
						_text[curLine].picfname.c_str(), _text[curLine].picalt.encode().c_str(),
						_text[curLine].pictitle.encode().c_str(), _text[curLine].picext.encode().c_str());
					break;
					}

				case 't': { // \016tXXXX -- switch to the requested font id
					s++;

					uint16 fontId;

					s = readHex(&fontId, s, 4);

					chunk.fontId = fontId == 0xffff ? _defaultFormatting.fontId : fontId;

					D(9, "** splitString[t]: fontId: %d", fontId);
					break;
					}

				case 'l': { // \016lLLllll -- link len and text
					s++;

					uint16 len;

					s = readHex(&len, s, 2);
					chunk.link = Common::U32String(s, len);
					s += len;

					D(9, "** splitString[l]: link: %s", chunk.link.c_str());
					break;
					}

				case 'T': { // \016T -- table
					s++;

					char cmd = *s++;

					if (cmd == 'h') { // Header, beginning of the table
						curTextLine->table = new Common::Array<MacTextTableRow>();
						inTable = true;

						D(9, "** splitString[table header]");
					} else if (cmd == 'b') { // Body start
						D(9, "** splitString[body start]");
					} else if (cmd == 'B') { // Body end
						inTable = false;

						D(9, "** splitString[body end]");
						processTable(curLine, _maxWidth);

						continue;
					} else if (cmd == 'r') { // Row
						curTextLine->table->push_back(MacTextTableRow());
						D(9, "** splitString[row]");
					} else if (cmd == 'c') { // Cell start
						uint16 align;
						s = readHex(&align, s, 2);

						curTextLine->table->back().cells.push_back(MacTextCanvas());

						MacTextCanvas *cellCanvas = &curTextLine->table->back().cells.back();
						cellCanvas->_textAlignment = (TextAlign)align;
						cellCanvas->_wm = _wm;
						cellCanvas->_macText = _macText;
						cellCanvas->_maxWidth = -1;
						cellCanvas->_macFontMode = _macFontMode;
						cellCanvas->_tfgcolor = _tfgcolor;
						cellCanvas->_tbgcolor = _tbgcolor;

						D(9, "** splitString[cell start]: align: %d", align);

						D(9, "** splitString[RECURSION start]");

						s = cellCanvas->splitString(s, curLine, _defaultFormatting);

						D(9, "** splitString[RECURSION end]");
					} else if (cmd == 'C') { // Cell end
						D(9, "** splitString[cell end]");

						return s;
					} else {
						error("MacText: Unknown table subcommand (%c)", cmd);
					}
					break;
					}

				default: {
					uint16 fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3;

					s = readHex(&fontId, s, 4);
					s = readHex(&textSlant, s, 2);
					s = readHex(&fontSize, s, 4);
					s = readHex(&palinfo1, s, 4);
					s = readHex(&palinfo2, s, 4);
					s = readHex(&palinfo3, s, 4);

					chunk.setValues(_wm, fontId, textSlant, fontSize, palinfo1, palinfo2, palinfo3);

					D(9, "** splitString: fontId: %d, textSlant: %d, fontSize: %d, fg: %04x (from %04x %04x %04x)",
							fontId, textSlant, fontSize, chunk.fgcolor, palinfo1, palinfo2, palinfo3);

					// So far, we enforce single font here, though in the future, font size could be altered
					if (!_macFontMode)
						chunk.font = _defaultFormatting.font;
					}
				}
			}

			D(9, "*** splitString: text[%d] indent: %d, fi: %d", curLine, indentSize, firstLineIndent);

			curTextLine->indent = indentSize;
			curTextLine->firstLineIndent = firstLineIndent;

			// Push new formatting
			curTextLine->chunks.push_back(chunk);

			// If we reached end of paragraph, go to outer loop
			if (endOfLine)
				break;
		}

		// We avoid adding new lines while in table. Recursive cell rendering
		// has this flag as false (obviously)
		if (inTable)
			continue;

		curTextLine->paragraphEnd = true;
		// if the chunks is empty, which means the line will not be rendered properly
		// so we add a empty string here
		if (curTextLine->chunks.empty()) {
			curTextLine->chunks.push_back(_defaultFormatting);
		}

		if (*s) {
			// Add new line
			D(9, "** splitString: new line");

			curLine++;
			_text.insert_at(curLine, MacTextLine());
			_text[curLine].chunks.push_back(chunk);

			curTextLine = &_text[curLine];

			curTextLine->indent = indentSize;
			curTextLine->firstLineIndent = firstLineIndent;
		}
	}

#if DEBUG
	debugPrint("** splitString");
#endif

	return s;
}


void MacTextCanvas::reallocSurface() {
	// round to closest 10
	//TODO: work out why this rounding doesn't correctly fill the entire width
	//int requiredH = (_text.size() + (_text.size() * 10 + 9) / 10) * lineH

	if (!_surface) {
		_surface = new ManagedSurface(_maxWidth, _textMaxHeight, _wm->_pixelformat);

		if (_textShadow)
			_shadowSurface = new ManagedSurface(_maxWidth, _textMaxHeight, _wm->_pixelformat);

		return;
	}

	if (_surface->w < _maxWidth || _surface->h < _textMaxHeight) {
		// realloc surface and copy old content
		ManagedSurface *n = new ManagedSurface(_maxWidth, _textMaxHeight, _wm->_pixelformat);
		n->clear(_tbgcolor);
		n->blitFrom(*_surface, Common::Point(0, 0));

		delete _surface;
		_surface = n;

		// same as shadow surface
		if (_textShadow) {
			ManagedSurface *newShadowSurface = new ManagedSurface(_maxWidth, _textMaxHeight, _wm->_pixelformat);
			newShadowSurface->clear(_tbgcolor);
			newShadowSurface->blitFrom(*_shadowSurface, Common::Point(0, 0));

			delete _shadowSurface;
			_shadowSurface = newShadowSurface;
		}
	}
}

void MacTextCanvas::render(int from, int to, int shadow) {
	int w = MIN(_maxWidth, _textMaxWidth);
	ManagedSurface *surface = shadow ? _shadowSurface : _surface;

	int myFrom = from, myTo = to + 1, delta = 1;

	if (_wm->_language == Common::HE_ISR) {
		myFrom = to;
		myTo = from - 1;
		delta = -1;
	}

	for (int i = myFrom; i != myTo; i += delta) {
		if (!_text[i].picfname.empty()) {
			const Surface *image = _macText->getImageSurface(_text[i].picfname);

			int xOffset = (_text[i].width - _text[i].charwidth) / 2;
			Common::Rect bbox(xOffset, _text[i].y, xOffset + _text[i].charwidth, _text[i].y + _text[i].height);

			if (image) {
				surface->blitFrom(image, Common::Rect(0, 0, image->w, image->h), bbox);

				D(9, "MacTextCanvas::render: Image %d x %d bbox: %d, %d, %d, %d", image->w, image->h, bbox.left, bbox.top,
						bbox.right, bbox.bottom);
			}

			continue;
		}

		if (_text[i].tableSurface) {
			surface->blitFrom(*_text[i].tableSurface, Common::Point(0, _text[i].y));

			D(9, "MacTextCanvas::render: Table %d x %d at: %d, %d", _text[i].tableSurface->w, _text[i].tableSurface->h, 0, _text[i].y);

			continue;
		}

		int xOffset = getAlignOffset(i) + _text[i].indent + _text[i].firstLineIndent;
		xOffset++;

		int start = 0, end = _text[i].chunks.size();
		if (_wm->_language == Common::HE_ISR) {
			start = _text[i].chunks.size() - 1;
			end = -1;
		}

		int maxAscentForRow = 0;
		for (int j = start; j != end; j += delta) {
			if (_text[i].chunks[j].font->getFontAscent() > maxAscentForRow)
				maxAscentForRow = _text[i].chunks[j].font->getFontAscent();
		}

		// TODO: _canvas._textMaxWidth, when -1, was not rendering ANY text.
		for (int j = start; j != end; j += delta) {
			D(9, "MacTextCanvas::render: line %d[%d] h:%d at %d,%d (%s) fontid: %d fontsize: %d on %dx%d, fgcolor: %08x bgcolor: %08x",
				  i, j, _text[i].height, xOffset, _text[i].y, _text[i].chunks[j].text.encode().c_str(),
				  _text[i].chunks[j].fontId, _text[i].chunks[j].fontSize, _surface->w, _surface->h, _text[i].chunks[j].fgcolor, _tbgcolor);

			if (_text[i].chunks[j].text.empty())
				continue;

			int yOffset = 0;
			if (_text[i].chunks[j].font->getFontAscent() < maxAscentForRow) {
				yOffset = maxAscentForRow - _text[i].chunks[j].font->getFontAscent();
			}

			if (_text[i].chunks[j].plainByteMode()) {
				Common::String str = _text[i].chunks[j].getEncodedText();
				_text[i].chunks[j].getFont()->drawString(surface, str, xOffset, _text[i].y + yOffset, w, shadow ? _wm->_colorBlack : _text[i].chunks[j].fgcolor, kTextAlignLeft, 0, true);
				xOffset += _text[i].chunks[j].getFont()->getStringWidth(str);
			} else {
				if (_wm->_language == Common::HE_ISR)
					_text[i].chunks[j].getFont()->drawString(surface, convertBiDiU32String(_text[i].chunks[j].text, Common::BIDI_PAR_RTL), xOffset, _text[i].y + yOffset, w, shadow ? _wm->_colorBlack : _text[i].chunks[j].fgcolor, kTextAlignLeft, 0, true);
				else
					_text[i].chunks[j].getFont()->drawString(surface, convertBiDiU32String(_text[i].chunks[j].text), xOffset, _text[i].y + yOffset, w, shadow ? _wm->_colorBlack : _text[i].chunks[j].fgcolor, kTextAlignLeft, 0, true);
				xOffset += _text[i].chunks[j].getFont()->getStringWidth(_text[i].chunks[j].text);
			}
		}
	}
}

void MacTextCanvas::render(int from, int to) {
	if (_text.empty())
		return;

	reallocSurface();

	from = MAX<int>(0, from);
	to = MIN<int>(to, _text.size() - 1);

	// Clear the screen
	_surface->fillRect(Common::Rect(0, _text[from].y, _surface->w, _text[to].y + getLineHeight(to)), _tbgcolor);

	// render the shadow surface;
	if (_textShadow)
		render(from, to, _textShadow);

	render(from, to, 0);

	debugPrint("MacTextCanvas::render");
}

int getStringMaxWordWidth(MacFontRun &format, const Common::U32String &str) {
	if (format.plainByteMode()) {
		Common::StringTokenizer tok(Common::convertFromU32String(str, format.getEncoding()));
		int maxW = 0;

		while (!tok.empty()) {
			int w = format.getFont()->getStringWidth(tok.nextToken());

			maxW = MAX(maxW, w);
		}

		return maxW;
	} else {
		Common::U32StringTokenizer tok(str);
		int maxW = 0;

		while (!tok.empty()) {
			int w = format.getFont()->getStringWidth(tok.nextToken());

			maxW = MAX(maxW, w);
		}

		return maxW;
	}
}

void MacTextCanvas::parsePicExt(const Common::U32String &ext, uint16 &wOut, uint16 &hOut, int defpercent) {
	const Common::U32String::value_type *s = ext.c_str();

	D(9, "P: %s", ext.encode().c_str());

	// wwwwWhhhhH
	// 0123456789

	bool useDefault = false;

	if (ext.size() == 10 && s[4] != ' ' && s[9] != ' ' && s[4] != s[9])  {
		warning("MacTextCanvas: Non-matching dimension unitss in image extension: '%s'", ext.encode().c_str());

		useDefault = true;
	}

	// if it is empty or without dimensions, use default width percrent
	if (useDefault || ext.size() < 10 || (s[4] == ' ' && s[9] == ' ')) {
		float ratio = _maxWidth * defpercent / 100.0 / (float)wOut;

		wOut = wOut * ratio;
		hOut = hOut * ratio;

		return;
	}

	uint16 w, h;

	(void)readHex(&w, s, 4);
	(void)readHex(&h, &s[5], 4);

	D(9, "w: %d%c h: %d%c", w, s[4], h, s[9]);

	if (s[9] == '%') {
		warning("MacTextCanvas: image height in %% is not supported");
		h = 0;
	}

	float ratio;

	// Percent of the total width
	if (s[4] == '%') {
		ratio = _maxWidth * w / 100.0 / (float)wOut;

	// Size in em (font height) units
	} else if (s[4] == 'm' || s[5] == 'm') {
		int em = _defaultFormatting.fontSize;
		D(9, "em: %d", em);
		if (w != 0 && h != 0) {
			wOut = em * w;
			hOut = em * h;

			return;
		}

		// now we need to compute ratio
		if (w != 0)
			ratio = em * w / (float)wOut;
		else
			ratio = em * h / (float)hOut;

	// Size in pixels
	} else if (s[4] == 'p' || s[5] == 'p') {
		if (w != 0 && h != 0) {
			wOut = w;
			hOut = h;

			return;
		}

		// now we need to compute ratio
		if (w != 0)
			ratio = w / (float)wOut;
		else
			ratio = h / (float)hOut;
	} else {
		error("MacTextCanvas: malformed image extension '%s", ext.encode().c_str());
	}

	D(9, "ratio is %f", ratio);

	wOut = wOut * ratio;
	hOut = hOut * ratio;
}

int MacTextCanvas::getLineWidth(int lineNum, bool enforce, int col) {
	if ((uint)lineNum >= _text.size())
		return 0;

	MacTextLine *line = &_text[lineNum];

	if (line->width != -1 && !enforce && col == -1)
		return line->width;

	if (!line->picfname.empty()) {
		const Surface *image = _macText->getImageSurface(line->picfname);

		if (image) {
			line->width = _maxWidth;

			uint16 w = image->w, h = image->h;

			parsePicExt(line->picext, w, h, line->picpercent);
			line->charwidth = w;
			line->height = h;
		} else {
			line->width = _maxWidth;
			line->height = 1;
			line->charwidth = 1;
		}

		return line->width;
	}

	if (line->table) {
		line->width = _maxWidth;
		line->height = line->tableSurface->h;
		line->charwidth = _maxWidth;

		return line->width;
	}

	int width = line->indent + line->firstLineIndent;
	int height = 0;
	int charwidth = 0;
	int minWidth = 0;
	bool firstWord = true;

	for (uint i = 0; i < line->chunks.size(); i++) {
		if (enforce && _macFontMode)
			line->chunks[i].font = nullptr;

		if (col >= 0) {
			if (col >= (int)line->chunks[i].text.size()) {
				col -= line->chunks[i].text.size();
			} else {
				Common::U32String tmp = line->chunks[i].text.substr(0, col);

				width += getStringWidth(line->chunks[i], tmp);

				return width;
			}
		}

		if (!line->chunks[i].text.empty()) {
			int w = getStringWidth(line->chunks[i], line->chunks[i].text);
			int mW = getStringMaxWordWidth(line->chunks[i], line->chunks[i].text);

			if (firstWord) {
				minWidth = mW + width; // Take indent into account
				firstWord = false;
			} else {
				minWidth = MAX(minWidth, mW);
			}
			width += w;
			charwidth += line->chunks[i].text.size();
		}

		height = MAX(height, line->chunks[i].getFont()->getFontHeight());
	}


	line->width = width;
	line->minWidth = minWidth;
	line->height = height;
	line->charwidth = charwidth;

	return width;
}

int MacTextCanvas::getLineCharWidth(int line, bool enforce) {
	if ((uint)line >= _text.size())
		return 0;

	if (_text[line].charwidth != -1 && !enforce)
		return _text[line].charwidth;

	int width = 0;

	for (uint i = 0; i < _text[line].chunks.size(); i++) {
		if (!_text[line].chunks[i].text.empty())
			width += _text[line].chunks[i].text.size();
	}

	_text[line].charwidth = width;

	return width;
}

int MacTextCanvas::getLineHeight(int line) {
	if ((uint)line >= _text.size())
		return 0;

	(void)getLineWidth(line); // This calculates height also

	return _text[line].height;
}

void MacTextCanvas::recalcDims() {
	if (_text.empty())
		return;

	int y = 0;
	_textMaxWidth = 0;

	for (uint i = 0; i < _text.size(); i++) {
		_text[i].y = y;

		// We must calculate width first, because it enforces
		// the computation. Calling Height() will return cached value!
		_textMaxWidth = MAX(_textMaxWidth, getLineWidth(i, true));
		y += MAX(getLineHeight(i), _interLinear);
	}

	_textMaxHeight = y;
}

int MacTextCanvas::getAlignOffset(int row) {
	int alignOffset = 0;
	if (_textAlignment == kTextAlignRight)
		alignOffset = MAX<int>(0, _maxWidth - getLineWidth(row) - 1);
	else if (_textAlignment == kTextAlignCenter)
		alignOffset = (_maxWidth / 2) - (getLineWidth(row) / 2);
	return alignOffset;
}

// If adjacent chunks have same format, then skip the format definition
// This happens when a long paragraph is split into several lines
#define ADDFORMATTING()                                                                      \
	if (formatted) {                                                                         \
		formatting = Common::U32String(_text[i].chunks[chunk].toString()); \
		if (formatting != prevformatting) {                                                  \
			res += formatting;                                                               \
			prevformatting = formatting;                                                     \
		}                                                                                    \
	}

Common::U32String MacTextCanvas::getTextChunk(int startRow, int startCol, int endRow, int endCol, bool formatted, bool newlines) {
	Common::U32String res("");

	if (endRow == -1)
		endRow = _text.size() - 1;

	if (endCol == -1)
		endCol = getLineCharWidth(endRow);
	if (_text.empty()) {
		return res;
	}

	startRow = CLIP(startRow, 0, (int)_text.size() - 1);
	endRow = CLIP(endRow, 0, (int)_text.size() - 1);

	Common::U32String formatting(""), prevformatting("");

	for (int i = startRow; i <= endRow; i++) {
		// We requested only part of one line
		if (i == startRow && i == endRow) {
			for (uint chunk = 0; chunk < _text[i].chunks.size(); chunk++) {
				if (_text[i].chunks[chunk].text.empty()) {
					// skip empty chunks, but keep them formatted,
					// a text input box needs to keep the formatting even when all text is removed.
					ADDFORMATTING();
					continue;
				}

				if (startCol <= 0) {
					ADDFORMATTING();

					if (endCol >= (int)_text[i].chunks[chunk].text.size())
						res += _text[i].chunks[chunk].text;
					else
						res += _text[i].chunks[chunk].text.substr(0, endCol);
				} else if ((int)_text[i].chunks[chunk].text.size() > startCol) {
					ADDFORMATTING();
					res += _text[i].chunks[chunk].text.substr(startCol, endCol - startCol);
				}

				startCol -= _text[i].chunks[chunk].text.size();
				endCol -= _text[i].chunks[chunk].text.size();

				if (endCol <= 0)
					break;
			}
		// We are at the top line and it is not completely requested
		} else if (i == startRow && startCol != 0) {
			for (uint chunk = 0; chunk < _text[i].chunks.size(); chunk++) {
				if (_text[i].chunks[chunk].text.empty()) // skip empty chunks
					continue;

				if (startCol <= 0) {
					ADDFORMATTING();
					res += _text[i].chunks[chunk].text;
				} else if ((int)_text[i].chunks[chunk].text.size() > startCol) {
					ADDFORMATTING();
					res += _text[i].chunks[chunk].text.substr(startCol);
				}

				startCol -= _text[i].chunks[chunk].text.size();
			}
			if (newlines && _text[i].paragraphEnd)
				res += '\n';
		// We are at the end row, and it could be not completely requested
		} else if (i == endRow) {
			for (uint chunk = 0; chunk < _text[i].chunks.size(); chunk++) {
				if (_text[i].chunks[chunk].text.empty()) // skip empty chunks
					continue;

				ADDFORMATTING();

				if (endCol >= (int)_text[i].chunks[chunk].text.size())
					res += _text[i].chunks[chunk].text;
				else
					res += _text[i].chunks[chunk].text.substr(0, endCol);

				endCol -= _text[i].chunks[chunk].text.size();

				if (endCol <= 0)
					break;
			}
		// We are in the middle of requested range, pass whole line
		} else {
			for (uint chunk = 0; chunk < _text[i].chunks.size(); chunk++) {
				if (_text[i].chunks[chunk].text.empty()) // skip empty chunks
					continue;

				ADDFORMATTING();
				res += _text[i].chunks[chunk].text;
			}

			if (newlines && _text[i].paragraphEnd)
				res += '\n';
		}
	}

	return res;
}

void MacTextCanvas::reshuffleParagraph(int *row, int *col, MacFontRun &defaultFormatting) {
	_defaultFormatting = defaultFormatting;

	// First, we looking for the paragraph start and end
	int start = *row, end = *row;

	// Since one previous line could be affected, compute it
	if (start && !_text[start - 1].paragraphEnd)
		start--;

	// Find end of the paragraph
	while (end < (int)_text.size() - 1 && !_text[end].paragraphEnd)
		end++;

	// Get character pos within paragraph
	int ppos = 0;

	for (int i = start; i < *row; i++)
		ppos += getLineCharWidth(i);

	ppos += *col;

	bool paragraphEnd = _text[end].paragraphEnd;

#if DEBUG
	D(9, "MacTextCanvas::reshuffleParagraph: ppos: %d", ppos);
	debugPrint("MacTextCanvas::reshuffleParagraph(1)");
#endif

	// Assemble all chunks to chop, combining the matching ones
	Common::Array<MacFontRun> chunks;

	for (int i = start; i <= end; i++) {
		for (auto &ch : _text[i].chunks) {
			if (!chunks.size()) {
				chunks.push_back(ch);
			} else {
				if (chunks.back().equals(ch))
					chunks.back().text += ch.text;
				else
					chunks.push_back(ch);
			}
		}

		if (i != end && !_text[i].wordContinuation)
			chunks.back().text += ' ';
	}

#if DEBUG
	D(9, "Chunks: ");
	for (auto &ch : chunks)
		ch.debugPrint();

	D(9, "");
#endif

	int curLine = start;
	int indent = _text[curLine].indent;
	int firstLineIndent = _text[curLine].firstLineIndent;

	// Remove paragraph from the text
	for (int i = start; i <= end; i++) {
		_text.remove_at(start);
	}

#if DEBUG
	debugPrint("MacTextCanvas::reshuffleParagraph(2)");
#endif

	// And now read it
	D(9, "start %d end %d", start, end);

	_text.insert_at(curLine, MacTextLine());
	_text[curLine].indent = indent;
	_text[curLine].firstLineIndent = firstLineIndent;

	for (auto &ch : chunks) {
		_text[curLine].chunks.push_back(ch);
		_text[curLine].chunks.back().text.clear(); // We wil add it later
		chopChunk(ch.text, &curLine, indent, _maxWidth);
	}

#if DEBUG
	debugPrint("MacTextCanvas::reshuffleParagraph(3)");
#endif

	// Restore the paragraph marker
	_text[curLine].paragraphEnd = paragraphEnd;

	// Find new pos within paragraph after reshuffling
	*row = start;

	while (ppos > getLineCharWidth(*row, true)) {
		ppos -= getLineCharWidth(*row, true);

		if (*row == (int)_text.size() - 1)
			break;

		(*row)++;
	}
	*col = ppos;
}

void MacTextCanvas::setMaxWidth(int maxWidth, MacFontRun &defaultFormatting) {
	if (maxWidth == _maxWidth)
		return;

	if (maxWidth < 0) {
		warning("MacTextCanvas::setMaxWidth(): trying to set maxWidth to %d", maxWidth);
		return;
	}

	_defaultFormatting = defaultFormatting;

	_maxWidth = maxWidth;

	int row, col = 0;

	for (uint i = 0; i < _text.size(); i++) {
		row = i;

		if (_text[i].table) {
			processTable(i, maxWidth);
			continue;
		}

		reshuffleParagraph(&row, &col, _defaultFormatting);

		while (i < _text.size() - 1 && !_text[i].paragraphEnd)
			i++;
	}
}

void MacTextCanvas::processTable(int line, int maxWidth) {
	Common::Array<MacTextTableRow> *table = _text[line].table;
	uint numCols = table->front().cells.size();
	uint numRows = table->size();
	Common::Array<int> maxW(numCols), maxL(numCols), colW(numCols), rowH(numRows);
	Common::Array<bool> flex(numCols), wrap(numCols);

	int width = maxWidth * 0.9;
	int gutter = 10;

	// Compute column widths, both minimal and maximal
	for (auto &row : *table) {
		int i = 0;
		for (auto &cell : row.cells) {
			int cW = 0, cL = 0;
			for (uint l = 0; l < cell._text.size(); l++) {
				(void)cell.getLineWidth(l); // calculate it

				cW = MAX(cW, cell._text[l].width);
				cL = MAX(cL, cell._text[l].minWidth);
			}

			maxW[i] = MAX(maxW[i], cW);
			maxL[i] = MAX(maxL[i], cL);

			i++;
		}
	}

	for (uint i = 0; i < numCols; i++) {
		D(8, "cell #%d: width range: %d - %d", i, maxL[i], maxW[i]);

		wrap[i] = (maxW[i] != maxL[i]);
	}

	int left = width - (numCols - 1) * gutter;
	int avg = left / numCols;
	int nflex = 0;

	// determine whether columns should be flexible and assign
	// width of non-flexible cells
	for (uint i = 0; i < numCols; i++) {
		flex[i] = (maxW[i] > 2 * avg);
		if (flex[i]) {
			nflex++;
		} else {
			colW[i] = maxW[i];
			left -= colW[i];
		}
	}

	// if there is not enough space, make columns that could
	// be word-wrapped flexible, too
	if (left < nflex * avg) {
		for (uint i = 0; i < numCols; i++) {
			if (!flex[i] && wrap[i]) {
				left += colW[i];
				colW[i] = 0;
				flex[i] = true;
				nflex += 1;
			}
		}
	}

	// Calculate weights for flexible columns. The max width
	// is capped at the page width to treat columns that have to
	// be wrapped more or less equal
	int tot = 0;
	for (uint i = 0; i < numCols; i++) {
		if (flex[i]) {
			maxW[i] = MIN(maxW[i], width);
			tot += maxW[i];
		}
	}

	// Now assign the actual width for flexible columns. Make
	// sure that it is at least as long as the longest word length
	for (uint i = 0; i < numCols; i++) {
		if (flex[i]) {
			colW[i] = left * maxW[i] / tot;
			colW[i] = MAX(colW[i], maxL[i]);
			left -= colW[i];
		}
	}

	for (uint i = 0; i < numCols; i++) {
		D(8, "Table cell #%d: width: %d", i, colW[i]);
	}

	int r = 0;
	for (auto &row : *table) {
		int c = 0;
		rowH[r] = 0;
		for (auto &cell : row.cells) {
			cell.setMaxWidth(colW[c], _defaultFormatting);

			cell.recalcDims();
			cell.reallocSurface();
			cell._surface->clear(_tbgcolor);
			cell.render(0, cell._text.size());

			rowH[r] = MAX(rowH[r], cell._textMaxHeight);

			c++;
		}

		r++;
	}

	int tW = 1, tH = 1;
	for (uint i = 0; i < table->size(); i++)
		tH += rowH[i] + gutter * 2 + 1;

	for (uint i = 0; i < table->front().cells.size(); i++)
		tW += colW[i] + gutter * 2 + 1;

	ManagedSurface *surf = new ManagedSurface(tW, tH, _wm->_pixelformat);
	_text[line].tableSurface = surf;
	_text[line].height = tH;
	_text[line].width = tW;
	surf->clear(_tbgcolor);

	surf->hLine(0, 0, tW, _tfgcolor);
	surf->vLine(0, 0, tH, _tfgcolor);

	int y = 1;
	for (uint i = 0; i < table->size(); i++) {
		y += gutter * 2 + rowH[i];
		surf->hLine(0, y, tW, _tfgcolor);
		y++;
	}

	int x = 1;
	for (uint i = 0; i < table->front().cells.size(); i++) {
		x += gutter * 2 + colW[i];
		surf->vLine(x, 0, tH, _tfgcolor);
		x++;
	}

	r = 0;
	y = 1 + gutter;
	for (auto &row : *table) {
		int c = 0;
		x = 1 + gutter;
		for (auto &cell : row.cells) {
			surf->blitFrom(*cell._surface, Common::Point(x, y));
			x += gutter * 2 + 1 + colW[c];
			c++;
		}
		y += gutter * 2 + 1 + rowH[r];
		r++;
	}
}

void MacFontRun::debugPrint() {
	DN(8, "{%d}[%d (%d)] \"%s\" ", text.size(), fontId, textSlant, Common::toPrintable(text.encode()).c_str());
}

void MacTextCanvas::debugPrint(const char *prefix) {
	for (uint i = 0; i < _text.size(); i++) {
		if (prefix)
			DN(8, "%s: ", prefix);
		DN(8, "%2d, %c %c fi: %d, i: %d ", i, _text[i].paragraphEnd ? '$' : '.', _text[i].table ? 'T' : ' ',
					_text[i].firstLineIndent, _text[i].indent);

		for (uint j = 0; j < _text[i].chunks.size(); j++)
			_text[i].chunks[j].debugPrint();

		DN(8, "\n");
	}

	if (prefix)
		DN(8, "%s: ", prefix);
	D(8, "[done]");
}

} // End of namespace Graphics
