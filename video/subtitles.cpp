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
	for (const auto *item : _entries)
		delete item;

	_entries.clear();
}

void SRTParser::parseTextAndTags(const Common::String &text, Common::Array<SubtitlePart> &parts) const {
	Common::String currentText = text;

	while (true) {
		Common::String::size_type pos_i_start = currentText.find("<i>");
		Common::String::size_type pos_sfx_start = currentText.find("<sfx>");

		Common::String::size_type first_tag_start = Common::String::npos;
		Common::String start_tag, end_tag;
		Common::String tag;

		if (pos_i_start != Common::String::npos && (pos_sfx_start == Common::String::npos || pos_i_start < pos_sfx_start)) {
			first_tag_start = pos_i_start;
			start_tag = "<i>";
			end_tag = "</i>";
			tag = "i";
		} else if (pos_sfx_start != Common::String::npos) {
			first_tag_start = pos_sfx_start;
			start_tag = "<sfx>";
			end_tag = "</sfx>";
			tag = "sfx";
		}

		if (first_tag_start == Common::String::npos) {
			if (!currentText.empty()) {
				parts.push_back(SubtitlePart(currentText, ""));
			}
			break;
		}

		if (first_tag_start > 0) {
			parts.push_back(SubtitlePart(currentText.substr(0, first_tag_start), ""));
		}

		Common::String::size_type end_tag_pos = currentText.find(end_tag, first_tag_start);
		if (end_tag_pos == Common::String::npos) {
			parts.push_back(SubtitlePart(currentText.substr(first_tag_start + start_tag.size()), tag));
			break;
		}

		parts.push_back(SubtitlePart(currentText.substr(first_tag_start + start_tag.size(), end_tag_pos - (first_tag_start + start_tag.size())), tag));

		currentText = currentText.substr(end_tag_pos + end_tag.size());
	}
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

bool SRTParser::parseFile(const Common::Path &fname) {
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
				warning("Bad SRT file format (spec): %s at line %d", fname.toString().c_str(), line);
				break;
			}
		}

		if (stimespec.empty() || text.empty()) {
			warning("Bad SRT file format (spec): %s at line %d", fname.toString().c_str(), line);
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
			warning("Bad SRT file format (seq): %s at line %d", fname.toString().c_str(), line);
			break;
		}

		// 00:20:41,150 --> 00:20:45,109
		if (stimespec.size() < 29) {
			warning("Bad SRT file format (timespec length %d): %s at line %d", stimespec.size(), fname.toString().c_str(), line);
			break;
		}

		const char *ptr = stimespec.c_str();
		uint32 start, end;
		if (!parseTime(&ptr, &start)) {
			warning("Bad SRT file format (timespec start): %s at line %d", fname.toString().c_str(), line);
			break;
		}

		while (*ptr == ' ')
			ptr++;

		while (*ptr == '-')
			ptr++;

		if (*ptr != '>') {
			warning("Bad SRT file format (timespec middle ('%c')): %s at line %d", *ptr, fname.toString().c_str(), line);
			break;
		}

		ptr++;

		while (*ptr == ' ')
			ptr++;

		if (!parseTime(&ptr, &end)) {
			warning("Bad SRT file format (timespec end): %s at line %d", fname.toString().c_str(), line);
			break;
		}

		SRTEntry *entry = new SRTEntry(seq, start, end);
		parseTextAndTags(text, entry->parts);
		_entries.push_back(entry);
	}

	qsort(_entries.data(), _entries.size(), sizeof(SRTEntry *), &SRTEntryComparator);

	debug(6, "SRTParser: Loaded %d entries", _entries.size());

	return true;
}

const Common::Array<SubtitlePart> *SRTParser::getSubtitleParts(uint32 timestamp) const {
	SRTEntry test(0, timestamp, 0, "");
	SRTEntry *testptr = &test;

	const SRTEntry **entry = (const SRTEntry **)bsearch(&testptr, _entries.data(), _entries.size(), sizeof(SRTEntry *), &SRTEntryComparatorBSearch);

	if (entry == NULL)
		return nullptr;

	return &(*entry)->parts;
}

#define SHADOW 1

Subtitles::Subtitles() : _loaded(false), _hPad(0), _vPad(0), _overlayHasAlpha(true),
	_lastOverlayWidth(-1), _lastOverlayHeight(-1) {
	_subtitleDev = ConfMan.getBool("subtitle_dev");
}

Subtitles::~Subtitles() {
	close();
	_surface.free();

	for (const auto &font : _fonts) {
		FontMan.mayDeleteFont(font._value);
	}
}

