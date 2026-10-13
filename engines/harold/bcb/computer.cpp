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
 * along with this source distribution. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/algorithm.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/system.h"
#include "graphics/screen.h"
#include "graphics/surface.h"

#include "harold/bcb/bcb.h"
#include "harold/bcb/script.h"
#include "harold/bcb/static_data.h"

namespace BCB {

static const int kComputerCompositeTransitionLeft = 36;
static const int kComputerCompositeTransitionTop = 36;
static const int kComputerCompositeTransitionRight = 606;
static const int kComputerCompositeTransitionBottom = 444;

const Graphics::Surface *BCBEngine::getComputerBitmap() {
	if (!_computerBitmap) {
		_computerBitmap = loadBitmap("256com.bmp");
	}
	return _computerBitmap;
}

bool BCBEngine::dispatchComputerMainControls() {
	if (!showComputerScreen())
		return false;

	_computerControlsActive = true;
	_computerDetailControlsActive = false;
	_computerActiveChoiceIds.clear();
	_computerActiveChoiceRects.clear();
	drawComputerChoiceGroupControls();
	drawComputerButton(kComputerPrimaryButtonTable[1], kComputerPrimaryButtonTable[1]);

	_screen->update();
	return true;
}

bool BCBEngine::dispatchComputerDetailControls() {
	_computerDetailVariantIndex = 0;
	_computerControlsActive = false;
	_computerDetailControlsActive = true;
	_computerActiveChoiceIds.clear();
	_computerActiveChoiceRects.clear();
	return showComputerDetailScreen(stateDword(kStateSelectedSceneChoiceValue));
}

bool BCBEngine::showComputerScreen() {
	const Graphics::Surface *computer = getComputerBitmap();
	if (!computer)
		return false;

	_screen->copyRectToSurface(computer->getPixels(), computer->pitch, 0, 0, MIN<int>(computer->w, 640), MIN<int>(computer->h, 480));
	_currentDisplayState = 4;
	return true;
}

void BCBEngine::drawComputerChoiceGroupControls() {
	_computerActiveChoiceIds.clear();
	_computerActiveChoiceRects.clear();

	const Graphics::Surface *computer = getComputerBitmap();
	if (!computer)
		return;

	Graphics::Surface *screen = _screen->surfacePtr();
	if (screen) {
		if (_computerChoiceGroupIndex >= 0 && _computerChoiceGroupIndex <= 2) {
			const int sourceY = _computerChoiceGroupIndex * 0x16 + 0x2a0;
			if (sourceY + 0x16 <= computer->h)
				_screen->copyRectToSurface(*computer, 0x3c, 0x43, Common::Rect(0, sourceY, 0xe8, sourceY + 0x16));
		}

		for (uint i = 1; i < ARRAYSIZE(kComputerChoiceLayoutSlots); ++i) {
			const ButtonControlRecord &slot = kComputerChoiceLayoutSlots[i];
			if (slot.x + slot.width <= computer->w && slot.y + slot.height <= computer->h)
				_screen->copyRectToSurface(*computer, slot.x, slot.y, Common::Rect(slot.x, slot.y, slot.x + slot.width, slot.y + slot.height));
		}
	}

	if (_computerChoiceGroupIndex == 0)
		drawComputerConditionalChoiceControls(kComputerChoiceButtonTable + 21, 21, 6);
	else if (_computerChoiceGroupIndex == 1)
		drawComputerConditionalChoiceControls(kComputerChoiceButtonTable, 0, 21);
	else if (_computerChoiceGroupIndex == 2)
		drawComputerConditionalChoiceControls(kComputerChoiceButtonTable + 27, 27, 3);
}

void BCBEngine::drawComputerButton(const ButtonControlRecord &sourceButton, const ButtonControlRecord &destButton) {
	const Graphics::Surface *computer = getComputerBitmap();
	if (!computer)
		return;

	Graphics::Surface *screen = _screen->surfacePtr();
	if (screen && sourceButton.x + sourceButton.width <= computer->w && sourceButton.y + sourceButton.height <= computer->h)
		_screen->copyRectToSurface(*computer, destButton.x, destButton.y, Common::Rect(sourceButton.x, sourceButton.y, sourceButton.x + sourceButton.width, sourceButton.y + sourceButton.height));
}

void BCBEngine::drawComputerButtonPressed(const ButtonControlRecord &button, int sourceX, int sourceY) {
	const Graphics::Surface *computer = getComputerBitmap();
	if (!computer)
		return;

	if (sourceX + button.width <= computer->w && sourceY + button.height <= computer->h) {
		_screen->copyRectToSurface(computer->getBasePtr(sourceX, sourceY), computer->pitch, button.x, button.y, button.width, button.height);
		_screen->update();
	}
}

void BCBEngine::drawComputerDetailButtonNormal(const ButtonControlRecord &button) {
	if (!_computerDetailBacking)
		return;

	if (button.x + button.width <= _computerDetailBacking->w && button.y + button.height <= _computerDetailBacking->h) {
		_screen->copyRectToSurface(_computerDetailBacking->getBasePtr(button.x, button.y), _computerDetailBacking->pitch, button.x, button.y, button.width, button.height);
		_screen->update();
	}
}

void BCBEngine::drawComputerUnavailableMarker(const ButtonControlRecord &destButton) {
	const Graphics::Surface *computer = getComputerBitmap();
	if (!computer)
		return;

	const int markerSourceX = 0x240;
	const int markerSourceY = 0x260;
	const int markerWidth = 0x36;
	const int markerHeight = 0x18;
	if (markerSourceX + markerWidth <= computer->w && markerSourceY + markerHeight <= computer->h)
		_screen->copyRectToSurface(computer->getBasePtr(markerSourceX, markerSourceY), computer->pitch, destButton.x + 5, destButton.y + 0x0c, markerWidth, markerHeight);
}

void BCBEngine::drawXorRectangleBorder(const Common::Rect &rect) {
	Graphics::Surface *screen = _screen->surfacePtr();
	if (!screen)
		return;

	const Common::Rect clipped = rect.findIntersectingRect(Common::Rect(0, 0, screen->w, screen->h));
	if (clipped.width() > 0 && clipped.height() > 0) {
		const auto invertPixel = [&](int x, int y) {
			byte *pixel = static_cast<byte *>(screen->getBasePtr(x, y));
			switch (screen->format.bytesPerPixel) {
			case 1:
				pixel[0] ^= 0xff;
				break;
			case 2:
				WRITE_UINT16(pixel, READ_UINT16(pixel) ^ 0xffff);
				break;
			case 3:
			case 4:
				pixel[0] ^= 0xff;
				pixel[1] ^= 0xff;
				pixel[2] ^= 0xff;
				break;
			default:
				break;
			}
		};

		for (int x = clipped.left; x < clipped.right; ++x) {
			invertPixel(x, clipped.top);
			if (rect.bottom != rect.top && rect.bottom >= 0 && rect.bottom < screen->h)
				invertPixel(x, rect.bottom);
		}
		for (int y = clipped.top; y < clipped.bottom; ++y) {
			invertPixel(clipped.left, y);
			if (rect.right != rect.left && rect.right >= 0 && rect.right < screen->w)
				invertPixel(rect.right, y);
		}
	}

	if (!clipped.isEmpty()) {
		_screen->addDirtyRect(Common::Rect(clipped.left, clipped.top, clipped.right, clipped.top + 1));
		_screen->addDirtyRect(Common::Rect(clipped.left, clipped.top + 1, clipped.left + 1, clipped.bottom));
		if (rect.bottom != rect.top && rect.bottom >= 0 && rect.bottom < screen->h)
			_screen->addDirtyRect(Common::Rect(clipped.left, rect.bottom, clipped.right, rect.bottom + 1));
		if (rect.right != rect.left && rect.right >= 0 && rect.right < screen->w)
			_screen->addDirtyRect(Common::Rect(rect.right, clipped.top, rect.right + 1, clipped.bottom));
	}
	_screen->update();
}

void BCBEngine::processXorRectangleBorderAnimation(bool expand, const Common::Rect &sourceRect, const Common::Rect &targetRect) {
	const double base = 0.7;
	double drawScale = pow(base, 15.0);
	double eraseScale = drawScale;
	const double divisor = expand ? base : (1.0 / base);
	if (!expand) {
		drawScale = 1.0 - drawScale;
		eraseScale = drawScale;
	}

	const auto transitionRect = [&](double scale) {
		return Common::Rect(
			sourceRect.left + (int16)(int)((targetRect.left - sourceRect.left) * scale),
			sourceRect.top + (int16)(int)((targetRect.top - sourceRect.top) * scale),
			sourceRect.right + (int16)(int)((targetRect.right - sourceRect.right) * scale),
			sourceRect.bottom + (int16)(int)((targetRect.bottom - sourceRect.bottom) * scale));
	};

	for (int step = 0; step < 22; ++step) {
		if (step < 14) {
			drawScale /= divisor;
			drawXorRectangleBorder(transitionRect(drawScale));
		}

		if (step > 7) {
			eraseScale /= divisor;
			drawXorRectangleBorder(transitionRect(eraseScale));
		}

		g_system->delayMillis(20);
		pollEvents(false);
	}
}

void BCBEngine::drawComputerConditionalChoiceControls(const ButtonControlRecord *choiceButtonRecords, int firstStateIndex, int stateCount) {
	const Graphics::Surface *computer = getComputerBitmap();
	if (!computer)
		return;

	Graphics::Surface *screen = _screen->surfacePtr();
	int visibleButtonSlot = 1;
	for (int i = 0; screen && i < stateCount && visibleButtonSlot < (int)ARRAYSIZE(kComputerChoiceLayoutSlots); ++i) {
		const int stateIndex = firstStateIndex + i;
		const byte dependencyStateIndex = stateIndex >= 0 && stateIndex < (int)ARRAYSIZE(kComputerChoiceDependencyStateIndex) ? kComputerChoiceDependencyStateIndex[stateIndex] : 0;
		const bool directlyAvailable = stateDword(kStateComputerChoiceStateByIndex + stateIndex * 4) != 0;
		const bool dependencyOnlyAvailable =
			dependencyStateIndex != 0 && stateDword(kStateComputerChoiceStateByIndex + dependencyStateIndex * 4) == 1;
		const bool visible = directlyAvailable || dependencyOnlyAvailable;
		if (!visible)
			continue;

		const ButtonControlRecord &source = choiceButtonRecords[i];
		const ButtonControlRecord &dest = kComputerChoiceLayoutSlots[visibleButtonSlot++];
		if (source.x + source.width <= computer->w && source.y + source.height <= computer->h)
			_screen->copyRectToSurface(*computer, dest.x, dest.y, Common::Rect(source.x, source.y, source.x + source.width, source.y + source.height));
		if (directlyAvailable) {
			_computerActiveChoiceIds.push_back(source.controlId);
			_computerActiveChoiceRects.push_back(Common::Rect(dest.x, dest.y, dest.x + dest.width, dest.y + dest.height));
		}
	}
	visibleButtonSlot = 1;
	for (int i = 0; i < stateCount && visibleButtonSlot < (int)ARRAYSIZE(kComputerChoiceLayoutSlots); ++i) {
		const int stateIndex = firstStateIndex + i;
		const byte dependencyStateIndex = stateIndex >= 0 && stateIndex < (int)ARRAYSIZE(kComputerChoiceDependencyStateIndex) ? kComputerChoiceDependencyStateIndex[stateIndex] : 0;
		const bool directlyAvailable = stateDword(kStateComputerChoiceStateByIndex + stateIndex * 4) != 0;
		const bool dependencyOnlyAvailable =
			dependencyStateIndex != 0 && stateDword(kStateComputerChoiceStateByIndex + dependencyStateIndex * 4) == 1;
		if (!directlyAvailable && !dependencyOnlyAvailable)
			continue;
		const ButtonControlRecord &dest = kComputerChoiceLayoutSlots[visibleButtonSlot++];
		if (!directlyAvailable && dependencyOnlyAvailable)
			drawComputerUnavailableMarker(dest);
	}
}

bool BCBEngine::showComputerDetailScreen(int compositeIndex) {
	if (compositeIndex < 0 || compositeIndex >= (int)ARRAYSIZE(kComputerDetailBitmapVariantsBySelectedValue))
		return false;

	const CompositeBitmapVariantSet &variantSet = kComputerDetailBitmapVariantsBySelectedValue[compositeIndex];
	if (variantSet.pathCount == 0 || _computerDetailVariantIndex < 0 || _computerDetailVariantIndex >= (int)variantSet.pathCount)
		return false;
	const int displayedVariantIndex = _computerDetailVariantIndex;
	const char *const compositePath = variantSet.paths[_computerDetailVariantIndex];

	const Graphics::Surface *computer = getComputerBitmap();
	if (!computer)
		return false;

	Graphics::Surface *composite = loadDiscBitmap(compositePath);
	if (!composite)
		return false;

	Graphics::Surface *screen = _screen->surfacePtr();
	if (screen) {
		screen->copyRectToSurface(*computer, 0, 0, Common::Rect(0, 0, MIN<int>(computer->w, 640), MIN<int>(computer->h, 480)));
		screen->copyRectToSurface(*composite, 0x24, 0x23, Common::Rect(0, 0, MIN<int>(composite->w, 640 - 0x24), MIN<int>(composite->h, 480 - 0x23)));
		if (_computerDetailBacking) {
			_computerDetailBacking->free();
			delete _computerDetailBacking;
			_computerDetailBacking = nullptr;
		}
		_computerDetailBacking = new Graphics::Surface();
		_computerDetailBacking->copyFrom(*screen);
		_screen->makeAllDirty();
	}

	composite->free();
	delete composite;

	_computerDetailNextVariantEnabled = variantSet.pathCount > 1;

	if (_computerDetailNextVariantEnabled && displayedVariantIndex + 1 >= (int)variantSet.pathCount)
		_computerDetailVariantIndex = -1;

	_currentDisplayState = 5;
	_screen->update();
	return true;
}

bool BCBEngine::handleComputerClick(const Common::Point &mousePosition) {
	if (_currentDisplayState == 4 && _computerControlsActive) {
		const ButtonControlRecord &cancelButton = kComputerPrimaryButtonTable[1];
		if (Common::Rect(cancelButton.x, cancelButton.y, cancelButton.x + cancelButton.width, cancelButton.y + cancelButton.height).contains(mousePosition)) {
			drawComputerButtonPressed(cancelButton, 0xf8, 0x2d0);
			playUiSound();
			_computerControlsActive = false;
			_computerActiveChoiceIds.clear();
			_computerActiveChoiceRects.clear();
			stateDword(kStateSelectedSceneChoiceValue) = 0xff;
			processGameStateDispatcher(true);
			return true;
		}

		for (uint groupIndex = 0; groupIndex < ARRAYSIZE(kComputerChoiceGroupHitRects); ++groupIndex) {
			if (!kComputerChoiceGroupHitRects[groupIndex].contains(mousePosition))
				continue;

			if (_computerChoiceGroupIndex != (int)groupIndex) {
				playUiSound();
				_computerChoiceGroupIndex = groupIndex;
				drawComputerChoiceGroupControls();
				_screen->update();
			}
			return true;
		}

		for (uint i = 0; i < _computerActiveChoiceRects.size(); ++i) {
			if (!_computerActiveChoiceRects[i].contains(mousePosition))
				continue;

			playUiSound();
			_computerSelectionXorRect = _computerActiveChoiceRects[i];
			processXorRectangleBorderAnimation(true, _computerSelectionXorRect, Common::Rect(kComputerCompositeTransitionLeft, kComputerCompositeTransitionTop, kComputerCompositeTransitionRight, kComputerCompositeTransitionBottom));
			_computerControlsActive = false;
			stateDword(kStateSelectedSceneChoiceValue) = _computerActiveChoiceIds[i] - 700;
			_computerActiveChoiceIds.clear();
			_computerActiveChoiceRects.clear();
			processGameStateDispatcher(true);
			return true;
		}
	}

	if (_currentDisplayState == 5 && _computerDetailControlsActive) {
		const ButtonControlRecord &continueButton = kComputerDetailButtonTable[1];
		if (Common::Rect(continueButton.x, continueButton.y, continueButton.x + continueButton.width, continueButton.y + continueButton.height).contains(mousePosition)) {
			drawComputerButtonPressed(continueButton, 0x149, 0x2b8);
			playUiSound();
			if (!_computerSelectionXorRect.isEmpty())
				processXorRectangleBorderAnimation(false, _computerSelectionXorRect, Common::Rect(kComputerCompositeTransitionLeft, kComputerCompositeTransitionTop, kComputerCompositeTransitionRight, kComputerCompositeTransitionBottom));
			_computerDetailControlsActive = false;
			processGameStateDispatcher(true);
			return true;
		}

		const ButtonControlRecord &nextButton = kComputerDetailButtonTable[2];
		if (_computerDetailNextVariantEnabled &&
			Common::Rect(nextButton.x, nextButton.y, nextButton.x + nextButton.width, nextButton.y + nextButton.height).contains(mousePosition)) {
			drawComputerButtonPressed(nextButton, _computerDetailVariantIndex >= 0 ? 0x188 : 0x228, 0x2a0);
			playUiSound();
			if (_computerDetailVariantIndex < 0) {
				_computerDetailVariantIndex = 0;
			} else {
				++_computerDetailVariantIndex;
			}
			showComputerDetailScreen(stateDword(kStateSelectedSceneChoiceValue));
			return true;
		}
	}

	return false;
}

bool BCBEngine::handleComputerKey(Common::KeyCode keycode) {
	if (_currentDisplayState == 4 && _computerControlsActive && keycode == Common::KEYCODE_ESCAPE) {
		const ButtonControlRecord &cancelButton = kComputerPrimaryButtonTable[1];
		return handleComputerClick(Common::Point(cancelButton.x + 1, cancelButton.y + 1));
	}

	if (_currentDisplayState == 5 && _computerDetailControlsActive) {
		if (keycode == Common::KEYCODE_ESCAPE) {
			const ButtonControlRecord &continueButton = kComputerDetailButtonTable[1];
			return handleComputerClick(Common::Point(continueButton.x + 1, continueButton.y + 1));
		}

		if (keycode == Common::KEYCODE_SPACE && _computerDetailNextVariantEnabled) {
			const ButtonControlRecord &nextButton = kComputerDetailButtonTable[2];
			return handleComputerClick(Common::Point(nextButton.x + 1, nextButton.y + 1));
		}
	}

	return false;
}
} // End of namespace BCB
