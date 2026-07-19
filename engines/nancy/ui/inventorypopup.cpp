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

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/inventorypopup.h"
#include "engines/nancy/ui/taskbar.h"

namespace Nancy {
namespace UI {

InventoryPopup::InventoryPopup() :
		// z=12: above the viewport (6) and the taskbar (7). All Nancy
		// 10+ taskbar popups render on top of the entire scene UI.
		RenderObject(12) {
	for (uint i = 0; i < kSlotsPerPage; ++i)
		_slotItemIDs[i] = -1;
}

void InventoryPopup::init() {
	_uiivData = GetEngineData(UIIV);
	assert(_uiivData);

	_invData = GetEngineData(INV);
	assert(_invData);

	g_nancy->_resource->loadImage(_uiivData->header.imageName, _overlayImage);
	g_nancy->_resource->loadImage(_invData->inventoryBoxIconsImageName, _itemIcons);

	Common::Rect popupRect = _uiivData->header.normalDestRect;
	if (_uiivData->header.overlayInGameFrame) {
		const VIEW *view = GetEngineData(VIEW);
		if (view)
			popupRect.translate(view->screenPosition.left, view->screenPosition.top);
	}
	moveTo(popupRect);

	Common::Rect bounds = _screenPosition;
	bounds.moveTo(0, 0);
	_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getScreenPixelFormat());

	setActiveFilterIndex(0);

	drawBackground();
	drawFilterTabs();
	drawFilterCaption();

	setTransparent(false);
	setVisible(false);

