/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "dune/dune.h"
#include "dune/clock.h"
#include "dune/resources.h"
#include "dune/sprite_sheet.h"
#include "dune/video.h"
#include "common/endian.h"
#include "common/system.h"
#include "graphics/paletteman.h"

namespace Dune {

/** A05E: only an unmarked, classified record enters the chronological book. */
void DuneEngine::rememberBookRecord(const Dialogue::Record &record, uint speaker) {
	if (!record.bookTopic || (record.flags & 0x80) || speaker > 16)
		return;
	const uint16 key = record.bookKey | (speaker << 11);
	for (uint i = 0; i < _bookEntries.size(); i++)
		if (_bookEntries[i] == key)
			return;
	_bookEntries.push_back(key);
	WRITE_LE_UINT16(_state.begin() + 0x11bd, 0xaa + 2 * _bookEntries.size());
}

/** AEDE: close the voice and lower the old room face BEFORE taking B2B9. */
void DuneEngine::openBook() {
	if (_state[0x11c9] || _onWorld || _onGlobe || _inCabin || _state[0x4774])
		return;
	_state[0x46df] = 0; // AEDE/18BA, retaining cancellation until3916 is due.
	stopSpeech();
	lowerPanelFace(false);
	Common::Array<byte> old(kScreenWidth * kScreenHeight);
	memcpy(old.begin(), _framebuffer1, old.size());
	byte palette[256 * 3];
	_system->getPaletteManager()->grabPalette(palette, 0, 256);
	_state[0xc6] = 3; // AEEC/AF43: book context and the title page.
	_state[0x2788]++;
	WRITE_LE_UINT16(_state.begin() + 0x2406, 0);
	_bookPage = -1;
	_bookTopic = 0;
	_menuKind = kMenuBook;
	_speech.clear();
	_speechParts.clear();
	_speakerPerson = -1; // DAA3 removes the old portrait's callback.
	playTheRightSong();
	drawBook();
	showMapEntryTransition(old.begin(), palette, false); // AEFA: effect34, no17E6.
}

/** B18B/1877/2DB1: release the lock, restore room controls, then raise Paul. */
void DuneEngine::closeBook() {
	Common::Array<byte> old(kScreenWidth * kScreenHeight);
	memcpy(old.begin(), _framebuffer1, old.size());
	byte palette[256 * 3];
	_system->getPaletteManager()->grabPalette(palette, 0, 256);
	stopBookCredits();
	_state[0x2788] = 0;
	_state[0xc6] = 0;
	_state[0x1c30] = 0;
	_state[0x479e] = _state[0x479f] = 0;
	if (!_state[0x2b])
		WRITE_LE_UINT16(_state.begin() + 0x14, 0);
	_speakerPerson = -1;
	_speaker.clear();
	_speech.clear();
	_speechParts.clear();
	resetPanel();
	if (_inDesert) {
		showDesertMenu(false);
		drawDesert(false);
	} else {
		showRoomMenu(false);
		drawScene(false);
	}
	playTheRightSong();
	showMapEntryTransition(old.begin(), palette);
}

/** AF58..AF76/B150: select the first logged page in this category. */
void DuneEngine::showBookTopic(byte topic) {
	if (topic > 3)
		return;
	for (uint i = 0; i < _bookEntries.size(); i++) {
		const Dialogue::Record *record = _dialogue.bookRecord(_bookEntries[i]);
		if (record && (!topic || record->bookTopic == topic)) {
			stopBookCredits();
			_bookTopic = topic;
			_bookPage = i;
			WRITE_LE_UINT16(_state.begin() + 0x11bf, 0xaa + 2 * i);
			_state[0xc6] = 1;
			WRITE_LE_UINT16(_state.begin() + 0x2406, topic ? (topic << 10) | 0x1c : 0);
			drawBook();
			return;
		}
	}
}

/** AFC7: running off the list returns to its title, not an invented wrap. */
void DuneEngine::turnBookPage(int direction) {
	if (_bookPage == -2) { // B006: the credits callback, C6 bit4.
		if (direction > 0)
			return;
		stopBookCredits();
		_bookPage = _bookEntries.size();
	}
	if (_bookPage == -1) {
		showBookTopic(_bookTopic);
		return;
	}
	for (int page = _bookPage + direction; page >= 0 && page < (int)_bookEntries.size(); page += direction) {
		const Dialogue::Record *record = _dialogue.bookRecord(_bookEntries[page]);
		if (record && (!_bookTopic || record->bookTopic == _bookTopic)) {
			_bookPage = page;
			WRITE_LE_UINT16(_state.begin() + 0x11bf, 0xaa + 2 * page);
			drawBook();
			return;
		}
	}
	if (direction > 0) {
		// B011/09F5 starts the film when the forward scan reaches its end.
		_system->getPaletteManager()->grabPalette(_bookPalette, 0, 256);
		_bookPage = -2;
		_state[0xc6] = 5;
		drawMovieFrame(INDEX_CREDITS_HNM, 0);
		_bookFrameTime = animationClockTicks(); // CA1B/CA59 samples after the first frame.
	} else {
		_bookPage = -1;
		_state[0xc6] = 3;
	}
	drawBook();
}

void DuneEngine::stopBookCredits() {
	if (_bookPage != -2)
		return;
	_video->stopAudio();
	_system->getPaletteManager()->setPalette(_bookPalette, 0, 256);
	_bookFrameTime = 0;
	_state[0xc6] = 1;
}

/** 0A16/0A23: service the credits callback without blocking book controls. */
void DuneEngine::updateBookCredits() {
	if (_menuKind != kMenuBook || _bookPage != -2 || _video->done())
		return;
	const uint32 now = animationClockTicks();
	// CREDITS.HNM's descriptor is0740: CAD4 waits seven CE7A ticks,
	// then CA59 rebases after one frame, including stalls and word wrap.
	if (!silentMovieFrameDue(_bookFrameTime, now, 7))
		return;
	_bookFrameTime = now;
	_video->decodeAVFrame();
	drawBook();
}

/** B254: the twelve resource-record links in DS:2426 select movies25..36. */
uint16 DuneEngine::bookMovie() const {
	if (_bookPage < 0 || (uint)_bookPage >= _bookEntries.size())
		return 0;
	for (uint i = 0; i < 12; i++)
		if (_bookEntries[_bookPage] == READ_LE_UINT16(_state.begin() + 0x2426 + 2 * i))
			return 25 + i;
	return 0;
}

void DuneEngine::drawBook() {
	_graphics.setFrameBuffer(_framebuffer1);
	_menu.clear();
	_menuParts.clear();
	_menuFlags.clear();
	_menuFrom = 0;
	for (uint topic = 0; topic < 4; topic++) {
		bool available = topic == 0;
		for (uint i = 0; i < _bookEntries.size() && !available; i++) {
			const Dialogue::Record *record = _dialogue.bookRecord(_bookEntries[i]);
			available = record && record->bookTopic == topic;
		}
		_menu.push_back(command(topic ? 227 + topic : 226));
		_menuParts.push_back(topic);
		_menuFlags.push_back(!available ? 0x4000 : topic == _bookTopic ? 0x8000 : 0);
	}
	_menu.push_back(command(227));
	_menuParts.push_back(4);
	_menuFlags.push_back(0);
	debugMenuIfChanged();
	if (_bookPage == -2) {
		memcpy(_framebuffer1, _video->_frameBuffer, kScreenWidth * kSceneHeight);
		_graphics.setFrameBuffer(_framebuffer1);
	} else if (_bookPage < 0) {
		memset(_framebuffer1, 0, kScreenWidth * kScreenHeight); // AF48/C432.
		openSpriteSheet("BOOK.HSQ");
		drawSprite(0, 0, 0);
		_state[0x1c30] = 0;
	} else {
		openSpriteSheet("BOOK.HSQ");
		// 8FF5/C370 tiles BOOK sprite3 over DS:2265. The movie callback
		// beneath this page is not its visible paper background.
		for (int y = 0; y < kSceneHeight; y += 29)
			for (int x = 0; x < kScreenWidth; x += 33)
				currentSpriteSheet->drawMapSprite(3, x, y,
					Common::Rect(0, 0, kScreenWidth, kSceneHeight));
		const uint16 key = _bookEntries[_bookPage];
		const Dialogue::Record *record = _dialogue.bookRecord(key);
		const uint speaker = key >> 11;
		int textBottom = 140;
		if (record) {
			Common::String text = command(242 + speaker); // CF70 decrements SI.
			text += fillInLine(phraseOf(speaker, record->line));
			text.trim();
			textBottom = drawBookText(text);
		}
		// B08B/C551 outlines DS:2408 after the paragraph has been painted.
		_graphics.fillRect(0, 0, 320, 1, 83);
		_graphics.fillRect(0, 151, 320, 152, 83);
		_graphics.fillRect(0, 0, 1, 152, 83);
		_graphics.fillRect(319, 0, 320, 152, 83);
		const uint16 movie = bookMovie();
		WRITE_LE_UINT16(_state.begin() + 0x243e, movie);
		_state[0x1c30] = movie ? 0x80 : 0;
		if (movie) {
			for (int y = 5; y <= 29; y += 12)
				drawSprite(movie - 11, 6, y);
		}
		const int ornamentY = (140 + textBottom) / 2;
		if (ornamentY < 138)
			drawSprite(4, 147, ornamentY); // B094..B0B2, including movie pages.
		_font.setFace(Font::kFaceAlternate);
		_font.drawString(_framebuffer1, 306, 3, Common::String::format("%2u", _bookPage + 1), 83);
		_font.setFace(Font::kFaceNormal);
		if (_bookPage && record)
			_font.drawString(_framebuffer1, 250, 139, command(258 + record->bookTopic), 100);
	}
	// D7AD installs the book's side box; D72B installs DS:1E1A controls.
	for (uint i = 0; i < kCompassElements; i++) {
		const byte *p = _state.begin() + 0x1e1a + 14 * i;
		_panel[kFirstCompassElement + i] = {(int16)READ_LE_UINT16(p), (int16)READ_LE_UINT16(p + 2),
			(int16)READ_LE_UINT16(p + 4), (int16)READ_LE_UINT16(p + 6), READ_LE_UINT16(p + 8),
			READ_LE_UINT16(p + 10), READ_LE_UINT16(p + 12)};
	}
	drawPanel();
	openSpriteSheet("BOOK.HSQ"); // Its palette also recolours the ICONES panel.
	drawMenu();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

void DuneEngine::pressBook(const Common::Point &at) {
	const int row = menuRowAt(at);
	if (row >= 0) {
		chooseFromMenu(row);
		return;
	}
	if (at.x > 21 && at.x < 42 && at.y > 161 && at.y <= 186)
		turnBookPage(-1);
	else if (at.x > 43 && at.x < 63 && at.y > 161 && at.y <= 186)
		turnBookPage(1);
	else if ((at.x > 0 && at.x < 92 && at.y > 154 && at.y <= 200) ||
			(at.x > 255 && at.x < 295 && at.y > 162 && at.y <= 192))
		closeBook();
	else if (at.x > 0 && at.x < 40 && at.y > 4 && at.y <= 46 && bookMovie()) {
		playVideo(bookMovie() - 1);
		drawBook();
	}
}

} // End of namespace Dune
