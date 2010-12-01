/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "mohawk/livingbooks.h"
#include "mohawk/resource.h"
#include "mohawk/cursors.h"

#include "common/events.h"
#include "common/EventRecorder.h"

#include "engines/util.h"

namespace Mohawk {

// read a null-terminated string from a stream
static Common::String readString(Common::SeekableSubReadStreamEndian *stream) {
	Common::String ret;
	while (!stream->eos()) {
		byte in = stream->readByte();
		if (!in)
			break;
		ret += in;
	}
	return ret;
}

// read a rect from a stream
Common::Rect MohawkEngine_LivingBooks::readRect(Common::SeekableSubReadStreamEndian *stream) {
	Common::Rect rect;

	// the V1 mac games have their rects in QuickDraw order
	if (getGameType() == GType_LIVINGBOOKSV1 && getPlatform() == Common::kPlatformMacintosh) {
		rect.top = stream->readSint16();
		rect.left = stream->readSint16();
		rect.bottom = stream->readSint16();
		rect.right = stream->readSint16();
	} else {
		rect.left = stream->readSint16();
		rect.top = stream->readSint16();
		rect.right = stream->readSint16();
		rect.bottom = stream->readSint16();
	}

	return rect;
}

MohawkEngine_LivingBooks::MohawkEngine_LivingBooks(OSystem *syst, const MohawkGameDescription *gamedesc) : MohawkEngine(syst, gamedesc) {
	_needsUpdate = false;
	_needsRedraw = false;
	_screenWidth = _screenHeight = 0;

	_curLanguage = 1;
	_curSelectedPage = 1;

	_alreadyShowedIntro = false;

	_rnd = new Common::RandomSource();
	g_eventRec.registerRandomSource(*_rnd, "livingbooks");
}

MohawkEngine_LivingBooks::~MohawkEngine_LivingBooks() {
	destroyPage();

	delete _console;
	delete _gfx;
	delete _rnd;
	_bookInfoFile.clear();
}

Common::Error MohawkEngine_LivingBooks::run() {
	MohawkEngine::run();

	_console = new LivingBooksConsole(this);
	// Load the book info from the detected file
	loadBookInfo(getBookInfoFileName());

	if (!_title.empty()) // Some games don't have the title stored
		debug("Starting Living Books Title \'%s\'", _title.c_str());
	if (!_copyright.empty())
		debug("Copyright: %s", _copyright.c_str());

	if (!_screenWidth || !_screenHeight)
		error("Could not find xRes/yRes variables");

	_gfx = new LBGraphics(this, _screenWidth, _screenHeight);

	if (getPlatform() == Common::kPlatformMacintosh)
		_cursor = new MacCursorManager(getAppName());
	else
		_cursor = new NECursorManager(getAppName());

	_cursor->setDefaultCursor();
	_cursor->showCursor();

	if (!loadPage(kLBIntroMode, 1, 0))
		error("Could not load intro page");

	Common::Event event;
	while (!shouldQuit()) {
		while (_eventMan->pollEvent(event)) {
			LBItem *found = NULL;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				_needsUpdate = true;
				break;

			case Common::EVENT_LBUTTONUP:
				if (_focus)
					_focus->handleMouseUp(event.mouse);
				break;

			case Common::EVENT_LBUTTONDOWN:
				for (uint16 i = 0; i < _items.size(); i++)
					if (_items[i]->contains(event.mouse))
						found = _items[i];

				if (found)
					found->handleMouseDown(event.mouse);
				break;

			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_d:
					if (event.kbd.flags & Common::KBD_CTRL) {
						_console->attach();
						_console->onFrame();
					}
					break;

				case Common::KEYCODE_SPACE:
					pauseGame();
					break;

				case Common::KEYCODE_ESCAPE:
					if (_curMode == kLBIntroMode)
						loadPage(kLBControlMode, 1, 0);
					break;

				case Common::KEYCODE_LEFT:
					prevPage();
					break;

				case Common::KEYCODE_RIGHT:
					nextPage();
					break;

				default:
					break;
				}
				break;

			default:
				break;
			}
		}

		updatePage();

		if (_needsUpdate) {
			_system->updateScreen();
			_needsUpdate = false;
		}

		// Cut down on CPU usage
		_system->delayMillis(10);

		// handle pending notifications
		while (_notifyEvents.size()) {
			NotifyEvent notifyEvent = _notifyEvents.pop();
			handleNotify(notifyEvent);
		}
	}

	return Common::kNoError;
}

void MohawkEngine_LivingBooks::loadBookInfo(const Common::String &filename) {
	if (!_bookInfoFile.loadFromFile(filename))
		error("Could not open %s as a config file", filename.c_str());

	_title = getStringFromConfig("BookInfo", "title");
	_copyright = getStringFromConfig("BookInfo", "copyright");

	_numPages = getIntFromConfig("BookInfo", "nPages");
	_numLanguages = getIntFromConfig("BookInfo", "nLanguages");
	_screenWidth = getIntFromConfig("BookInfo", "xRes");
	_screenHeight = getIntFromConfig("BookInfo", "yRes");
	// nColors is here too, but it's always 256 anyway...

	// this is 1 in The New Kid on the Block, changes the hardcoded UI
	_poetryMode = (getIntFromConfig("BookInfo", "poetry") == 1);

	// The later Living Books games add some more options:
	//     - fNeedPalette                (always true?)
	//     - fUse254ColorPalette         (always true?)
	//     - nKBRequired                 (4096, RAM requirement?)
	//     - fDebugWindow                (always 0?)
}

Common::String MohawkEngine_LivingBooks::stringForMode(LBMode mode) {
	Common::String language = getStringFromConfig("Languages", Common::String::format("Language%d", _curLanguage));

	switch (mode) {
	case kLBIntroMode:
		return "Intro";
	case kLBControlMode:
		return "Control";
	case kLBCreditsMode:
		return "Credits";
	case kLBPreviewMode:
		return "Preview";
	case kLBReadMode:
		return language + ".Read";
	case kLBPlayMode:
		return language + ".Play";
	default:
		error("unknown game mode %d", (int)mode);
	}
}

void MohawkEngine_LivingBooks::destroyPage() {
	_sound->stopSound();
	_gfx->clearCache();

	_eventQueue.clear();

	for (uint32 i = 0; i < _items.size(); i++)
		delete _items[i];
	_items.clear();

	for (uint32 i = 0; i < _mhk.size(); i++)
		delete _mhk[i];
	_mhk.clear();

	_notifyEvents.clear();

	_focus = NULL;
}

bool MohawkEngine_LivingBooks::loadPage(LBMode mode, uint page, uint subpage) {
	destroyPage();

	Common::String name = stringForMode(mode);

	Common::String base;
	if (subpage)
		base = Common::String::format("Page%d.%d", page, subpage);
	else
		base = Common::String::format("Page%d", page);

	Common::String filename;

	filename = getFileNameFromConfig(name, base);
	_readOnly = false;

	if (filename.empty()) {
		filename = getFileNameFromConfig(name, base + ".r");
		_readOnly = true;
	}

	// TODO: fading between pages
	bool fade = false;
	if (filename.hasSuffix(" fade")) {
		fade = true;
		filename = Common::String(filename.c_str(), filename.size() - 5);
	}

	MohawkArchive *pageArchive = createMohawkArchive();
	if (!filename.empty() && pageArchive->open(filename)) {
		_mhk.push_back(pageArchive);
	} else {
		delete pageArchive;
		debug(2, "Could not find page %d.%d for '%s'", page, subpage, name.c_str());
		return false;
	}

	if (getFeatures() & GF_LB_10) {
		if (_readOnly) {
			error("found .r entry in Living Books 1.0 game");
		} else {
			// some very early versions of the LB engine don't have
			// .r entries in their book info; instead, it is just hardcoded
			// like this (which would unfortunately break later games)
			_readOnly = (mode != kLBControlMode && mode != kLBPlayMode);
		}
	}

	debug(1, "Stack Version: %d", getResourceVersion());

	_curMode = mode;
	_curPage = page;
	_curSubPage = subpage;

	_cursor->showCursor();

	_gfx->setPalette(1000);
	loadBITL(1000);

	for (uint32 i = 0; i < _items.size(); i++)
		_items[i]->init();

	_phase = 0;
	_introDone = false;

	_needsRedraw = true;

	return true;
}

