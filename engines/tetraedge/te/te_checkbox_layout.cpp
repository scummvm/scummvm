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

#include "tetraedge/tetraedge.h"

#include "tetraedge/te/te_checkbox_layout.h"
#include "tetraedge/te/te_color.h"
#include "tetraedge/te/te_callback.h"
#include "tetraedge/te/te_input_mgr.h"

namespace Tetraedge {

TeCheckboxLayout::TeCheckboxLayout() : _activeLayout(nullptr), _unactiveLayout(nullptr),
_activeDisabledLayout(nullptr), _unactiveDisabledLayout(nullptr),
_activeRollOverLayout(nullptr), _unactiveRollOverLayout(nullptr), _hitZone(nullptr),
_clickPassThrough(false), _state(CheckboxState6)
{
	_onMouseLeftUpCallback.reset(new TeCallback1Param<TeCheckboxLayout, const Common::Point &>(this, &TeCheckboxLayout::onMouseLeftUp));
	_onMouseLeftUpMaxPriorityCallback.reset(new TeCallback1Param<TeCheckboxLayout, const Common::Point &>(this, &TeCheckboxLayout::onMouseLeftUpMaxPriority));
	_onMouseLeftDownCallback.reset(new TeCallback1Param<TeCheckboxLayout, const Common::Point &>(this, &TeCheckboxLayout::onMouseLeftDown));
	_onMousePositionChangedCallback.reset(new TeCallback1Param<TeCheckboxLayout, const Common::Point &>(this, &TeCheckboxLayout::onMousePositionChanged));

	TeInputMgr *inputmgr = g_engine->getInputMgr();
	inputmgr->_mouseMoveSignal.push_back(_onMousePositionChangedCallback);
	inputmgr->_mouseLDownSignal.push_back(_onMouseLeftDownCallback);
	inputmgr->_mouseLUpSignal.push_back(_onMouseLeftUpCallback);
	inputmgr->_mouseLUpSignal.push_back(_onMouseLeftUpMaxPriorityCallback);
}

TeCheckboxLayout::~TeCheckboxLayout() {
	TeInputMgr *inputmgr = g_engine->getInputMgr();
	inputmgr->_mouseMoveSignal.remove(_onMousePositionChangedCallback);
	inputmgr->_mouseLDownSignal.remove(_onMouseLeftDownCallback);
	inputmgr->_mouseLUpSignal.remove(_onMouseLeftUpCallback);
	inputmgr->_mouseLUpSignal.remove(_onMouseLeftUpMaxPriorityCallback);
}

void TeCheckboxLayout::setActiveLayout(TeLayout *layout) {
	if (_activeLayout)
		removeChild(_activeLayout);
	_activeLayout = layout;
	if (layout) {
		addChild(layout);
		layout->setColor(TeColor(0, 0, 0, 0));
	}
	setState(_state);
}

void TeCheckboxLayout::setUnactiveLayout(TeLayout *layout) {
	if (_unactiveLayout)
		removeChild(_unactiveLayout);
	_unactiveLayout = layout;
	warning("TODO: Add extra code in TeCheckboxLayout::setUnactiveLayout.");
	if (layout) {
		addChild(layout);
		//layout->setColor(TeColor(0, 0, 0, 0));
	}
	setState(_state);
}

void TeCheckboxLayout::setActiveDisabledLayout(TeLayout *layout) {
	if (_activeDisabledLayout)
		removeChild(_activeDisabledLayout);
	_activeDisabledLayout = layout;
	if (layout) {
		addChild(layout);
		//layout->setColor(TeColor(0, 0, 0, 0));
	}
	setState(_state);
}

void TeCheckboxLayout::setUnactiveDisabledLayout(TeLayout *layout) {
	if (_unactiveDisabledLayout)
		removeChild(_unactiveDisabledLayout);
	_unactiveDisabledLayout = layout;
	if (layout) {
		addChild(layout);
		//layout->setColor(TeColor(0, 0, 0, 0));
	}
	setState(_state);
}

void TeCheckboxLayout::setActiveRollOverLayout(TeLayout *layout) {
	if (_activeRollOverLayout)
		removeChild(_activeRollOverLayout);
	_activeRollOverLayout = layout;
	if (layout) {
		addChild(layout);
		//layout->setColor(TeColor(0, 0, 0, 0));
	}
	setState(_state);
}

void TeCheckboxLayout::setUnactiveRollOverLayout(TeLayout *layout) {
	if (_unactiveRollOverLayout)
		removeChild(_unactiveRollOverLayout);
	_unactiveRollOverLayout = layout;
	if (layout) {
		addChild(layout);
		//layout->setColor(TeColor(0, 0, 0, 0));
	}
	setState(_state);
}

void TeCheckboxLayout::setHitZone(TeLayout *layout) {
	if (_hitZone)
		removeChild(_hitZone);
	_hitZone = layout;
	if (layout) {
		addChild(layout);
		//layout->setColor(TeColor(0, 0, 0xff, 0xff));
	}
}


void TeCheckboxLayout::setClickPassThrough(bool val) {
	_clickPassThrough = val;
}

void TeCheckboxLayout::setActivationSound(const Common::String &sound) {
	_activationSound = sound;
}

void TeCheckboxLayout::setUnactivationSound(const Common::String &sound) {
	_unactivationSound = sound;
}

bool TeCheckboxLayout::isMouseIn(const TeVector2s32 &pt) {
	if (_hitZone) {
		return _hitZone->isMouseIn(pt);
	} else {
		return TeLayout::isMouseIn(pt);
	}
}

void TeCheckboxLayout::setState(enum State state) {
	if (_state == state)
		return;

	_state = state;

	if (_activeLayout)
		_activeLayout->setVisible(state == CheckboxStateActive);
	if (_unactiveLayout)
		_unactiveLayout->setVisible(state == CheckboxStateUnactive);
	if (_activeRollOverLayout)
		_activeRollOverLayout->setVisible(state == CheckboxStateActiveRollover);
	if (_unactiveRollOverLayout)
		_unactiveRollOverLayout->setVisible(state == CheckboxStateUnactiveRollover);
	if (_activeDisabledLayout)
		_activeDisabledLayout->setVisible(state == CheckboxStateActiveDisabled);
	if (_unactiveDisabledLayout)
		_unactiveDisabledLayout->setVisible(state == CheckboxStateUnactiveDisabled);

	_onStateChangedSignal.call(state);
}

bool TeCheckboxLayout::onMouseLeftUp(const Common::Point &pt) {
	if (!worldVisible())
		return false;

	bool mouseIn = isMouseIn(pt);

	State newState = _state;

	switch (_state) {
	case CheckboxStateActive:
		if (mouseIn)
			newState = CheckboxStateUnactive;
		break;
	case CheckboxStateUnactive:
		if (mouseIn)
			newState = CheckboxStateActive;
		break;
	case CheckboxStateActiveRollover:
		if (mouseIn)
			newState = CheckboxStateUnactiveRollover;
		else
			newState = CheckboxStateActive;
		break;
	case CheckboxStateUnactiveRollover:
		if (mouseIn)
			newState = CheckboxStateActiveRollover;
		else
			newState = CheckboxStateUnactive;
		break;
	case CheckboxStateActiveDisabled:
	case CheckboxStateUnactiveDisabled:
	default:
		break;
	}
	setState(newState);
	return !_clickPassThrough;
}

bool TeCheckboxLayout::onMouseLeftUpMaxPriority(const Common::Point &pt) {
	if (!worldVisible())
		return false;

	//bool mouseIn = isMouseIn(pt);

	//error("TODO: Implement TeCheckboxLayout::onMouseLeftUpMaxPriority");
	return false;
}

bool TeCheckboxLayout::onMouseLeftDown(const Common::Point &pt) {
	if (!worldVisible())
		return false;

	//bool mouseIn = isMouseIn(pt);

	//error("TODO: Implement TeCheckboxLayout::onMouseLeftDown");
	return false;
}

bool TeCheckboxLayout::onMousePositionChanged(const Common::Point &pt) {
	if (!worldVisible())
		return false;

	State newState = _state;
	bool mouseIn = isMouseIn(pt);

	if (!mouseIn) {
		if (_state == CheckboxStateActiveRollover)
			newState = CheckboxStateActive;
		else if (_state == CheckboxStateUnactiveRollover)
			newState = CheckboxStateUnactive;
	} else {
		if (_state == CheckboxStateActive)
			newState = CheckboxStateActiveRollover;
		else if (_state == CheckboxStateUnactive)
			newState = CheckboxStateUnactiveRollover;
	}

	setState(newState);
	return false;
}


} // end namespace Tetraedge
