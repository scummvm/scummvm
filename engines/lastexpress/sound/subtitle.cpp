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

#include "lastexpress/sound/subtitle.h"
#include "lastexpress/sound/sound.h"
#include "lastexpress/data/archive.h"

#include "lastexpress/helpers.h"
#include "lastexpress/lastexpress.h"

#include "common/memstream.h"
#include "subtitle.h"

namespace LastExpress {

Subtitle::Subtitle(LastExpressEngine *engine, const char *filename, Slot *slot) {
	_engine = engine;

	memset(_filename, 0, sizeof(_filename));
	_slot = slot;

	if (_engine->getSubtitleManager()->_subtitlesQueue) {
		Subtitle *i;
		for (i = _engine->getSubtitleManager()->_subtitlesQueue; i->_next; i = i->_next);

		i->_next = this;
	} else {
		_engine->getSubtitleManager()->_subtitlesQueue = this;
	}

	// Original bug: the _filename was 12 chars long, but sometimes
	// the sound files are a couple of characters longer. This would trigger
	// a truncated string warning within sprintf_s; we have raised the size
	// to be able to make it work like the original did (since it used plain sprintf).
	Common::sprintf_s(_filename, sizeof(_filename), "%s.SBE", filename);

	HPF *archive = _engine->getArchiveManager()->openHPF(_filename);
	if (archive) {
		_engine->getArchiveManager()->closeHPF(archive);

		if ((_engine->getSubtitleManager()->_flags & kSubFlagLoaded) == 0) {
			load();
		}
	} else {
		_status = kSubFlagStatusKilled;
	}
}

Subtitle::~Subtitle() {
	if (_engine->getSubtitleManager()->_subtitlesQueue) {
		if (_engine->getSubtitleManager()->_subtitlesQueue == this) {
			_engine->getSubtitleManager()->_subtitlesQueue = _engine->getSubtitleManager()->_subtitlesQueue->_next;
		} else {
			Subtitle *next;
			Subtitle *queue = _engine->getSubtitleManager()->_subtitlesQueue;

			if (_engine->getSubtitleManager()->_subtitlesQueue->_next == this) {
				queue->_next = _next;
			} else {
				do {
					next = queue->_next;
					if (!next)
						break;

					queue = queue->_next;
				} while (next->_next != this);

				if (queue->_next == this)
					queue->_next = _next;
			}

		}
	}

	if (_engine->getSubtitleManager()->_currentSubtitle == this) {
		if (!_engine->shouldQuit())
			_engine->getSubtitleManager()->clearSubArea();

		_engine->getSubtitleManager()->_currentSubtitle = nullptr;
		_engine->getSubtitleManager()->_flags = 0;
	}
}

void Subtitle::load() {
	HPF *archive;

	archive = _engine->getArchiveManager()->openHPF(_filename);
	_data = _engine->getSubtitleManager()->_subtitlesData + 1;
	_engine->getSubtitleManager()->_subtitleIndex = -1;

	if (archive) {
		_engine->getArchiveManager()->readHPF(archive, _engine->getSubtitleManager()->_subtitlesData, archive->size);
		_engine->getArchiveManager()->closeHPF(archive);

		for (int i = 0; i < (archive->size * MEM_PAGE_SIZE) / 2; i++) {
			_engine->getSubtitleManager()->_subtitlesData[i] = READ_LE_UINT16(&_engine->getSubtitleManager()->_subtitlesData[i]);
		}

		if (_engine->getSubtitleManager()->_subtitlesData[0]) {
			for (int i = 0; i < _engine->getSubtitleManager()->_subtitlesData[0]; i++) {
				if (!_data[1])
					_data[1] = _data[_data[3] + 4 + _data[2]];

				_data += _data[3] + _data[2] + 4;
			}
		}

		_engine->getSubtitleManager()->_flags |= kSubFlagLoaded;
		_engine->getSubtitleManager()->_currentSubtitle = this;
	}
}

void Subtitle::update() {
	int32 count = 0;

	_data = _engine->getSubtitleManager()->_subtitlesData + 1;

	if (_data[1] <= _slot->getTime()) {
		do {
			if (_engine->getSubtitleManager()->_subtitlesData[0] <= count)
				break;

			count++;
			_data = &_data[_data[3] + 4 + _data[2]];
		} while (_data[1] <= _slot->getTime());
	}

	if (_engine->getSubtitleManager()->_subtitlesData[0] <= count) {
		_status = kSubFlagStatusKilled;

		if ((_engine->getSubtitleManager()->_flags & kSubFlagDrawOnScreen) != 0)
			_engine->getSubtitleManager()->clearSubArea();
	} else {
		if (_data[0] > _slot->getTime() || _data[1] <= _slot->getTime()) {
			if ((_engine->getSubtitleManager()->_flags & kSubFlagDrawOnScreen) != 0) {
				_engine->getSubtitleManager()->clearSubArea();
				_engine->getSubtitleManager()->_currentSubtitle = this;

				return;
			}
		} else if (count != _engine->getSubtitleManager()->_subtitleIndex) {
			_engine->getSubtitleManager()->drawSubArea(&_data[2]);
			_engine->getSubtitleManager()->_subtitleIndex = count;
			_engine->getSubtitleManager()->_currentSubtitle = this;

			return;
		}
	}

	_engine->getSubtitleManager()->_currentSubtitle = this;
}

void Subtitle::kill() {
	_status = kSubFlagStatusKilled;
}

SubtitleManager::SubtitleManager(LastExpressEngine *engine) {
	_engine = engine;

	memset(_upperLineCharWidths, 0, sizeof(_upperLineCharWidths));
	memset(_lowerLineCharWidths, 0, sizeof(_lowerLineCharWidths));
	memset(_upperLineChars, 0, sizeof(_upperLineChars));
	memset(_lowerLineChars, 0, sizeof(_lowerLineChars));
}

SubtitleManager::~SubtitleManager() {
	SAFE_DELETE(_font);
}

void SubtitleManager::initSubtitles() {
	HPF *archive = _engine->getArchiveManager()->openHPF("FONT.DAT");

	if (archive) {
		if (_font->fontData) {
			free(_font->fontData);
			_font->fontData = nullptr;
		}

		byte *fontData = (byte *)malloc(MEM_PAGE_SIZE * archive->size);

		if (fontData) {
			_engine->getArchiveManager()->readHPF(archive, fontData, archive->size);
			_engine->getArchiveManager()->closeHPF(archive);

			Common::MemoryReadStream *fontStream = new Common::MemoryReadStream(fontData, MEM_PAGE_SIZE * archive->size, DisposeAfterUse::YES);

			for (int i = 0; i < 16; i++) {
				_font->palette[i] = fontStream->readUint16LE();
			}

			for (int i = 0; i < 256; i++) {
				_font->charMap[i] = fontStream->readByte();
			}

			for (int i = 0; i < 256; i++) {
				_font->charKerning[i] = fontStream->readByte();
			}

			uint32 sizeOfData = MEM_PAGE_SIZE * archive->size - (16 * sizeof(uint16) + 256 + 256);
			_font->fontData = (byte *)malloc(sizeOfData);

			assert(_font->fontData);

			for (uint i = 0; !fontStream->eos() && i < sizeOfData; i++) {
				_font->fontData[i] = fontStream->readByte();
			}

			delete fontStream;
		} else {
			_font->fontData = nullptr;
		}
	} else {
		_font->fontData = nullptr;
	}

	_engine->getGraphicsManager()->modifyPalette(_font->palette, 16);
}

void SubtitleManager::storeVArea(PixMap *pixels) {
	if (_engine->getGraphicsManager()->acquireSurface()) {
		PixMap *screenSurface = (PixMap *)((byte *)_engine->getGraphicsManager()->_screenSurface.getPixels() + 537760);

		for (int i = 38; i > 0; i--) {
			for (int j = 480; j > 0; j--) {
				*pixels++ = *screenSurface++;
			}

			screenSurface += 160;
		}

		_engine->getGraphicsManager()->unlockSurface();
	}
}

void SubtitleManager::restoreVArea(PixMap *pixels) {
	if (_engine->getGraphicsManager()->acquireSurface()) {
		PixMap *screenSurface = (PixMap *)((byte *)_engine->getGraphicsManager()->_screenSurface.getPixels() + 537760);

		for (int i = 38; i > 0; i--) {
			for (int j = 480; j > 0; j--) {
				*screenSurface++ = *pixels++;
			}

			screenSurface += 160;
		}

		_engine->getGraphicsManager()->unlockSurface();
	}
}

void SubtitleManager::vSubOn() {
	storeVArea(_engine->getGraphicsManager()->_subtitlesBackBuffer);

	if (_font->fontData[0]) {
		if (_engine->getGraphicsManager()->acquireSurface()) {
			PixMap *surfaceLine1 = (PixMap *)((byte *)_engine->getGraphicsManager()->_screenSurface.getPixels() + ((640 - _upperLineXStart) & ~1) + 537600);
			for (int i = 0; i < _upperLineLength; ++i) {
				drawChar(surfaceLine1, _upperLineChars[i]);
				surfaceLine1 += _upperLineCharWidths[i];
			}

			PixMap *surfaceLine2 = (PixMap *)((byte *)_engine->getGraphicsManager()->_screenSurface.getPixels() + ((640 - _lowerLineXStart) & ~1) + 563200);
			for (int i = 0; i < _lowerLineLength; ++i) {
				drawChar(surfaceLine2, _lowerLineChars[i]);
				surfaceLine2 += _lowerLineCharWidths[i];
			}

			_engine->getGraphicsManager()->unlockSurface();
		}
	}
}

void SubtitleManager::vSubOff() {
	restoreVArea(_engine->getGraphicsManager()->_subtitlesBackBuffer);
}

void SubtitleManager::clearSubArea() {
	_flags &= ~kSubFlagDrawOnScreen;
	_engine->getGraphicsManager()->burstBox(80, 420, 480, 38);
}

void SubtitleManager::drawChar(PixMap *destBuf, uint8 whichChar) {
	byte *fontPtr = &_font->fontData[(288 * _font->charMap[whichChar] >> 1) + 2];

	for (int row = 0; row < 18; row++) {
		for (int col = 0; col < 8; col++) {
			uint8 pixelByte = *fontPtr;

			uint8 upperNibble = pixelByte >> 4;
			if (upperNibble != 0) {
				*destBuf = _font->palette[upperNibble];
			}

			destBuf++;

			uint8 lowerNibble = pixelByte & 0x0F;
			if (lowerNibble != 0) {
				*destBuf = _font->palette[lowerNibble];
			}

			destBuf++;

			fontPtr++;
		}

		destBuf += 624;
	}
}

void SubtitleManager::drawSubArea(uint16 *subtitleData) {
	uint16 *data = subtitleData + 2;
	_upperLineLength = subtitleData[0];
	_lowerLineLength = subtitleData[1];

	_upperLineXStart = 0;
	for (int i = 0; i < _upperLineLength; i++) {
		_upperLineChars[i] = *data;
		_upperLineCharWidths[i] = _font->charKerning[_upperLineChars[i]] + 1;
		_upperLineXStart += _font->charKerning[_upperLineChars[i]] + 1;
		data++;
	}

	_lowerLineXStart = 0;
	for (int i = 0; i < _lowerLineLength; i++) {
		_lowerLineChars[i] = *data;
		_lowerLineCharWidths[i] = _font->charKerning[_lowerLineChars[i]] + 1;
		_lowerLineXStart += _font->charKerning[_lowerLineChars[i]] + 1;
		data++;
	}

	_flags |= kSubFlagDrawOnScreen;
	_engine->getGraphicsManager()->burstBox(80, 420, 480, 38);
}

void SubtitleManager::subThread() {
	int32 maxPriority;
	Subtitle *queueElement;
	Subtitle *selectedSubtitle;
	Slot *slot;
	int32 curPriority;

	maxPriority = 0;
	queueElement = _subtitlesQueue;

	for (selectedSubtitle = 0; queueElement; queueElement = queueElement->_next) {
		slot = queueElement->_slot;

		if ((slot->getStatusFlags() & kSoundFlagPlaying) == 0 ||
			(slot->getStatusFlags() & kSoundFlagMute) != 0 ||
			!slot->getTime() ||
			((slot->getStatusFlags() & kSoundVolumeMask) < kVolume6) ||
			((_engine->getNISManager()->getNISFlag() & kNisFlagSoundFade) != 0 && slot->getPriority() < 90)) {

			curPriority = 0;
		} else {
			curPriority = slot->getPriority() + (slot->getStatusFlags() & kSoundVolumeMask);

			if (_currentSubtitle == queueElement)
				curPriority += 4;
		}

		if (maxPriority < curPriority) {
			maxPriority = curPriority;
			selectedSubtitle = queueElement;
		}
	}

	if (_currentSubtitle == selectedSubtitle) {
		if (selectedSubtitle)
			selectedSubtitle->update();

		return;
	}

	if ((_flags & kSubFlagDrawOnScreen) != 0)
		clearSubArea();

	if (selectedSubtitle) {
		selectedSubtitle->load();

		if (selectedSubtitle)
			selectedSubtitle->update();
	}
}

} // End of namespace LastExpress
