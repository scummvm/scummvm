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
#include "mohawk/cursors.h"

#include "common/system.h"
#include "common/textconsole.h"

namespace Mohawk {

LBItem::LBItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : _vm(vm), _page(page), _rect(rect) {
	if (_vm->getGameType() == GType_LIVINGBOOKSV1 || _vm->getGameType() == GType_LIVINGBOOKSV2)
		_phase = kLBPhaseInit;
	else
		_phase = kLBPhaseLoad;

	_loopMode = 0;
	_delayMin = 0;
	_delayMax = 0;
	_timingMode = kLBAutoNone;
	_periodMin = 0;
	_periodMax = 0;
	_controlMode = kLBControlNone;
	_soundMode = 0;

	_loaded = false;
	_enabled = false;
	_visible = true;
	_playing = false;
	_globalEnabled = true;
	_globalVisible = true;
	_nextTime = 0;
	_startTime = 0;
	_loops = 0;

	_isAmbient = false;
	_doHitTest = true;
}

LBItem::~LBItem() {
	for (uint i = 0; i < _scriptEntries.size(); i++)
		delete _scriptEntries[i];
}

void LBItem::destroySelf() {
	if (!this->_itemId)
		error("destroySelf() on an item which was already dead");

	_vm->queueDelayedEvent(DelayedEvent(this, kLBDelayedEventDestroy));

	_itemId = 0;
}

void LBItem::setEnabled(bool enabled) {
	if (enabled && !_loaded && !_playing) {
		if (_timingMode == kLBAutoUserIdle) {
			setNextTime(_periodMin, _periodMax);
			debug(2, "Enable time startup");
		}
	}

	_enabled = enabled;
}

void LBItem::setGlobalEnabled(bool enabled) {
	bool wasEnabled = _loaded && _enabled && _globalEnabled;
	_globalEnabled = enabled;
	if (wasEnabled != (_loaded && _enabled && _globalEnabled))
		setEnabled(enabled);
}

bool LBItem::contains(Common::Point point) {
	if (!_loaded)
		return false;

	if (_playing && _loopMode == 0xFFFF)
		stop();

	if (!_playing && _timingMode == kLBAutoUserIdle)
		setNextTime(_periodMin, _periodMax);

	return _visible && _globalVisible && _rect.contains(point);
}

void LBItem::update() {
	if (_phase != kLBPhaseNone && (!_loaded || !_enabled || !_globalEnabled))
		return;

	if (_nextTime == 0 || _nextTime > (uint32)(_vm->_system->getMillis() / 16))
		return;

	if (togglePlaying(_playing, true)) {
		_nextTime = 0;
	} else if (_loops == 0 && _timingMode == kLBAutoUserIdle) {
		debug(9, "Looping in update()");
		setNextTime(_periodMin, _periodMax);
	}
}

void LBItem::handleMouseDown(Common::Point pos) {
	if (!_loaded || !_enabled || !_globalEnabled)
		return;

	_vm->setFocus(this);
	runScript(kLBEventMouseDown);
	runScript(kLBEventMouseTrackIn);
}

void LBItem::handleMouseMove(Common::Point pos) {
	// TODO: handle drag
}

void LBItem::handleMouseUp(Common::Point pos) {
	_vm->setFocus(nullptr);
	runScript(kLBEventMouseUp);
	runScript(kLBEventMouseUpIn);
}

bool LBItem::togglePlaying(bool playing, bool restart) {
	if (playing) {
		_vm->queueDelayedEvent(DelayedEvent(this, kLBDelayedEventDone));
		return true;
	}
	if (((_loaded && _enabled && _globalEnabled) || _phase == kLBPhaseNone) && !_playing) {
		_playing = togglePlaying(true, restart);
		if (_playing) {
			_nextTime = 0;
			_startTime = _vm->_system->getMillis() / 16;

			if (_loopMode == 0xFFFF || _loopMode == 0xFFFE)
				_loops = 0xFFFF;
			else
				_loops = _loopMode;

			if (_controlMode >= kLBControlHideMouse) {
				debug(2, "Hiding cursor");
				_vm->_cursor->hideCursor();
				_vm->lockSound(this, true);

				if (_controlMode >= kLBControlPauseItems) {
					debug(2, "Disabling all");
					_vm->setEnableForAll(false, this);
				}
			}

			runScript(kLBEventStarted);
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

	if (_controlMode >= kLBControlHideMouse) {
		debug(2, "Showing cursor");
		_vm->_cursor->showCursor();
		_vm->lockSound(this, false);

		if (_controlMode >= kLBControlPauseItems) {
			debug(2, "Enabling all");
			_vm->setEnableForAll(true, this);
		}
	}

	if (_timingMode == kLBAutoUserIdle) {
		debug(9, "Looping in done() - %d to %d", _periodMin, _periodMax);
		setNextTime(_periodMin, _periodMax);
	}

	runScript(kLBEventDone);
	notify(0xFFFF, _itemId);
}

void LBItem::init() {
	runScript(kLBEventInit);
}

void LBItem::setVisible(bool visible) {
	if (visible == _visible)
		return;

	_visible = visible;
	_vm->_needsRedraw = true;
}

void LBItem::setGlobalVisible(bool visible) {
	bool wasEnabled = _visible && _globalVisible;
	_globalVisible = visible;
	if (wasEnabled != (_visible && _globalVisible))
		_vm->_needsRedraw = true;
}

void LBItem::startPhase(uint phase) {
	if (_phase == phase) {
		if (_phase != kLBPhaseNone) {
			setEnabled(true);
		}

		load();
	}

	switch (phase) {
	case kLBPhaseLoad:
		runScript(kLBEventListLoad);
		break;
	case kLBPhaseCreate:
		runScript(kLBEventPhaseCreate);
		if (_timingMode == kLBAutoCreate) {
			debug(2, "Phase create: time startup");
			setNextTime(_periodMin, _periodMax);
		}
		break;
	case kLBPhaseInit:
		runScript(kLBEventPhaseInit);
		if (_timingMode == kLBAutoInit) {
			debug(2, "Phase init: time startup");
			setNextTime(_periodMin, _periodMax);
		}
		break;
	case kLBPhaseIntro:
		runScript(kLBEventPhaseIntro);
		if (_timingMode == kLBAutoIntro || _timingMode == kLBAutoUserIdle) {
			debug(2, "Phase intro: time startup");
			setNextTime(_periodMin, _periodMax);
		}
		break;
	case kLBPhaseMain:
		runScript(kLBEventPhaseMain);
		if (_timingMode == kLBAutoUserIdle || _timingMode == kLBAutoMain) {
			debug(2, "Phase main: time startup");
			setNextTime(_periodMin, _periodMax);
		}
		break;
	default:
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

void LBItem::load() {
	if (_loaded)
		return;

	_loaded = true;

	// FIXME: events etc
	if (_timingMode == kLBAutoLoad) {
		debug(2, "Load: time startup");
		setNextTime(_periodMin, _periodMax);
	}
}

void LBItem::unload() {
	if (!_loaded)
		return;

	_loaded = false;

	// FIXME: stuff
}

void LBItem::moveBy(const Common::Point &pos) {
	_rect.translate(pos.x, pos.y);
}

void LBItem::moveTo(const Common::Point &pos) {
	_rect.moveTo(pos);
}

LBItem *LBItem::clone(uint16 newId, const Common::String &newName) {
	LBItem *item = createClone();

	item->_itemId = newId;
	item->_desc = newName;

	item->_resourceId = _resourceId;
	// FIXME: the rest

	_page->addClonedItem(item);
	// FIXME: zorder?
	return item;
}

LBItem *LBItem::createClone() {
	return new LBItem(_vm, _page, _rect);
}

void LBItem::setNextTime(uint16 min, uint16 max) {
	setNextTime(min, max, _vm->_system->getMillis() / 16);
}

void LBItem::setNextTime(uint16 min, uint16 max, uint32 start) {
	_nextTime = start + _vm->_rnd->getRandomNumberRng((uint)min, (uint)max);
	debug(9, "nextTime is now %d frames away", _nextTime - (uint)(_vm->_system->getMillis() / 16));
}

} // End of namespace Mohawk