void MohawkEngine_LivingBooks::updatePage() {
	switch (_phase) {
	case 0:
		for (uint32 i = 0; i < _items.size(); i++)
			_items[i]->startPhase(_phase);

		if (_curMode == kLBControlMode) {
			// hard-coded control page startup
			LBItem *item;

			item = getItemById(10);
			if (item)
				item->togglePlaying(false);

			uint16 page = _curPage;
			if (getFeatures() & GF_LB_10) {
				// Living Books 1.0 had the meanings of these pages reversed
				if (page == 2)
					page = 3;
				else if (page == 3)
					page = 2;
			}

			switch (page) {
			case 1:
				debug(2, "updatePage() for control page 1 (menu)");

				for (uint16 i = 0; i < _numLanguages; i++) {
					item = getItemById(100 + i);
					if (item)
						item->seek((i + 1 == _curLanguage) ? 0xFFFF : 1);
					item = getItemById(200 + i);
					if (item)
						item->setVisible(false);
				}

				item = getItemById(12);
				if (item)
					item->setVisible(false);

				if (_alreadyShowedIntro) {
					item = getItemById(10);
					if (item) {
						item->setVisible(false);
						item->seek(0xFFFF);
					}
				} else {
					_alreadyShowedIntro = true;
					item = getItemById(11);
					if (item)
						item->setVisible(false);
				}
				break;

			case 2:
				debug(2, "updatePage() for control page 2 (quit)");

				item = getItemById(12);
				if (item)
					item->setVisible(false);
				item = getItemById(13);
				if (item)
					item->setVisible(false);
				break;

			case 3:
				debug(2, "updatePage() for control page 3 (options)");

				for (uint i = 0; i < _numLanguages; i++) {
					item = getItemById(100 + i);
					if (item)
						item->setVisible(_curLanguage == i + 1);
				}
				for (uint i = 0; i < _numPages; i++) {
					item = getItemById(1000 + i);
					if (item)
						item->setVisible(_curSelectedPage == i + 1);
					item = getItemById(1100 + i);
					if (item)
						item->setVisible(_curSelectedPage == i + 1);
				}
				item = getItemById(202);
				if (item)
					item->setVisible(false);
				break;
			}
		}
		_phase++;
		break;

	case 1:
		for (uint32 i = 0; i < _items.size(); i++)
			_items[i]->startPhase(_phase);

		_phase++;
		break;

	case 2:
		if (!_introDone)
			break;

		for (uint32 i = 0; i < _items.size(); i++)
			_items[i]->startPhase(_phase);

		_phase++;
		break;
	}

	while (_eventQueue.size()) {
		DelayedEvent delayedEvent = _eventQueue.pop();
		for (uint32 i = 0; i < _items.size(); i++) {
			if (_items[i] != delayedEvent.item)
				continue;

			switch (delayedEvent.type) {
			case kLBEventDestroy:
				_items.remove_at(i);
				delete delayedEvent.item;
				if (_focus == delayedEvent.item)
					_focus = NULL;
				break;
			case kLBEventSetNotVisible:
				_items[i]->setVisible(false);
				break;
			case kLBEventDone:
				_items[i]->done(true);
				break;
			}

			break;
		}
	}

	for (uint16 i = 0; i < _items.size(); i++)
		_items[i]->update();

	if (_needsRedraw) {
		for (uint16 i = 0; i < _items.size(); i++)
			_items[i]->draw();

		_needsUpdate = true;
	}
}

LBItem *MohawkEngine_LivingBooks::getItemById(uint16 id) {
	for (uint16 i = 0; i < _items.size(); i++)
		if (_items[i]->getId() == id)
			return _items[i];

	return NULL;
}

void MohawkEngine_LivingBooks::setFocus(LBItem *focus) {
	_focus = focus;
}

void MohawkEngine_LivingBooks::setEnableForAll(bool enable, LBItem *except) {
	for (uint16 i = 0; i < _items.size(); i++)
		if (except != _items[i])
			_items[i]->setEnabled(enable);
}

void MohawkEngine_LivingBooks::notifyAll(uint16 data, uint16 from) {
	for (uint16 i = 0; i < _items.size(); i++)
		_items[i]->notify(data, from);
}

void MohawkEngine_LivingBooks::queueDelayedEvent(DelayedEvent event) {
	_eventQueue.push(event);
}

// Only 1 VSRN resource per stack, Id 1000
uint16 MohawkEngine_LivingBooks::getResourceVersion() {
	Common::SeekableReadStream *versionStream = getResource(ID_VRSN, 1000);

	if (versionStream->size() != 2)
		warning("Version Record size mismatch");

	uint16 version = versionStream->readUint16BE();

	delete versionStream;
	return version;
}

void MohawkEngine_LivingBooks::loadBITL(uint16 resourceId) {
	Common::SeekableSubReadStreamEndian *bitlStream = wrapStreamEndian(ID_BITL, resourceId);

	while (true) {
		Common::Rect rect = readRect(bitlStream);
		uint16 type = bitlStream->readUint16();

		LBItem *res;
		switch (type) {
		case kLBPictureItem:
			res = new LBPictureItem(this, rect);
			break;
		case kLBAnimationItem:
			res = new LBAnimationItem(this, rect);
			break;
		case kLBPaletteItem:
			res = new LBPaletteItem(this, rect);
			break;
		case kLBGroupItem:
			res = new LBGroupItem(this, rect);
			break;
		case kLBSoundItem:
			res = new LBSoundItem(this, rect);
			break;
		case kLBLiveTextItem:
			res = new LBLiveTextItem(this, rect);
			break;
		default:
			warning("Unknown item type %04x", type);
		case 3: // often used for buttons
			res = new LBItem(this, rect);
			break;
		}

		res->readFrom(bitlStream);
		_items.push_back(res);

		if (bitlStream->size() == bitlStream->pos())
			break;
	}
}

Common::SeekableSubReadStreamEndian *MohawkEngine_LivingBooks::wrapStreamEndian(uint32 tag, uint16 id) {
	Common::SeekableReadStream *dataStream = getResource(tag, id);
	return new Common::SeekableSubReadStreamEndian(dataStream, 0, dataStream->size(), isBigEndian(), DisposeAfterUse::YES);
}

Common::String MohawkEngine_LivingBooks::getStringFromConfig(const Common::String &section, const Common::String &key) {
	Common::String x;
	_bookInfoFile.getKey(key, section, x);
	return removeQuotesFromString(x);
}

int MohawkEngine_LivingBooks::getIntFromConfig(const Common::String &section, const Common::String &key) {
	return atoi(getStringFromConfig(section, key).c_str());
}

Common::String MohawkEngine_LivingBooks::getFileNameFromConfig(const Common::String &section, const Common::String &key) {
	Common::String x = getStringFromConfig(section, key);
	return (getPlatform() == Common::kPlatformMacintosh) ? convertMacFileName(x) : convertWinFileName(x);
}

Common::String MohawkEngine_LivingBooks::removeQuotesFromString(const Common::String &string) {
	// The last char isn't necessarily a quote, the line could have "fade" in it
	// (which is then handled in loadPage).

	// Some versions wrap in quotations, some don't...
	Common::String tmp = string;
	for (uint32 i = 0; i < tmp.size(); i++) {
		if (tmp[i] == '\"') {
			tmp.deleteChar(i);
			i--;
		}
	}

	return tmp;
}

