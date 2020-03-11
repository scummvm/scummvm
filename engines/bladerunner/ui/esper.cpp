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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bladerunner/ui/esper.h"

#include "bladerunner/actor.h"
#include "bladerunner/ambient_sounds.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/debugger.h"
#include "bladerunner/decompress_lcw.h"
#include "bladerunner/font.h"
#include "bladerunner/game_info.h"
#include "bladerunner/mouse.h"
#include "bladerunner/scene.h"
#include "bladerunner/shape.h"
#include "bladerunner/script/esper_script.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/time.h"
#include "bladerunner/ui/ui_image_picker.h"
#include "bladerunner/vqa_player.h"
#include "bladerunner/subtitles.h"

#include "common/rect.h"
#include "common/str.h"

namespace BladeRunner {

ESPER::ESPER(BladeRunnerEngine *vm) {
	_vm = vm;

	_screen = Common::Rect(135, 123, 435, 387);

	_isWaiting          = false;
	_regionSelectedAck  = false;
	_isDrawingSelection = false;

	_isOpen             = false;
	_shapesButtons      = new Shapes(vm);
	_shapesPhotos       = new Shapes(vm);
	_shapeThumbnail     = nullptr;
	_vqaPlayerMain      = nullptr;
	_vqaPlayerPhoto     = nullptr;
	_script             = nullptr;

	_buttons = new UIImagePicker(vm, kPhotoCount + 4);

	reset();
}

ESPER::~ESPER() {
	reset();

	delete _buttons;
	delete _shapesPhotos;
	delete _shapesButtons;
}

void ESPER::open(Graphics::Surface *surface) {
	// CD-changing logic has been removed

	while (!_vm->playerHasControl()) {
		_vm->playerGainsControl();
	}

	while (_vm->_mouse->isDisabled()) {
		_vm->_mouse->enable();
	}

	_vm->_time->pause();

	_ambientVolume = _vm->_ambientSounds->getVolume();
	_vm->_ambientSounds->setVolume(_ambientVolume / 2);

	reset();

	if (!_vm->openArchive("MODE.MIX")) {
		return;
	}

	_surfacePhoto.create(kPhotoWidth, kPhotoHeight, gameDataPixelFormat());
	_surfaceViewport.create(_screen.width(), _screen.height(), screenPixelFormat());

	_viewportNext = _viewport;

	if (!_shapesButtons->load("ESPBUTTN.SHP")) {
		return;
	}

	if (!_shapesPhotos->load("ESPTHUMB.SHP")) {
		return;
	}

	_vqaPlayerMain = new VQAPlayer(_vm, &_vm->_surfaceBack, "ESPER.VQA");
	if (!_vqaPlayerMain->open()) {
		return;
	}
	_vqaPlayerMain->setLoop(2, -1, kLoopSetModeJustStart, nullptr, nullptr);

	_isOpen = true;
	_flash = false;

	_script = new ESPERScript(_vm);

	activate(true);
}

void ESPER::close() {
	// CD-changing logic has been removed
	delete _script;
	_script = nullptr;

	_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBR035_7B), 25, 0, 0, 50, 0);

	unloadPhotos();

	_buttons->deactivate();
	_buttons->resetImages();

	_shapesButtons->unload();
	_shapesPhotos->unload();

	_surfacePhoto.free();
	_surfaceViewport.free();

	delete _vqaPlayerMain;
	_vqaPlayerMain = nullptr;

	_vm->closeArchive("MODE.MIX");

	_vm->_time->resume();

	_vm->_ambientSounds->setVolume(_ambientVolume);
	_vm->_scene->resume();

	reset();
}

bool ESPER::isOpen() const {
	return _isOpen;
}

void ESPER::handleMouseUp(int x, int y, bool mainButton) {
	bool actionHandled = _buttons->handleMouseAction(x, y, false, true, false);
	if (mainButton) {
		_isMouseDown = false;
		if (!actionHandled) {
			if (_isScrolling) {
				scrollingStop();
			} else if (_isDrawingSelection && _mouseOverScroll == 4) {
				_isDrawingSelection = false;
				resetSelectionRect();
			}
		}
	} else if (_statePhoto == kEsperPhotoStatePhotoZoomOut) {
		zoomOutStop();
	}
}

void ESPER::handleMouseDown(int x, int y, bool mainButton) {
	bool actionHandled = _buttons->handleMouseAction(x, y, true, false, false);

	if (actionHandled || _vm->_mouse->isDisabled()) {
		return;
	}

	if (mainButton) {
		if (_statePhoto != kEsperPhotoStateVideoZoomOut) {
			if (_screen.contains(x, y)) {
				_isMouseDown = true;
				playSound(kSfxBRTARGET, 100);
			}
			if (_mouseOverScroll >= 0 && _mouseOverScroll <= 3 && !_isScrolling) {
				scrollingStart(_mouseOverScroll);
			}
			tick();
		}
	} else {
		if (_statePhoto == kEsperPhotoStateShow || _statePhoto == kEsperPhotoStateVideoShow) {
			zoomOutStart();
		}
	}
}

void ESPER::tick() {
	if (!_vm->_windowIsActive) {
		return;
	}

	tickSound();

	blit(_vm->_surfaceBack, _vm->_surfaceFront);

	int mouseX, mouseY;
	_vm->_mouse->getXY(&mouseX, &mouseY);
	if (!_vm->_mouse->isDisabled()) {
		_buttons->handleMouseAction(mouseX, mouseY, false, false, false);
	}

	if (!_isOpen) {
		return;
	}

	draw(_vm->_surfaceFront);
	_buttons->draw(_vm->_surfaceFront);
	drawMouse(_vm->_surfaceFront);

	tickSound();
	_vm->_subtitles->tick(_vm->_surfaceFront);
	_vm->blitToScreen(_vm->_surfaceFront);

	if (_statePhoto == kEsperPhotoStateVideoShow) {
		if (_regionSelectedAck)	{
			_regionSelectedAck = false;
			_script->specialRegionSelected(_photoIdSelected, _regions[_regionSelected].regionId);
		}
	}
}

void ESPER::addPhoto(const char *name, int photoId, int shapeId) {
	int i = findEmptyPhoto();
	if (i >= 0) {
		_photos[i].shapeId   = shapeId;
		_photos[i].isPresent = true;
		_photos[i].photoId   = photoId;
		_photos[i].name      = name;

		_buttons->defineImage(i,
			Common::Rect(
				100 * (i % 3) + _screen.left + 3,
				 66 * (i / 3) + _screen.top  + 3,
				100 * (i % 3) + _screen.left + 100 - 3,
				 66 * (i / 3) + _screen.top  +  66 - 3
			),
			_shapesPhotos->get(shapeId),
			_shapesPhotos->get(shapeId),
			_shapesPhotos->get(shapeId),
			nullptr);
	}
	playSound(kSfxBR028_2A, 25);
	wait(300);
	tick();
}

void ESPER::defineRegion(int regionId, Common::Rect inner, Common::Rect outer, Common::Rect selection, const char *name) {
	int i = findEmptyRegion();
	if (i >= 0) {
		_regions[i].isPresent     = true;
		_regions[i].regionId      = regionId;
		_regions[i].rectInner     = inner;
		_regions[i].rectOuter     = outer;
		_regions[i].rectSelection = selection;
		_regions[i].name          = name;
	}
}

void ESPER::mouseDownCallback(int buttonId, void *callbackData) {
	ESPER *self = ((ESPER *)callbackData);
	if (self->_statePhoto != kEsperPhotoStateVideoZoomOut && buttonId == kPhotoCount + 2) {
		self->zoomOutStart();
	}
}

void ESPER::mouseUpCallback(int buttonId, void *callbackData) {
	ESPER *self = (ESPER *)callbackData;
	if (buttonId < kPhotoCount) {
		self->selectPhoto(buttonId);
	} else if (self->_statePhoto != kEsperPhotoStateVideoZoomOut) {
		if (buttonId == kPhotoCount + 2) {
			self->zoomOutStop();
		} else if (buttonId == kPhotoCount + 3) {
			self->goBack();
		}
	}
}

