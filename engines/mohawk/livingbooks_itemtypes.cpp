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

#include "mohawk/livingbooks.h"
#include "mohawk/resource.h"
#include "mohawk/video.h"

#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/paletteman.h"

#include "gui/message.h"

namespace Mohawk {

LBSoundItem::LBSoundItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	debug(3, "new LBSoundItem");
	_running = false;
}

LBSoundItem::~LBSoundItem() {
	if (_running)
		_vm->_sound->stopSound(_resourceId);
}

void LBSoundItem::update() {
	if (_running && !_vm->_sound->isPlaying(_resourceId)) {
		_running = false;
		done(true);
	}

	LBItem::update();
}

bool LBSoundItem::togglePlaying(bool playing, bool restart) {
	if (!playing)
		return LBItem::togglePlaying(playing, restart);

	if (_running) {
		_running = false;
		_vm->_sound->stopSound(_resourceId);
	}

	if (!_loaded || !_enabled || !_globalEnabled)
		return false;

	_running = true;
	debug(4, "sound %d play for item %d (%s)", _resourceId, _itemId, _desc.c_str());
	_vm->playSound(this, _resourceId);
	return true;
}

void LBSoundItem::stop() {
	if (_running) {
		_running = false;
		_vm->_sound->stopSound(_resourceId);
	}

	LBItem::stop();
}

LBItem *LBSoundItem::createClone() {
	return new LBSoundItem(_vm, _page, _rect);
}

LBGroupItem::LBGroupItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	debug(3, "new LBGroupItem");
	_starting = false;
}

void LBGroupItem::readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream) {
	switch (type) {
	case kLBGroupData:
		{
		_groupEntries.clear();
		uint16 count = stream->readUint16();
		debug(3, "Group data: %d entries", count);

		if (size != 2 + count * 4)
			error("kLBGroupData was wrong size (%d, for %d entries)", size, count);

		for (uint i = 0; i < count; i++) {
			GroupEntry entry;
			// TODO: is type important for any game? at the moment, we ignore it
			entry.entryType = stream->readUint16();
			entry.entryId = stream->readUint16();
			// HACK: The Living Books v3 sampler includes the ID for the group as
			// one of the entries in the Green Eggs and Ham section, which leads
			// to infinite recursion when the group is loaded.
			if (entry.entryId != getId())
				_groupEntries.push_back(entry);
			debug(3, "group entry: id %d, type %d", entry.entryId, entry.entryType);
		}
		}
		break;

	default:
		LBItem::readData(type, size, stream);
	}
}

void LBGroupItem::destroySelf() {
	LBItem::destroySelf();

	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->destroySelf();
	}
}

void LBGroupItem::setEnabled(bool enabled) {
	if (_starting) {
		_starting = false;
		LBItem::setEnabled(enabled);
	} else {
		for (uint i = 0; i < _groupEntries.size(); i++) {
			LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
			if (item)
				item->setEnabled(enabled);
		}
	}
}

void LBGroupItem::setGlobalEnabled(bool enabled) {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->setGlobalEnabled(enabled);
	}
}

bool LBGroupItem::contains(Common::Point point) {
	return false;
}

bool LBGroupItem::togglePlaying(bool playing, bool restart) {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->togglePlaying(playing, restart);
	}

	return false;
}

void LBGroupItem::seek(uint16 pos) {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->seek(pos);
	}
}

void LBGroupItem::setVisible(bool visible) {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->setVisible(visible);
	}
}

void LBGroupItem::setGlobalVisible(bool visible) {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->setGlobalVisible(visible);
	}
}

void LBGroupItem::startPhase(uint phase) {
	_starting = true;
	LBItem::startPhase(phase);
	_starting = false;
}

void LBGroupItem::stop() {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->stop();
	}
}

void LBGroupItem::load() {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->load();
	}
}

void LBGroupItem::unload() {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->unload();
	}
}

void LBGroupItem::moveBy(const Common::Point &pos) {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->moveBy(pos);
	}
}

void LBGroupItem::moveTo(const Common::Point &pos) {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->moveTo(pos);
	}
}

LBItem *LBGroupItem::createClone() {
	// TODO: needed?
	error("LBGroupItem::createClone unimplemented");
	return new LBGroupItem(_vm, _page, _rect);
}

LBPaletteItem::LBPaletteItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	debug(3, "new LBPaletteItem");

	_fadeInStart = 0;
	_palette = nullptr;
}