Common::String MohawkEngine_LivingBooks::convertMacFileName(const Common::String &string) {
	Common::String filename;

	for (uint32 i = 0; i < string.size(); i++) {
		if (i == 0 && string[i] == ':') // First character should be ignored (another colon)
			continue;
		if (string[i] == ':')
			filename += '/';
		else
			filename += string[i];
	}

	return filename;
}

Common::String MohawkEngine_LivingBooks::convertWinFileName(const Common::String &string) {
	Common::String filename;

	for (uint32 i = 0; i < string.size(); i++) {
		if (string[i] == '\\')
			filename += '/';
		else
			filename += string[i];
	}

	return filename;
}

MohawkArchive *MohawkEngine_LivingBooks::createMohawkArchive() const {
	return (getGameType() == GType_LIVINGBOOKSV1) ? new LivingBooksArchive_v1() : new MohawkArchive();
}

void MohawkEngine_LivingBooks::addNotifyEvent(NotifyEvent event) {
	_notifyEvents.push(event);
}

bool MohawkEngine_LivingBooks::tryLoadPageStart(LBMode mode, uint page) {
	// try first subpage of the page
	if (loadPage(mode, page, 1))
		return true;

	// then just the plain page
	if (loadPage(mode, page, 0))
		return true;

	return false;
}

bool MohawkEngine_LivingBooks::tryDefaultPage() {
	if (_curMode == kLBCreditsMode || _curMode == kLBPreviewMode) {
		// go to options page
		if (getFeatures() & GF_LB_10) {
			if (loadPage(kLBControlMode, 2, 0))
				return true;
		} else {
			if (loadPage(kLBControlMode, 3, 0))
				return true;
		}
	} else {
		// go to menu page
		if (loadPage(kLBControlMode, 1, 0))
			return true;
	}

	return false;
}

void MohawkEngine_LivingBooks::prevPage() {
	if (_curPage > 1 && (tryLoadPageStart(_curMode, _curPage - 1)))
		return;

	if (tryDefaultPage())
		return;

	error("Could not find page before %d.%d for mode %d", _curPage, _curSubPage, (int)_curMode);
}

void MohawkEngine_LivingBooks::nextPage() {
	// we try the next subpage first
	if (loadPage(_curMode, _curPage, _curSubPage + 1))
		return;

	if (tryLoadPageStart(_curMode, _curPage + 1))
		return;

	if (tryDefaultPage())
		return;

	error("Could not find page after %d.%d for mode %d", _curPage, _curSubPage, (int)_curMode);
}

void MohawkEngine_LivingBooks::handleNotify(NotifyEvent &event) {
	// hard-coded behavior (GUI/navigation)

	switch (event.type) {
	case kLBNotifyGUIAction:
		debug(2, "kLBNotifyGUIAction: %d", event.param);

		if (_curMode != kLBControlMode)
			break;

		// The scripting passes us the control ID as param, so we work
		// out which control was clicked, then run the relevant code.

		uint16 page;
		page = _curPage;
		if (getFeatures() & GF_LB_10) {
			// Living Books 1.0 had the meanings of these pages reversed
			if (page == 2)
				page = 3;
			else if (page == 3)
				page = 2;
		}

		LBItem *item;
		switch (page) {
		case 1:
			// main menu
			// TODO: poetry mode

			switch (event.param) {
			case 1:
				if (getFeatures() & GF_LB_10) {
					loadPage(kLBControlMode, 2, 0);
				} else {
					loadPage(kLBControlMode, 3, 0);
				}
				break;

			case 2:
				item = getItemById(10);
				if (item)
					item->destroySelf();
				item = getItemById(11);
				if (item)
					item->destroySelf();
				item = getItemById(199 + _curLanguage);
				if (item) {
					item->setVisible(true);
					item->togglePlaying(true);
				}
				break;

			case 3:
				item = getItemById(10);
				if (item)
					item->destroySelf();
				item = getItemById(11);
				if (item)
					item->destroySelf();
				item = getItemById(12);
				if (item) {
					item->setVisible(true);
					item->togglePlaying(true);
				}
				break;

			case 4:
				if (getFeatures() & GF_LB_10) {
					loadPage(kLBControlMode, 3, 0);
				} else {
					loadPage(kLBControlMode, 2, 0);
				}
				break;

			case 10:
				item = getItemById(10);
				if (item)
					item->destroySelf();
				item = getItemById(11);
				if (item)
					item->setVisible(true);
				if (item)
					item->togglePlaying(false);
				break;

			case 11:
				item = getItemById(11);
				if (item)
					item->togglePlaying(true);
				break;

			case 12:
				// start game, in play mode
				loadPage(kLBPlayMode, 1, 0);
				break;

			default:
				if (event.param >= 100 && event.param < 100 + (uint)_numLanguages) {
					uint newLanguage = event.param - 99;
					if (newLanguage == _curLanguage)
						break;
					item = getItemById(99 + _curLanguage);
					if (item)
						item->seek(1);
					_curLanguage = newLanguage;
				} else if (event.param >= 200 && event.param < 200 + (uint)_numLanguages) {
					// start game, in read mode
					loadPage(kLBReadMode, 1, 0);
				}
				break;
			}
			break;

		case 2:
			// quit screen

			switch (event.param) {
			case 1:
			case 2:
				// button clicked, run animation
				item = getItemById(10);
				if (item)
					item->destroySelf();
				item = getItemById(11);
				if (item)
					item->destroySelf();
				item = getItemById((event.param == 1) ? 12 : 13);
				if (item) {
					item->setVisible(true);
					item->togglePlaying(false);
				}
				break;

			case 10:
			case 11:
				item = getItemById(11);
				if (item)
					item->togglePlaying(true);
				break;

			case 12:
				// 'yes', I want to quit
				quitGame();
				break;

			case 13:
				// 'no', go back to menu
				loadPage(kLBControlMode, 1, 0);
				break;
			}
			break;

		case 3:
			// options screen

			switch (event.param) {
			case 1:
				item = getItemById(10);
				if (item)
					item->destroySelf();
				item = getItemById(202);
				if (item) {
					item->setVisible(true);
					item->togglePlaying(true);
				}
				break;

			case 2:
				// back
				item = getItemById(2);
				if (item)
					item->seek(1);
				if (_curSelectedPage == 1) {
					_curSelectedPage = _numPages;
				} else {
					_curSelectedPage--;
				}
				for (uint i = 0; i < _numPages; i++) {
					item = getItemById(1000 + i);
					if (item)
						item->setVisible(_curSelectedPage == i + 1);
					item = getItemById(1100 + i);
					if (item)
						item->setVisible(_curSelectedPage == i + 1);
				}
				break;

			case 3:
				// forward
				item = getItemById(3);
				if (item)
					item->seek(1);
				if (_curSelectedPage == _numPages) {
					_curSelectedPage = 1;
				} else {
					_curSelectedPage++;
				}
				for (uint i = 0; i < _numPages; i++) {
					item = getItemById(1000 + i);
					if (item)
						item->setVisible(_curSelectedPage == i + 1);
					item = getItemById(1100 + i);
					if (item)
						item->setVisible(_curSelectedPage == i + 1);
				}
				break;

			case 4:
				loadPage(kLBCreditsMode, 1, 0);
				break;

			case 5:
				loadPage(kLBPreviewMode, 1, 0);
				break;

			case 202:
				if (!tryLoadPageStart(kLBPlayMode, _curSelectedPage))
					error("failed to load page %d", _curSelectedPage);
				break;
			}
			break;
		}
		break;

	case kLBNotifyGoToControls:
		debug(2, "kLBNotifyGoToControls: %d", event.param);

		if (!loadPage(kLBControlMode, 1, 0))
			error("couldn't load controls page");
		break;

	case kLBNotifyChangePage:
		switch (event.param) {
		case 0xfffe:
			debug(2, "kLBNotifyChangePage: next page");
			nextPage();
			return;

		case 0xffff:
			debug(2, "kLBNotifyChangePage: previous page");
			prevPage();
			break;

		default:
			debug(2, "kLBNotifyChangePage: trying %d", event.param);
			if (!tryLoadPageStart(_curMode, event.param)) {
				if (!tryDefaultPage()) {
					error("failed to load default page after change to page %d (mode %d) failed", event.param, _curMode);
				}
			}
			break;
		}
		break;

	case kLBNotifyIntroDone:
		debug(2, "kLBNotifyIntroDone: %d", event.param);

		if (event.param != 1)
			break;

		_introDone = true;

		// TODO: if !_readOnly, go to next page (-2 case above)
		// if in older one (not in e.g. 1.4 w/tortoise),
		//   if mode is 6 (kLBPlayMode?), go to next page (-2 case) if curr page > nPages (i.e. the end)
		// else, nothing

		if (!_readOnly)
			break;

		nextPage();
		break;

	case kLBNotifyQuit:
		debug(2, "kLBNotifyQuit: %d", event.param);

		quitGame();
		break;

	case kLBNotifyCursorChange:
		debug(2, "kLBNotifyCursorChange: %d", event.param);

		// TODO: show/hide cursor according to parameter?
		break;

	default:
		error("Unknown notification %d (param 0x%04x)", event.type, event.param);
	}
}