void ESPER::reset() {
	_surfacePhoto.free();
	_surfaceViewport.free();

	_shapesButtons->unload();
	_shapesPhotos->unload();

	delete _vqaPlayerMain;
	_vqaPlayerMain = nullptr;

	delete _vqaPlayerPhoto;
	_vqaPlayerPhoto = nullptr;

	delete _script;
	_script = nullptr;

	_isOpen = false;

	resetData();
}

void ESPER::resetData() {
	delete _vqaPlayerPhoto;
	_vqaPlayerPhoto = nullptr;

	_shapeThumbnail = nullptr;

	_viewport     = Common::Rect();
	_viewportNext = Common::Rect();
	_selection    = Common::Rect(-1, -1, -1, -1);

	_regionSelectedAck = false;

	_mouseOverScroll    = -1;
	_scrollingDirection = -1;

	_selectionCrosshairX = -1;
	_selectionCrosshairY = -1;

	_stateMain  = kEsperMainStatePhoto;
	_statePhoto = kEsperPhotoStateShow;

	_isMouseDown        = false;
	_isDrawingSelection = false;
	_flash              = false;
	_isScrolling        = false;

	_timeScrollNextStart = 0u;

	resetPhotos();
	resetRegions();
	resetViewport();
	resetSelectionBlinking();
	prepareZoom();
	resetPhotoZooming();
	resetPhotoOpening();

	_soundId1 = -1;
	_soundId2 = -1;
	_soundId3 = -1;
}

void ESPER::resetPhotos() {
	for (int i = 0; i < kPhotoCount; ++i) {
		_photos[i].isPresent = false;
		_photos[i].photoId   = -1;
	}
}

void ESPER::resetRegions() {
	for (int i = 0; i < kRegionCount; ++i) {
		_regions[i].isPresent = false;
		_regions[i].regionId  = -1;
	}
}

void ESPER::resetViewport() {
	_zoomHorizontal = (float)(_screen.width())  / (float)kPhotoWidth;
	_zoomVertical   = (float)(_screen.height()) / (float)kPhotoHeight;
	_zoom           = _zoomVertical;
	_zoomMin        = _zoom;

	_timeZoomOutNextStart = 0u;

	_viewportPositionX = kPhotoWidth  / 2;
	_viewportPositionY = kPhotoHeight / 2;

	updateViewport();

	_screenHalfWidth  = _screen.width()  / 2;
	_screenHalfHeight = _screen.height() / 2;
}

void ESPER::resetSelectionRect() {
	_selection           = _screen;
	_selectionCrosshairX = -1;
	_selectionCrosshairY = -1;
}

void ESPER::resetSelectionBlinking() {
	_selectionBlinkingCounter       = 0;
	_selectionBlinkingStyle         = 0;
	_timeSelectionBlinkingNextStart = 0u;
}

void ESPER::resetPhotoZooming() {
	_zoomStep     = 0;
	_timeZoomNextDiff  = 0u;
	_timeZoomNextStart = 0u;
}

void ESPER::resetPhotoOpening() {
	_photoOpeningWidth    = _screen.left + 1;
	_photoOpeningHeight   = _screen.top  + 1;
	_timePhotoOpeningNextDiff  = 0u;
	_timePhotoOpeningNextStart = 0u;
}

void ESPER::updateViewport() {
	float halfWidth = (1.0f / 2.0f) * ((float)kPhotoWidth * (_zoomHorizontal / _zoom));

	_viewport.left  = _viewportPositionX - halfWidth;
	_viewport.right = _viewportPositionX + halfWidth;

	if (_viewport.left < 0) {
		_viewport.right -= _viewport.left;
		_viewport.left = 0;
	}
	if (_viewport.right >= kPhotoWidth) {
		_viewport.left -= _viewport.right - (kPhotoWidth - 1);
		if (_viewport.left < 0) {
			_viewport.left = 0;
		}
		_viewport.right = kPhotoWidth - 1;
	}

	float halfHeight = 1.0f / 2.0f * ((float)kPhotoHeight * (_zoomVertical / _zoom));

	_viewport.top    = _viewportPositionY - halfHeight;
	_viewport.bottom = _viewportPositionY + halfHeight;

	if (_viewport.top < 0) {
		_viewport.bottom -= _viewport.top;
		_viewport.top = 0;
	}
	if (_viewport.bottom >= kPhotoHeight) {
		_viewport.top -= _viewport.bottom - (kPhotoHeight - 1);
		if (_viewport.top < 0) {
			_viewport.top = 0;
		}
		_viewport.bottom = kPhotoHeight - 1;
	}

	_viewportWidth  = _viewport.right  + 1 - _viewport.left;
	_viewportHeight = _viewport.bottom + 1 - _viewport.top;

	int centerX = (_viewport.left + _viewport.right ) / 2;
	int centerY = (_viewport.top  + _viewport.bottom) / 2;

	float range = _zoom / _zoomHorizontal * 1.0f;
	if ((_viewportPositionX > centerX + range) || (_viewportPositionX < centerX - range)) {
		_viewportPositionX = centerX;
	}

	range = _zoom / _zoomVertical * 1.0f;
	if ((_viewportPositionY > centerY + range) || (_viewportPositionY < centerY - range)) {
		_viewportPositionY = centerY;
	}
}

void ESPER::activate(bool withOpening) {
	_vm->_mouse->disable();

	_buttons->resetImages();

	if (withOpening) {
		setStateMain(kEsperMainStateOpening);
		playSound(kSfxBR025_5A, 25);
		wait(1000);
		playSound(kSfxBR027_1P, 25);
		wait(2000);
	} else {
		_buttons->deactivate();
		setStateMain(kEsperMainStateClear);
	}

	_buttons->activate(nullptr, nullptr, mouseDownCallback, mouseUpCallback, this);
	_buttons->defineImage(kPhotoCount + 3, Common::Rect(42, 403, 76, 437), nullptr, nullptr, _shapesButtons->get(0), nullptr);

	playSound(kSfxBR024_4B, 25);
	wait(1000);

	setStateMain(kEsperMainStateList);
	resetPhotos();
	_script->initialize();

	_vm->_mouse->enable();
}

void ESPER::setStateMain(EsperMainStates state) {
	if (_isOpen) {
		_stateMain = state;
	}
}

void ESPER::setStatePhoto(EsperPhotoStates state) {
	_statePhoto = state;
}

void ESPER::wait(uint32 timeout) {
	if (!_isWaiting) {
		_isWaiting = true;
//		int timeEnd = timeout + _vm->_time->current();
		uint32  timeStart = _vm->_time->current();
		// unsigned difference is intentional
		while (_vm->_gameIsRunning && (_vm->_time->current() - timeStart < timeout)) {
			_vm->gameTick();
		}
		_isWaiting = false;
	}
}

void ESPER::playSound(int soundId, int volume) {
	if (_soundId1 == -1) {
		_soundId1 = soundId;
		_volume1  = volume;
	} else if (_soundId2 == -1) {
		_soundId2 = soundId;
		_volume2  = volume;
	} else if (_soundId3 == -1) {
		_soundId3 = soundId;
		_volume3  = volume;
	}
}

