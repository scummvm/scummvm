/*
 * State.cpp
 *
 *  Created on: 13-Jun-2009
 *      Author: alex
 */

#include "asylum/state.h"

namespace Asylum {

State::State(AsylumEngine *vm) {
	_vm     = vm;
	_resMgr = _vm->getResourceManager();

	_mouseX  = 0;
	_mouseY  = 0;
}

void State::handleEvent(Common::Event *event) {
	_ev = event;

	// handle common event assignment tasks
	// before bubbling to the subclass' virtual
	// update method
	if (_ev->type == Common::EVENT_MOUSEMOVE) {
		_mouseX = _ev->mouse.x;
		_mouseY = _ev->mouse.y;
	}

	update();
}


///////////////
// MenuState //
///////////////

MenuState::MenuState(AsylumEngine *vm): State(vm) {
	_activeIcon         = -1;
	_previousActiveIcon = -1;
	_curIconFrame       = 0;
	_curMouseCursor     = 0;
	_cursorStep         = 1;

	init();
}

void MenuState::init() {
	_resMgr->loadGraphic(1, 0, 0);
	_resMgr->loadPalette(1, 17);
	_resMgr->loadCursor(1, 2, 0);
	_resMgr->loadMusic();
}

void MenuState::update() {
	// TODO: Just some proof-of concept to change icons here for now
	if (_mouseY >= 20 && _mouseY <= 20 + 48) {
		// Top row
		for (int i = 0; i <= 5; i++) {
			int curX = 40 + i * 100;
			if (_mouseX >= curX && _mouseX <= curX + 55) {
				GraphicResource *res = _resMgr->getGraphic(1, i + 4, _curIconFrame);
				_vm->_system->copyRectToScreen(res->data, res->width, curX, 20, res->width, res->height);

				// Cycle icon frame
				// Icon animations have 15 frames, 0-14
				_curIconFrame++;
				if (_curIconFrame == 15)
					_curIconFrame = 0;

				_activeIcon = i;

				// Play creepy voice
				if (!_vm->_mixer->isSoundHandleActive(_sfxHandle) && _activeIcon != _previousActiveIcon) {
					_vm->_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, _resMgr->loadSFX(1, i + 44));
					_previousActiveIcon = _activeIcon;
				}

				break;
			}
		}
	} else if (_mouseY >= 400 && _mouseY <= 400 + 48) {
		// Bottom row
		for (int i = 0; i <= 5; i++) {
			int curX = 40 + i * 100;
			if (_mouseX >= curX && _mouseX <= curX + 55) {
				int iconNum = i + 10;

				// The last 2 icons are swapped
				if (iconNum == 14)
					iconNum = 15;
				else if (iconNum == 15)
					iconNum = 14;

				GraphicResource *res = _resMgr->getGraphic(1, iconNum, _curIconFrame);
				_vm->_system->copyRectToScreen(res->data, res->width, curX, 400, res->width, res->height);

				// Cycle icon frame
				// Icon animations have 15 frames, 0-14
				_curIconFrame++;
				if (_curIconFrame == 15)
					_curIconFrame = 0;

				_activeIcon = i + 6;

				// HACK: the credits icon has less frames (0 - 9). Currently, there's no way to find the number
				// of frames with the current resource manager implementation, so we just hardcode it here
				if (_activeIcon == 10 && _curIconFrame >= 10)
					_curIconFrame = 0;

				// Play creepy voice
				if (!_vm->_mixer->isSoundHandleActive(_sfxHandle) && _activeIcon != _previousActiveIcon) {
					_vm->_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, _resMgr->loadSFX(1, iconNum + 40));
					_previousActiveIcon = _activeIcon;
				}

				break;
			}
		}
	} else {
		// No selection
		_previousActiveIcon = _activeIcon = -1;
	}

	// Eyes animation
	// Get the appropriate eye resource depending on the mouse position
	int eyeResource = kEyesFront;

	if (_mouseX <= 200) {
		if (_mouseY <= 160)
			eyeResource = kEyesTopLeft;
		else if (_mouseY > 160 && _mouseY <= 320)
			eyeResource = kEyesLeft;
		else
			eyeResource = kEyesBottomLeft;
	} else if (_mouseX > 200 && _mouseX <= 400) {
		if (_mouseY <= 160)
			eyeResource = kEyesTop;
		else if (_mouseY > 160 && _mouseY <= 320)
			eyeResource = kEyesFront;
		else
			eyeResource = kEyesBottom;
	} else if (_mouseX > 400) {
		if (_mouseY <= 160)
			eyeResource = kEyesTopRight;
		else if (_mouseY > 160 && _mouseY <= 320)
			eyeResource = kEyesRight;
		else
			eyeResource = kEyesBottomRight;
	}
	// TODO: kEyesCrossed state

	GraphicResource *res = _resMgr->getGraphic(1, 1, eyeResource);
	_vm->copyRectToScreenWithTransparency(res->data, 265, 230, res->width, res->height);

	updateCursor();
}

void MenuState::updateCursor() {
	_curMouseCursor += _cursorStep;
	if (_curMouseCursor == 0)
		_cursorStep = 1;
	if (_curMouseCursor == 6)
		_cursorStep = -1;

	_resMgr->loadCursor(1, 2, _curMouseCursor);
}

} // end of namespace Asylum