LBAnimationNode::LBAnimationNode(MohawkEngine_LivingBooks *vm, LBAnimation *parent, uint16 scriptResourceId) : _vm(vm), _parent(parent) {
	_currentCel = 0;

	loadScript(scriptResourceId);
}

LBAnimationNode::~LBAnimationNode() {
	for (uint32 i = 0; i < _scriptEntries.size(); i++)
		delete[] _scriptEntries[i].data;
}

void LBAnimationNode::loadScript(uint16 resourceId) {
	Common::SeekableSubReadStreamEndian *scriptStream = _vm->wrapStreamEndian(ID_SCRP, resourceId);

	reset();

	while (byte opcodeId = scriptStream->readByte()) {
		byte size = scriptStream->readByte();

		LBAnimScriptEntry entry;
		entry.opcode = opcodeId;
		entry.size = size;

		if (!size) {
			entry.data = NULL;
		} else {
			entry.data = new byte[entry.size];
			scriptStream->read(entry.data, entry.size);
		}

		_scriptEntries.push_back(entry);
	}

	byte size = scriptStream->readByte();
	if (size != 0 || scriptStream->pos() != scriptStream->size())
		error("Failed to read script correctly");

	delete scriptStream;
}

void LBAnimationNode::draw(const Common::Rect &_bounds) {
	if (!_currentCel)
		return;

	// this is also checked in SetCel, below
	if (_currentCel > _parent->getNumResources())
		error("Animation cel %d was too high, this shouldn't happen!", _currentCel);

	int16 xOffset = _xPos + _bounds.left;
	int16 yOffset = _yPos + _bounds.top;

	uint16 resourceId = _parent->getResource(_currentCel - 1);

	if (_vm->getGameType() != GType_LIVINGBOOKSV1) {
		Common::Point offset = _parent->getOffset(_currentCel - 1);
		xOffset -= offset.x;
		yOffset -= offset.y;
	}

	_vm->_gfx->copyImageToScreen(resourceId, true, xOffset, yOffset);
}

void LBAnimationNode::reset() {
	// TODO: this causes stupid flickering
	//if (_currentCel)
	//	_vm->_needsRedraw = true;

	_currentCel = 0;
	_currentEntry = 0;

	_xPos = 0;
	_yPos = 0;
}

NodeState LBAnimationNode::update(bool seeking) {
	if (_currentEntry == _scriptEntries.size())
		return kLBNodeDone;

	while (_currentEntry < _scriptEntries.size()) {
		LBAnimScriptEntry &entry = _scriptEntries[_currentEntry];
		_currentEntry++;
		debug(5, "Running script entry %d of %d", _currentEntry, _scriptEntries.size());

		switch (entry.opcode) {
		case kLBAnimOpPlaySound:
		case kLBAnimOpWaitForSound:
		case kLBAnimOpReleaseSound:
		case kLBAnimOpResetSound:
			{
			uint16 soundResourceId = READ_BE_UINT16(entry.data);

			if (!soundResourceId) {
				error("Unhandled named wave file, tell clone2727 where you found this");
				break;
			}

			assert(entry.size == 4);
			uint16 strLen = READ_BE_UINT16(entry.data + 2);

			if (strLen)
				error("String length for unnamed wave file");

			switch (entry.opcode) {
			case kLBAnimOpPlaySound:
				if (seeking)
					break;
				debug(4, "a: PlaySound(%0d)", soundResourceId);
				_vm->_sound->playSound(soundResourceId);
				break;
			case kLBAnimOpWaitForSound:
				if (seeking)
					break;
				debug(4, "b: WaitForSound(%0d)", soundResourceId);
				if (!_vm->_sound->isPlaying(soundResourceId))
					break;
				_currentEntry--;
				return kLBNodeWaiting;
			case kLBAnimOpReleaseSound:
				debug(4, "c: ReleaseSound(%0d)", soundResourceId);
				// TODO
				_vm->_sound->stopSound(soundResourceId);
				break;
			case kLBAnimOpResetSound:
				debug(4, "d: ResetSound(%0d)", soundResourceId);
				// TODO
				_vm->_sound->stopSound(soundResourceId);
				break;
			}
			}
			break;

		case kLBAnimOpSetTempo:
		case kLBAnimOpUnknownE: // TODO: complete guesswork, not in 1.x
			{
			assert(entry.size == 2);
			uint16 tempo = (int16)READ_BE_UINT16(entry.data);

			debug(4, "3: SetTempo(%d)", tempo);
			if (entry.opcode == kLBAnimOpUnknownE) {
				debug(4, "(beware, stupid OpUnknownE guesswork)");
			}

			_parent->setTempo(tempo);
			}
			break;

		case kLBAnimOpWait:
			assert(entry.size == 0);
			debug(5, "6: Wait()");
			return kLBNodeRunning;

		case kLBAnimOpMoveTo:
			{
			assert(entry.size == 4);
			int16 x = (int16)READ_BE_UINT16(entry.data);
			int16 y = (int16)READ_BE_UINT16(entry.data + 2);
			debug(4, "5: MoveTo(%d, %d)", x, y);

			_xPos = x;
			_yPos = y;
			_vm->_needsRedraw = true;
			}
			break;

		case kLBAnimOpDrawMode:
			{
			assert(entry.size == 2);
			uint16 mode = (int16)READ_BE_UINT16(entry.data);
			debug(4, "9: DrawMode(%d)", mode);

			// TODO
			}
			break;

		case kLBAnimOpSetCel:
			{
			assert(entry.size == 2);
			uint16 cel = (int16)READ_BE_UINT16(entry.data);
			debug(4, "7: SetCel(%d)", cel);

			_currentCel = cel;
			if (_currentCel > _parent->getNumResources())
				error("SetCel set current cel to %d, but we only have %d cels", _currentCel, _parent->getNumResources());
			_vm->_needsRedraw = true;
			}
			break;

		case kLBAnimOpNotify:
			{
			assert(entry.size == 2);
			uint16 data = (int16)READ_BE_UINT16(entry.data);

			if (seeking)
				break;

			debug(4, "2: Notify(%d)", data);
			_vm->notifyAll(data, _parent->getParentId());
			}
			break;

		case kLBAnimOpSleepUntil:
			{
			assert(entry.size == 4);
			uint32 frame = READ_BE_UINT32(entry.data);
			debug(4, "8: SleepUntil(%d)", frame);

			if (frame > _parent->getCurrentFrame()) {
				// *not* kLBNodeWaiting
				_currentEntry--;
				return kLBNodeRunning;
			}
			}
			break;

		case kLBAnimOpUnknownF:
			// TODO: Found in maggiesfa
			// Seems to always be a uint32 as the data
			assert(entry.size == 4);
			warning("f: UnknownF(%d)", READ_BE_UINT32(entry.data));
			break;

		default:
			error("Unknown opcode id %02x (size %d)", entry.opcode, entry.size);
			break;
		}
	}

	return kLBNodeRunning;
}