void ESPER::draw(Graphics::Surface &surface) {
	if (!_isOpen) {
		return;
	}
	_vqaPlayerMain->update(false);
	switch (_stateMain) {
	case kEsperMainStateOpening:
	case kEsperMainStateList:
		return;
	case kEsperMainStatePhotoOpening:
		drawPhotoOpening(surface);
		break;
	case kEsperMainStateClear:
		surface.fillRect(_screen, surface.format.RGBToColor(0, 0, 0));
		break;
	case kEsperMainStatePhoto:
		if (_isScrolling) {
			tickScroll();
		}
		switch (_statePhoto) {
		case kEsperPhotoStateShow:
			drawPhotoWithGrid(surface);
			if (_isDrawingSelection) {
				drawSelection(surface, true, 1);
			}

			if (_vm->_debugger->_viewUI) {
				for (int i = 0; i < kRegionCount; ++i) {
					if (_regions[i].isPresent) {
						surface.frameRect(
							Common::Rect(
								viewportXToScreenX(_regions[i].rectInner.left),
								viewportYToScreenY(_regions[i].rectInner.top),
								viewportXToScreenX(_regions[i].rectInner.right),
								viewportYToScreenY(_regions[i].rectInner.bottom)
							),
							surface.format.RGBToColor(248, 248, 0)
						);
						surface.frameRect(
							Common::Rect(
								viewportXToScreenX(_regions[i].rectOuter.left),
								viewportYToScreenY(_regions[i].rectOuter.top),
								viewportXToScreenX(_regions[i].rectOuter.right),
								viewportYToScreenY(_regions[i].rectOuter.bottom)
							),
							surface.format.RGBToColor(248, 248, 0)
						);
					}
				}
			}
			break;
		case kEsperPhotoStateScrolling:
			scrollUpdate();
			drawPhotoWithGrid(surface);
			break;
		case kEsperPhotoStateSelectionZooming:
			drawPhotoWithGrid(surface);
			if (!drawSelectionZooming(surface)) {
				setStatePhoto(kEsperPhotoStateSelectionBlinking);
				playSound(kSfxBR030_3A, 25);
			}
			break;
		case kEsperPhotoStateSelectionBlinking:
			drawPhotoWithGrid(surface);
			if (!drawSelectionBlinking(surface)) {
				setStatePhoto(kEsperPhotoStatePhotoZooming);
			}
			break;
		case kEsperPhotoStatePhotoZooming:
			drawPhotoZooming(surface);
			break;
		case kEsperPhotoStatePhotoSharpening:
			drawPhotoSharpening(surface);
			break;
		case kEsperPhotoStatePhotoZoomOut:
			drawPhotoZoomOut(surface);
			break;
		case kEsperPhotoStateVideoZooming:
			drawVideoZooming(surface);
			break;
		case kEsperPhotoStateVideoShow:
			drawVideoFrame(surface);
			drawGrid(surface);
			break;
		case kEsperPhotoStateVideoZoomOut:
			drawVideoZoomOut(surface);
			break;
		default:
			break;
		}
		drawTextCoords(surface);

		break;
	}
}

void ESPER::drawPhotoOpening(Graphics::Surface &surface) {
	bool needMoreZooming = true;
	uint32 timeNow = _vm->_time->current();
	// unsigned difference is intentional
	if (timeNow - _timePhotoOpeningNextStart >= _timePhotoOpeningNextDiff) {
		_photoOpeningWidth  = MIN(_photoOpeningWidth  + 8, _screen.right  - 1);
		_photoOpeningHeight = MIN(_photoOpeningHeight + 7, _screen.bottom - 1);

		if (_photoOpeningWidth == _screen.right - 1 && _photoOpeningHeight == _screen.bottom - 1) {
			needMoreZooming = false;
		}
		_timePhotoOpeningNextDiff  = 20u;
		_timePhotoOpeningNextStart = timeNow;
	}
	copyImageScale(_surfacePhoto, _viewport, surface, Common::Rect(_screen.left, _screen.top, _photoOpeningWidth, _photoOpeningHeight));

	surface.hLine(_screen.left,           _photoOpeningHeight,     _screen.right  - 1, surface.format.RGBToColor(0, 248, 0));
	surface.vLine(_photoOpeningWidth,     _screen.top,             _screen.bottom - 1, surface.format.RGBToColor(0, 248, 0));
	surface.hLine(_screen.left,           _photoOpeningHeight - 1, _screen.right  - 1, surface.format.RGBToColor(0, 144, 0));
	surface.vLine(_photoOpeningWidth - 1, _screen.top,             _screen.bottom - 1, surface.format.RGBToColor(0, 144, 0));

	drawGrid(surface);

	if (!needMoreZooming) {
		setStateMain(kEsperMainStatePhoto);
		setStatePhoto(kEsperPhotoStateShow);
		_vm->_mouse->enable();
	}
}

bool ESPER::drawSelectionZooming(Graphics::Surface &surface) {
	bool zooming = false;
	bool needMoreZooming = true;
	uint32 timeNow = _vm->_time->current();
	// unsigned difference is intentional
	if (timeNow - _timeSelectionZoomNextStart > 150u) {
		zooming = true;
		_selection.left   += _selectionDelta.left;
		_selection.top    += _selectionDelta.top;
		_selection.right  += _selectionDelta.right;
		_selection.bottom += _selectionDelta.bottom;
		++_selectionZoomStep;
		_timeSelectionZoomNextStart = timeNow;
		if (_selectionZoomStep > kSelectionZoomSteps) {
			needMoreZooming = false;
			_selection.left   = _selectionTarget.left;
			_selection.top    = _selectionTarget.top;
			_selection.right  = _selectionTarget.right;
			_selection.bottom = _selectionTarget.bottom;
		}
	}
	drawSelection(surface, false, 1);
	if (!needMoreZooming) {
		_statePhoto = kEsperPhotoStatePhotoZooming;
		resetPhotoZooming();
		zooming = false;
	}
	if (zooming) {
		playSound(kSfxBR029_3A, 20);
	}
	return needMoreZooming;
}

bool ESPER::drawSelectionBlinking(Graphics::Surface &surface) {
	bool needMoreBlinking = true;
	uint32 timeNow = _vm->_time->current();
	// unsigned difference is intentional
	if (timeNow - _timeSelectionBlinkingNextStart > 100u) {
		_timeSelectionBlinkingNextStart = timeNow;
		_selectionBlinkingStyle ^= 1;
		++_selectionBlinkingCounter;
		if (_selectionBlinkingCounter > 10) {
			needMoreBlinking = false;
			_selectionBlinkingStyle = 0;
		}
	}
	drawSelection(surface, false, _selectionBlinkingStyle);
	if (!needMoreBlinking) {
		resetSelectionBlinking();
	}
	return needMoreBlinking;
}

void ESPER::drawPhotoZooming(Graphics::Surface &surface) {
	uint32 timeNow = _vm->_time->current();
	// unsigned difference is intentional
	if ((timeNow - _timeZoomNextStart > _timeZoomNextDiff) && (_zoomStep < _zoomSteps)) {
		_flash = true;

		_viewportPositionXCurrent += _viewportPositionXDelta;
		_viewportPositionYCurrent += _viewportPositionYDelta;
		_viewportPositionX         = _viewportPositionXCurrent;
		_viewportPositionY         = _viewportPositionYCurrent;

		_zoom += _zoomDelta;
		if (_zoomDelta > 0.0f) {
			if (_zoom > _zoomTarget) {
				_zoom = _zoomTarget;
				_zoomStep = _zoomSteps;
			} else {
				_blur += _zoomDelta * 2.0f;
			}
		} else if (_zoomDelta < 0.0f) {
			if (_zoom < _zoomTarget) {
				_zoom = _zoomTarget;
				_zoomStep = _zoomSteps;
			}
		}
		++_zoomStep;
		if (_zoomStep >= _zoomSteps) {
			_zoom = _zoomTarget;
			_viewportPositionX = _viewportPositionXTarget;
			_viewportPositionY = _viewportPositionYTarget;
		}
		updateViewport();
		_timeZoomNextDiff  = 300u;
		_timeZoomNextStart = timeNow;
	}

	if (_zoomDelta >= 0.0f) {
		drawPhoto(surface);
	} else {
		drawPhotoWithGrid(surface);
	}
	drawGrid(surface);

	// unsigned difference is intentional
	if ((timeNow - _timeZoomNextStart > _timeZoomNextDiff) && (_zoomStep >= _zoomSteps)) {
		if (_regionSelectedAck) {
			if (!_regions[_regionSelected].name.empty()) {
				if (_zoomDelta < 0.0f) {
					_blur = 1.0f;
					_zoomDelta = (_zoom * 1.5f - _zoom) / (float)_zoomSteps; // 0.5f * _zoom ???
				}
				setStatePhoto(kEsperPhotoStateVideoZooming);
				_timeZoomNextDiff += 300u;
			} else {
				_regionSelectedAck = false;
				_selection.left   = viewportXToScreenX(_regions[_regionSelected].rectInner.left);
				_selection.right  = viewportXToScreenX(_regions[_regionSelected].rectInner.right);
				_selection.top    = viewportYToScreenY(_regions[_regionSelected].rectInner.top);
				_selection.bottom = viewportYToScreenY(_regions[_regionSelected].rectInner.bottom);
				prepareZoom();
				resetPhotoZooming();
				updateSelection();
				setStatePhoto(kEsperPhotoStatePhotoZooming);
			}
		} else {
			setStatePhoto(kEsperPhotoStatePhotoSharpening);
		}
		resetPhotoOpening();
	}
}