LBPaletteItem::~LBPaletteItem() {
	delete[] _palette;
}

void LBPaletteItem::readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream) {
	switch (type) {
	case kLBPaletteXData:
		{
		assert(size >= 8);
		_fadeInPeriod = stream->readUint16();
		_fadeInStep = stream->readUint16();
		_drawStart = stream->readUint16();
		_drawCount = stream->readUint16();
		if (_drawStart + _drawCount > 256)
			error("encountered palette trying to set more than 256 colors");
		assert(size == 8 + _drawCount * 4);

		// TODO: _drawCount is really more like _drawEnd, so once we're sure that
		// there's really no use for the palette entries before _drawCount, we
		// might want to just discard them here, at load time.
		_palette = new byte[_drawCount * 3];
		for (uint i = 0; i < _drawCount; i++) {
			_palette[i*3 + 0] = stream->readByte();
			_palette[i*3 + 1] = stream->readByte();
			_palette[i*3 + 2] = stream->readByte();
			stream->readByte();
		}
		}
		break;

	default:
		LBItem::readData(type, size, stream);
	}
}

bool LBPaletteItem::togglePlaying(bool playing, bool restart) {
	// TODO: this likely isn't the right place

	if (playing) {
		_fadeInStart = _vm->_system->getMillis();
		_fadeInCurrent = 0;

		return true;
	}

	return LBItem::togglePlaying(playing, restart);
}

void LBPaletteItem::update() {
	if (_fadeInStart) {
		if (!_palette)
			error("LBPaletteItem had no palette on startup");

		uint32 elapsedTime = _vm->_system->getMillis() - _fadeInStart;
		uint32 divTime = elapsedTime / _fadeInStep;

		if (divTime > _fadeInPeriod)
			divTime = _fadeInPeriod;

		if (_fadeInCurrent != divTime) {
			_fadeInCurrent = divTime;

			// TODO: actual fading-in
			if (_visible && _globalVisible) {
				_vm->_system->getPaletteManager()->setPalette(_palette + _drawStart * 3, _drawStart, _drawCount - _drawStart);
				_vm->_needsRedraw = true;
			}
		}

		if (elapsedTime >= (uint32)_fadeInPeriod * (uint32)_fadeInStep) {
			// TODO: correct?
			_fadeInStart = 0;
		}
	}

	LBItem::update();
}

LBItem *LBPaletteItem::createClone() {
	error("can't clone LBPaletteItem");
}

LBLiveTextItem::LBLiveTextItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	_currentPhrase = 0xFFFF;
	_currentWord = 0xFFFF;
	debug(3, "new LBLiveTextItem");
}

void LBLiveTextItem::readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream) {
	switch (type) {
	case kLBLiveTextData:
		{
		stream->read(_backgroundColor, 4); // unused?
		stream->read(_foregroundColor, 4);
		stream->read(_highlightColor, 4);
		_paletteIndex = stream->readUint16();
		uint16 phraseCount = stream->readUint16();
		uint16 wordCount = stream->readUint16();

		debug(3, "LiveText has %d words in %d phrases, palette index 0x%04x", wordCount, phraseCount, _paletteIndex);
		debug(3, "LiveText colors: background %02x%02x%02x%02x, foreground %02x%02x%02x%02x, highlight %02x%02x%02x%02x",
			_backgroundColor[0], _backgroundColor[1], _backgroundColor[2], _backgroundColor[3],
			_foregroundColor[0], _foregroundColor[1], _foregroundColor[2], _foregroundColor[3],
			_highlightColor[0], _highlightColor[1], _highlightColor[2], _highlightColor[3]);

		if (size != 18 + 14 * wordCount + 18 * phraseCount)
			error("Bad Live Text data size (got %d, wanted %d words and %d phrases)", size, wordCount, phraseCount);

		_words.clear();
		for (uint i = 0; i < wordCount; i++) {
			LiveTextWord word;
			word.bounds = _vm->readRect(stream);
			word.soundId = stream->readUint16();
			word.itemType = stream->readUint16();
			word.itemId = stream->readUint16();
			debug(4, "Word: (%d, %d) to (%d, %d), sound %d, item %d (type %d)",
				word.bounds.left, word.bounds.top, word.bounds.right, word.bounds.bottom, word.soundId, word.itemId, word.itemType);
			_words.push_back(word);
		}

		_phrases.clear();
		for (uint i = 0; i < phraseCount; i++) {
			LiveTextPhrase phrase;
			phrase.wordStart = stream->readUint16();
			phrase.wordCount = stream->readUint16();
			phrase.highlightStart = stream->readUint16();
			phrase.startId = stream->readUint16();
			phrase.highlightEnd = stream->readUint16();
			phrase.endId = stream->readUint16();

			// The original stored the values in uint32's so we need to swap here
			if (_vm->isBigEndian()) {
				SWAP(phrase.highlightStart, phrase.startId);
				SWAP(phrase.highlightEnd, phrase.endId);
			}

			uint32 unknown1 = stream->readUint16();
			uint16 unknown2 = stream->readUint32();

			if (unknown1 != 0 || unknown2 != 0)
				error("Unexpected unknowns %08x/%04x in LiveText word", unknown1, unknown2);

			debug(4, "Phrase: start %d, count %d, start at %d (from %d), end at %d (from %d)",
				phrase.wordStart, phrase.wordCount, phrase.highlightStart, phrase.startId, phrase.highlightEnd, phrase.endId);

			_phrases.push_back(phrase);
		}
		}
		break;

	default:
		LBItem::readData(type, size, stream);
	}
}