bool LBAnimationNode::transparentAt(int x, int y) {
	if (!_currentCel)
		return true;

	uint16 resourceId = _parent->getResource(_currentCel - 1);

	if (_vm->getGameType() != GType_LIVINGBOOKSV1) {
		Common::Point offset = _parent->getOffset(_currentCel - 1);
		x += offset.x;
		y += offset.y;
	}

	// TODO: only check pixels if necessary
	return _vm->_gfx->imageIsTransparentAt(resourceId, true, x - _xPos, y - _yPos);
}

LBAnimation::LBAnimation(MohawkEngine_LivingBooks *vm, LBAnimationItem *parent, uint16 resourceId) : _vm(vm), _parent(parent) {
	Common::SeekableSubReadStreamEndian *aniStream = _vm->wrapStreamEndian(ID_ANI, resourceId);

	if (aniStream->size() != 30)
		warning("ANI Record size mismatch");

	uint16 version = aniStream->readUint16();
	if (version != 1)
		warning("ANI version not 1");

	_bounds = _vm->readRect(aniStream);
	_clip = _vm->readRect(aniStream);
	// TODO: what is colorId for?
	uint32 colorId = aniStream->readUint32();
	uint32 sprResourceId = aniStream->readUint32();
	uint32 sprResourceOffset = aniStream->readUint32();

	debug(5, "ANI bounds: (%d, %d), (%d, %d)", _bounds.left, _bounds.top, _bounds.right, _bounds.bottom);
	debug(5, "ANI clip: (%d, %d), (%d, %d)", _clip.left, _clip.top, _clip.right, _clip.bottom);
	debug(5, "ANI color id: %d", colorId);
	debug(5, "ANI SPRResourceId: %d, offset %d", sprResourceId, sprResourceOffset);

	if (aniStream->pos() != aniStream->size())
		error("Still %d bytes at the end of anim stream", aniStream->size() - aniStream->pos());

	delete aniStream;

	if (sprResourceOffset)
		error("Cannot handle non-zero ANI offset yet");

	Common::SeekableSubReadStreamEndian *sprStream = _vm->wrapStreamEndian(ID_SPR, sprResourceId);

	uint16 numBackNodes = sprStream->readUint16();
	uint16 numFrontNodes = sprStream->readUint16();
	uint32 shapeResourceID = sprStream->readUint32();
	uint32 shapeResourceOffset = sprStream->readUint32();
	uint32 scriptResourceID = sprStream->readUint32();
	uint32 scriptResourceOffset = sprStream->readUint32();
	uint32 scriptResourceLength = sprStream->readUint32();
	debug(5, "SPR# stream: %d front, %d background", numFrontNodes, numBackNodes);
	debug(5, "Shape ID %d (offset 0x%04x), script ID %d (offset 0x%04x, length %d)", shapeResourceID, shapeResourceOffset,
		scriptResourceID, scriptResourceOffset, scriptResourceLength);

	Common::Array<uint16> scriptIDs;
	for (uint16 i = 0; i < numFrontNodes; i++) {
		uint32 unknown1 = sprStream->readUint32();
		uint32 unknown2 = sprStream->readUint32();
		uint32 unknown3 = sprStream->readUint32();
		uint16 scriptID = sprStream->readUint32();
		uint32 unknown4 = sprStream->readUint32();
		uint32 unknown5 = sprStream->readUint32();
		scriptIDs.push_back(scriptID);
		debug(6, "Front node %d: script ID %d", i, scriptID);
		if (unknown1 != 0 || unknown2 != 0 || unknown3 != 0 || unknown4 != 0 || unknown5 != 0)
			error("Anim node %d had non-zero unknowns %08x, %08x, %08x, %08x, %08x",
				i, unknown1, unknown2, unknown3, unknown4, unknown5);
	}

	if (numBackNodes)
		error("Ignoring %d back nodes", numBackNodes);

	if (sprStream->pos() != sprStream->size())
		error("Still %d bytes at the end of sprite stream", sprStream->size() - sprStream->pos());

	delete sprStream;

	loadShape(shapeResourceID);

	_nodes.push_back(new LBAnimationNode(_vm, this, scriptResourceID));
	for (uint16 i = 0; i < scriptIDs.size(); i++)
		_nodes.push_back(new LBAnimationNode(_vm, this, scriptIDs[i]));

	_currentFrame = 0;
	_running = false;
	_tempo = 1;
}

LBAnimation::~LBAnimation() {
	for (uint32 i = 0; i < _nodes.size(); i++)
		delete _nodes[i];
}

void LBAnimation::loadShape(uint16 resourceId) {
	if (resourceId == 0)
		return;

	Common::SeekableSubReadStreamEndian *shapeStream = _vm->wrapStreamEndian(ID_SHP, resourceId);

	if (_vm->getGameType() == GType_LIVINGBOOKSV1) {
		if (shapeStream->size() < 6)
			error("V1 SHP Record size too short (%d)", shapeStream->size());

		uint16 u0 = shapeStream->readUint16();
		if (u0 != 3)
			error("V1 SHP Record u0 is %04x, not 3", u0);

		uint16 u1 = shapeStream->readUint16();
		if (u1 != 0)
			error("V1 SHP Record u1 is %04x, not 0", u1);

		uint16 idCount = shapeStream->readUint16();
		debug(8, "V1 SHP: idCount: %d", idCount);

		if (shapeStream->size() != (idCount * 2) + 6)
			error("V1 SHP Record size mismatch (%d)", shapeStream->size());

		for (uint16 i = 0; i < idCount; i++) {
			_shapeResources.push_back(shapeStream->readUint16());
			debug(8, "V1 SHP: BMAP Resource Id %d: %d", i, _shapeResources[i]);
		}
	} else {
		uint16 idCount = shapeStream->readUint16();
		debug(8, "SHP: idCount: %d", idCount);

		if (shapeStream->size() != (idCount * 6) + 2)
			error("SHP Record size mismatch (%d)", shapeStream->size());

		for (uint16 i = 0; i < idCount; i++) {
			_shapeResources.push_back(shapeStream->readUint16());
			int16 x = shapeStream->readSint16();
			int16 y = shapeStream->readSint16();
			_shapeOffsets.push_back(Common::Point(x, y));
			debug(8, "SHP: tBMP Resource Id %d: %d, at (%d, %d)", i, _shapeResources[i], x, y);
		}
	}

	for (uint16 i = 0; i < _shapeResources.size(); i++)
		_vm->_gfx->preloadImage(_shapeResources[i]);

	delete shapeStream;
}

void LBAnimation::draw() {
	for (uint32 i = 0; i < _nodes.size(); i++)
		_nodes[i]->draw(_bounds);
}

bool LBAnimation::update() {
	if (!_running)
		return false;

	if (_vm->_system->getMillis() / 16 <= _lastTime + (uint32)_tempo)
		return false;

	// the second check is to try 'catching up' with lagged animations, might be crazy
	if (_lastTime == 0 || (_vm->_system->getMillis() / 16) > _lastTime + (uint32)(_tempo * 2))
		_lastTime = _vm->_system->getMillis() / 16;
	else
		_lastTime += _tempo;

	NodeState state = kLBNodeDone;
	for (uint32 i = 0; i < _nodes.size(); i++) {
		NodeState s = _nodes[i]->update();
		if (s == kLBNodeWaiting) {
			state = kLBNodeWaiting;
			if (i != 0)
				warning("non-primary node was waiting");
			break;
		}
		if (s == kLBNodeRunning)
			state = kLBNodeRunning;
	}

	if (state == kLBNodeRunning) {
		_currentFrame++;
	} else if (state == kLBNodeDone) {
		_running = false;
		return true;
	}

	return false;
}