void ESPER::drawPhotoSharpening(Graphics::Surface &surface) {
	uint32 timeNow = _vm->_time->current();
	bool needMoreSharpening = true;
	// unsigned difference is intentional
	if (timeNow - _timePhotoOpeningNextStart >= _timePhotoOpeningNextDiff) {
		_photoOpeningWidth  = MIN(_photoOpeningWidth  + 8, _screen.right  - 1);
		_photoOpeningHeight = MIN(_photoOpeningHeight + 7, _screen.bottom - 1);

		if (_photoOpeningWidth == _screen.right - 1 && _photoOpeningHeight == _screen.bottom - 1) {
			needMoreSharpening = false;
		}

		_timePhotoOpeningNextDiff  = 50u;
		_timePhotoOpeningNextStart = timeNow;
	}

	if (_regionSelectedAck && !_regions[_regionSelected].name.empty()) {
		_vqaPlayerPhoto->update(true, false);
		copyImageBlur(_surfaceViewport, Common::Rect(0, 0, 299, 263), surface, _screen, _blur);
		copyImageBlit(_surfaceViewport, Common::Rect(0, 0, 0, 0), surface, Common::Rect(_screen.left, _screen.top, _photoOpeningWidth, _photoOpeningHeight));
	} else {
		drawPhoto(surface);
		copyImageScale(_surfacePhoto, _viewport, _surfaceViewport, Common::Rect(0, 0, _screen.width(), _screen.height()));
		copyImageBlit(_surfaceViewport, Common::Rect(0, 0, 0, 0), surface, Common::Rect(_screen.left, _screen.top, _photoOpeningWidth, _photoOpeningHeight));

	}
	drawGrid(surface);
	surface.hLine(_screen.left,           _photoOpeningHeight,     _screen.right  - 1, surface.format.RGBToColor(0, 248, 0));
	surface.vLine(_photoOpeningWidth,     _screen.top,             _screen.bottom - 1, surface.format.RGBToColor(0, 248, 0));
	surface.hLine(_screen.left,           _photoOpeningHeight - 1, _screen.right  - 1, surface.format.RGBToColor(0, 144, 0));
	surface.vLine(_photoOpeningWidth - 1, _screen.top,             _screen.bottom - 1, surface.format.RGBToColor(0, 144, 0));
	if (!needMoreSharpening) {
		if (_regionSelectedAck && !_regions[_regionSelected].name.empty()) {
			setStatePhoto(kEsperPhotoStateVideoShow);
		} else {
			setStatePhoto(kEsperPhotoStateShow);
		}
		resetPhotoZooming();
		resetPhotoOpening();
		_vm->_mouse->enable();
	}
}

void ESPER::drawPhotoZoomOut(Graphics::Surface &surface) {
	uint32 timeNow = _vm->_time->current();
	// unsigned difference is intentional
	if (timeNow - _timeZoomOutNextStart >= 300u) {
		_timeZoomOutNextStart = timeNow;

		if (_zoom > _zoomMin) {
			_zoom /= 1.3f;
			_flash = true;
			if (_zoomHorizontal <= _zoomVertical) {
				if (_zoom < _zoomVertical) {
					_zoom = _zoomVertical;
				}
			} else {
				if (_zoom < _zoomHorizontal) {
					_zoom = _zoomHorizontal;
				}
			}
			updateViewport();
		} else {
			_statePhoto = kEsperPhotoStateShow;
		}
	}
	drawPhotoWithGrid(surface);
}

void ESPER::drawVideoZooming(Graphics::Surface &surface) {
	if (_vqaPlayerPhoto == nullptr) {
		_vqaPlayerPhoto = new VQAPlayer(_vm, &_surfaceViewport, Common::String(_regions[_regionSelected].name) + ".VQA");
		if (!_vqaPlayerPhoto->open()) {
			setStatePhoto(kEsperPhotoStateShow);
			_vm->_mouse->enable();

			delete _vqaPlayerPhoto;
			_vqaPlayerPhoto = nullptr;

			return;
		}

		_timeZoomNextDiff  = 0u;
		_timeZoomNextStart = 0u;
	}

	bool flash = false;
	bool advanceFrame = false;
	uint32 timeNow = _vm->_time->current();
	// unsigned difference is intentional
	if (timeNow - _timeZoomNextStart > _timeZoomNextDiff) {
		_timeZoomNextDiff  = 300u;
		_timeZoomNextStart = timeNow;
		playSound(kSfxBR031_1P, 25);
		flash = true;
		advanceFrame = true;
		_blur += _zoomDelta * 5.0f;
	}

	int frame = _vqaPlayerPhoto->update(true, advanceFrame);
	if (frame == _vqaPlayerPhoto->getFrameCount() - 1) {
		_vqaLastFrame = frame;
		setStatePhoto(kEsperPhotoStatePhotoSharpening);
	} else if (flash) {
		// TODO? Temporary workaround for very noticeable blue tint in the first frame during zoom-out:
		// Don't flash for the last frame of the photo (which is the starting frame when zooming out)
		flashViewport();
	}

	copyImageBlur(_surfaceViewport, Common::Rect(0, 0, 299, 263), surface, _screen, _blur);
	drawGrid(surface);
}

void ESPER::drawVideoZoomOut(Graphics::Surface &surface) {
	bool flash = false;
	bool advanceFrame = false;
	uint32 timeNow = _vm->_time->current();
	// unsigned difference is intentional
	if (timeNow - _timeZoomNextStart > _timeZoomNextDiff && _vqaLastFrame > 0) {
		_timeZoomNextDiff  = 300u;
		_timeZoomNextStart = timeNow;
		playSound(kSfxBR031_1P, 25);
		_vqaPlayerPhoto->seekToFrame(_vqaLastFrame);
		int nextFrame = _vqaPlayerPhoto->getFrameCount() / 4;
		if (nextFrame <= 0) {
			nextFrame = 1;
		} else if (nextFrame > 4) {
			nextFrame = 4;
		}

		if (_vqaLastFrame < _vqaPlayerPhoto->getFrameCount() - 1) {
			// TODO? Temporary workaround for persistent blue tint in the last frame:
			// Don't flash for the last frame of the photo (starting frame when zooming out)
			flash = true;
		}
		advanceFrame = true;
		_vqaLastFrame -= nextFrame;
	}

	_vqaPlayerPhoto->update(true, advanceFrame);
	if (flash) {
		flashViewport();
	}
	copyImageBlit(_surfaceViewport, Common::Rect(0, 0, 0, 0), surface, _screen);
	drawGrid(surface);
	// unsigned difference is intentional
	if (timeNow - _timeZoomNextStart > _timeZoomNextDiff && _vqaLastFrame <= 0) {
		_vqaPlayerPhoto->close();
		delete _vqaPlayerPhoto;
		_vqaPlayerPhoto = nullptr;

		if (_vm->isMouseButtonDown()) {
			zoomOutStart();
		} else {
			zoomOutStop();
		}
	}
}

void ESPER::drawPhoto(Graphics::Surface &surface) {
	copyImageBlur(_surfacePhoto, _viewport, surface, _screen, _blur);
}

void ESPER::drawGrid(Graphics::Surface &surface) {
	for (int i = 0; i < 7; ++i) {
		surface.drawLine(_screen.left + i * 50, _screen.top, _screen.left + i * 50, _screen.bottom - 1, surface.format.RGBToColor(32, 32, 224));
	}

	for (int i = 0; i < 7; ++i) {
		surface.drawLine(_screen.left, _screen.top + i * 44, _screen.right - 1, _screen.top + i * 44, surface.format.RGBToColor(32, 32, 224));
	}
}

