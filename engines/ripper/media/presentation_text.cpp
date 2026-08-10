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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/media/presentation_text.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/paletteman.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

static const int kPresentationTextWidth = 400;
static const int kPresentationTextInitialTop = 300;
static const int kPresentationTextFollowingTop = 260;
static const int kPresentationContinueTop = 25;
static const int kPresentationContinueWidth = 60;
static const int kPresentationContinueHeight = 27;
static const int kPresentationContinueGap = 5;
static const uint kPresentationContinueResource = 0x48;
static const uint kPresentationCursor = 14;

} // End of anonymous namespace

Common::Rect calculatePresentationTextBounds(uint sequenceId, uint videoMode,
		bool largeMedia, int displayTop) {
	const int left = (640 - kPresentationTextWidth) / 2;
	int top = displayTop + kPresentationTextInitialTop;
	if (sequenceId != 1) {
		// ServiceMediaPresentationTextControl at 0x17014 moves the wrapped
		// chooser down to the bottom of an unscaled small-media page. Scaled
		// and large branches move it to y=400, outside the active display.
		if (videoMode >= 2 || largeMedia)
			return Common::Rect(left, 400,
				left + kPresentationTextWidth, 400);
		top = displayTop + kPresentationTextFollowingTop;
	}
	if (top >= 400)
		return Common::Rect(left, 400,
			left + kPresentationTextWidth, 400);
	return Common::Rect(left, top, left + kPresentationTextWidth, 400);
}

uint calculatePresentationTextAutoScrollLine(uint progress, uint total,
		uint maximumFirstVisible) {
	if (total == 0 || maximumFirstVisible == 0)
		return 0;
	const uint64 stepCount = maximumFirstVisible + 1;
	return MIN<uint64>((uint64)progress * stepCount / total,
		maximumFirstVisible);
}

PresentationTextControl::PresentationTextControl(RipperEngine *engine,
		InputManager *input, const Common::String &text, int displayTop) :
		_engine(engine), _input(input), _text(text), _bounds(),
		_firstVisible(0), _maximumFirstVisible(0), _visibleRows(0),
		_totalFrames(0), _sequenceId(0), _displayTop(displayTop),
		_dismissed(false), _continuePressed(false),
		_hoveredScrollControl(ModalDialogManager::kTextPanelScrollNone) {
}

void PresentationTextControl::configureSegment(uint sequenceId,
		uint totalFrames, uint mediaWidth, uint mediaHeight, uint videoMode) {
	_sequenceId = sequenceId;
	_totalFrames = totalFrames;
	const bool largeMedia = mediaWidth >= 321 || mediaHeight >= 201;
	_bounds = calculatePresentationTextBounds(sequenceId, videoMode,
		largeMedia, _displayTop);
	debugC(2, kDebugVideo,
		"Ripper: configured retail presentation text sequence=%u frames=%u "
		"media=%ux%u large=%d videoMode=%u bounds=%d,%d,%d,%d",
		sequenceId, totalFrames, mediaWidth, mediaHeight, largeMedia,
		videoMode, _bounds.left, _bounds.top, _bounds.width(),
		_bounds.height());
}

void PresentationTextControl::applySharedPalettePatch() {
	byte palette[256 * 3];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);
	_engine->applySharedPalettePatch(palette, 256);
	g_system->getPaletteManager()->setPalette(palette, 0, 256);
}

void PresentationTextControl::scrollTo(uint firstVisible,
		const char *source) {
	firstVisible = MIN(firstVisible, _maximumFirstVisible);
	if (firstVisible == _firstVisible)
		return;
	_firstVisible = firstVisible;
	debugC(3, kDebugVideo,
		"Ripper: scrolled retail presentation text source=%s sequence=%u "
		"firstLine=%u limit=%u",
		source, _sequenceId, _firstVisible, _maximumFirstVisible);
}

