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

#include "common/algorithm.h"
#include "common/system.h"
#include "graphics/font.h"
#include "graphics/screen.h"
#include "graphics/sjis.h"
#include "graphics/surface.h"
#include "harold/bcb/bcb.h"
#include "harold/bcb/script.h"
#include "harold/bcb/static_data.h"
#include "video/avi_decoder.h"

namespace BCB {

static const StaticRect kTitleCaptionTextRect = {120, 312, 520, 352};
static const StaticRect kTitleCaptionBackingRect = {120, 312, 520, 362};
static const StaticRect kMainSceneCaptionTextRect = {48, 260, 384, 300};
static const StaticRect kMainSceneCaptionShadowRect = {50, 262, 384, 300};
static const StaticRect kMapSubmodeCaptionTextRect = {152, 268, 488, 308};

static uint32 readUint32BE(const byte *data) {
	return ((uint32)data[0] << 24) | ((uint32)data[1] << 16) | ((uint32)data[2] << 8) | data[3];
}

const byte *BCBEngine::findAllBinRecordData(const Common::String &recordName, uint32 &recordSize) const {
	recordSize = 0;
	if (_allBinData.size() < 4)
		return nullptr;

	const uint32 entryCount = readUint32BE(&_allBinData[0]);
	for (uint32 entryIndex = 0; entryIndex < entryCount; ++entryIndex) {
		const uint entryOffset = 4 + entryIndex * 0x28;
		if (entryOffset + 0x28 > _allBinData.size())
			return nullptr;
		if (readUint32BE(&_allBinData[entryOffset]) == 0xffffffff)
			return nullptr;

		uint nameLength = 0;
		while (nameLength < 0x20 && _allBinData[entryOffset + 8 + nameLength] != 0)
			++nameLength;
		const Common::String entryName(reinterpret_cast<const char *>(&_allBinData[entryOffset + 8]), nameLength);
		if (!recordName.equals(entryName))
			continue;

		const uint32 dataOffset = readUint32BE(&_allBinData[entryOffset]);
		const uint32 dataSize = readUint32BE(&_allBinData[entryOffset + 4]);
		if (dataOffset + dataSize > _allBinData.size())
			return nullptr;

		recordSize = dataSize;
		return &_allBinData[dataOffset];
	}

	return nullptr;
}

void BCBEngine::loadTimedCaptionRecord(int movieId) {
	_timedCaptionRecordData.clear();
	_timedCaptionCursorOffset = 0;

	uint32 recordSize = 0;
	const Common::String recordName = Common::String::format("MES%04d.TXT", movieId);
	const byte *recordData = findAllBinRecordData(recordName, recordSize);
	if (recordData && recordSize) {
		_timedCaptionRecordData.resize(recordSize);
		Common::copy(recordData, recordData + recordSize, _timedCaptionRecordData.begin());
	}

	loadNextTimedCaptionText();
}

bool BCBEngine::readTimedCaptionLine(Common::String &line) {
	line.clear();
	if (_timedCaptionCursorOffset >= _timedCaptionRecordData.size())
		return false;

	int copiedCharCount = 0;
	while (_timedCaptionCursorOffset + 1 < _timedCaptionRecordData.size()) {
		if (_timedCaptionRecordData[_timedCaptionCursorOffset] == '\r' && _timedCaptionRecordData[_timedCaptionCursorOffset + 1] == '\n') {
			_timedCaptionCursorOffset += 2;
			return true;
		}

		line += (char)_timedCaptionRecordData[_timedCaptionCursorOffset++];
		++copiedCharCount;
		if (copiedCharCount > 0x27)
			return false;
	}

	return false;
}

static bool isCaptionEdgeSpace(byte c) {
	return c == ' ' || c == '\t';
}

static void trimTimedCaptionLine(Common::String &line) {
	const byte *text = reinterpret_cast<const byte *>(line.c_str());
	uint start = 0;
	uint end = line.size();

	while (start < end) {
		if (isCaptionEdgeSpace(text[start])) {
			++start;
		} else if (start + 1 < end && ((text[start] == 0x81 && text[start + 1] == 0x40) ||
									   (text[start] == 0xa1 && text[start + 1] == 0xa1))) {
			start += 2;
		} else {
			break;
		}
	}

	while (end > start) {
		if (isCaptionEdgeSpace(text[end - 1])) {
			--end;
		} else if (end >= start + 2 && ((text[end - 2] == 0x81 && text[end - 1] == 0x40) ||
										(text[end - 2] == 0xa1 && text[end - 1] == 0xa1))) {
			end -= 2;
		} else {
			break;
		}
	}

	if (start != 0 || end != line.size())
		line = Common::String(line.c_str() + start, line.c_str() + end);
}

void BCBEngine::loadNextTimedCaptionText() {
	if (_timedCaptionRecordData.empty() || _timedCaptionCursorOffset >= _timedCaptionRecordData.size()) {
		_timedCaptionText.clear();
		_timedCaptionTextSjis.clear();
		_nextTimedCaptionThreshold = 99999;
		return;
	}

	Common::String thresholdText;
	Common::String firstLine;
	Common::String secondLine;
	if (!readTimedCaptionLine(thresholdText) || !readTimedCaptionLine(firstLine) || !readTimedCaptionLine(secondLine)) {
		_timedCaptionText.clear();
		_timedCaptionTextSjis.clear();
		_nextTimedCaptionThreshold = 99999;
		return;
	}

	trimTimedCaptionLine(firstLine);
	trimTimedCaptionLine(secondLine);

	_nextTimedCaptionThreshold = atoi(thresholdText.c_str());
	_timedCaptionTextSjis = firstLine;
	if (!secondLine.empty()) {
		_timedCaptionTextSjis += '\n';
		_timedCaptionTextSjis += secondLine;
	}
	_timedCaptionText = Common::U32String(firstLine.c_str(), firstLine.size(), _executableTextEncoding);
	if (!secondLine.empty()) {
		_timedCaptionText += U'\n';
		_timedCaptionText += Common::U32String(secondLine.c_str(), secondLine.size(), _executableTextEncoding);
	}
}

void BCBEngine::restoreCaptionBackingRect(const Common::Rect &rect) {
	if (_activeMovieOnBlankScreen) {
		Graphics::Surface *screen = _screen->surfacePtr();
		if (screen) {
			_screen->fillRect(rect, screen->format.RGBToColor(0, 0, 0));
		}
		return;
	}

	const Graphics::Surface *backing = nullptr;
	if (_currentDisplayState == 0 && _titleBitmap) {
		backing = _titleBitmap;
	} else if (_currentDisplayState == 1 || _currentDisplayState == 7) {
		backing = getUiBitmap(stateDword(kStateMainSceneNightFlag) != 0 ? "16mmainn.bmp" : "16mmain.bmp");
	} else if (_currentDisplayState == 8) {
		Common::Rect restoreRect = rect;
		restoreRect.clip(kMapSubmodeMovieBackdropRect);

		const Graphics::Surface *map = getUiBitmap("16mmap.bmp");
		if (map && !restoreRect.isEmpty()) {
			const int sourceX = kMapSubmodeMovieBackdropSourceX + restoreRect.left - kMapSubmodeMovieBackdropRect.left;
			const int sourceY = kMapSubmodeMovieBackdropSourceY + restoreRect.top - kMapSubmodeMovieBackdropRect.top;
			if (sourceX >= 0 && sourceY >= 0 && sourceX + restoreRect.width() <= map->w && sourceY + restoreRect.height() <= map->h)
				_screen->copyRectToSurface(map->getBasePtr(sourceX, sourceY), map->pitch, restoreRect.left, restoreRect.top, restoreRect.width(), restoreRect.height());
		}

		return;
	}

	if (backing)
		_screen->copyRectToSurface(backing->getBasePtr(rect.left, rect.top), backing->pitch, rect.left, rect.top, rect.width(), rect.height());
}

Common::Rect BCBEngine::getCaptionBackingRect(const Common::String &sjisText, const Common::U32String &unicodeText, const Common::Rect &nativeRect) const {
	int lineCount = 1;
	int fontHeight = nativeRect.height();

	if (_captionTextFont) {
		lineCount = 0;
		for (uint i = 0; i <= unicodeText.size(); ++i) {
			if (i == unicodeText.size() || unicodeText[i] == U'\n')
				++lineCount;
		}
		fontHeight = _captionTextFont->getFontHeight();
	} else if (_sjisFont) {
		lineCount = 0;
		for (uint i = 0; i <= sjisText.size(); ++i) {
			if (i == sjisText.size() || sjisText[i] == '\n')
				++lineCount;
		}
		fontHeight = _sjisFont->getFontHeight();
	}

	Common::Rect backingRect = nativeRect;
	backingRect.bottom = MAX<int>(backingRect.bottom, nativeRect.top + MAX<int>(lineCount, 1) * fontHeight);
	backingRect.clip(Common::Rect(0, 0, 640, 480));
	return backingRect;
}

static Common::U32String makeU32Substring(const Common::U32String &text, uint start, uint end) {
	return Common::U32String(text.c_str() + start, end - start);
}

void BCBEngine::drawCaptionTextBlock(const Common::String &sjisText, const Common::U32String &unicodeText, const Common::Rect &rect, uint32 color) {
	if (!_captionTextFont && !_sjisFont)
		return;

	Graphics::Surface *screen = _screen->surfacePtr();
	if (!screen)
		return;

	if (_captionTextFont) {
		int y = rect.top;
		for (uint lineStart = 0; lineStart <= unicodeText.size() && y < rect.bottom;) {
			uint lineEnd = lineStart;
			while (lineEnd < unicodeText.size() && unicodeText[lineEnd] != U'\n')
				++lineEnd;

			_captionTextFont->drawString(screen, makeU32Substring(unicodeText, lineStart, lineEnd), rect.left, y, rect.width(), color, Graphics::kTextAlignCenter);
			y += _captionTextFont->getFontHeight();
			lineStart = lineEnd + 1;
		}
	} else {
		int y = rect.top;
		for (uint lineStart = 0; lineStart <= sjisText.size() && y < rect.bottom;) {
			uint lineEnd = lineStart;
			while (lineEnd < sjisText.size() && sjisText[lineEnd] != '\n')
				++lineEnd;

			Common::Array<uint16> chars;
			int textWidth = 0;
			for (uint charOffset = lineStart; charOffset < lineEnd;) {
				const byte firstByte = (byte)sjisText[charOffset++];
				uint16 sjisChar = firstByte;
				if (((firstByte >= 0x81 && firstByte <= 0x9f) || (firstByte >= 0xe0 && firstByte <= 0xef)) && charOffset < lineEnd)
					sjisChar |= (uint16)((byte)sjisText[charOffset++]) << 8;
				chars.push_back(sjisChar);
				textWidth += _sjisFont->getCharWidth(sjisChar);
			}

			int lineX = rect.left + MAX<int>((rect.width() - textWidth) / 2, 0);
			for (uint i = 0; i < chars.size(); ++i) {
				if (lineX >= rect.right)
					break;
				_sjisFont->drawChar(*screen, chars[i], lineX, y, color, 0);
				lineX += _sjisFont->getCharWidth(chars[i]);
			}

			y += _sjisFont->getFontHeight();
			lineStart = lineEnd + 1;
		}
	}

	_screen->addDirtyRect(getCaptionBackingRect(sjisText, unicodeText, rect));
}

bool BCBEngine::drawTimedVideoTextOverlay() {
	if (!_activeMovieVideo || _nextTimedCaptionThreshold == 99999 || _activeMovieVideo->getCurFrame() < _nextTimedCaptionThreshold)
		return false;

	const Graphics::PixelFormat format = g_system->getScreenFormat();
	const uint32 white = format.RGBToColor(255, 255, 255);
	const uint32 black = format.RGBToColor(0, 0, 0);

	restoreActiveCaptionBackingRect();
	if (_timedCaptionTextSjis.empty() || _timedCaptionText.empty()) {
		loadNextTimedCaptionText();
		return true;
	}

	if (_currentDisplayState == 0) {
		_activeCaptionBackingRect = getCaptionBackingRect(_timedCaptionTextSjis, _timedCaptionText, kTitleCaptionBackingRect);
		drawCaptionTextBlock(_timedCaptionTextSjis, _timedCaptionText, kTitleCaptionTextRect, white);
	} else if (_currentDisplayState == 1 || _currentDisplayState == 7) {
		_activeCaptionBackingRect = getCaptionBackingRect(_timedCaptionTextSjis, _timedCaptionText, kMainSceneCaptionTextRect);
		_activeCaptionBackingRect.extend(getCaptionBackingRect(_timedCaptionTextSjis, _timedCaptionText, kMainSceneCaptionShadowRect));
		drawCaptionTextBlock(_timedCaptionTextSjis, _timedCaptionText, kMainSceneCaptionShadowRect, black);
		drawCaptionTextBlock(_timedCaptionTextSjis, _timedCaptionText, kMainSceneCaptionTextRect, white);
	} else if (_currentDisplayState == 8) {
		_activeCaptionBackingRect = getCaptionBackingRect(_timedCaptionTextSjis, _timedCaptionText, kMapSubmodeCaptionTextRect);
		drawCaptionTextBlock(_timedCaptionTextSjis, _timedCaptionText, kMapSubmodeCaptionTextRect, white);
	}

	loadNextTimedCaptionText();
	return true;
}

void BCBEngine::restoreActiveCaptionBackingRect() {
	if (!_activeCaptionBackingRect.isEmpty()) {
		restoreCaptionBackingRect(_activeCaptionBackingRect);
		_activeCaptionBackingRect = Common::Rect();
		return;
	}

	if (_currentDisplayState == 0) {
		restoreCaptionBackingRect(kTitleCaptionBackingRect);
	} else if (_currentDisplayState == 1 || _currentDisplayState == 7) {
		restoreCaptionBackingRect(kMainSceneCaptionTextRect);
	} else if (_currentDisplayState == 8) {
		restoreCaptionBackingRect(kMapSubmodeCaptionTextRect);
	}
}

} // End of namespace BCB
