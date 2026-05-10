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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/ui/notebookpopup.h"

namespace Nancy {
namespace UI {

NotebookPopup::NotebookPopup() :
		// z=12: above the viewport (6) and the taskbar (7). All Nancy
		// 10+ taskbar popups render on top of the entire scene UI.
		RenderObject(12),
		_uinbData(nullptr),
		_isOpen(false),
		_activeTab(0) {}

void NotebookPopup::init() {
	_uinbData = GetEngineData(UINB);
	assert(_uinbData);

	g_nancy->_resource->loadImage(_uinbData->header.imageName, _overlayImage);

	Common::Rect popupRect = _uinbData->header.normalDestRect;
	if (_uinbData->header.overlayInGameFrame) {
		const VIEW *view = GetEngineData(VIEW);
		if (view) {
			popupRect.translate(view->screenPosition.left, view->screenPosition.top);
		}
	}
	moveTo(popupRect);
	Common::Rect bounds = _screenPosition;
	bounds.moveTo(0, 0);
	_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getInputPixelFormat());

	// Pick the first enabled tab as the initially active one
	_activeTab = 0;
	for (uint i = 0; i < kNumTabs; ++i) {
		if (_uinbData->tabs[i].enabled) {
			_activeTab = (int)i;
			break;
		}
	}

	drawBackground();
	drawTabs();

	// TODO: Draw the actual notebook page contents

	setTransparent(false);
	setVisible(false);

	RenderObject::init();
}

void NotebookPopup::registerGraphics() {
	RenderObject::registerGraphics();
}

void NotebookPopup::open() {
	if (_isOpen) {
		return;
	}
	_isOpen = true;
	setVisible(true);

	if (!_uinbData->header.sounds[0].name.empty()) {
		g_nancy->_sound->loadSound(_uinbData->header.sounds[0]);
		g_nancy->_sound->playSound(_uinbData->header.sounds[0]);
	}
}

void NotebookPopup::close() {
	if (!_isOpen) {
		return;
	}
	_isOpen = false;
	setVisible(false);

	if (!_uinbData->header.sounds[1].name.empty()) {
		g_nancy->_sound->loadSound(_uinbData->header.sounds[1]);
		g_nancy->_sound->playSound(_uinbData->header.sounds[1]);
	}
}

void NotebookPopup::drawBackground() {
	_drawSurface.blitFrom(_overlayImage, _uinbData->header.normalSrcRect, Common::Point(0, 0));
}

void NotebookPopup::drawTabs() {
	// Sub-rects in the chunk are stored relative to header.normalDestRect.
	// After we translate the popup by the viewport offset, _screenPosition
	// no longer matches that origin, so popup-local conversions must
	// subtract chunk.normalDestRect.topLeft, not _screenPosition.
	const Common::Point chunkOrigin(_uinbData->header.normalDestRect.left,
									_uinbData->header.normalDestRect.top);

	for (uint i = 0; i < kNumTabs; ++i) {
		const UIButtonSlot &tab = _uinbData->tabs[i];
		if (!tab.enabled) {
			continue;
		}

		// Use the active state's sprite for the selected tab, idle for
		// the rest.
		const Common::Rect src = ((int)i == _activeTab && !tab.button.sourceRects[2].isEmpty())
									? tab.button.sourceRects[2]
									: tab.button.sourceRects[0];
		if (src.isEmpty()) {
			continue;
		}

		_drawSurface.blitFrom(_overlayImage, src,
								Common::Point(tab.button.destRect.left - chunkOrigin.x,
												tab.button.destRect.top - chunkOrigin.y));
	}

	_needsRedraw = true;
}

void NotebookPopup::handleInput(NancyInput &input) {
	if (!_isOpen) {
		return;
	}

	// Bring the mouse into the chunk's coordinate system so hit-tests
	// against tab destRects work after the popup was translated.
	const Common::Point chunkMouse(
		input.mousePos.x - _screenPosition.left + _uinbData->header.normalDestRect.left,
		input.mousePos.y - _screenPosition.top  + _uinbData->header.normalDestRect.top);

	// Tab clicks
	for (uint i = 0; i < kNumTabs; ++i) {
		const UIButtonSlot &tab = _uinbData->tabs[i];
		if (!tab.enabled) {
			continue;
		}
		if (!tab.button.destRect.contains(chunkMouse)) {
			continue;
		}

		g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			if (_activeTab != (int)i) {
				_activeTab = (int)i;

				// Play the page-flip sound (first slot of either
				// actionable or no-action set; both have 3 alternates).
				const Common::Path &soundName = _uinbData->noActionClickSounds[0];
				if (!soundName.empty()) {
					g_nancy->_sound->playSound(soundName.toString());
				}

				drawBackground();
				drawTabs();
			}
			input.eatMouseInput();
			return;
		}
		break;
	}

	// Swallow clicks on the popup itself so they don't fall through.
	if (_screenPosition.contains(input.mousePos)) {
		input.eatMouseInput();
	}
}

} // End of namespace UI
} // End of namespace Nancy