void Subtitles::setFont(const char *fontname, int height, FontStyle type) {
	_fontHeight = height;

	if (_fonts[type]) {
		FontMan.mayDeleteFont(_fonts[type]);
		_fonts[type] = nullptr;
	}

#ifdef USE_FREETYPE2
	Graphics::Font *font = nullptr;
	Common::File *file = new Common::File();
	if (file->open(fontname)) {
		font = Graphics::loadTTFFont(file, DisposeAfterUse::YES, _fontHeight, Graphics::kTTFSizeModeCharacter, 96);
		if (!font)
			delete file;
		else
			_fonts[type] = font;
	} else {
		delete file;
	}

	if (!font) {
		_fonts[type] = Graphics::loadTTFFontFromArchive(fontname, _fontHeight, Graphics::kTTFSizeModeCharacter, 96);
	}
#endif

	if (!_fonts[type]) {
		debug(1, "Cannot load font %s directly", fontname);
		_fonts[type] = FontMan.getFontByName(fontname);
	}

	if (!_fonts[type]) {
		warning("Cannot load font %s", fontname);

		_fonts[type] = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	}
}

void Subtitles::loadSRTFile(const Common::Path &fname) {
	debug(1, "loadSRTFile('%s')", fname.toString().c_str());

	if (_subtitleDev)
		_fname = fname;

	_srtParser = new SRTParser();
	_loaded = _srtParser->parseFile(fname);
}

void Subtitles::close() {
	_loaded = false;
	_parts = nullptr;
	_fname.clear();
	delete _srtParser;
	_srtParser = nullptr;
}

void Subtitles::setBBox(const Common::Rect &bbox) {
	_requestedBBox = bbox;

	Graphics::PixelFormat overlayFormat = g_system->getOverlayFormat();
	_overlayHasAlpha = overlayFormat.aBits() != 0;
	_surface.create(_requestedBBox.width() + SHADOW * 2, _requestedBBox.height() + SHADOW * 2, overlayFormat);
	// Force recalculation of real bounding box
	_lastOverlayWidth = -1;
	_lastOverlayHeight = -1;
}

void Subtitles::setColor(byte r, byte g, byte b) {
	_color = _surface.format.ARGBToColor(255, r, g, b);
	_blackColor = _surface.format.ARGBToColor(255, 0, 0, 0);
	_transparentColor = _surface.format.ARGBToColor(0, 0, 0, 0);
}

void Subtitles::setPadding(uint16 horizontal, uint16 vertical) {
	_hPad = horizontal;
	_vPad = vertical;
}

bool Subtitles::recalculateBoundingBox() const {
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

		return true;
	}

	return false;
}

bool Subtitles::drawSubtitle(uint32 timestamp, bool force, bool showSFX) const {
	const Common::Array<SubtitlePart> *parts;
	bool isSFX = false;
	if (_loaded && _srtParser) {
		parts = _srtParser->getSubtitleParts(timestamp);
		if (parts && !parts->empty()) {
			isSFX = (*parts)[0].tag == "sfx";
		}
	} else if (_subtitleDev) {
		// Force refresh
		_parts = nullptr;

		Common::String subtitle = _fname.toString('/');
		uint32 hours, mins, secs, msecs;
		secs = timestamp / 1000;
		hours = secs / 3600;
		mins = (secs / 60) % 60;
		secs %= 60;
		msecs = timestamp % 1000;
		subtitle += " " + Common::String::format("%02u:%02u:%02u,%03u", hours, mins, secs, msecs);

		if (_devParts.empty()) {
			_devParts.push_back(SubtitlePart("", ""));
		}
		_devParts[0].text = subtitle;

		parts = &_devParts;
	} else {
		return false;
	}

	force |= recalculateBoundingBox();

	if (!force && _overlayHasAlpha && parts == _parts)
		return false;

	if (force || parts != _parts) {
		if (debugLevelSet(1)) {
			Common::String subtitle;
			if (parts) {
				for (const auto &part : *parts) {
					subtitle += part.text;
				}
			}
			debug(1, "%d: %s", timestamp, subtitle.c_str());
		}

		_parts = parts;

		if (!isSFX || showSFX)
			renderSubtitle();
	}

	updateSubtitleOverlay();

	return true;
}

void Subtitles::clearSubtitle() const {
	if (!_loaded)
		return;

	g_system->hideOverlay();
	_drawRect.setEmpty();
	_surface.fillRect(Common::Rect(0, 0, _surface.w, _surface.h), _transparentColor);
}

void Subtitles::updateSubtitleOverlay() const {
	if (!_loaded)
		return;

	if (!shouldShowSubtitle()) {
		g_system->hideOverlay();
		return;
	}

	if (!g_system->isOverlayVisible()) {
		g_system->clearOverlay();
		g_system->showOverlay(false);
	}

	if (_overlayHasAlpha) {
		// When we have alpha, draw the whole surface without thinking it more
		g_system->copyRectToOverlay(_surface.getPixels(), _surface.pitch, _realBBox.left, _realBBox.top, _realBBox.width(), _realBBox.height());
	} else {
		// When overlay doesn't have alpha, showing it hides the underlying game screen
		// We force a copy of the game screen to the overlay by clearing it
		// We then draw the smallest possible surface to minimize black rectangle behind text
		g_system->clearOverlay();
		g_system->copyRectToOverlay((byte *)_surface.getPixels() + _drawRect.top * _surface.pitch + _drawRect.left * _surface.format.bytesPerPixel, _surface.pitch,
									_realBBox.left + _drawRect.left, _realBBox.top + _drawRect.top, _drawRect.width(), _drawRect.height());
	}
}