void LBAnimation::start() {
	_lastTime = 0;
	_running = true;
}

void LBAnimation::seek(uint16 pos) {
	_lastTime = 0;
	_currentFrame = 0;

	for (uint32 i = 0; i < _nodes.size(); i++)
		_nodes[i]->reset();

	for (uint16 n = 0; n < pos; n++) {
		bool ranSomething = false;
		// nodes don't wait while seeking
		for (uint32 i = 0; i < _nodes.size(); i++)
			ranSomething |= (_nodes[i]->update(true) != kLBNodeDone);

		_currentFrame++;

		if (!ranSomething) {
			_running = false;
			break;
		}
	}
}

void LBAnimation::stop() {
	_running = false;
}

bool LBAnimation::transparentAt(int x, int y) {
	for (uint32 i = 0; i < _nodes.size(); i++)
		if (!_nodes[i]->transparentAt(x - _bounds.left, y - _bounds.top))
			return false;

	return true;
}

void LBAnimation::setTempo(uint16 tempo) {
	_tempo = tempo;
}

uint16 LBAnimation::getParentId() {
	return _parent->getId();
}

LBScriptEntry::LBScriptEntry() {
	argvParam = NULL;
	argvTarget = NULL;
}

LBScriptEntry::~LBScriptEntry() {
	delete[] argvParam;
	delete[] argvTarget;
}

LBItem::LBItem(MohawkEngine_LivingBooks *vm, Common::Rect rect) : _vm(vm), _rect(rect) {
	_phase = 0;
	_timingMode = 0;
	_delayMin = 0;
	_delayMax = 0;
	_loopMode = 0;
	_loopCount = 0;
	_periodMin = 0;
	_periodMax = 0;
	_controlMode = 0;

	_neverEnabled = true;
	_enabled = false;
	_visible = true;
	_playing = false;
	_nextTime = 0;
	_startTime = 0;
	_loops = 0;
}

LBItem::~LBItem() {
	for (uint i = 0; i < _scriptEntries.size(); i++)
		delete _scriptEntries[i];
}

void LBItem::readFrom(Common::SeekableSubReadStreamEndian *stream) {
	_resourceId = stream->readUint16();
	_itemId = stream->readUint16();
	uint16 size = stream->readUint16();
	_desc = readString(stream);

	debug(2, "Item: size %d, resource %d, id %d", size, _resourceId, _itemId);
	debug(2, "Coords: %d, %d, %d, %d", _rect.left, _rect.top, _rect.right, _rect.bottom);
	debug(2, "String: '%s'", _desc.c_str());

	if (!_itemId)
		error("Item had invalid item id");

	int endPos = stream->pos() + size;
	if (endPos > stream->size())
		error("Item is larger (should end at %d) than stream (size %d)", endPos, stream->size());

	while (true) {
		if (stream->pos() == endPos)
			break;

		uint16 dataType = stream->readUint16();
		uint16 dataSize = stream->readUint16();

		debug(4, "Data type %04x, size %d", dataType, dataSize);
		readData(dataType, dataSize, stream);

		if (stream->pos() > endPos)
			error("Read off the end (at %d) of data (ends at %d)", stream->pos(), endPos);

		assert(!stream->eos());
	}
}

void LBItem::readData(uint16 type, uint16 size, Common::SeekableSubReadStreamEndian *stream) {
	switch (type) {
	case kLBMsgListScript:
	case kLBNotifyScript:
		{
			if (size < 6)
				error("Script entry of type 0x%04x was too small (%d)", type, size);

			LBScriptEntry *entry = new LBScriptEntry;
			entry->type = type;
			entry->action = stream->readUint16();
			entry->opcode = stream->readUint16();
			entry->param = stream->readUint16();
			debug(4, "Script entry: type 0x%04x, action 0x%04x, opcode 0x%04x, param 0x%04x",
				entry->type, entry->action, entry->opcode, entry->param);

			if (type == kLBMsgListScript) {
				if (size < 8)
					error("Script entry of type 0x%04x was too small (%d)", type, size);

				entry->argc = stream->readUint16();
				entry->argvParam = new uint16[entry->argc];
				entry->argvTarget = new uint16[entry->argc];
				debug(4, "With %d targets:", entry->argc);

				if (size < (8 + entry->argc * 4))
					error("Script entry of type 0x%04x was too small (%d)", type, size);

				for (uint i = 0; i < entry->argc; i++) {
					entry->argvParam[i] = stream->readUint16();
					entry->argvTarget[i] = stream->readUint16();
					debug(4, "Target %d, param 0x%04x", entry->argvTarget[i], entry->argvParam[i]);
				}

				if (size > (8 + entry->argc * 4)) {
					// TODO
					warning("Skipping %d probably-important bytes", size - (8 + entry->argc * 4));
					stream->skip(size - (8 + entry->argc * 4));
				}
			} else {
				if (size > 6) {
					// TODO
					warning("Skipping %d probably-important bytes", size - 6);
					stream->skip(size - 6);
				}
			}

			_scriptEntries.push_back(entry);
		}
		break;

	case kLBSetPlayInfo:
		{
		if (size != 20)
			error("kLBSetPlayInfo had wrong size (%d)", size);

		_loopMode = stream->readUint16();
		_delayMin = stream->readUint16();
		_delayMax = stream->readUint16();
		_timingMode = stream->readUint16();
		_periodMin = stream->readUint16();
		_periodMax = stream->readUint16();
		_relocPoint.x = stream->readSint16();
		_relocPoint.y = stream->readSint16();
		_controlMode = stream->readUint16();
		uint16 unknown10 = stream->readUint16();
		// TODO: unknowns

		debug(2, "kLBSetPlayInfo: loop mode %d (%d to %d), timing mode %d (%d to %d), reloc (%d, %d), unknowns %04x, %04x",
			_loopMode, _delayMin, _delayMax,
			_timingMode, _periodMin, _periodMax,
			_relocPoint.x, _relocPoint.y,
			_controlMode, unknown10);
		}
		break;

	case kLBSetPlayPhase:
		if (size != 2)
			error("SetPlayPhase had wrong size (%d)", size);
		_phase = stream->readUint16();
		break;

	case 0x7b:
		assert(size == 0);
		debug(2, "LBItem: 0x7b");
		// TODO
		break;

	case kLBCommand:
		{
			Common::String command = readString(stream);
			if (size != command.size() + 1)
				error("failed to read command string");
			warning("ignoring command '%s'", command.c_str());
		}
		break;

	case 0x69:
		// TODO: ??
	case 0x6a:
		// TODO: ??
	case 0x6d:
		// TODO: one-shot?
	default:
		for (uint i = 0; i < size; i++)
			debugN("%02x ", stream->readByte());
		warning("Unknown message %04x (size 0x%04x)", type, size);
		break;
	}
}

void LBItem::destroySelf() {
	if (!this->_itemId)
		error("destroySelf() on an item which was already dead");

	_vm->queueDelayedEvent(DelayedEvent(this, kLBEventDestroy));

	_itemId = 0;
}

void LBItem::setEnabled(bool enabled) {
	if (enabled && _neverEnabled && !_playing) {
		if (_timingMode == 2) {
			setNextTime(_periodMin, _periodMax);
			debug(2, "Enable time startup");
		}
	}

	_neverEnabled = false;
	_enabled = enabled;
}