void ESPER::drawPhotoWithGrid(Graphics::Surface &surface) {
	copyImageScale(_surfacePhoto, _viewport, surface, _screen);
	drawGrid(surface);
}

void ESPER::drawSelection(Graphics::Surface &surface, bool crosshair, int style) {
	int left   = CLIP(_selection.left,   _screen.left, (int16)(_screen.right  - 1));
	int top    = CLIP(_selection.top,    _screen.top,  (int16)(_screen.bottom - 1));
	int right  = CLIP(_selection.right,  _screen.left, (int16)(_screen.right  - 1));
	int bottom = CLIP(_selection.bottom, _screen.top,  (int16)(_screen.bottom - 1));

	int color = surface.format.RGBToColor(0, 144, 0);
	if (style) {
		color = surface.format.RGBToColor(0, 248, 0);
	}

	// selection rectangle
	Common::Rect selectedRect(MIN(left, right), MIN(top, bottom), MAX(left, right) + 1, MAX(top, bottom) + 1);
	Common::Rect selectedRect2 = selectedRect;
	selectedRect2.grow(-1);
	surface.frameRect(selectedRect, color);
	surface.frameRect(selectedRect2, color);

	if (crosshair) {
		if (_selectionCrosshairX == -1) {
			if (_selection.left < (_screen.left + _screen.right) / 2) {
				_selectionCrosshairX = _screen.left;
			} else {
				_selectionCrosshairX = _screen.right - 1;
			}
		}
		if (_selectionCrosshairY == -1) {
			if (_selection.top < (_screen.top + _screen.bottom) / 2) {
				_selectionCrosshairY = _screen.top;
			} else {
				_selectionCrosshairY = _screen.bottom - 1;
			}
		}

		// ghosting
		if (_selectionCrosshairX != right) {
			surface.vLine(_selectionCrosshairX, _screen.top, _screen.bottom - 1, surface.format.RGBToColor(0, 144, 0));
			if (abs(_selectionCrosshairX - right) <= 1) {
				_selectionCrosshairX = right;
			} else {
				_selectionCrosshairX = (_selectionCrosshairX + right) / 2;
			}
		}
		if (_selectionCrosshairY != bottom) {
			surface.hLine(_screen.left, _selectionCrosshairY, _screen.right - 1, surface.format.RGBToColor(0, 144, 0));
			if (abs(_selectionCrosshairY - bottom) <= 1) {
				_selectionCrosshairY = bottom;
			} else {
				_selectionCrosshairY = (_selectionCrosshairY + bottom) / 2;
			}
		}

		surface.vLine(right,        _screen.top, _screen.bottom - 1, surface.format.RGBToColor(0, 248, 0));
		surface.hLine(_screen.left, bottom,      _screen.right  - 1, surface.format.RGBToColor(0, 248, 0));
	}
}

void ESPER::drawVideoFrame(Graphics::Surface &surface) {
	_vqaPlayerPhoto->update(true, false);
	copyImageBlit(_surfaceViewport, Common::Rect(0, 0, 0, 0), surface, _screen);
}

void ESPER::drawTextCoords(Graphics::Surface &surface) {
	const char *zm = "ZM %04.0f";
	const char *ns = "NS %04d";
	const char *ew = "EW %04d";
	if (_vm->_language == Common::RU_RUS) {
		// ПР, ВР, ГР
		if (_vm->_russianCP1251) {
			// Patched transalation by Siberian Studio is using Windows-1251 encoding
			zm = "\xcf\xd0 %04.0f";
			ns = "\xc2\xd0 %04d";
			ew = "\xc3\xd0 %04d";
		} else {
			// Original release uses custom encoding
			zm = "gh %04.0f";
			ns = "dh %04d";
			ew = "uh %04d";
		}
	}
	_vm->_mainFont->drawString(&surface, Common::String::format(zm, _zoom / _zoomMin * 2.0f  ), 155, 364, surface.w, surface.format.RGBToColor(0, 0, 255));
	_vm->_mainFont->drawString(&surface, Common::String::format(ns, 12 * _viewport.top  +  98), 260, 364, surface.w, surface.format.RGBToColor(0, 0, 255));
	_vm->_mainFont->drawString(&surface, Common::String::format(ew, 12 * _viewport.left + 167), 364, 364, surface.w, surface.format.RGBToColor(0, 0, 255));
}

void ESPER::drawMouse(Graphics::Surface &surface) {
	if (_vm->_mouse->isDisabled()) {
		return;
	}

	int cursor = -1;

	Common::Point p = _vm->getMousePos();

	_mouseOverScroll = 4;
	if (_stateMain == kEsperMainStatePhoto) {
		if (_screen.contains(p)) {
			if ((_statePhoto == kEsperPhotoStateShow) && ( _zoom != 2.0f)) {
				if (_isMouseDown) {
					if (_isDrawingSelection) {
						_selection.right  = p.x;
						_selection.bottom = p.y;
					} else {
						_selection.left   = p.x;
						_selection.top    = p.y;
						_selection.right  = p.x + 1;
						_selection.bottom = p.y + 1;
						_isDrawingSelection = true;
					}
				} else {
					if (_isDrawingSelection) {
						_selection.right  = p.x;
						_selection.bottom = p.y;

						if (_selection.right < _selection.left) {
							SWAP(_selection.left, _selection.right);
						}
						if (_selection.bottom < _selection.top) {
							SWAP(_selection.bottom, _selection.top);
						}

						if (_selection.right >= _selection.left + 3) {
							updateSelection();
							_vm->_mouse->disable();
							zoomingStart();
						} else {
							resetSelectionRect();
						}
					}
					_isDrawingSelection = false;
				}
			}
			surface.vLine(p.x,     p.y - 8, p.y - 1, surface.format.RGBToColor(0, 248, 0));
			surface.vLine(p.x,     p.y + 8, p.y + 1, surface.format.RGBToColor(0, 248, 0));
			surface.hLine(p.x - 8, p.y,     p.x - 1, surface.format.RGBToColor(0, 248, 0));
			surface.hLine(p.x + 8, p.y,     p.x + 1, surface.format.RGBToColor(0, 248, 0));
			_mouseOverScroll = -1;
		} else if (p.x >= 85 && p.y >= 73 && p.x <= 484 && p.y <= 436) {
			if (!_isDrawingSelection && _statePhoto != kEsperPhotoStateVideoShow && _zoom != 2.0f) {
				_mouseOverScroll = (angle_1024((_screen.left + _screen.right) / 2, (_screen.top + _screen.bottom) / 2, p.x, p.y) + 128) / 256;
				if (_mouseOverScroll >= 4) {
					_mouseOverScroll = 0;
				}
				if (_mouseOverScroll == 0 && this->_viewport.top == 0) {
					_mouseOverScroll = 4;
				} else if (_mouseOverScroll == 1 && this->_viewport.right == kPhotoWidth - 1) {
					_mouseOverScroll = 4;
				} else if (_mouseOverScroll == 2 && this->_viewport.bottom ==  kPhotoHeight - 1) {
					_mouseOverScroll = 4;
				} else if (_mouseOverScroll == 3 && this->_viewport.left == 0) {
					_mouseOverScroll = 4;
				}
				if (_mouseOverScroll != 4) {
					cursor = _mouseOverScroll + 2;
				}
			}
		}
	}

	if (_mouseOverScroll == 4) {
		cursor = _buttons->hasHoveredImage() ? 1 : 0;
	}
	if (cursor != -1) {
		_vm->_mouse->setCursor(cursor);
		_vm->_mouse->draw(surface, p.x, p.y);
	}
}

void ESPER::flashViewport() {
	for (int y = 0; y < _surfaceViewport.h; ++y) {
		for (int x = 0; x < _surfaceViewport.w; ++x) {
			uint8 r, g, b;
			void *ptr = _surfaceViewport.getBasePtr(x, y);
			_surfaceViewport.format.colorToRGB(READ_UINT32(ptr), r, g, b);
			b *= 2;
			drawPixel(_surfaceViewport, ptr, _surfaceViewport.format.RGBToColor(r, g, b));
		}
	}
}

