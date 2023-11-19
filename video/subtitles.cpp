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

#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "common/ustr.h"
#include "common/unicode-bidi.h"

#include "graphics/fonts/ttf.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/surface.h"

#include "video/subtitles.h"
#include "common/config-manager.h"

namespace Video {

SRTParser::SRTParser() {
}

SRTParser::~SRTParser() {
	cleanup();
}

void SRTParser::cleanup() {
	for (Common::Array<SRTEntry *>::const_iterator item = _entries.begin(); item != _entries.end(); ++item)
		delete *item;

	_entries.clear();
}

bool parseTime(const char **pptr, uint32 *res) {
	int hours, mins, secs, msecs;
	const char *ptr = *pptr;

	hours = (*ptr++ - '0') * 10;
	hours += *ptr++ - '0';

	if (hours > 24 || hours < 0)
		return false;

	if (*ptr++ != ':')
		return false;

	mins = (*ptr++ - '0') * 10;
	mins += *ptr++ - '0';

	if (mins > 60 || mins < 0)
		return false;

	if (*ptr++ != ':')
		return false;

	secs = (*ptr++ - '0') * 10;
	secs += *ptr++ - '0';

	if (secs > 60 || secs < 0)
		return false;

	if (*ptr++ != ',')
		return false;

	msecs =  (*ptr++ - '0') * 100;
	msecs += (*ptr++ - '0') * 10;
	msecs +=  *ptr++ - '0';

	if (msecs > 1000 || msecs < 0)
		return false;

	*res = 1000 * (3600 * hours + 60 * mins + secs) + msecs;

	*pptr = ptr;

	return true;
}

int SRTEntryComparator(const void *item1, const void *item2) {
	const SRTEntry *l = *(const SRTEntry * const *)item1;
	const SRTEntry *r = *(const SRTEntry * const *)item2;

	return l->start - r->start;
}

int SRTEntryComparatorBSearch(const void *item1, const void *item2) {
	const SRTEntry *k = *(const SRTEntry * const *)item1;
	const SRTEntry *i = *(const SRTEntry * const *)item2;

	if (k->start < i->start)
		return -1;

	if (k->start > i->end - 1)
		return 1;

	return 0;
}

bool SRTParser::parseFile(const char *fname) {
	Common::File f;

	cleanup();

	if (!f.open(fname)) {
		return false;
	}

	byte buf[3];
	f.read(buf, 3);

	int line = 0;

	// Skip UTF header if present
	if (buf[0] != 0xef || buf[1] != 0xbb || buf[2] != 0xbf)
		f.seek(0);

	while (!f.eos()) {
		Common::String sseq, stimespec, stmp, text;

		sseq = f.readLine(); line++;
		stimespec = f.readLine(); line++;
		text = f.readLine(); line++;

		if (sseq.empty()) {
			if (f.eos()) {
				// Normal end of stream
				break;
			} else {
				warning("Bad SRT file format (spec): %s at line %d", fname, line);
				break;
			}
		}

		if (stimespec.empty() || text.empty()) {
			warning("Bad SRT file format (spec): %s at line %d", fname, line);
			break;
		}

		// Read all multiline text
		while (!f.eos()) {
			stmp = f.readLine(); line++;

			if (!stmp.empty()) {
				text += '\n';
				text += stmp;
			} else {
				break;
			}
		}

		uint32 seq = atol(sseq.c_str());
		if (seq == 0) {
			warning("Bad SRT file format (seq): %s at line %d", fname, line);
			break;
		}

		// 00:20:41,150 --> 00:20:45,109
		if (stimespec.size() < 29) {
			warning("Bad SRT file format (timespec length %d): %s at line %d", stimespec.size(), fname, line);
			break;
		}

		const char *ptr = stimespec.c_str();
		uint32 start, end;
		if (!parseTime(&ptr, &start)) {
			warning("Bad SRT file format (timespec start): %s at line %d", fname, line);
			break;
		}

		while (*ptr == ' ')
			ptr++;

		while (*ptr == '-')
			ptr++;

		if (*ptr != '>') {
			warning("Bad SRT file format (timespec middle ('%c')): %s at line %d", *ptr, fname, line);
			break;
		}

		ptr++;

		while (*ptr == ' ')
			ptr++;

		if (!parseTime(&ptr, &end)) {
			warning("Bad SRT file format (timespec end): %s at line %d", fname, line);
			break;
		}

		_entries.push_back(new SRTEntry(seq, start, end, text));
	}

	qsort(_entries.data(), _entries.size(), sizeof(SRTEntry *), &SRTEntryComparator);

	debug(6, "SRTParser: Loaded %d entries", _entries.size());

	return true;
}

Common::String SRTParser::getSubtitle(uint32 timestamp) const {
	SRTEntry test(0, timestamp, 0, "");
	SRTEntry *testptr = &test;

	const SRTEntry **entry = (const SRTEntry **)bsearch(&testptr, _entries.data(), _entries.size(), sizeof(SRTEntry *), &SRTEntryComparatorBSearch);

	if (entry == NULL)
		return "";

	return (*entry)->text;
}

#define SHADOW 1

Subtitles::Subtitles() : _loaded(false), _font(nullptr), _hPad(0), _vPad(0), _overlayHasAlpha(true),
	_lastOverlayWidth(-1), _lastOverlayHeight(-1) {
	_surface = new Graphics::Surface();
	_subtitleDev = ConfMan.getBool("subtitle_dev");
}

Subtitles::~Subtitles() {
	delete _surface;
}

void Subtitles::setFont(const char *fontname, int height) {
	Common::File file;

	_fontHeight = height;

#ifdef USE_FREETYPE2
	if (file.open(fontname)) {
		_font = Graphics::loadTTFFont(file, _fontHeight, Graphics::kTTFSizeModeCharacter, 96);
	}

	if (!_font) {
		_font = Graphics::loadTTFFontFromArchive(fontname, _fontHeight, Graphics::kTTFSizeModeCharacter, 96);
	}
#endif

	if (!_font) {
		debug(1, "Cannot load font %s directly", fontname);
		_font = FontMan.getFontByName(fontname);
	}

	if (!_font) {
		warning("Cannot load font %s", fontname);

		_font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	}

}

void Subtitles::loadSRTFile(const char *fname) {
	debug(1, "loadSRTFile('%s')", fname);

	if (_subtitleDev) {
		_fname = fname;
	}
	_loaded = _srtParser.parseFile(fname);
}

void Subtitles::setBBox(const Common::Rect bbox) {
	_requestedBBox = bbox;

	Graphics::PixelFormat overlayFormat = g_system->getOverlayFormat();
	_overlayHasAlpha = overlayFormat.aBits() != 0;
	_surface->create(_requestedBBox.width() + SHADOW * 2, _requestedBBox.height() + SHADOW * 2, overlayFormat);
	// Force recalculation of real bounding box
	_lastOverlayWidth = -1;
	_lastOverlayHeight = -1;
}

void Subtitles::setColor(byte r, byte g, byte b) {
	_color = _surface->format.ARGBToColor(255, r, g, b);
	_blackColor = _surface->format.ARGBToColor(255, 0, 0, 0);
	_transparentColor = _surface->format.ARGBToColor(0, 0, 0, 0);
}

void Subtitles::setPadding(uint16 horizontal, uint16 vertical) {
	_hPad = horizontal;
	_vPad = vertical;
}

bool Subtitles::drawSubtitle(uint32 timestamp, bool force) const {
	Common::String subtitle;
	if (_loaded) {
		subtitle = _srtParser.getSubtitle(timestamp);
	} else if (_subtitleDev) {
		subtitle = _fname;
		uint32 hours, mins, secs, msecs;
		secs = timestamp / 1000;
		hours = secs / 3600;
		mins = (secs / 60) % 60;
		secs %= 60;
		msecs = timestamp % 1000;
		subtitle += " " + Common::String::format("%02u:%02u:%02u,%03u", hours, mins, secs, msecs);
	} else {
		return false;
	}

	int16 width = g_system->getOverlayWidth(),
		  height = g_system->getOverlayHeight();

	if (width != _lastOverlayWidth ||
		height != _lastOverlayHeight) {
		_lastOverlayWidth = width;
		_lastOverlayHeight = height;

		// Recalculate the real bounding box to use
		_realBBox = _requestedBBox;

		if (_realBBox.bottom > height) {
			// First try to move the bounding box
			_realBBox.top -= _realBBox.bottom - height;
			_realBBox.bottom = height;
		}
		if (_realBBox.top < 0) {
			// Not enough space
			_realBBox.top = 0;
		}

		if (_realBBox.right > width) {
			// First try to move the bounding box
			_realBBox.left -= _realBBox.right - width;
			_realBBox.right = width;
		}
		if (_realBBox.left < 0) {
			// Not enough space
			_realBBox.left = 0;
		}

		force = true;
	}

	if (!force && _overlayHasAlpha && subtitle == _subtitle)
		return false;

	if (force || subtitle != _subtitle) {
		debug(1, "%d: %s", timestamp, subtitle.c_str());

		_subtitle = subtitle;
		renderSubtitle();
	}

	if (_overlayHasAlpha) {
		// When we have alpha, draw the whole surface without thinking it more
		g_system->copyRectToOverlay(_surface->getPixels(), _surface->pitch, _realBBox.left, _realBBox.top, _realBBox.width(), _realBBox.height());
	} else {
		// When overlay doesn't have alpha, showing it hides the underlying game screen
		// We force a copy of the game screen to the overlay by clearing it
		// We then draw the smallest possible surface to minimize black rectangle behind text
		g_system->clearOverlay();
		g_system->copyRectToOverlay((byte *)_surface->getPixels() + _drawRect.top * _surface->pitch + _drawRect.left * _surface->format.bytesPerPixel, _surface->pitch,
				_realBBox.left + _drawRect.left, _realBBox.top + _drawRect.top, _drawRect.width(), _drawRect.height());
	}

	return true;
}

void Subtitles::renderSubtitle() const {
	_surface->fillRect(Common::Rect(0, 0, _surface->w, _surface->h), _transparentColor);

	Common::Array<Common::U32String> lines;

	_font->wordWrapText(convertUtf8ToUtf32(_subtitle), _realBBox.width(), lines);

	if (lines.empty()) {
		_drawRect.left = 0;
		_drawRect.top = 0;
		_drawRect.right = 0;
		_drawRect.bottom = 0;

		return;
	}

	int height = _vPad;

	int width = 0;
	for (uint i = 0; i < lines.size(); i++)
		width = MAX(_font->getStringWidth(lines[i]), width);
	width = MIN(width + 2 * _hPad, (int)_realBBox.width());

	int originX = (_realBBox.width() - width) / 2;

	for (uint i = 0; i < lines.size(); i++) {
		Common::U32String line = convertBiDiU32String(lines[i]).visual;

		_font->drawString(_surface, line, originX, height, width, _blackColor, Graphics::kTextAlignCenter);
		_font->drawString(_surface, line, originX + SHADOW * 2, height, width, _blackColor, Graphics::kTextAlignCenter);
		_font->drawString(_surface, line, originX, height + SHADOW * 2, width, _blackColor, Graphics::kTextAlignCenter);
		_font->drawString(_surface, line, originX + SHADOW * 2, height + SHADOW * 2, width, _blackColor, Graphics::kTextAlignCenter);

		_font->drawString(_surface, line, originX + SHADOW, height + SHADOW, width, _color, Graphics::kTextAlignCenter);

		height += _font->getFontHeight();

		if (height + _vPad > _realBBox.bottom)
			break;
	}

	height += _vPad;

	_drawRect.left = originX;
	_drawRect.top = 0;
	_drawRect.setWidth(width + SHADOW * 2);
	_drawRect.setHeight(height + SHADOW * 2);
}

} // End of namespace Video