bool LBItem::contains(Common::Point point) {
	if (_playing && _loopMode == 0xFFFF)
		stop();

	if (!_playing && _timingMode == 2)
		setNextTime(_periodMin, _periodMax);

	return _visible && _rect.contains(point);
}

void LBItem::update() {
	if (_neverEnabled || !_enabled)
		return;

	if (_nextTime == 0 || _nextTime > (uint32)(_vm->_system->getMillis() / 16))
		return;

	if (togglePlaying(_playing, true)) {
		_nextTime = 0;
	} else if (_loops == 0 && _timingMode == 2) {
		debug(9, "Looping in update()");
		setNextTime(_periodMin, _periodMax);
	}
}

void LBItem::handleMouseDown(Common::Point pos) {
	if (_neverEnabled || !_enabled)
		return;

	_vm->setFocus(this);
	runScript(kLBActionMouseDown);
}

void LBItem::handleMouseMove(Common::Point pos) {
	// TODO: handle drag
}

void LBItem::handleMouseUp(Common::Point pos) {
	_vm->setFocus(NULL);
	runScript(kLBActionMouseUp);
}

bool LBItem::togglePlaying(bool playing, bool restart) {
	if (playing) {
		_vm->queueDelayedEvent(DelayedEvent(this, kLBEventDone));
		return true;
	}
	if (!_neverEnabled && _enabled && !_playing) {
		_playing = togglePlaying(true, restart);
		if (_playing) {
			_nextTime = 0;
			_startTime = _vm->_system->getMillis() / 16;

			if (_loopMode == 0xFFFF || _loopMode == 0xFFFE)
				_loops = 0xFFFF;
			else
				_loops = _loopMode;

			if (_controlMode >= 1) {
				debug(2, "Hiding cursor");
				_vm->_cursor->hideCursor();
				// TODO: lock sound?

				if (_controlMode >= 2) {
					debug(2, "Disabling all");
					_vm->setEnableForAll(false, this);
				}
			}

			runScript(kLBActionStarted);
			notify(0, _itemId);
		}
	}
	return _playing;
}

void LBItem::done(bool onlyNotify) {
	if (onlyNotify) {
		if (_relocPoint.x || _relocPoint.y) {
			_rect.translate(_relocPoint.x, _relocPoint.y);
			// TODO: does drag box need adjusting?
		}

		if (_loops && --_loops) {
			debug(9, "Real looping (now 0x%04x left)", _loops);
			setNextTime(_delayMin, _delayMax, _startTime);
		} else
			done(false);

		return;
	}

	_playing = false;
	_loops = 0;
	_startTime = 0;

	if (_controlMode >= 1) {
		debug(2, "Showing cursor");
		_vm->_cursor->showCursor();
		// TODO: unlock sound?

		if (_controlMode >= 2) {
			debug(2, "Enabling all");
			_vm->setEnableForAll(true, this);
		}
	}

	if (_timingMode == 2) {
		debug(9, "Looping in done() - %d to %d", _periodMin, _periodMax);
		setNextTime(_periodMin, _periodMax);
	}

	runScript(kLBActionDone);
	notify(0xFFFF, _itemId);
}

void LBItem::setVisible(bool visible) {
	if (visible == _visible)
		return;

	_visible = visible;
	_vm->_needsRedraw = true;
}

void LBItem::startPhase(uint phase) {
	if (_phase == phase)
		setEnabled(true);

	switch (phase) {
	case 0:
		runScript(kLBActionPhase0);
		break;
	case 1:
		runScript(kLBActionPhase1);
		if (_timingMode == 1 || _timingMode == 2) {
			debug(2, "Phase 1 time startup");
			setNextTime(_periodMin, _periodMax);
		}
		break;
	case 2:
		runScript(kLBActionPhase2);
		if (_timingMode == 2 || _timingMode == 3) {
			debug(2, "Phase 2 time startup");
			setNextTime(_periodMin, _periodMax);
		}
		break;
	}
}

void LBItem::stop() {
	if (!_playing)
		return;

	_loops = 0;
	seek(0xFFFF);
	done(true);
}

void LBItem::notify(uint16 data, uint16 from) {
	if (_timingMode != 4)
		return;

	// TODO: is this correct?
	if (_periodMin != from)
		return;
	if (_periodMax != data)
		return;

	debug(2, "Handling notify 0x%04x (from %d)", data, from);
	setNextTime(0, 0);
}

void LBItem::runScript(uint id) {
	for (uint i = 0; i < _scriptEntries.size(); i++) {
		LBScriptEntry *entry = _scriptEntries[i];
		if (entry->action != id)
			continue;

		if (entry->type == kLBNotifyScript) {
			if (entry->opcode == kLBNotifyGUIAction)
				_vm->addNotifyEvent(NotifyEvent(entry->opcode, _itemId));
			else
				_vm->addNotifyEvent(NotifyEvent(entry->opcode, entry->param));
		} else {
			if (entry->param != 0xffff) {
				// TODO: if param is 1/2/3..
				warning("Ignoring script entry (type 0x%04x, action 0x%04x, opcode 0x%04x, param 0x%04x)",
					entry->type, entry->action, entry->opcode, entry->param);
				continue;
			}

			for (uint n = 0; n < entry->argc; n++) {
				uint16 targetId = entry->argvTarget[n];
				// TODO: is this type, perhaps?
				uint16 param = entry->argvParam[n];
				LBItem *target = _vm->getItemById(targetId);
			
				debug(2, "Script run: type 0x%04x, action 0x%04x, opcode 0x%04x, param 0x%04x, target id %d",
					entry->type, entry->action, entry->opcode, entry->param, targetId);
			
				if (!target)
					continue;

				switch (entry->opcode) {
				case 1:
					// TODO: should be setVisible(true) - not a delayed event -
					// when we're doing the param 1/2/3 stuff above?
					// and in modern LB this is perhaps just a direct target->setVisible(true)..
					_vm->queueDelayedEvent(DelayedEvent(this, kLBEventSetNotVisible));
					break;

				case 2:
					target->togglePlaying(false);
					break;

				case 3:
					target->setVisible(false);
					break;

				case 4:
					target->setVisible(true);
					break;

				case 5:
					target->destroySelf();
					break;

				case 6:
					target->seek(1);
					break;

				case 7:
					target->stop();
					break;

				case 8:
					target->setEnabled(false);
					break;

				case 9:
					target->setEnabled(true);
					break;

				case 0xf: // apply palette? seen in greeneggs
				default:
					// TODO
					warning("Ignoring script entry (type 0x%04x, action 0x%04x, opcode 0x%04x, param 0x%04x) for %d (param %04x)",
					entry->type, entry->action, entry->opcode, entry->param, targetId, param);
				}
			}
		}
	}
}

void LBItem::setNextTime(uint16 min, uint16 max) {
	setNextTime(min, max, _vm->_system->getMillis() / 16);
}

void LBItem::setNextTime(uint16 min, uint16 max, uint32 start) {
	_nextTime = start + _vm->_rnd->getRandomNumberRng((uint)min, (uint)max);
	debug(9, "nextTime is now %d frames away", _nextTime - (uint)(_vm->_system->getMillis() / 16));
}

LBSoundItem::LBSoundItem(MohawkEngine_LivingBooks *vm, Common::Rect rect) : LBItem(vm, rect) {
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

	if (_neverEnabled || !_enabled)
		return false;

	_running = true;
	_vm->_sound->playSound(_resourceId, Audio::Mixer::kMaxChannelVolume, false);
	return true;
}

void LBSoundItem::stop() {
	if (_running) {
		_running = false;
		_vm->_sound->stopSound(_resourceId);
	}

	LBItem::stop();
}

LBGroupItem::LBGroupItem(MohawkEngine_LivingBooks *vm, Common::Rect rect) : LBItem(vm, rect) {
	debug(3, "new LBGroupItem");
	_starting = false;
}