void ESPER::copyImageScale(Graphics::Surface &src, Common::Rect srcRect, Graphics::Surface &dst, Common::Rect dstRect) {
	if (_flash) {
		playSound(kSfxBR031_1P, 25);
	}

	int srcDstWidthRatio  = srcRect.width()  / dstRect.width();
	int srcDstWidthRest   = srcRect.width()  % dstRect.width();
	int srcDstHeightRatio = srcRect.height() / dstRect.height();
	int srcDstHeightRest  = srcRect.height() % dstRect.height();

	if (srcRect.width() > dstRect.width() && srcRect.height() > dstRect.height()) {
		// reduce
		int srcY = srcRect.top;
		int srcYCounter = 0;
		for (int dstY = dstRect.top; dstY < dstRect.bottom; ++dstY) {
			int srcX = srcRect.left;
			int srcXCounter = 0;
			for (int dstX = dstRect.left; dstX < dstRect.right; ++dstX) {
				srcX = CLIP(srcX, 0, src.w - 1);
				srcY = CLIP(srcY, 0, src.h - 1);

				dstX = CLIP(dstX, 0, dst.w - 1);
				dstY = CLIP(dstY, 0, dst.h - 1);

				uint8 r, g, b;
				src.format.colorToRGB(READ_UINT32(src.getBasePtr(srcX, srcY)), r, g, b);
				if (_flash) {
					// add blue-ish tint
					b *= 2;
				}
				drawPixel(dst, dst.getBasePtr(dstX, dstY), dst.format.RGBToColor(r, g, b));

				srcX += srcDstWidthRatio;
				srcXCounter += srcDstWidthRest;
				if (srcXCounter >= dstRect.width()) {
					srcXCounter -= dstRect.width();
					++srcX;
				}
			}

			srcY += srcDstHeightRatio;
			srcYCounter += srcDstHeightRest;
			if (srcYCounter >= dstRect.height()) {
				srcYCounter -= dstRect.height();
				++srcY;
			}
		}
	} else {
		// enlarge
		int srcY = srcRect.top;
		int srcYCounter = 0;
		for (int dstY = dstRect.top; dstY < dstRect.bottom; ++dstY) {
			int srcX = srcRect.left;
			int srcXCounter = 0;
			for (int dstX = dstRect.left; dstX < dstRect.right; ++dstX) {
				srcXCounter += srcRect.width();
				if (srcXCounter >= dstRect.width()) {
					srcXCounter -= dstRect.width();
					++srcX;
				}

				srcX = CLIP(srcX, 0, src.w - 1);
				srcY = CLIP(srcY, 0, src.h - 1);

				dstX = CLIP(dstX, 0, dst.w - 1);
				dstY = CLIP(dstY, 0, dst.h - 1);

				uint8 r, g, b;
				src.format.colorToRGB(READ_UINT32(src.getBasePtr(srcX, srcY)), r, g, b);
				if (_flash) {
					// add blue-ish tint
					b *= 2;
				}
				drawPixel(dst, dst.getBasePtr(dstX, dstY), dst.format.RGBToColor(r, g, b));
			}

			srcYCounter += srcRect.height();
			if (srcYCounter >= dstRect.height()) {
				srcYCounter -= dstRect.height();
				++srcY;
			}
		}
	}
	_flash = false;
}

void ESPER::copyImageBlur(Graphics::Surface &src, Common::Rect srcRect, Graphics::Surface &dst, Common::Rect dstRect, float blur) {
	if (_flash) {
		playSound(kSfxBR031_1P, 25);
	}

	int srcDstWidthRatio  = srcRect.width()  / dstRect.width();
	int srcDstWidthRest   = srcRect.width()  % dstRect.width();
	int srcDstHeightRatio = srcRect.height() / dstRect.height();
	int srcDstHeightRest  = srcRect.height() % dstRect.height();

	int skipStep = (blur - (int)blur) * 1000.0f;
	if (srcRect.width() > dstRect.width() && srcRect.height() > dstRect.height()) {
		// reduce
		int srcY = srcRect.top;
		int dstY = dstRect.top;
		int srcYCounter = 0;
		int skipYMaxCounter = 0;
		while (dstY < dstRect.bottom) {
			skipYMaxCounter += skipStep;
			int skipYMax = blur;
			if (skipYMaxCounter >= 1000) {
				skipYMaxCounter -= 1000;
				++skipYMax;
			}
			int skipY = 0;
			while (dstY < dstRect.bottom && skipY < skipYMax) {
				int srcX = srcRect.left;
				int dstX = dstRect.left;
				int srcXCounter = 0;
				int skipXMaxCounter = 0;
				while (dstX < dstRect.right) {
					skipXMaxCounter += skipStep;
					int skipXMax = blur;
					if (skipXMaxCounter >= 1000) {
						skipXMaxCounter -= 1000;
						++skipXMax;
					}
					int skipX = 0;
					while (dstX < dstRect.right && skipX < skipXMax) {

						srcX = CLIP(srcX, 0, src.w - 1);
						srcY = CLIP(srcY, 0, src.h - 1);

						dstX = CLIP(dstX, 0, dst.w - 1);
						dstY = CLIP(dstY, 0, dst.h - 1);

						uint8 r, g, b;
						src.format.colorToRGB(READ_UINT32(src.getBasePtr(srcX, srcY)), r, g, b);
						if (_flash) {
							// add blue-ish tint
							b *= 2;
						}
						drawPixel(dst, dst.getBasePtr(dstX, dstY), dst.format.RGBToColor(r, g, b));

						++dstX;
						++skipX;
					}
					srcXCounter += srcDstWidthRest;
					srcX += srcDstWidthRatio * skipX;
					if (srcXCounter >= dstRect.width()) {
						srcXCounter -= dstRect.width();
						srcX += skipX;
					}
				}

				++dstY;
				++skipY;
			}

			srcYCounter += srcDstHeightRest;
			srcY += srcDstHeightRatio * skipY;
			if (srcYCounter >= dstRect.height()) {
				srcYCounter -= dstRect.height();
				srcY += skipY;
			}
		}
	} else {
		// enlarge
		int srcY = srcRect.top;
		int dstY = dstRect.top;
		int srcYCounter = srcRect.height(); // TODO: look at this again because in original source this is 0, but then first line is doubled
		int skipYMaxCounter = 0;
		while (dstY < dstRect.bottom) {
			skipYMaxCounter += skipStep;
			int skipYMax = blur;
			if (skipYMaxCounter >= 1000) {
				skipYMaxCounter -= 1000;
				++skipYMax;
			}
			int skipY = 0;
			while (dstY < dstRect.bottom && skipY < skipYMax) {
				int srcX = srcRect.left;
				int dstX = dstRect.left;
				int srcXCounter = 0;
				int skipXMaxCounter = 0;
				while (dstX < dstRect.right) {
					skipXMaxCounter += skipStep;
					int skipXMax = blur;
					if (skipXMaxCounter >= 1000) {
						skipXMaxCounter -= 1000;
						++skipXMax;
					}
					int skipX = 0;
					while (dstX < dstRect.right && skipX < skipXMax) {
						srcXCounter += srcRect.width();
						if (srcXCounter >= dstRect.width()) {
							srcXCounter -= dstRect.width();
							++srcX; // bug in original game? Advancing by 1 instead of skipX as for Y
						}

						srcX = CLIP(srcX, 0, src.w - 1);
						srcY = CLIP(srcY, 0, src.h - 1);

						dstX = CLIP(dstX, 0, dst.w - 1);
						dstY = CLIP(dstY, 0, dst.h - 1);

						uint8 r, g, b;
						src.format.colorToRGB(READ_UINT32(src.getBasePtr(srcX, srcY)), r, g, b);
						if (_flash) {
							// add blue-ish tint
							b *= 2;
						}
						drawPixel(dst, dst.getBasePtr(dstX, dstY), dst.format.RGBToColor(r, g, b));

						++dstX;
						++skipX;
					}
				}

				++dstY;
				++skipY;
			}

			srcYCounter += srcRect.height();
			if (srcYCounter >= dstRect.height()) {
				srcYCounter -= dstRect.height();
				srcY += skipY;
			}
		}
	}
	_flash = false;
}