bool LBLiveTextItem::contains(Common::Point point) {
	if (!LBItem::contains(point))
		return false;

	point.x -= _rect.left;
	point.y -= _rect.top;

	for (uint i = 0; i < _words.size(); i++) {
		if (_words[i].bounds.contains(point))
			return true;
	}

	return false;
}

void LBLiveTextItem::paletteUpdate(uint16 word, bool on) {
	_vm->_needsRedraw = true;

	// Sometimes the last phrase goes out-of-bounds, the original engine
	// only checks the words which are valid in the palette updating code.
	if (word >= _words.size())
		return;

	if (_resourceId) {
		// with a resource, we draw a bitmap in draw() rather than changing the palette
		return;
	}

	if (on) {
		_vm->_system->getPaletteManager()->setPalette(_highlightColor, _paletteIndex + word, 1);
	} else {
		_vm->_system->getPaletteManager()->setPalette(_foregroundColor, _paletteIndex + word, 1);
	}
}

void LBLiveTextItem::update() {
	if (_currentWord != 0xFFFF) {
		uint16 soundId = _words[_currentWord].soundId;
		if (soundId && !_vm->_sound->isPlaying(soundId)) {
			paletteUpdate(_currentWord, false);

			// TODO: check this in RE
			LBItem *item = _vm->getItemById(_words[_currentWord].itemId);
			if (item)
				item->togglePlaying(false, true);

			_currentWord = 0xFFFF;
		}
	}

	LBItem::update();
}

void LBLiveTextItem::draw() {
	// this is only necessary when we are drawing using a bitmap
	if (!_resourceId)
		return;

	if (_currentWord != 0xFFFF) {
		uint yPos = 0;
		if (_currentWord > 0) {
			for (uint i = 0; i < _currentWord; i++) {
				yPos += (_words[i].bounds.bottom - _words[i].bounds.top);
			}
		}
		drawWord(_currentWord, yPos);
		return;
	}

	if (_currentPhrase == 0xFFFF)
		return;

	uint wordStart = _phrases[_currentPhrase].wordStart;
	uint wordCount = _phrases[_currentPhrase].wordCount;
	if (wordStart + wordCount > _words.size())
		error("phrase %d was invalid (%d words, from %d, out of only %d total)",
			_currentPhrase, wordCount, wordStart, _words.size());

	uint yPos = 0;
	for (uint i = 0; i < wordStart + wordCount; i++) {
		if (i >= wordStart)
			drawWord(i, yPos);
		yPos += (_words[i].bounds.bottom - _words[i].bounds.top);
	}
}

void LBLiveTextItem::drawWord(uint word, uint yPos) {
	Common::Rect srcRect(0, yPos, _words[word].bounds.right - _words[word].bounds.left,
		yPos + _words[word].bounds.bottom - _words[word].bounds.top);
	Common::Rect dstRect = _words[word].bounds;
	dstRect.translate(_rect.left, _rect.top);
	_vm->_gfx->copyAnimImageSectionToScreen(_resourceId, srcRect, dstRect);
}

