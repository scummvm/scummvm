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

#include "engines/nancy/nancy.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/cameraaction.h"

#include "engines/nancy/ui/camera.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

// How long a control stays visibly pressed before it acts.
static const uint32 kButtonPressDelay = 200;
static const uint32 kMessageDuration = 1500;
static const uint32 kConfirmDuration = 15000;

void CameraAction::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);			// 0x00
	_buttonCursorType = stream.readUint16LE();	// 0x21

	for (uint i = 0; i < kNumControls; ++i) {	// 0x23
		readRect(stream, _buttons[i].destRect);
		readRect(stream, _buttons[i].srcRect);
	}

	readRect(stream, _pictureRect);				// 0xa3
	readRect(stream, _cameraFullRect);			// 0xb3
	readRect(stream, _confirmDeleteRect);		// 0xc3
	readRect(stream, _deletedRect);				// 0xd3

	_buttonSound.readData(stream);				// 0xe3

	// Trailing count-prefixed array of 23-byte give-up hotspots. The exit always
	// jumps to the scene's first frame.
	int16 numExitZones = stream.readSint16LE();
	for (int16 i = 0; i < numExitZones; ++i) {
		Common::Rect r;
		readRect(stream, r);
		uint16 cursorType = stream.readUint16LE();
		uint16 sceneID = stream.readUint16LE();
		int16 flagLabel = stream.readSint16LE();
		byte flagValue = stream.readByte();

		if (i == 0) {
			_exitHotspot = r;
			_exitCursorType = cursorType;
			_exitScene.sceneID = sceneID;
			_exitScene.frameID = 0;
			_exitFlag.label = flagLabel;
			_exitFlag.flag = flagValue;
		}
	}
}

void CameraAction::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	_pendingButton = -1;
	_takePictureRequested = false;
	_exitRequested = false;

	resetToBrowsing();
	redraw();
}

void CameraAction::resetToBrowsing() {
	_messageRect = Common::Rect();

	uint count = UI::Camera::numPictures();
	if (count) {
		_pictureIndex = (int)count - 1;
		_screen = kViewing;
	} else {
		_pictureIndex = -1;
		_screen = kEmpty;
	}
}

void CameraAction::showMessage(Screen screen) {
	_screen = screen;

	switch (screen) {
	case kCameraFull:
		_messageRect = _cameraFullRect;
		_messageEndTime = g_nancy->getTotalPlayTime() + kMessageDuration;
		break;
	case kConfirmDelete:
		_messageRect = _confirmDeleteRect;
		_messageEndTime = g_nancy->getTotalPlayTime() + kConfirmDuration;
		break;
	default:
		_messageRect = _deletedRect;
		_messageEndTime = g_nancy->getTotalPlayTime() + kMessageDuration;
		break;
	}
}

bool CameraAction::isButtonEnabled(uint button) const {
	if (_screen == kDeleted) {
		return false;
	}

	// Taking a picture is all an empty camera can do.
	if (button != kTakePicture && _screen == kEmpty) {
		return false;
	}

	switch (button) {
	case kDelete:
		return _screen != kConfirmDelete;
	case kPrevious:
		return _screen != kViewing || _pictureIndex > 0;
	case kNext:
		return _screen != kViewing || _pictureIndex + 1 < (int)UI::Camera::numPictures();
	default:
		return true;
	}
}

int CameraAction::buttonAtCursor(const Common::Point &mousePos) const {
	for (uint i = 0; i < kNumControls; ++i) {
		if (!_buttons[i].destRect.isEmpty() &&
				NancySceneState.getViewport().convertViewportToScreen(_buttons[i].destRect).contains(mousePos)) {
			return isButtonEnabled(i) ? (int)i : -1;
		}
	}

	return -1;
}

void CameraAction::pressButton(uint button) {
	switch (button) {
	case kTakePicture: {
		auto *uicm = GetEngineData(UICM);
		if (uicm && UI::Camera::numPictures() >= uicm->maxPictures) {
			showMessage(kCameraFull);
		} else {
			_takePictureRequested = true;
		}

		break;
	}
	case kDelete:
		showMessage(kConfirmDelete);
		break;
	case kPrevious:
		// Yes, while confirming a deletion.
		if (_screen == kConfirmDelete) {
			deleteCurrentPicture();
			showMessage(kDeleted);
		} else if (_screen == kViewing) {
			--_pictureIndex;
		}

		break;
	case kNext:
		// No, while confirming a deletion.
		if (_screen == kConfirmDelete) {
			resetToBrowsing();
		} else if (_screen == kViewing) {
			++_pictureIndex;
		}

		break;
	default:
		break;
	}
}