void ESPER::copyImageBlit(Graphics::Surface &src, Common::Rect srcRect, Graphics::Surface &dst, Common::Rect dstRect) {
	for (int y = 0; y < dstRect.height(); ++y) {
		for (int x = 0; x < dstRect.width(); ++x) {
			uint8 r, g, b;
			src.format.colorToRGB(READ_UINT32(src.getBasePtr(CLIP(srcRect.left + x, 0, src.w - 1), CLIP(srcRect.top + y, 0, src.h - 1))), r, g, b);
			drawPixel(dst, dst.getBasePtr(CLIP(dstRect.left + x, 0, dst.w - 1), CLIP(dstRect.top + y, 0, dst.h - 1)), dst.format.RGBToColor(r, g, b));
		}
	}
}

void ESPER::tickSound() {
	if (_soundId1 != -1) {
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(_soundId1), _volume1, 0, 0, 50, 0);
		_soundId1 = -1;
	}
	if (_soundId2 != -1) {
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(_soundId2), _volume2, 0, 0, 50, 0);
		_soundId2 = -1;
	}
	if (_soundId3 != -1) {
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(_soundId3), _volume3, 0, 0, 50, 0);
		_soundId3 = -1;
	}
}

void ESPER::tickScroll() {
	uint32 timeNow = _vm->_time->current();
	// unsigned difference is intentional
	if (timeNow - _timeScrollNextStart <= 300u) {
		return;
	}
	_timeScrollNextStart = timeNow;

	if (_scrollingDirection == 0) {
		scrollUp();
	} else if (_scrollingDirection == 1) {
		scrollRight();
	} else if (_scrollingDirection == 2) {
		scrollDown();
	} else if (_scrollingDirection == 3) {
		scrollLeft();
	}
}

int ESPER::findEmptyPhoto() {
	for (int i = 0; i < kPhotoCount; ++i) {
		if (!_photos[i].isPresent) {
			return i;
		}
	}
	return -1;
}

void ESPER::selectPhoto(int photoId) {
	_vm->_mouse->disable();
	_photoIdSelected = _photos[photoId].photoId;
	unloadPhotos();
	_script->photoSelected(_photoIdSelected);

	Common::ScopedPtr<Common::SeekableReadStream> s(_vm->getResourceStream(_photos[photoId].name));

	uint photoSize = _surfacePhoto.w * _surfacePhoto.h * _surfacePhoto.format.bytesPerPixel;

	s->skip(3); // not used, but there is compression type
	uint width  = s->readUint32LE();
	uint height = s->readUint32LE();
	uint photoCompressedSize = s->size() - s->pos();
	uint8 *photoCompressed = (uint8 *)_surfacePhoto.getPixels() + photoSize - photoCompressedSize;
	s->read(photoCompressed, photoCompressedSize);

	decompress_lcw(photoCompressed, photoCompressedSize, (uint8 *)_surfacePhoto.getPixels(), photoSize);
#ifdef SCUMM_BIG_ENDIAN
	// As the compression is working with 8-bit data, on big-endian architectures we have to switch order of bytes in uncompressed data
	uint8 *rawData = (uint8 *)_surfacePhoto.getPixels();
	for (size_t i = 0; i < photoSize - 1; i += 2) {
		SWAP(rawData[i], rawData[i + 1]);
	}
#endif

	// apply palette
	for (uint j = 0; j < width * height; ++j) {
		// _surfacePhoto[j] = Palette[_surfacePhoto[j]];
	}

	_shapeThumbnail = _shapesPhotos->get(_photos[photoId].shapeId);
	_buttons->resetImages();
	_buttons->defineImage(kPhotoCount + 2, Common::Rect(480, 350, 578, 413), _shapeThumbnail, _shapeThumbnail, _shapeThumbnail, nullptr);
	_buttons->defineImage(kPhotoCount + 3, Common::Rect(42, 403, 76, 437), nullptr, nullptr, _shapesButtons->get(0), nullptr);

	resetPhotoOpening();
	resetViewport();
	setStateMain(kEsperMainStatePhotoOpening);
	setStatePhoto(kEsperPhotoStateOpening);
	playSound(kSfxBR032_7B, 25);
	playSound(kSfxBR033_4B, 25);
}

void ESPER::unloadPhotos() {
	for (int i = 0; i < kPhotoCount; ++i) {
		if (_photos[i].isPresent) {
			_photos[i].isPresent = false;
			_buttons->resetImage(i);
		}
	}
}

int ESPER::findEmptyRegion() {
	for (int i = 0; i < kRegionCount; ++i) {
		if (!_regions[i].isPresent) {
			return i;
		}
	}
	return -1;
}

int ESPER::findRegion(Common::Rect where) {
	for (int i = 0; i < kRegionCount; ++i) {
		if (_regions[i].isPresent && _regions[i].rectOuter.contains(where) && where.contains(_regions[i].rectInner)) {
			return i;
		}
	}
	return -1;
}

void ESPER::zoomingStart() {
	prepareZoom();
	setStatePhoto(kEsperPhotoStateSelectionZooming);
}

void ESPER::zoomOutStart() {
	if (_statePhoto == kEsperPhotoStateVideoShow) {
		resetPhotoZooming();
		setStatePhoto(kEsperPhotoStateVideoZoomOut);
	} else {
		zoomOutStop();
		if (_zoomMin < _zoom) {
			_isZoomingOut = true;
			setStatePhoto(kEsperPhotoStatePhotoZoomOut);
		}
	}
}

void ESPER::zoomOutStop() {
	_isZoomingOut = false;
	_statePhoto = kEsperPhotoStateShow;
}

void ESPER::scrollingStart(int direction) {
	scrollingStop();
	if ((direction != 3 || _viewport.left > 0)
	 && (direction != 0 || _viewport.top  > 0)
	 && (direction != 1 || _viewport.right  != kPhotoWidth  - 1)
	 && (direction != 2 || _viewport.bottom != kPhotoHeight - 1)) {
		_isScrolling = true;
		_scrollingDirection = direction;
	}
}

void ESPER::scrollingStop() {
	_isScrolling = false;
	_scrollingDirection = -1;
}

void ESPER::scrollUpdate() {
	if ((_viewport.left == _viewportNext.left) && (_viewportNext.top == _viewport.top)) {
		setStatePhoto(kEsperPhotoStateShow);
		return;
	}

	if (_viewport.left != _viewportNext.left) {
		_viewport.left  = _viewportNext.left;
		_viewport.right = _viewportNext.right;

		_viewportPositionX = (_viewportNext.left + _viewportNext.right) / 2;
	}

	if (_viewport.top != _viewportNext.top) {
		_viewport.top    = _viewportNext.top;
		_viewport.bottom = _viewportNext.bottom;

		_viewportPositionY = (_viewportNext.top + _viewportNext.bottom) / 2;
	}
}

void ESPER::scrollLeft() {this->_flash = 1;
	_flash = true;
	setStatePhoto(kEsperPhotoStateScrolling);

	_viewportNext.left  = _viewport.left  - 40;
	_viewportNext.right = _viewport.right - 40;
	if (_viewportNext.left < 0) {
		_viewportNext.right -= _viewportNext.left;
		_viewportNext.left   = 0;
		scrollingStop();
	}
	_viewportNext.top    = _viewport.top;
	_viewportNext.bottom = _viewport.bottom;
}

void ESPER::scrollUp() {
	_flash = true;
	setStatePhoto(kEsperPhotoStateScrolling);

	_viewportNext.top    = _viewport.top    - 40;
	_viewportNext.bottom = _viewport.bottom - 40;
	if (_viewportNext.top < 0) {
		_viewportNext.bottom -= _viewportNext.top;
		_viewportNext.top     = 0;
		scrollingStop();
	}
	_viewportNext.left  = _viewport.left;
	_viewportNext.right = _viewport.right;
}