void LBLiveTextItem::handleMouseDown(Common::Point pos) {
	if (!_loaded || !_enabled || !_globalEnabled || _playing)
		return LBItem::handleMouseDown(pos);

	pos.x -= _rect.left;
	pos.y -= _rect.top;

	for (uint i = 0; i < _words.size(); i++) {
		if (_words[i].bounds.contains(pos)) {
			if (_currentWord != 0xFFFF) {
				paletteUpdate(_currentWord, false);
				_currentWord = 0xFFFF;
			}
			uint16 soundId = _words[i].soundId;
			if (!soundId) {
				// TODO: can we be smarter here, using timing?
				warning("ignoring click due to no soundId");
				return;
			}
			_currentWord = i;
			_vm->playSound(this, soundId);
			paletteUpdate(_currentWord, true);
			return;
		}
	}

	return LBItem::handleMouseDown(pos);
}

bool LBLiveTextItem::togglePlaying(bool playing, bool restart) {
	if (!playing)
		return LBItem::togglePlaying(playing, restart);
	if (!_loaded || !_enabled || !_globalEnabled)
		return _playing;

	// TODO: handle this properly
	_vm->_sound->stopSound();

	_currentWord = 0xFFFF;
	_currentPhrase = 0xFFFF;

	return true;
}

void LBLiveTextItem::stop() {
	// TODO: stop sound, refresh palette

	LBItem::stop();
}

void LBLiveTextItem::notify(uint16 data, uint16 from) {
	if (!_loaded || !_enabled || !_globalEnabled || !_playing)
		return LBItem::notify(data, from);

	if (_currentWord != 0xFFFF) {
		// TODO: handle this properly
		_vm->_sound->stopSound();
		paletteUpdate(_currentWord, false);
		_currentWord = 0xFFFF;
	}

	for (uint i = 0; i < _phrases.size(); i++) {
		if (_phrases[i].highlightStart == data && _phrases[i].startId == from) {
			debug(2, "Enabling phrase %d", i);
			for (uint j = 0; j < _phrases[i].wordCount; j++) {
				paletteUpdate(_phrases[i].wordStart + j, true);
			}
			_currentPhrase = i;
			// TODO: not sure this is the correct logic
			if (i == _phrases.size() - 1) {
				_currentPhrase = 0xFFFF;
				done(true);
			}
		} else if (_phrases[i].highlightEnd == data && _phrases[i].endId == from) {
			debug(2, "Disabling phrase %d", i);
			for (uint j = 0; j < _phrases[i].wordCount; j++) {
				paletteUpdate(_phrases[i].wordStart + j, false);
			}
			_currentPhrase = 0xFFFF;
		}
	}

	LBItem::notify(data, from);
}

LBItem *LBLiveTextItem::createClone() {
	error("can't clone LBLiveTextItem");
}

LBPictureItem::LBPictureItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	debug(3, "new LBPictureItem");
}

void LBPictureItem::readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream) {
	switch (type) {
	case kLBSetDrawMode:
		{
		assert(size == 2);
		// TODO: this probably sets whether points are always contained (0x10)
		// or whether the bitmap contents are checked (00, or anything else?)
		uint16 val = stream->readUint16();
		debug(2, "LBPictureItem: kLBSetDrawMode: %04x", val);
		}
		break;

	default:
		LBItem::readData(type, size, stream);
	}
}

bool LBPictureItem::contains(Common::Point point) {
	if (!LBItem::contains(point))
		return false;

	if (!_doHitTest)
		return true;

	// TODO: only check pixels if necessary
	return !_vm->_gfx->imageIsTransparentAt(_resourceId, false, point.x - _rect.left, point.y - _rect.top);
}

void LBPictureItem::init() {
	_vm->_gfx->preloadImage(_resourceId);

	LBItem::init();
}

void LBPictureItem::draw() {
	if (!_loaded || !_visible || !_globalVisible)
		return;

	_vm->_gfx->copyAnimImageToScreen(_resourceId, _rect.left, _rect.top);
}

LBItem *LBPictureItem::createClone() {
	return new LBPictureItem(_vm, _page, _rect);
}

LBMovieItem::LBMovieItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	debug(3, "new LBMovieItem");
}

LBMovieItem::~LBMovieItem() {
}

void LBMovieItem::update() {
	if (_playing) {
		VideoEntryPtr video = _vm->_video->findVideo(_resourceId);
		if (!video || video->endOfVideo())
			done(true);
	}

	LBItem::update();
}