	RenderObject::init();
}

void InventoryPopup::open() {
	if (_isVisible)
		return;

	_currentPage = 0;

	rebuildVisibleList();
	refreshGrid();

	setVisible(true);

	g_nancy->_cursor->warpCursor(Common::Point(_screenPosition.left + _screenPosition.width() / 2,
												_screenPosition.top + _screenPosition.height() / 2));

	NancySceneState.getTaskbar()->clearAllNotifications(kTaskButtonInventory);

	if (!_uiivData->header.sounds[0].name.empty()) {
		g_nancy->_sound->loadSound(_uiivData->header.sounds[0]);
		g_nancy->_sound->playSound(_uiivData->header.sounds[0]);
	}
}

void InventoryPopup::close() {
	if (!_isVisible)
		return;

	setVisible(false);

	if (!_uiivData->header.sounds[1].name.empty()) {
		g_nancy->_sound->loadSound(_uiivData->header.sounds[1]);
		g_nancy->_sound->playSound(_uiivData->header.sounds[1]);
	}
}

void InventoryPopup::refreshGrid() {
	rebuildVisibleList();

	// The whole grid is redrawn below with every slot in its plain state, so
	// any previous hover highlight is gone.
	_hoveredSlot = -1;

	drawBackground();
	drawFilterTabs();
	drawFilterCaption();

	for (uint i = 0; i < kSlotsPerPage; ++i) {
		const uint listIndex = _currentPage * kSlotsPerPage + i;
		const int16 itemID = (listIndex < _visibleItems.size()) ? _visibleItems[listIndex] : -1;
		_slotItemIDs[i] = itemID;
		drawSlot(i, itemID);
	}

	_needsRedraw = true;
}

bool InventoryPopup::itemMatchesFilter(int16 itemID) const {
	const INV::ItemDescription &desc = _invData->itemDescriptions[itemID];

	switch (_activeFilterIndex) {
	case kFilterViewable:
		return desc.keepItem == 3;
	case kFilterPortable:
		return desc.keepItem <= 2;
	case kFilterAll:
	default:
		return true;
	}
}

bool InventoryPopup::syncOrderWithInventory() {
	const uint16 numItems = MIN<uint16>(g_nancy->getStaticData().numItems,
										_invData->itemDescriptions.size());
	const int16 heldItem = NancySceneState.getHeldItem();
	Common::Array<int16> &order = NancySceneState.getInventoryBox().getOrder();

	bool changed = false;

	// Drop entries that are out of range, not owned, or being carried
	// (`hasItem` reports the held item as owned, so exclude it explicitly).
	for (uint i = 0; i < order.size();) {
		const int16 id = order[i];
		if (id < 0 || id >= (int16)numItems || id == heldItem ||
				NancySceneState.hasItem(id) != g_nancy->_true) {
			order.remove_at(i);
			changed = true;
		} else {
			++i;
		}
	}

	// Append any owned items missing from the list, in item-ID order.
	for (uint16 id = 0; id < numItems; ++id) {
		if ((int16)id == heldItem || NancySceneState.hasItem(id) != g_nancy->_true)
			continue;

		bool inOrder = false;
		for (uint i = 0; i < order.size(); ++i) {
			if (order[i] == (int16)id) {
				inOrder = true;
				break;
			}
		}
		if (!inOrder) {
			order.push_back(id);
			changed = true;
		}
	}

	return changed;
}

void InventoryPopup::rebuildVisibleList() {
	_visibleItems.clear();

	// Make sure the persisted order list holds exactly the items the player
	// owns before reading from it. This heals older saves (or any desync) in
	// one pass, instead of waiting for the player to pick up and drop each
	// item individually.
	syncOrderWithInventory();

	const uint16 numItems = MIN<uint16>(g_nancy->getStaticData().numItems,
										_invData->itemDescriptions.size());

	const int16 heldItem = NancySceneState.getHeldItem();

	// The grid follows the shared, save-persisted inventory order (see
	// Scene::addItemToInventory), so the sequence in which items are picked
	// up and dropped is preserved rather than always sorting by item ID.
	const Common::Array<int16> &order = NancySceneState.getInventoryBox().getOrder();

	for (uint i = 0; i < order.size(); ++i) {
		const int16 id = order[i];
		if (id < 0 || id >= (int16)numItems)
			continue;

		// `hasItem` reports the held item as owned; the player is already
		// carrying it, so don't list it in the grid too.
		if (id == heldItem || NancySceneState.hasItem(id) != g_nancy->_true)
			continue;

		if (itemMatchesFilter(id))
			_visibleItems.push_back(id);
	}
}

void InventoryPopup::drawBackground() {
	Common::Rect src = _uiivData->header.normalSrcRect;
	_drawSurface.blitFrom(_overlayImage, src, Common::Point(0, 0));

	drawCloseButton(_closeButtonHovered);

	drawScrollbar(_scrollbarDragging ? kUIButtonPressed : (_scrollbarHovered ? kUIButtonHover : kUIButtonIdle));
}

Common::Rect InventoryPopup::computeSliderRect() const {
	const UISliderRecord &sl = _uiivData->header.slider;
	if (!_uiivData->header.sliderEnabled)
		return Common::Rect();

	const int trackHeight = sl.destRect.height();
	const int thumbHeight = sl.sourceRects[0].height();
	const int travel = MAX(0, trackHeight - thumbHeight);
	const int thumbY = sl.destRect.top + (int)(_scrollPos * travel);

	const Common::Point chunkOrigin(_uiivData->header.normalDestRect.left,
									_uiivData->header.normalDestRect.top);

	Common::Rect r(sl.destRect.left, thumbY,
					sl.destRect.left + sl.sourceRects[0].width(),
					thumbY + thumbHeight);
	r.translate(-chunkOrigin.x, -chunkOrigin.y);
	return r;
}

void InventoryPopup::drawScrollbar(UIButtonState state) {
	const UISliderRecord &sl = _uiivData->header.slider;
	if (!_uiivData->header.sliderEnabled)
		return;

	Common::Rect spr = sl.sourceRects[state];
	if (spr.isEmpty())
		spr = sl.sourceRects[0];
	if (spr.isEmpty())
		return;

	const Common::Rect thumb = computeSliderRect();
	if (thumb.isEmpty())
		return;

	_drawSurface.blitFrom(_overlayImage, spr, Common::Point(thumb.left, thumb.top));
}

void InventoryPopup::updatePageFromScroll() {
	const uint numPages = (_visibleItems.size() + kSlotsPerPage - 1) / kSlotsPerPage;
	if (numPages <= 1) {
		_currentPage = 0;
		_scrollPos = 0.0f;
		return;
	}

	const uint maxPage = numPages - 1;
	uint page = (uint)(_scrollPos * maxPage + 0.5f);
	_currentPage = MIN<uint>(page, maxPage);
}

void InventoryPopup::drawCloseButton(bool hovered) {
	const UIButtonRecord &btn = _uiivData->header.secondaryButton;
	Common::Rect spr = btn.sourceRects[hovered ? kUIButtonHover : kUIButtonIdle];
	Common::Rect dstRect = btn.destRect;
	if (btn.destUsesGameFrameOffset) {
		const VIEW *view = GetEngineData(VIEW);
		if (view) {
			dstRect.translate(view->screenPosition.left, view->screenPosition.top);
		}
	}
	const Common::Point dst(dstRect.left - _screenPosition.left,
							dstRect.top - _screenPosition.top);

	_drawSurface.blitFrom(_overlayImage, spr, dst);
}

void InventoryPopup::drawFilterTabs() {
	for (uint i = 0; i < kNumFilters; ++i) {
		drawFilterTab(i);
	}
}

void InventoryPopup::drawFilterTab(uint index, bool drawHover) {
	const Common::Point chunkOrigin(_uiivData->header.normalDestRect.left,
									_uiivData->header.normalDestRect.top);

	const UIButtonSlot &filter = _uiivData->filters[index];
	if (!filter.enabled)
		return;

	const bool isActive = (index == _activeFilterIndex);
	uint rectIndex = isActive ? kStatePressed : kStateIdle;
	if (drawHover)
		rectIndex = kStateHover;
	Common::Rect spr = filter.button.sourceRects[rectIndex];

	_drawSurface.blitFrom(_overlayImage, spr,
						  Common::Point(filter.button.destRect.left - chunkOrigin.x,
										filter.button.destRect.top - chunkOrigin.y));
}


void InventoryPopup::drawFilterCaption() {
	if (_activeFilterIndex >= _uiivData->tabCaptionSrcRects.size())
		return;

	const Common::Rect &spr = _uiivData->tabCaptionSrcRects[_activeFilterIndex];
	const Common::Point chunkOrigin(_uiivData->header.normalDestRect.left,
									_uiivData->header.normalDestRect.top);
	_drawSurface.blitFrom(_overlayImage, spr,
							Common::Point(_uiivData->tabCaptionDestRect.left - chunkOrigin.x,
											_uiivData->tabCaptionDestRect.top - chunkOrigin.y));
}

void InventoryPopup::setActiveFilterIndex(uint index) {
	if (index >= kNumFilters)
		return;

	_activeFilterIndex = index;
}

void InventoryPopup::playButtonClickSound(const UIButtonRecord &button) {
	SoundDescription sound = button.clickSound;
	if (sound.name.empty() || sound.name.equalsIgnoreCase("NO SOUND"))
		return;

	g_nancy->_sound->loadSound(sound);
	g_nancy->_sound->playSound(sound);
}

void InventoryPopup::drawSlot(uint slotIndex, int16 itemId, bool highlighted) {
	if (slotIndex >= _uiivData->slotDestRects.size())
		return;

	if (itemId < 0 || itemId >= (int16)_invData->itemDescriptions.size())
		return;

	const INV::ItemDescription &desc = _invData->itemDescriptions[itemId];

	// Hovering an item shows its highlighted sprite (the icon with a blue glow
	// baked in) instead of the plain one. Both are opaque and cell-sized, so
	// one fully overwrites the other. Fall back to the plain sprite if the game
	// has no highlighted variant.
	const Common::Rect &src = (highlighted && !desc.highlightedSourceRect.isEmpty())
									? desc.highlightedSourceRect : desc.sourceRect;
	if (src.isEmpty())
		return;

	const Common::Point chunkOrigin(_uiivData->header.normalDestRect.left,
									_uiivData->header.normalDestRect.top);
	const Common::Rect &slotDst = _uiivData->slotDestRects[slotIndex];
	_drawSurface.blitFrom(_itemIcons, src,
							Common::Point(slotDst.left - chunkOrigin.x,
											slotDst.top - chunkOrigin.y));
}

void InventoryPopup::handleInput(NancyInput &input) {
	if (!_isVisible)
		return;

	const Common::Point chunkMouse(
		input.mousePos.x - _screenPosition.left + _uiivData->header.normalDestRect.left,
		input.mousePos.y - _screenPosition.top  + _uiivData->header.normalDestRect.top);

	// Scrollbar interaction takes priority while dragging.
	const UISliderRecord &slider = _uiivData->header.slider;
	if (_uiivData->header.sliderEnabled) {
		const Common::Rect &track = slider.destRect;
		// The thumb rect sits inside the track; build it in chunk coords
		// for hit-testing against `chunkMouse` (which is also in chunk
		// coords since we did the conversion above).
		const int trackHeight = track.height();
		const int thumbHeight = slider.sourceRects[0].height();
		const int travel = MAX(0, trackHeight - thumbHeight);
		const int thumbY = track.top + (int)(_scrollPos * travel);
		Common::Rect thumbInChunk(track.left, thumbY,
									track.left + slider.sourceRects[0].width(),
									thumbY + thumbHeight);

		const bool overScrollbar = thumbInChunk.contains(chunkMouse);

		if (_scrollbarDragging) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);

			const int newThumbTop = chunkMouse.y - _scrollbarGrabOffset;
			const int clamped = CLIP<int>(newThumbTop, track.top, track.top + travel);
			const float newScrollPos = travel > 0 ? (float)(clamped - track.top) / (float)travel : 0.0f;

			// Only re-render when the thumb actually moves. refreshGrid() redraws the
			// whole popup, so calling it every frame while the button is merely held
			// down pins the CPU and makes dragging choppy.
			if (newScrollPos != _scrollPos) {
				_scrollPos = newScrollPos;
				updatePageFromScroll();
				refreshGrid();
			}

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_scrollbarDragging = false;
				drawScrollbar(overScrollbar ? kUIButtonHover : kUIButtonIdle);
				_needsRedraw = true;
			}
			input.eatMouseInput();
			return;
		}

		if (overScrollbar != _scrollbarHovered) {
			_scrollbarHovered = overScrollbar;
			drawScrollbar(overScrollbar ? kUIButtonHover : kUIButtonIdle);
			_needsRedraw = true;
		}
		if (overScrollbar) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (slider.isDraggable && (input.input & NancyInput::kLeftMouseButtonDown)) {
				_scrollbarDragging = true;
				_scrollbarGrabOffset = chunkMouse.y - thumbY;
				drawScrollbar(kUIButtonPressed);
				_needsRedraw = true;
				input.eatMouseInput();
				return;
			}
		}
	}

	if (_uiivData->header.secondaryButtonEnabled) {
		const UIButtonRecord &closeBtn = _uiivData->header.secondaryButton;
		Common::Rect closeScreen = closeBtn.destRect;
		if (closeBtn.destUsesGameFrameOffset) {
			const VIEW *view = GetEngineData(VIEW);
			if (view) {
				closeScreen.translate(view->screenPosition.left, view->screenPosition.top);
			}
		}
		const bool overClose = closeScreen.contains(input.mousePos);
		if (overClose != _closeButtonHovered) {
			_closeButtonHovered = overClose;
			drawCloseButton(overClose);
			_needsRedraw = true;
		}
		if (overClose) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				playButtonClickSound(closeBtn);
				input.eatMouseInput();
				close();
				return;
			}
		}
	}

	const int16 heldItem = NancySceneState.getHeldItem();

	// While the player is carrying an item, the whole popup acts as a single
	// drop zone: a click anywhere inside it drops the held item back into the
	// inventory (appended to the end of the list) and keeps the popup open, so
	// the player never has to carefully target one of the slot squares. The
	// close button, handled above, still closes without dropping.
	if (heldItem != -1) {
		if (_screenPosition.contains(input.mousePos)) {
			// kHotspot keeps the carried-item sprite showing on Nancy 10+.
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				NancySceneState.addItemToInventory(heldItem);
			}
			input.eatMouseInput();
		}
		return;
	}

	// Empty-handed: a click on an occupied slot picks that item up (or
	// navigates to its close-up scene). Hovering an item uses the yellow
	// hotspot arrow rather than the magnifier.
	int hoveredSlot = -1;
	for (uint i = 0; i < kSlotsPerPage; ++i) {
		if (i >= _uiivData->slotDestRects.size())
			break;
		if (!_uiivData->slotDestRects[i].contains(chunkMouse))
			continue;
		if (_slotItemIDs[i] < 0)
			continue;
		hoveredSlot = (int)i;
		break;
	}

	// Swap the glow between slots when the hovered slot changes: restore the
	// slot we left to its plain sprite and draw the newly hovered one with its
	// highlighted sprite.
	if (hoveredSlot != _hoveredSlot) {
		if (_hoveredSlot != -1)
			drawSlot((uint)_hoveredSlot, _slotItemIDs[_hoveredSlot], false);
		if (hoveredSlot != -1)
			drawSlot((uint)hoveredSlot, _slotItemIDs[hoveredSlot], true);
		_hoveredSlot = hoveredSlot;
		_needsRedraw = true;
	}

	if (hoveredSlot != -1) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			const int16 itemID = _slotItemIDs[hoveredSlot];
			const INV::ItemDescription &item = _invData->itemDescriptions[itemID];
			const byte disabled = NancySceneState.getItemDisabledState(itemID);

			if (disabled) {
				if (disabled == 2) {
					NancySceneState.playItemCantSound(itemID);
				}
				input.eatMouseInput();
				return;
			}

			const bool pickUp = item.keepItem != kInvItemNewSceneView;
			NancySceneState.removeItemFromInventory(itemID, pickUp);

			if (item.keepItem == kInvItemNewSceneView) {
				// Close-up view: stash the item and warp to its scene, which
				// dismisses the popup. A normal pickup keeps the popup open.
				g_nancy->_sound->playSound("BUOK");
				NancySceneState.pushScene(itemID);
				SceneChangeDescription sceneChange;
				sceneChange.sceneID = item.sceneID;
				sceneChange.continueSceneSound = item.sceneSoundFlag;
				NancySceneState.changeScene(sceneChange);
				close();
			}

			input.eatMouseInput();
			return;
		}
	}

	bool wasHovered = _filterHovered;
	_filterHovered = false;

	for (uint i = 0; i < kNumFilters; ++i) {
		const UIButtonSlot &filter = _uiivData->filters[i];
		if (filter.button.destRect.contains(chunkMouse)) {
			_filterHovered = true;
			break;
		}
	}

	for (uint i = 0; i < kNumFilters; ++i) {
		const UIButtonSlot &filter = _uiivData->filters[i];
		if (!filter.enabled)
			continue;

		if (!filter.button.destRect.contains(chunkMouse)) {
			if (_filterHovered || wasHovered)
				drawFilterTab(i);
			continue;
		}

		g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);

		drawFilterTab(i, true);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			playButtonClickSound(filter.button);
			setActiveFilterIndex(i);
			_currentPage = 0;
			_scrollPos = 0.0f;
			_scrollbarDragging = false;
			refreshGrid();
			input.eatMouseInput();
			return;
		}
	}

	if (_filterHovered || wasHovered)
		_needsRedraw = true;

	// While the popup is open, swallow clicks that fall on the popup so
	// the underlying scene/viewport doesn't react.
	if (_screenPosition.contains(input.mousePos))
		input.eatMouseInput();
}

} // End of namespace UI
} // End of namespace Nancy