void ESPER::scrollRight() {
	if (_viewport.right < kPhotoWidth - 1) {
		_flash = true;
		setStatePhoto(kEsperPhotoStateScrolling);

		_viewportNext.left   = _viewport.left  + 40;
		_viewportNext.right  = _viewport.right + 40;
		_viewportNext.top    = _viewport.top;
		_viewportNext.bottom = _viewport.bottom;

		if (_viewportNext.right > kPhotoWidth - 1) {
			_viewportNext.left -= _viewportNext.right - (kPhotoWidth - 1);
			_viewportNext.right = kPhotoWidth - 1;
			scrollingStop();
		}
	}
}

void ESPER::scrollDown() {
	if (_viewport.bottom < kPhotoHeight - 1) {
		_flash = true;
		setStatePhoto(kEsperPhotoStateScrolling);

		_viewportNext.top    = _viewport.top    + 40;
		_viewportNext.bottom = _viewport.bottom + 40;
		_viewportNext.left   = _viewport.left;
		_viewportNext.right  = _viewport.right;

		if (_viewportNext.bottom > kPhotoHeight - 1) {
			_viewportNext.top   -= _viewportNext.bottom - (kPhotoHeight - 1);
			_viewportNext.bottom = kPhotoHeight - 1;
			scrollingStop();
		}
	}
}

void ESPER::goBack() {
	// CD-changing logic has been removed

	if (_stateMain == kEsperMainStateList) {
		close();
	} else {
		resetData();
		activate(false);
	}
}

void ESPER::prepareZoom() {
	_selectionZoomStep          = 0;
	_timeSelectionZoomNextStart = 0u;

	_selectionTarget = _selection;
	resetSelectionRect();
	_selectionDelta.left   = (_selectionTarget.left   - _selection.left)   / kSelectionZoomSteps;
	_selectionDelta.top    = (_selectionTarget.top    - _selection.top)    / kSelectionZoomSteps;
	_selectionDelta.right  = (_selectionTarget.right  - _selection.right)  / kSelectionZoomSteps;
	_selectionDelta.bottom = (_selectionTarget.bottom - _selection.bottom) / kSelectionZoomSteps;

	Common::Rect rect = _selectionTarget;
	if (_regionSelectedAck) {
		rect.left   = viewportXToScreenX(_regions[_regionSelected].rectSelection.left);
		rect.top    = viewportYToScreenY(_regions[_regionSelected].rectSelection.top);
		rect.right  = viewportXToScreenX(_regions[_regionSelected].rectSelection.right);
		rect.bottom = viewportYToScreenY(_regions[_regionSelected].rectSelection.bottom);
	}

	_zoomSteps = 10;
	float ratio = (rect.width() + 1.0f) / (float)_screen.width();
	if (ratio == 0.0f) {
		_zoomTarget = ratio;
		_zoomDelta  = 0.0f;
	} else {
		_zoomTarget = CLIP(_zoom / ratio, _zoomMin, 2.0f);
		_zoomSteps  = CLIP((int)(_zoomTarget / _zoom) - 1, 0, 5) + 5;
		_zoomDelta  = (_zoomTarget - _zoom) / (float)_zoomSteps;
	}

	_blur = 1.0f;

	_viewportPositionXTarget  = _viewport.left + ((rect.left + rect.right) / 2 - _screen.left) * _viewport.width()  / _screen.width();
	_viewportPositionYTarget  = _viewport.top +  ((rect.top + rect.bottom) / 2 - _screen.top ) * _viewport.height() / _screen.height();
	_viewportPositionXDelta   = (_viewportPositionXTarget - _viewportPositionX) / (float)_zoomSteps;
	_viewportPositionYDelta   = (_viewportPositionYTarget - _viewportPositionY) / (float)_zoomSteps;
	_viewportPositionXCurrent = _viewportPositionX;
	_viewportPositionYCurrent = _viewportPositionY;
}

void ESPER::updateSelection() {
	int selectionWidth  = abs(_selection.right + 1 - _selection.left);
	int selectionHeight = abs(_selection.bottom + 1 - _selection.top);

	int photoSelectedWidth = _viewport.width() * selectionWidth / _screen.width();
	if (photoSelectedWidth < _screenHalfWidth) {
		// minimal width of selection
		selectionWidth = _screen.width() * _screenHalfWidth / _viewport.width();
	}

	photoSelectedWidth = _viewport.height() * selectionHeight / _screen.height();
	if (photoSelectedWidth < _screenHalfHeight) {
		// minimal height of selection
		selectionHeight = _screen.height() * _screenHalfHeight / _viewport.height();
	}

	// correct aspect ratio
	if (selectionWidth / (float)_screen.width() <= selectionHeight / (float)_screen.height()) {
		while (selectionWidth / (float)_screen.width() <= selectionHeight / (float)_screen.height()) {
			++selectionWidth;
		}
	} else {
		while (selectionHeight / (float)_screen.height() <= selectionWidth / (float)_screen.width()) {
			++selectionHeight;
		}
	}

	if (selectionWidth > _screen.width()) {
		selectionWidth = _screen.width();
	}
	if (selectionHeight > _screen.height()) {
		selectionHeight = _screen.height();
	}

	int left   = _viewport.right  - (_screen.right - 1  - _selection.left) * _viewport.width()  / _screen.width();
	int right  = _viewport.left   + (_selection.right   - _screen.left   ) * _viewport.width()  / _screen.width();
	int top    = _viewport.bottom - (_screen.bottom - 1 - _selection.top ) * _viewport.height() / _screen.height();
	int bottom = _viewport.top    + (_selection.bottom  - _screen.top    ) * _viewport.height() / _screen.height();

	bool stop = false;
	bool alternate = false;

	while (selectionWidth > abs(_selection.right + 1 - _selection.left)) {
		if (alternate) {
			--_selection.left;
			if (_selection.left < 0) {
				left = _viewport.right - (_screen.right - 1 + 100 - _selection.left) * _viewport.width() / _screen.width();
				if (left < 0) {
					left = 0;
					++_selection.left;
					if (stop) {
						break;
					}
					stop = true;
					alternate = false;
				}
			}
		} else {
			++_selection.right;
			if (_selection.right > _screen.right - 1) {
				right = _viewport.left + (_selection.right - _screen.left) * _viewport.width() / _screen.width();
				if (right > kPhotoWidth - 1) {
					right = kPhotoWidth - 1;
					--_selection.right;
					if (stop) {
						break;
					}
					stop = true;
					alternate = true;
				}
			}
		}
		if (!stop) {
			alternate = !alternate;
		}
	}

	alternate = false;
	stop = false;
	while (selectionHeight > abs(_selection.bottom + 1 - _selection.top)) {
		if (alternate) {
			--_selection.top;
			if (_selection.top < 0) {
				top = _viewport.bottom - (_screen.bottom - 1 - _selection.top) * _viewport.height() / _screen.height();
				if (top < 0) {
					top = 0;
					++_selection.top;
					if (stop) {
						break;
					}
					stop = true;
					alternate = false;
				}
			}
		} else {
			++_selection.bottom;
			if (_selection.bottom > _screen.bottom - 1)	{
				bottom = _viewport.top + (_selection.bottom - _screen.top) * _viewport.height() / _screen.height();
				if (bottom > kPhotoHeight - 1) {
					bottom = kPhotoHeight - 1;
					--_selection.bottom;
					if (stop) {
						break;
					}
					alternate = true;
					stop = true;
				}
			}
		}
		if (!stop) {
			alternate = !alternate;
		}
	}

	if (left > right) {
		SWAP(left, right);
	}

	if (top > bottom) {
		SWAP(top, bottom);
	}

	_regionSelected = findRegion(Common::Rect(left, top, right, bottom));
	if (_regionSelected >= 0) {
		_regionSelectedAck = true;
		setStatePhoto(kEsperPhotoStatePhotoZooming);
	}
}

int ESPER::viewportXToScreenX(int x) {
	return _screen.width() * (x - _viewport.left) / _viewport.width() + _screen.left;
}

int ESPER::viewportYToScreenY(int y) {
	return _screen.height() * (y - _viewport.top) / _viewport.height() + _screen.top;
}

} // End of namespace BladeRunner
