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

#include "common/math.h"

#include "tetraedge/tetraedge.h"

#include "tetraedge/te/te_button_layout.h"
#include "tetraedge/te/te_sound_manager.h"
#include "tetraedge/te/te_input_mgr.h"
#include "tetraedge/te/te_sprite_layout.h"

namespace Tetraedge {

class TeZPriorityMouseCallback : public TeCallback1Param<TeButtonLayout, const Common::Point &> {
public:
	TeZPriorityMouseCallback(TeButtonLayout *layout, TMethod method) : TeCallback1Param<TeButtonLayout, const Common::Point &>(layout, method), _pri(0.0) {}
	virtual float &priority() override {
		_pri =_object->worldPosition().z();
		return _pri;
	}
	float _pri;
};

/*static*/
bool TeButtonLayout::_mousePositionChangedCatched = false;
/*static*/
TeTimer *TeButtonLayout::_doubleValidationProtectionTimer = nullptr;

/*static*/
TeTimer *TeButtonLayout::getDoubleValidationProtectionTimer() {
	if (!_doubleValidationProtectionTimer) {
		_doubleValidationProtectionTimer = new TeTimer();
	}
	return _doubleValidationProtectionTimer;
}

TeButtonLayout::TeButtonLayout() : _currentState(BUTTON_STATE_UP),
_clickPassThrough(false), _validationSoundVolume(1.0),
_ignoreMouseEvents(false), _doubleValidationProtectionEnabled(true),
_upLayout(nullptr), _downLayout(nullptr), _rolloverLayout(nullptr),
_disabledLayout(nullptr), _hitZoneLayout(nullptr), _ownedLayouts(false)
{
	_onMousePositionChangedMaxPriorityCallback.reset(new TeCallback1Param<TeButtonLayout, const Common::Point &>(this, &TeButtonLayout::onMousePositionChangedMaxPriority, FLT_MAX));

	_onMousePositionChangedCallback.reset(new TeZPriorityMouseCallback(this, &TeButtonLayout::onMousePositionChanged));
	_onMouseLeftDownCallback.reset(new TeZPriorityMouseCallback(this, &TeButtonLayout::onMouseLeftDown));
	_onMouseLeftUpMaxPriorityCallback.reset(new TeCallback1Param<TeButtonLayout, const Common::Point &>(this, &TeButtonLayout::onMouseLeftUpMaxPriority, FLT_MAX));
	_onMouseLeftUpCallback.reset(new TeZPriorityMouseCallback(this, &TeButtonLayout::onMouseLeftUp));

	TeInputMgr *inputmgr = g_engine->getInputMgr();
	inputmgr->_mouseMoveSignal.push_back(_onMousePositionChangedCallback);
	inputmgr->_mouseMoveSignal.push_back(_onMousePositionChangedMaxPriorityCallback);
	inputmgr->_mouseLDownSignal.push_back(_onMouseLeftDownCallback);
	inputmgr->_mouseLUpSignal.push_back(_onMouseLeftUpCallback);
	inputmgr->_mouseLUpSignal.push_back(_onMouseLeftUpMaxPriorityCallback);

	setEditionColor(TeColor(128, 128, 128, 255));
	if (!getDoubleValidationProtectionTimer()->running())
		getDoubleValidationProtectionTimer()->start();
}

TeButtonLayout::~TeButtonLayout() {
	TeInputMgr *inputmgr = g_engine->getInputMgr();
	inputmgr->_mouseMoveSignal.remove(_onMousePositionChangedCallback);
	inputmgr->_mouseMoveSignal.remove(_onMousePositionChangedMaxPriorityCallback);
	inputmgr->_mouseLDownSignal.remove(_onMouseLeftDownCallback);
	inputmgr->_mouseLUpSignal.remove(_onMouseLeftUpCallback);
	inputmgr->_mouseLUpSignal.remove(_onMouseLeftUpMaxPriorityCallback);
	if (_ownedLayouts) {
		if (_upLayout)
			delete _upLayout;
		if (_downLayout)
			delete _downLayout;
		if (_rolloverLayout)
			delete _rolloverLayout;
		if (_hitZoneLayout)
			delete _hitZoneLayout;
		if (_disabledLayout)
			delete _disabledLayout;
	}
}

bool TeButtonLayout::isMouseIn(const TeVector2s32 &mouseloc) {
	if (!_hitZoneLayout) {
		return TeLayout::isMouseIn(mouseloc);
	} else {
		return _hitZoneLayout->isMouseIn(mouseloc);
	}
}

void TeButtonLayout::load(const Common::String &upImg, const Common::String &downImg, const Common::String &overImg) {
	TeSpriteLayout *upSprite = nullptr;
	if (upImg.size()) {
		upSprite = new TeSpriteLayout();
		if (!upSprite->load(upImg))
			warning("Failed to load button up img %s", upImg.c_str());
	}
	setUpLayout(upSprite);

	TeSpriteLayout *downSprite = nullptr;
	if (downImg.size()) {
		downSprite = new TeSpriteLayout();
		if (!downSprite->load(upImg))
			warning("Failed to load button down img %s", downImg.c_str());
	}
	setDownLayout(downSprite);

	TeSpriteLayout *overSprite = nullptr;
	if (overImg.size()) {
		overSprite = new TeSpriteLayout();
		if (!overSprite->load(overImg))
			warning("Failed to load button over img %s", overImg.c_str());
	}
	setRollOverLayout(overSprite);
	setHitZone(nullptr);
	setDisabledLayout(nullptr);
	_ownedLayouts = true;
}

bool TeButtonLayout::onMouseLeftDown(const Common::Point &pt) {
	if (!worldVisible() || _currentState == BUTTON_STATE_DISABLED || _ignoreMouseEvents)
		return false;

	// Note: This doesn't exactly reproduce the original behavior, it's
	// very simplified.
	bool mouseIn = isMouseIn(pt);

	//if (mouseIn)
	//	debug("mouse down on button '%s' (current state %d)", name().c_str(), _currentState);

	enum State newState = _currentState;
	switch (_currentState) {
	case BUTTON_STATE_DOWN:
		if (!mouseIn)
			newState = BUTTON_STATE_UP;
		/*
		// TODO: should this be a click?
		if (mouseIn) {
			newState = BUTTON_STATE_UP;
			debug("mouse clicked button '%s' (from leftdown)", name().c_str());
			if (!_validationSound.empty()) {
				TeSoundManager *sndMgr = g_engine->getSoundManager();
				sndMgr->playFreeSound(_validationSound, _validationSoundVolume, "sfx");
			}
			setState(newState);
			_onMouseClickValidatedSignal.call();
			return !_clickPassThrough;
		}*/
		break;
	case BUTTON_STATE_ROLLOVER:
	case BUTTON_STATE_UP:
		if (mouseIn)
			newState = BUTTON_STATE_DOWN;
		break;
	case BUTTON_STATE_DISABLED:
		break;
	}
	setState(newState);
	return mouseIn && !_clickPassThrough;
}

bool TeButtonLayout::onMouseLeftUp(const Common::Point &pt) {
	if (!worldVisible() || _currentState == BUTTON_STATE_DISABLED)
		return false;

	// Note: This doesn't exactly reproduce the original behavior, it's
	// somewhat simplified.
	bool mouseIn = isMouseIn(pt);

	//if (mouseIn)
	//	debug("mouse up on button '%s' (current state %d)", name().c_str(), _currentState);

	enum State newState = _currentState;
	switch (_currentState) {
	case BUTTON_STATE_DOWN:
		newState = BUTTON_STATE_UP;
		if (mouseIn) {
			debug("mouse clicked button '%s' (from leftup)", name().c_str());
			if (!_validationSound.empty()) {
				TeSoundManager *sndMgr = g_engine->getSoundManager();
				sndMgr->playFreeSound(_validationSound, _validationSoundVolume, "sfx");
			}
			setState(newState);
			bool stopProcessing = _onMouseClickValidatedSignal.call();
			return !_clickPassThrough || stopProcessing;
		}
		break;
	case BUTTON_STATE_ROLLOVER:
	case BUTTON_STATE_UP:
	case BUTTON_STATE_DISABLED:
		break;
	}
	setState(newState);
	return mouseIn && !_clickPassThrough;
}

bool TeButtonLayout::onMousePositionChanged(const Common::Point &pt) {
	if (!worldVisible() || _ignoreMouseEvents)
		return false;

	// Note: This doesn't exactly reproduce the original behavior, it's
	// very simplified.
	bool mouseIn = isMouseIn(pt);

	enum State newState = _currentState;
	switch (_currentState) {
	case BUTTON_STATE_UP:
		if (mouseIn) {
			newState = BUTTON_STATE_ROLLOVER;
		}
		break;
	case BUTTON_STATE_DOWN:
	case BUTTON_STATE_ROLLOVER:
		if (!mouseIn) {
			newState = BUTTON_STATE_UP;
		}
		break;
	case BUTTON_STATE_DISABLED:
		break;
	}
	setState(newState);
	return false;
}

void TeButtonLayout::reset() {
	_intArr.clear();
	State newState = (_currentState == BUTTON_STATE_DISABLED ? BUTTON_STATE_DISABLED : BUTTON_STATE_UP);
	setState(newState);
}

void TeButtonLayout::resetTimeFromLastValidation() {
	TeTimer *timer = getDoubleValidationProtectionTimer();
	if (!timer->running()) {
		timer->start();
	}
	timer->timeElapsed();
}

uint64 TeButtonLayout::timeFromLastValidation() {
	// probably not needed because we reimplemented how this works.
	error("TODO: Implement TeButtonLayout::timeFromLastValidation.");
}

void TeButtonLayout::setDisabledLayout(TeLayout *disabledLayout) {
	if (_disabledLayout)
		removeChild(_disabledLayout);

	_disabledLayout = disabledLayout;
	if (_disabledLayout) {
		_sizeChanged = true;
		addChild(_disabledLayout);
		//_disabledLayout->setColor(TeColor(0, 0, 0, 0));
		//_disabledLayout->setName(name() + "_disabledLayout");
	}

	setState(_currentState);
}

void TeButtonLayout::setHitZone(TeLayout *hitZoneLayout) {
	if (_hitZoneLayout)
		removeChild(_hitZoneLayout);

	_hitZoneLayout = hitZoneLayout;
	if (_hitZoneLayout) {
		_sizeChanged = true;
		addChild(_hitZoneLayout);
		//_hitZoneLayout->setColor(TeColor(0, 0, 0xff, 0xff));
		//_hitZoneLayout->setName(name() + "_hitZoneLayout");
	}
}

void TeButtonLayout::setDownLayout(TeLayout *downLayout) {
	if (_downLayout)
		removeChild(_downLayout);

	if (downLayout)
		addChild(downLayout);
	_downLayout = downLayout;

	if (sizeType() == ABSOLUTE &&
			size().x() == 1.0f && size().y() == 1.0f &&
			!_upLayout && _downLayout) {
		setSize(_downLayout->size());
	}

	//if (_downLayout) {
	//	_downLayout->setColor(TeColor(0, 0, 0, 0));
		//_downLayout->setName(name() + "_downLayout");
	//}

	setState(_currentState);
}

void TeButtonLayout::setRollOverLayout(TeLayout *rollOverLayout) {
	if (_rolloverLayout)
		removeChild(_rolloverLayout);

	_rolloverLayout = rollOverLayout;
	if (_rolloverLayout) {
		addChild(_rolloverLayout);
		//_rolloverLayout->setName(name() + "_rolloverLayout");
	}

	// This is not a copy paste error, or at least, not *my*
	// copy paste error.. it's what the original game does.
	//if (_disabledLayout)
	//	_disabledLayout->setColor(TeColor(0, 0, 0, 0));

	setState(_currentState);
}

void TeButtonLayout::setUpLayout(TeLayout *upLayout) {
	if (_upLayout)
		removeChild(_upLayout);

	if (upLayout)
		addChild(upLayout);
	_upLayout = upLayout;

	if (sizeType() == ABSOLUTE &&
			size().x() == 1.0f && size().y() == 1.0f &&
			!_downLayout && _upLayout) {
		setSize(_upLayout->size());
	}

	if (_upLayout) {
		//_upLayout->setColor(TeColor(0, 0, 0, 0));
		//_upLayout->setName(name() + "_upLayout");
	}

	setState(_currentState);
}

void TeButtonLayout::setDoubleValidationProtectionEnabled(bool enable) {
	_doubleValidationProtectionEnabled = enable;
}

void TeButtonLayout::setEnable(bool enable) {
	if (enable) {
		_currentState = BUTTON_STATE_UP;
		setState(_currentState);
	} else {
		_currentState = BUTTON_STATE_DISABLED;
		setState(_currentState);
	}
}

void TeButtonLayout::setPosition(const TeVector3f32 &pos) {
	TeLayout::setPosition(pos);

	if (_currentState != BUTTON_STATE_DISABLED) {
		int somethingCount = 0;
		if (!_intArr.empty()) {
			// probably not needed as we reimplememted how this works.
			error("TODO: Implement setPosition logic for up/down state");
		}
		if (!_ignoreMouseEvents) {
			setState(somethingCount ? BUTTON_STATE_DOWN : BUTTON_STATE_UP);
		}
	}
}

void TeButtonLayout::setState(State newState) {
	if (_currentState != newState) {
		switch (newState) {
		case BUTTON_STATE_UP:
			_onButtonChangedToStateUpSignal.call();
			break;
		case BUTTON_STATE_DOWN:
			_onButtonChangedToStateDownSignal.call();
			break;
		case BUTTON_STATE_ROLLOVER:
			_onButtonChangedToStateRolloverSignal.call();
			break;
		default:
			break;
		}
		_currentState = newState;
	}

	if (_upLayout)
		_upLayout->setVisible(_currentState == BUTTON_STATE_UP
				|| (_currentState == BUTTON_STATE_ROLLOVER && _rolloverLayout == nullptr)
				|| (_currentState == BUTTON_STATE_DOWN && _downLayout == nullptr)
				|| (_currentState == BUTTON_STATE_DISABLED && _disabledLayout == nullptr));
	if (_downLayout)
		_downLayout->setVisible(_currentState == BUTTON_STATE_DOWN);
	if (_disabledLayout)
		_disabledLayout->setVisible(_currentState == BUTTON_STATE_DISABLED);
	if (_rolloverLayout)
		_rolloverLayout->setVisible(_currentState == BUTTON_STATE_ROLLOVER);
}

} // end namespace Tetraedge