void CameraAction::deleteCurrentPicture() {
	CellPhonePictureData *pd =
		(CellPhonePictureData *)NancySceneState.getPuzzleData(CellPhonePictureData::getTag());
	if (!pd || _pictureIndex < 0 || _pictureIndex >= (int)pd->pictures.size()) {
		return;
	}

	// A subject stays "photographed" only while some picture still holds it.
	const Common::Array<int16> subjects = pd->pictures[_pictureIndex].subjects;
	pd->pictures.remove_at(_pictureIndex);

	for (int16 subjectID : subjects) {
		bool stillHeld = false;
		for (const CapturedPicture &pic : pd->pictures) {
			for (int16 held : pic.subjects) {
				if (held == subjectID) {
					stillHeld = true;
					break;
				}
			}

			if (stillHeld) {
				break;
			}
		}

		if (!stillHeld) {
			NancySceneState.setEventFlag(subjectID, g_nancy->_false);
		}
	}

	UI::Camera::setPictureCount(pd->pictures.size());
}

void CameraAction::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	if (_pendingButton >= 0) {
		const Button &button = _buttons[_pendingButton];
		_drawSurface.blitFrom(_image, button.srcRect,
			Common::Point(button.destRect.left, button.destRect.top));
	}

	if (!_messageRect.isEmpty()) {
		// A message plate takes over the display area.
		_drawSurface.blitFrom(_image, _messageRect,
			Common::Point(_pictureRect.left, _pictureRect.top));
	} else if (_screen == kViewing) {
		const CellPhonePictureData *pd =
			(const CellPhonePictureData *)NancySceneState.getPuzzleData(CellPhonePictureData::getTag());
		if (pd && _pictureIndex >= 0 && _pictureIndex < (int)pd->pictures.size()) {
			const CapturedPicture &pic = pd->pictures[_pictureIndex];
			if (pic.width && pic.height &&
					pic.pixels.size() >= (uint)pic.width * (uint)pic.height * 4) {
				// Pictures are captured at the display area's size.
				Graphics::Surface src;
				src.init(pic.width, pic.height, pic.width * 4,
					const_cast<byte *>(pic.pixels.data()),
					g_nancy->_graphics->getScreenPixelFormat());

				Common::Rect srcRect(MIN<int16>(pic.width, _pictureRect.width()),
					MIN<int16>(pic.height, _pictureRect.height()));
				_drawSurface.blitFrom(src, srcRect,
					Common::Point(_pictureRect.left, _pictureRect.top));
			}
		}
	}

	_needsRedraw = true;
}

void CameraAction::handleInput(NancyInput &input) {
	if (_state != kRun || _takePictureRequested || _exitRequested) {
		return;
	}

	if (!_exitHotspot.isEmpty() &&
			NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)_exitCursorType, true);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_exitRequested = true;
		}

		input.eatMouseInput();
		return;
	}

	int button = buttonAtCursor(input.mousePos);
	if (button < 0) {
		return;
	}

	g_nancy->_cursor->setCursorType((CursorManager::CursorType)_buttonCursorType, true);

	if ((input.input & NancyInput::kLeftMouseButtonUp) && _pendingButton < 0) {
		UI::Camera::playSoundBlock(_buttonSound);
		_pendingButton = button;
		_pendingTime = g_nancy->getTotalPlayTime() + kButtonPressDelay;
		redraw();
	}

	input.eatMouseInput();
}

void CameraAction::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		break;
	case kRun:
		if (_exitRequested) {
			NancySceneState.setEventFlag(_exitFlag);
			NancySceneState.changeScene(_exitScene);
			break;
		}

		if (_takePictureRequested) {
			_state = kActionTrigger;
			break;
		}

		if (!_messageRect.isEmpty() && g_nancy->getTotalPlayTime() > _messageEndTime) {
			// Every plate, the delete prompt included, times out.
			resetToBrowsing();
			redraw();
			break;
		}

		if (_pendingButton >= 0 && g_nancy->getTotalPlayTime() > _pendingTime) {
			uint button = (uint)_pendingButton;
			_pendingButton = -1;
			pressButton(button);
			redraw();
		}

		break;
	case kActionTrigger:
		// Back to the scene the camera was opened from, viewfinder up.
		NancySceneState.popScene(true);
		if (UI::Camera *camera = NancySceneState.getCamera()) {
			camera->activate();
		}

		finishExecution();
		break;
	}
}

} // End of namespace Action
} // End of namespace Nancy