struct SubtitleRenderingPart {
	Common::U32String text;
	const Graphics::Font *font;
	bool newLine;
	int left;
	int right;
};

void Subtitles::renderSubtitle() const {
	_surface.fillRect(Common::Rect(0, 0, _surface.w, _surface.h), _transparentColor);

	if (!_parts || _parts->empty()) {
		_drawRect.setEmpty();
		return;
	}

	Common::Array<SubtitleRenderingPart> splitParts;

	// First, calculate all positions as if we were left aligned
	bool newLine = true;
	int currentX = 0;
	for (const auto &part : *_parts) {
		const Graphics::Font *font = _fonts[part.tag == "i" ? kFontStyleItalic : kFontStyleRegular];
		if (!font) font = _fonts[kFontStyleRegular];

		Common::Array<Common::U32String> lines;

		font->wordWrapText(part.text.decode(Common::kUtf8), _realBBox.width(), lines, currentX);

		if (lines.empty()) {
			continue;
		}
		splitParts.reserve(splitParts.size() + lines.size());

		int width = 0;
		for (auto line = lines.begin(); line != lines.end() - 1; line++) {
			width = font->getStringWidth(*line);
			splitParts.emplace_back(SubtitleRenderingPart{*line, font, newLine, currentX, currentX + width});
			newLine = true;
			currentX = 0;
		}
		width = font->getStringWidth(lines.back());
		splitParts.emplace_back(SubtitleRenderingPart{lines.back(), font, newLine, currentX, currentX + width});
		newLine = false;
		currentX += width;

		// Last newline doesn't trigger an empty line in wordWrapText
		if (part.text.hasSuffix("\n")) {
			newLine = true;
			currentX = 0;
		}
	}

	_splitPartCount = (uint16)splitParts.size();

	// Then, center all lines and calculate the drawing box
	auto lineBegin = splitParts.begin();
	int minX = _realBBox.width();
	int maxWidth = 0;

	for (auto splitPart = splitParts.begin() + 1; splitPart != splitParts.end(); splitPart++) {
		if (!splitPart->newLine) {
			continue;
		}
		int width = MIN(splitPart[-1].right + 2 * _hPad, (int)_realBBox.width());
		int origin = (_realBBox.width() - width) / 2;
		minX = MIN(minX, origin);
		maxWidth = MAX(maxWidth, width);

		for(auto part = lineBegin; part != splitPart; part++) {
			part->left += origin;
			part->right += origin;
		}

		lineBegin = splitPart;
	}
	if (lineBegin != splitParts.end()) {
		int width = MIN(splitParts.back().right + 2 * _hPad, (int)_realBBox.width());
		int origin = (_realBBox.width() - width) / 2;
		minX = MIN(minX, origin);
		maxWidth = MAX(maxWidth, width);

		for(auto part = lineBegin; part != splitParts.end(); part++) {
			part->left += origin;
			part->right += origin;
		}
	}

	// Finally, render every part on the surface
	int currentY = _vPad;
	int lineHeight = 0;
	for (const auto &part : splitParts) {
		const Graphics::Font *font = part.font;
		int partWidth = part.right - part.left;

		if (part.newLine) {
			currentY += lineHeight;
			if (currentY + _vPad > _realBBox.bottom) {
				lineHeight = 0;
				break;
			}

			lineHeight = font->getFontHeight();
		}

		Common::U32String u32_text = convertBiDiU32String(part.text).visual;

		font->drawString(&_surface, u32_text, part.left, currentY, partWidth, _blackColor, Graphics::kTextAlignLeft);
		font->drawString(&_surface, u32_text, part.left + SHADOW * 2, currentY, partWidth, _blackColor, Graphics::kTextAlignLeft);
		font->drawString(&_surface, u32_text, part.left, currentY + SHADOW * 2, partWidth, _blackColor, Graphics::kTextAlignLeft);
		font->drawString(&_surface, u32_text, part.left + SHADOW * 2, currentY + SHADOW * 2, partWidth, _blackColor, Graphics::kTextAlignLeft);
		font->drawString(&_surface, u32_text, part.left + SHADOW, currentY + SHADOW, partWidth, _color, Graphics::kTextAlignLeft);

	}

	currentY += lineHeight + _vPad;

	_drawRect.left = minX;
	_drawRect.top = 0;
	_drawRect.setWidth(maxWidth + SHADOW * 2);
	_drawRect.setHeight(currentY + SHADOW * 2);
	_drawRect.clip(_realBBox.width(), _realBBox.height());
}

} // End of namespace Video