bool PresentationTextControl::serviceInput(bool allowDismissal) {
	bool stop = false;
	while (_input->hasPendingKey()) {
		const uint16 command = _input->consumeKey();
		if (command == 0x1b) {
			_dismissed = true;
			stop = true;
			debugC(2, kDebugVideo,
				"Ripper: dismissed retail presentation text by Escape "
				"sequence=%u postPlayback=%d",
				_sequenceId, allowDismissal);
			break;
		}
		uint nextFirstVisible = _firstVisible;
		switch (command) {
		case 0x4700:
			nextFirstVisible = 0;
			break;
		case 0x4800:
			nextFirstVisible = _firstVisible > 0 ? _firstVisible - 1 : 0;
			break;
		case 0x4900:
			nextFirstVisible = _firstVisible > _visibleRows ?
				_firstVisible - _visibleRows : 0;
			break;
		case 0x4f00:
			nextFirstVisible = _maximumFirstVisible;
			break;
		case 0x5000:
			nextFirstVisible = MIN(_firstVisible + 1,
				_maximumFirstVisible);
			break;
		case 0x5100:
			nextFirstVisible = MIN(_firstVisible + _visibleRows,
				_maximumFirstVisible);
			break;
		default:
			break;
		}
		scrollTo(nextFirstVisible, "keyboard");
	}

	const MouseState mouse = _input->publishMouseState();
	if (!_bounds.isEmpty()) {
		ModalDialogManager *modal = _engine->getModalDialog();
		_hoveredScrollControl = modal->findTextPanelScrollControl(
			_bounds, mouse.position, _firstVisible,
			_maximumFirstVisible,
			ModalDialogManager::kSceneEntryPresentation);
		if (_bounds.contains(mouse.position) && mouse.wheel != 0) {
			const int next = CLIP<int>((int)_firstVisible - mouse.wheel,
				0, _maximumFirstVisible);
			scrollTo(next, "mouse-wheel");
		}
		if ((mouse.released & kMouseButtonLeft) != 0) {
			if (_hoveredScrollControl == ModalDialogManager::kTextPanelScrollUp)
				scrollTo(_firstVisible > 0 ? _firstVisible - 1 : 0,
					"scroll-up");
			else if (_hoveredScrollControl == ModalDialogManager::kTextPanelScrollDown)
				scrollTo(_firstVisible + 1, "scroll-down");
		}
	}

	if (allowDismissal) {
		const Common::Rect continueBounds(_bounds.right + kPresentationContinueGap,
			kPresentationContinueTop,
			_bounds.right + kPresentationContinueGap + kPresentationContinueWidth,
			kPresentationContinueTop + kPresentationContinueHeight);
		if ((mouse.pressed & kMouseButtonLeft) != 0)
			_continuePressed = continueBounds.contains(mouse.position);
		if ((mouse.released & kMouseButtonLeft) != 0) {
			if (_continuePressed && continueBounds.contains(mouse.position)) {
				_dismissed = true;
				stop = true;
				debugC(2, kDebugVideo,
					"Ripper: dismissed retail presentation text through "
					"continue control sequence=%u point=%d,%d",
					_sequenceId, mouse.position.x, mouse.position.y);
			}
			_continuePressed = false;
		}
	}
	return stop;
}

bool PresentationTextControl::draw(bool includeContinueControl) {
	ModalDialogManager *modal = _engine->getModalDialog();
	if (!_bounds.isEmpty()) {
		if (!modal->drawRetainedTextPanelText(_text, _bounds, _firstVisible,
				_maximumFirstVisible, _visibleRows,
				ModalDialogManager::kSceneEntryPresentation,
				_hoveredScrollControl, false))
			return false;
	}
	if (includeContinueControl) {
		const Common::Rect continueBounds(_bounds.right + kPresentationContinueGap,
			kPresentationContinueTop,
			_bounds.right + kPresentationContinueGap + kPresentationContinueWidth,
			kPresentationContinueTop + kPresentationContinueHeight);
		if (!modal->drawRetainedTitlePanel(kPresentationContinueResource,
				continueBounds, ModalDialogManager::kSceneEntryPresentation,
				false))
			return false;
	}
	return true;
}

uint16 PresentationTextControl::service(uint progress) {
	if (!isActive() || _dismissed)
		return _dismissed ? 1 : 0;
	serviceInput(false);
	if (_dismissed)
		return 1;
	if (!draw(false))
		return 1;
	const uint automaticFirstVisible = calculatePresentationTextAutoScrollLine(
		progress, _totalFrames, _maximumFirstVisible);
	if (automaticFirstVisible > _firstVisible) {
		scrollTo(automaticFirstVisible, "auto");
		if (!draw(false))
			return 1;
	}
	return 0;
}

bool PresentationTextControl::waitForDismissal() {
	if (!isActive() || _dismissed)
		return !_engine->shouldQuit();
	_input->discardMouseTransitions();
	_engine->getCursor()->update(kPresentationCursor);
	_engine->getCursor()->setVisible(true);
	debugC(1, kDebugVideo,
		"Ripper: waiting for retail presentation text dismissal "
		"sequence=%u firstLine=%u limit=%u continueResource=0x%x",
		_sequenceId, _firstVisible, _maximumFirstVisible,
		kPresentationContinueResource);
	while (!_engine->shouldQuit() && !_dismissed) {
		if (_input->pollEvents()) {
			_engine->quitGame();
			break;
		}
		serviceInput(true);
		if (!draw(true))
			return false;
		presentScreen();
		g_system->delayMillis(10);
	}
	_input->discardMouseTransitions();
	debugC(1, kDebugVideo,
		"Ripper: completed retail presentation text dismissal "
		"sequence=%u quit=%d",
		_sequenceId, _engine->shouldQuit());
	return !_engine->shouldQuit();
}

PresentationTextMediaCallback::PresentationTextMediaCallback(
		RipperEngine *engine, InputManager *input, const Common::String &text,
		int displayTop, uint videoMode) : _engine(engine),
		_control(engine, input, text, displayTop), _videoMode(videoMode) {
}

void PresentationTextMediaCallback::beginIavfSegment(uint sequenceId,
		uint frameCount, uint width, uint height) {
	_control.configureSegment(sequenceId, frameCount, width, height,
		_videoMode);
}

uint16 PresentationTextMediaCallback::service(uint frame) {
	return _control.service(frame);
}

void PresentationTextMediaCallback::transformPalette(byte *palette,
		uint colorCount) const {
	_engine->applySharedPalettePatch(palette, colorCount);
}

} // End of namespace Ripper