bool LBMovieItem::togglePlaying(bool playing, bool restart) {
	if (playing) {
		if ((_loaded && _enabled && _globalEnabled) || _phase == kLBPhaseNone) {
			debug("toggled video for phase %d", _phase);
			VideoEntryPtr video = _vm->_video->playMovie(_resourceId);
			if (!video)
				error("Failed to open tMOV %d", _resourceId);

			video->moveTo(_rect.left, _rect.top);
			return true;
		}
	}

	return LBItem::togglePlaying(playing, restart);
}

LBItem *LBMovieItem::createClone() {
	return new LBMovieItem(_vm, _page, _rect);
}

LBMiniGameItem::LBMiniGameItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	debug(3, "new LBMiniGameItem");
}

LBMiniGameItem::~LBMiniGameItem() {
}

bool LBMiniGameItem::togglePlaying(bool playing, bool restart) {
	// HACK: Since we don't support any of these hardcoded mini games yet,
	// just skip to the most logical page. For optional minigames, this
	// will return the player to the previous page. For mandatory minigames,
	// this will send the player to the next page.

	uint16 destPage = 0;
	bool returnToMenu = false;

	// Figure out what minigame we have and bring us back to a page where
	// the player can continue
	if (_desc == "Kitch")     // Green Eggs and Ham: Kitchen minigame
		destPage = 4;
	else if (_desc == "Eggs") // Green Eggs and Ham: Eggs minigame
		destPage = 5;
	else if (_desc == "Fall") // Green Eggs and Ham: Fall minigame
		destPage = 13;
	else if (_desc == "MagicWrite3") // Arthur's Reading Race: "Let Me Write" minigame (Page 3)
		destPage = 3;
	else if (_desc == "MagicWrite4") // Arthur's Reading Race: "Let Me Write" minigame (Page 4)
		destPage = 4;
	else if (_desc == "MagicSpy5") // Arthur's Reading Race: "I Spy" minigame (Page 5)
		destPage = 5;
	else if (_desc == "MagicSpy6") // Arthur's Reading Race: "I Spy" minigame (Page 6)
		destPage = 6;
	else if (_desc == "MagicWrite7") // Arthur's Reading Race: "Let Me Write" minigame (Page 7)
		destPage = 7;
	else if (_desc == "MagicSpy8") // Arthur's Reading Race: "I Spy" minigame (Page 8)
		destPage = 8;
	else if (_desc == "MagicRace") // Arthur's Reading Race: Race minigame
		returnToMenu = true;
	else
		error("Unknown minigame '%s'", _desc.c_str());

	GUI::MessageDialog dialog(Common::String::format("The '%s' minigame is not supported yet.", _desc.c_str()));
	dialog.runModal();

	// Go back to the menu if requested, otherwise go to the requested page
	if (returnToMenu)
		_vm->addNotifyEvent(NotifyEvent(kLBNotifyGoToControls, 1));
	else
		_vm->addNotifyEvent(NotifyEvent(kLBNotifyChangePage, destPage));

	return false;
}

LBItem *LBMiniGameItem::createClone() {
	error("can't clone LBMiniGameItem");
}

LBProxyItem::LBProxyItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	debug(3, "new LBProxyItem");

	_page = nullptr;
}

LBProxyItem::~LBProxyItem() {
	delete _page;
}

void LBProxyItem::load() {
	if (_loaded)
		return;

	Common::String leftover;
	Common::String filename = _vm->getFileNameFromConfig("Proxies", _desc.c_str(), leftover);
	if (!leftover.empty())
		error("LBProxyItem tried loading proxy '%s' but got leftover '%s'", _desc.c_str(), leftover.c_str());
	uint16 baseId = 0;
	for (uint i = 0; i < filename.size(); i++) {
		if (filename[i] == ';') {
			baseId = atoi(filename.c_str() + i + 1);
			filename = Common::String(filename.c_str(), i);
		}
	}

	debug(1, "LBProxyItem loading archive '%s' with id %d", filename.c_str(), baseId);
	Archive *pageArchive = _vm->createArchive();
	if (!tryOpenPage(pageArchive, filename))
		error("failed to open archive '%s' (for proxy '%s')", filename.c_str(), _desc.c_str());
	_page = new LBPage(_vm);
	_page->open(pageArchive, baseId);

	LBItem::load();
}

void LBProxyItem::unload() {
	delete _page;
	_page = nullptr;

	LBItem::unload();
}

LBItem *LBProxyItem::createClone() {
	return new LBProxyItem(_vm, _page, _rect);
}

} // End of namespace Mohawk