void LBGroupItem::readData(uint16 type, uint16 size, Common::SeekableSubReadStreamEndian *stream) {
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
			_groupEntries.push_back(entry);
			debug(3, "group entry: id %d, type %d", entry.entryId, entry.entryType);
		}
		}
		break;

	default:
		LBItem::readData(type, size, stream);
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

LBPaletteItem::LBPaletteItem(MohawkEngine_LivingBooks *vm, Common::Rect rect) : LBItem(vm, rect) {
	debug(3, "new LBPaletteItem");
}

void LBPaletteItem::readData(uint16 type, uint16 size, Common::SeekableSubReadStreamEndian *stream) {
	switch (type) {
	case 0x72:
		{
		assert(size == 4 + 256 * 4);
		// TODO
		_start = stream->readUint16();
		_count = stream->readUint16();
		stream->read(_palette, 256 * 4);
		}
		break;

	case 0x75:
		assert(size == 0);
		debug(2, "LBPaletteItem: 0x75");
		// TODO
		break;

	default:
		LBItem::readData(type, size, stream);
	}
}

void LBPaletteItem::startPhase(uint phase) {
	//if (_phase != phase)
	//	return;

	/*printf("palette: start %d, count %d\n", _start, _count);
	byte *localpal = _palette;
	for (unsigned int i = 0; i < 256 * 4; i++) {
		printf("%02x ", *localpal++);
	}
	printf("\n");*/

	// TODO: huh?
	if (_start != 1)
		return;

	// TODO
	//_vm->_system->setPalette(_start - 1, _count - (_start - 1), _palette + (_start * 4));
	_vm->_system->setPalette(_palette + _start * 4, 0, 256 - _start);
}

LBLiveTextItem::LBLiveTextItem(MohawkEngine_LivingBooks *vm, Common::Rect rect) : LBItem(vm, rect) {
	_currentPhrase = 0xFFFF;
	_currentWord = 0xFFFF;
	debug(3, "new LBLiveTextItem");
}

void LBLiveTextItem::readData(uint16 type, uint16 size, Common::SeekableSubReadStreamEndian *stream) {
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
			// TODO: unknowns
			uint16 unknown1 = stream->readUint16();
			uint16 unknown2 = stream->readUint16();
			debug(4, "Word: (%d, %d) to (%d, %d), sound %d, unknowns %04x, %04x",
				word.bounds.left, word.bounds.top, word.bounds.right, word.bounds.bottom, word.soundId, unknown1, unknown2);
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
	if (_resourceId) {
		// with a resource, we draw a bitmap in draw() rather than changing the palette
		_vm->_needsRedraw = true;
		return;
	}

	if (on) {
		_vm->_system->setPalette(_highlightColor, _paletteIndex + word, 1);
	} else {
		_vm->_system->setPalette(_foregroundColor, _paletteIndex + word, 1);
	}
}

void LBLiveTextItem::update() {
	if (_currentWord != 0xFFFF) {
		uint16 soundId = _words[_currentWord].soundId;
		if (soundId && !_vm->_sound->isPlaying(soundId)) {
			paletteUpdate(_currentWord, false);
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
	_vm->_gfx->copyImageSectionToScreen(_resourceId, srcRect, dstRect);
}

void LBLiveTextItem::handleMouseDown(Common::Point pos) {
	if (_neverEnabled || !_enabled || _currentPhrase != 0xFFFF)
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
			_vm->_sound->playSound(soundId);
			paletteUpdate(_currentWord, true);
			return;
		}
	}

	return LBItem::handleMouseDown(pos);
}

bool LBLiveTextItem::togglePlaying(bool playing, bool restart) {
	if (!playing)
		return LBItem::togglePlaying(playing, restart);
	if (_neverEnabled || !_enabled)
		return (_currentPhrase != 0xFFFF);

	// TODO: handle this properly
	_vm->_sound->stopSound();

	_currentWord = 0xFFFF;
	_currentPhrase = 0;

	return true;
}

void LBLiveTextItem::stop() {
	// TODO: stop sound, refresh palette

	LBItem::stop();
}

void LBLiveTextItem::notify(uint16 data, uint16 from) {
	if (_neverEnabled || !_enabled || _currentPhrase == 0xFFFF)
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
			if (i == _phrases.size() - 1)
				_currentPhrase = 0xFFFF;
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

LBPictureItem::LBPictureItem(MohawkEngine_LivingBooks *vm, Common::Rect rect) : LBItem(vm, rect) {
	debug(3, "new LBPictureItem");
}

void LBPictureItem::readData(uint16 type, uint16 size, Common::SeekableSubReadStreamEndian *stream) {
	switch (type) {
	case 0x6b:
		{
		assert(size == 2);
		// TODO: this probably sets whether points are always contained (0x10)
		// or whether the bitmap contents are checked (00, or anything else?)
		uint16 val = stream->readUint16();
		debug(2, "LBPictureItem: 0x6b: %04x", val);
		}
		break;

	default:
		LBItem::readData(type, size, stream);
	}
}

bool LBPictureItem::contains(Common::Point point) {
	if (!LBItem::contains(point))
		return false;

	// TODO: only check pixels if necessary
	return !_vm->_gfx->imageIsTransparentAt(_resourceId, false, point.x - _rect.left, point.y - _rect.top);
}

void LBPictureItem::init() {
	_vm->_gfx->preloadImage(_resourceId);
}

void LBPictureItem::draw() {
	if (!_visible)
		return;

	_vm->_gfx->copyImageToScreen(_resourceId, false, _rect.left, _rect.top);
}

LBAnimationItem::LBAnimationItem(MohawkEngine_LivingBooks *vm, Common::Rect rect) : LBItem(vm, rect) {
	_anim = NULL;
	_running = false;
	debug(3, "new LBAnimationItem");
}

LBAnimationItem::~LBAnimationItem() {
	// TODO: handle this properly
	if (_running)
		_vm->_sound->stopSound();

	delete _anim;
}

void LBAnimationItem::setEnabled(bool enabled) {
	if (_running) {
		if (enabled && _neverEnabled)
			_anim->start();
		else if (!_neverEnabled && !enabled && _enabled)
			if (_running) {
				_anim->stop();

				// TODO: handle this properly
				_vm->_sound->stopSound();
			}
	}

	return LBItem::setEnabled(enabled);
}

bool LBAnimationItem::contains(Common::Point point) {
	return LBItem::contains(point) && !_anim->transparentAt(point.x, point.y);
}

void LBAnimationItem::update() {
	if (!_neverEnabled && _enabled && _running) {
		bool wasDone = _anim->update();
		if (wasDone)
			done(true);
	}

	LBItem::update();
}

bool LBAnimationItem::togglePlaying(bool playing, bool restart) {
	if (playing) {
		if (!_neverEnabled && _enabled) {
			if (restart)
				seek(1);
			_running = true;
			_anim->start();
		}

		return _running;
	}

	return LBItem::togglePlaying(playing, restart);
}

void LBAnimationItem::done(bool onlyNotify) {
	if (!onlyNotify) {
		_anim->stop();
	}

	LBItem::done(onlyNotify);
}

void LBAnimationItem::init() {
	_anim = new LBAnimation(_vm, this, _resourceId);
}

void LBAnimationItem::stop() {
	if (_running) {
		_anim->stop();
		seek(0xFFFF);
	}

	// TODO: handle this properly
	_vm->_sound->stopSound();

	_running = false;

	LBItem::stop();
}

void LBAnimationItem::seek(uint16 pos) {
	_anim->seek(pos);
}

void LBAnimationItem::startPhase(uint phase) {
	if (phase == _phase)
		seek(1);

	LBItem::startPhase(phase);
}

void LBAnimationItem::draw() {
	if (!_visible)
		return;

	_anim->draw();
}

} // End of namespace Mohawk
