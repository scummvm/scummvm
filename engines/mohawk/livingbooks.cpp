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
#include "mohawk/cursors.h"
#include "mohawk/video.h"

#include "common/config-manager.h"
#include "common/error.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/archive.h"
#include "common/textconsole.h"
#include "common/system.h"
#include "common/memstream.h"

#include "graphics/paletteman.h"

#include "engines/util.h"

#include "gui/message.h"

#include "graphics/cursorman.h"

namespace Mohawk {

// read a null-terminated string from a stream
Common::String MohawkEngine_LivingBooks::readString(Common::ReadStream *stream) {
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
Common::Rect MohawkEngine_LivingBooks::readRect(Common::ReadStreamEndian *stream) {
	Common::Rect rect;

	// the V1 mac games have their rects in QuickDraw order
	if (isPreMohawk() && getPlatform() == Common::kPlatformMacintosh) {
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
	DebugMan.addDebugChannel(kDebugCode, "Code", "Track Script Execution");

	_needsUpdate = false;
	_needsRedraw = false;
	_screenWidth = _screenHeight = 0;

	_curLanguage = 1;
	_curSelectedPage = 1;

	_alreadyShowedIntro = false;

	_rnd = new Common::RandomSource("livingbooks");

	_sound = nullptr;
	_video = nullptr;
	_page = nullptr;

	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	// Rugrats
	SearchMan.addSubDirectoryMatching(gameDataDir, "program", 0, 2);
	SearchMan.addSubDirectoryMatching(gameDataDir, "Rugrats Adventure Game", 0, 2);
	// CarmenTQ
	SearchMan.addSubDirectoryMatching(gameDataDir, "95instal", 0, 4);

	// Sheila Rae, the Brave (Europe version) contains a junk line (bug #13920) 
	_bookInfoFile.requireKeyValueDelimiter();
}

MohawkEngine_LivingBooks::~MohawkEngine_LivingBooks() {
	destroyPage();

	delete _sound;
	delete _video;
	delete _gfx;
	delete _rnd;
	_bookInfoFile.clear();
}

Common::Error MohawkEngine_LivingBooks::run() {
	MohawkEngine::run();

	if (!_mixer->isReady()) {
		return Common::kAudioDeviceInitFailed;
	}

	setDebugger(new LivingBooksConsole(this));
	// Load the book info from the detected file
	loadBookInfo(getBookInfoFileName());

	if (!_title.empty()) // Some games don't have the title stored
		debug("Starting Living Books Title \'%s\'", _title.c_str());
	if (!_copyright.empty())
		debug("Copyright: %s", _copyright.c_str());
	debug("This book has %d page(s) in %d language(s).", _numPages, _numLanguages);
	if (_poetryMode)
		debug("Running in poetry mode.");

	if (!_screenWidth || !_screenHeight)
		error("Could not find xRes/yRes variables");

	_gfx = new LBGraphics(this, _screenWidth, _screenHeight);
	_video = new VideoManager(this);
	_sound = new Sound(this);

	if (getGameType() != GType_LIVINGBOOKSV1)
		_cursor = new LivingBooksCursorManager_v2();
	else if (getPlatform() == Common::kPlatformMacintosh)
		_cursor = new MacCursorManager(getAppName());
	else
		_cursor = new NECursorManager(getAppName());

	_cursor->setDefaultCursor();
	_cursor->showCursor();

	if (!tryLoadPageStart(kLBIntroMode, 1))
		error("Could not load intro page");

	Common::Event event;
	while (!shouldQuit()) {
		while (_eventMan->pollEvent(event)) {
			LBItem *found = nullptr;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				_needsUpdate = true;
				break;

			case Common::EVENT_LBUTTONUP:
				if (_focus)
					_focus->handleMouseUp(event.mouse);
				break;

			case Common::EVENT_LBUTTONDOWN:
				for (Common::List<LBItem *>::const_iterator i = _orderedItems.begin(); i != _orderedItems.end(); ++i) {
					if ((*i)->contains(event.mouse)) {
						found = *i;
						break;
					}
				}

				if (found && CursorMan.isVisible())
					found->handleMouseDown(event.mouse);
				break;

			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_SPACE:
					pauseGame();
					break;

				case Common::KEYCODE_ESCAPE:
					if (_curMode == kLBIntroMode)
						tryLoadPageStart(kLBControlMode, 1);
					else
						_video->stopVideos();
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

		if (_video->updateMovies())
			_needsUpdate = true;

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

void MohawkEngine_LivingBooks::pauseEngineIntern(bool pause) {
	MohawkEngine::pauseEngineIntern(pause);

	if (pause) {
		if (_video != nullptr) {
			_video->pauseVideos();
		}
	} else {
		if (_video != nullptr) {
			_video->resumeVideos();
		}
		_system->updateScreen();
	}
}

void MohawkEngine_LivingBooks::destroyPage() {
	_sound->stopSound();
	_lastSoundOwner = 0;
	_lastSoundId = 0;
	_soundLockOwner = 0;

	_gfx->clearCache();
	_video->stopVideos();

	_eventQueue.clear();

	delete _page;
	assert(_items.empty());
	assert(_orderedItems.empty());
	_page = nullptr;

	_notifyEvents.clear();

	_focus = nullptr;
}

// Replace any colons (originally a slash) with another character
static Common::String replaceColons(const Common::String &in, char replace) {
	Common::String out;

	for (uint32 i = 0; i < in.size(); i++) {
		if (in[i] == ':')
			out += replace;
		else
			out += in[i];
	}

	return out;
}

// Helper function to assist in opening pages
bool tryOpenPage(Archive *archive, const Common::String &fileName) {
	// Try the plain file name first
	if (archive->openFile(Common::Path(fileName)))
		return true;

	// No colons, then bail out
	if (!fileName.contains(':'))
		return false;

	// Try replacing colons with underscores (in case the original was
	// a Mac version and had slashes not as a separator).
	if (archive->openFile(Common::Path(replaceColons(fileName, '_'))))
		return true;

	// Try replacing colons with slashes (in case the original was a Mac
	// version and had slashes as a separator).
	if (archive->openFile(Common::Path(replaceColons(fileName, '/'))))
		return true;

	// Failed to open the archive
	return false;
}

bool MohawkEngine_LivingBooks::loadPage(LBMode mode, uint page, uint subpage) {
	destroyPage();

	Common::String name = stringForMode(mode);

	Common::String base;
	if (subpage)
		base = Common::String::format("Page%d.%d", page, subpage);
	else
		base = Common::String::format("Page%d", page);

	Common::String filename, leftover;

	filename = getFileNameFromConfig(name, base, leftover);
	_readOnly = false;

	if (filename.empty()) {
		leftover.clear();
		filename = getFileNameFromConfig(name, base + ".r", leftover);
		_readOnly = true;
	}

	// TODO: fading between pages
#if 0
	bool fade = false;
	if (leftover.contains("fade")) {
		fade = true;
	}
#endif

	if (leftover.contains("read")) {
		_readOnly = true;
	}
	if (leftover.contains("load")) {
		// FIXME: don't ignore this
		warning("ignoring 'load' for filename '%s'", filename.c_str());
	}
	if (leftover.contains("cut")) {
		// FIXME: don't ignore this
		warning("ignoring 'cut' for filename '%s'", filename.c_str());
	}
	if (leftover.contains("killgag")) {
		// FIXME: don't ignore this
		warning("ignoring 'killgag' for filename '%s'", filename.c_str());
	}

	Archive *pageArchive = createArchive();
	if (!filename.empty() && tryOpenPage(pageArchive, filename)) {
		_page = new LBPage(this);
		_page->open(pageArchive, 1000);
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

	debug(1, "Page Version: %d", _page->getResourceVersion());

	_curMode = mode;
	_curPage = page;
	_curSubPage = subpage;

	_cursor->showCursor();

	_gfx->setPalette(1000);

	_phase = 0;
	_introDone = false;

	_needsRedraw = true;

	return true;
}

void MohawkEngine_LivingBooks::updatePage() {
	switch (_phase) {
	case kLBPhaseInit:
		for (uint32 i = 0; i < _items.size(); i++)
			_items[i]->startPhase(kLBPhaseCreate);

		for (uint32 i = 0; i < _items.size(); i++)
			_items[i]->startPhase(_phase);

		if (_curMode == kLBControlMode) {
			// hard-coded control page startup
			LBItem *item;

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

				if (_poetryMode) {
					for (uint16 i = 0; i < _numPages; i++) {
						item = getItemById(1000 + i);
						if (item)
							item->setVisible(_curSelectedPage == i + 1);
						item = getItemById(1100 + i);
						if (item)
							item->setVisible(_curSelectedPage == i + 1);
					}
				}

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

			default:
				break;
			}
		}
		_phase++;
		break;

	case kLBPhaseIntro:
		for (uint32 i = 0; i < _items.size(); i++)
			_items[i]->startPhase(_phase);

		if (_curMode == kLBControlMode) {
			LBItem *item = getItemById(10);
			if (item)
				item->togglePlaying(false);
		}

		_phase++;
		break;

	case kLBPhaseMain:
		if (!_introDone)
			break;

		for (uint32 i = 0; i < _items.size(); i++)
			_items[i]->startPhase(_phase);

		_phase++;
		break;

	default:
		break;
	}

	while (_eventQueue.size()) {
		DelayedEvent delayedEvent = _eventQueue.pop();
		for (uint32 i = 0; i < _items.size(); i++) {
			if (_items[i] != delayedEvent.item)
				continue;

			switch (delayedEvent.type) {
			case kLBDelayedEventDestroy:
				_items.remove_at(i);
				i--;
				_orderedItems.remove(delayedEvent.item);
				_page->itemDestroyed(delayedEvent.item);
				delete delayedEvent.item;
				if (_focus == delayedEvent.item)
					_focus = nullptr;
				break;
			case kLBDelayedEventSetNotVisible:
				_items[i]->setVisible(false);
				break;
			case kLBDelayedEventDone:
				_items[i]->done(true);
				break;
			default:
				break;
			}

			break;
		}
	}

	for (uint16 i = 0; i < _items.size(); i++)
		_items[i]->update();

	if (_needsRedraw) {
		for (Common::List<LBItem *>::const_iterator i = _orderedItems.reverse_begin(); i != _orderedItems.end(); --i)
			(*i)->draw();

		_needsRedraw = false;
		_needsUpdate = true;
	}
}

void MohawkEngine_LivingBooks::addArchive(Archive *archive) {
	_mhk.push_back(archive);
}

void MohawkEngine_LivingBooks::removeArchive(Archive *archive) {
	for (uint i = 0; i < _mhk.size(); i++) {
		if (archive != _mhk[i])
			continue;
		_mhk.remove_at(i);
		return;
	}

	error("removeArchive didn't find archive");
}

void MohawkEngine_LivingBooks::addItem(LBItem *item) {
	_items.push_back(item);
	_orderedItems.push_front(item);
	item->_iterator = _orderedItems.begin();
}

void MohawkEngine_LivingBooks::removeItems(const Common::Array<LBItem *> &items) {
	for (uint i = 0; i < items.size(); i++) {
		bool found = false;
		for (uint16 j = 0; j < _items.size(); j++) {
			if (items[i] != _items[j])
				continue;
			found = true;
			_items.remove_at(j);
			break;
		}
		assert(found);
		_orderedItems.erase(items[i]->_iterator);
	}
}

LBItem *MohawkEngine_LivingBooks::getItemById(uint16 id) {
	for (uint16 i = 0; i < _items.size(); i++)
		if (_items[i]->getId() == id)
			return _items[i];

	return nullptr;
}

LBItem *MohawkEngine_LivingBooks::getItemByName(Common::String name) {
	for (uint16 i = 0; i < _items.size(); i++)
		if (_items[i]->getName() == name)
			return _items[i];

	return nullptr;
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

bool MohawkEngine_LivingBooks::playSound(LBItem *source, uint16 resourceId) {
	if (_lastSoundId && !_sound->isPlaying(_lastSoundId))
		_lastSoundId = 0;

	if (!source->isAmbient() || !_sound->isPlaying()) {
		if (!_soundLockOwner) {
			if (_lastSoundId && _lastSoundOwner != source->getId())
				if (source->getSoundPriority() >= _lastSoundPriority)
					return false;
		} else {
			if (_soundLockOwner != source->getId() && source->getSoundPriority() >= _maxSoundPriority)
				return false;
		}

		if (_lastSoundId)
			_sound->stopSound(_lastSoundId);

		_lastSoundOwner = source->getId();
		_lastSoundPriority = source->getSoundPriority();
	}

	_lastSoundId = resourceId;
	_sound->playSound(resourceId);

	return true;
}

void MohawkEngine_LivingBooks::lockSound(LBItem *owner, bool lock) {
	if (!lock) {
		_soundLockOwner = 0;
		return;
	}

	if (_soundLockOwner || (owner->isAmbient() && _sound->isPlaying()))
		return;

	if (_lastSoundId && !_sound->isPlaying(_lastSoundId))
		_lastSoundId = 0;

	_soundLockOwner = owner->getId();
	_maxSoundPriority = owner->getSoundPriority();
	if (_lastSoundId && _maxSoundPriority <= _lastSoundPriority) {
		_sound->stopSound(_lastSoundId);
		_lastSoundId = 0;
	}
}

Common::SeekableReadStreamEndian *MohawkEngine_LivingBooks::wrapStreamEndian(uint32 tag, uint16 id) {
	Common::SeekableReadStream *dataStream = getResource(tag, id);
	return new Common::SeekableReadStreamEndianWrapper(dataStream, isBigEndian(), DisposeAfterUse::YES);
}

Archive *MohawkEngine_LivingBooks::createArchive() const {
	if (isPreMohawk())
		return new LivingBooksArchive_v1();

	return new MohawkArchive();
}

bool MohawkEngine_LivingBooks::isPreMohawk() const {
	return getGameType() == GType_LIVINGBOOKSV1
		|| (getGameType() == GType_LIVINGBOOKSV2 && getPlatform() == Common::kPlatformMacintosh);
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
			if (tryLoadPageStart(kLBControlMode, 2))
				return true;
		} else {
			if (tryLoadPageStart(kLBControlMode, 3))
				return true;
		}
	}

	// go to menu page
	if (tryLoadPageStart(kLBControlMode, 1))
		return true;

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

		switch (page) {
		case 1:
			// main menu
			if (_poetryMode)
				handleUIPoetryMenuClick(event.param);
			else
				handleUIMenuClick(event.param);
			break;

		case 2:
			// quit screen
			handleUIQuitClick(event.param);
			break;

		case 3:
			// options screen
			handleUIOptionsClick(event.param);
			break;

		default:
			break;
		}
		break;

	case kLBNotifyGoToControls:
		debug(2, "kLBNotifyGoToControls: %d", event.param);

		if (!tryLoadPageStart(kLBControlMode, 1))
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

	case kLBNotifyGotoQuit:
		debug(2, "kLBNotifyGotoQuit: %d", event.param);

		if (!tryLoadPageStart(kLBControlMode, 2))
			error("couldn't load quit page");
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

	case kLBNotifyChangeMode:
		if (getGameType() == GType_LIVINGBOOKSV1) {
			debug(2, "kLBNotifyChangeMode: %d", event.param);
			quitGame();
			break;
		}

		debug(2, "kLBNotifyChangeMode: v2 type %d", event.param);
		switch (event.param) {
		case 1:
			debug(2, "kLBNotifyChangeMode: mode %d, page %d.%d",
				event.newMode, event.newPage, event.newSubpage);
			// TODO: what is entry.newUnknown?
			if (!event.newMode)
				event.newMode = _curMode;
			if (!loadPage((LBMode)event.newMode, event.newPage, event.newSubpage)) {
				if (event.newPage != 0 || !loadPage((LBMode)event.newMode, _curPage, event.newSubpage))
					if (event.newSubpage != 0 || !loadPage((LBMode)event.newMode, event.newPage, 1))
						if (event.newSubpage != 1 || !loadPage((LBMode)event.newMode, event.newPage, 0))
							error("kLBNotifyChangeMode failed to move to mode %d, page %d.%d",
								event.newMode, event.newPage, event.newSubpage);
			}
			break;
		case 3:
			debug(2, "kLBNotifyChangeMode: new cursor '%s'", event.newCursor.c_str());
			_cursor->setCursor(event.newCursor);
			break;
		default:
			error("unknown v2 kLBNotifyChangeMode type %d", event.param);
		}
		break;

	case kLBNotifyCursorChange:
		debug(2, "kLBNotifyCursorChange: %d", event.param);

		// TODO: show/hide cursor according to parameter?
		break;

	case kLBNotifyPrintPage:
		debug(2, "kLBNotifyPrintPage: %d", event.param);

		warning("kLBNotifyPrintPage unimplemented");
		break;

	case kLBNotifyQuit:
		debug(2, "kLBNotifyQuit: %d", event.param);

		quitGame();
		break;

	default:
		error("Unknown notification %d (param 0x%04x)", event.type, event.param);
	}
}

} // End of namespace Mohawk
