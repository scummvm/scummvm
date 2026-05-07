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

#ifndef NANCY_UI_INVENTORYPOPUP_H
#define NANCY_UI_INVENTORYPOPUP_H

#include "engines/nancy/renderobject.h"

namespace Nancy {

struct NancyInput;
struct INV;
struct UIIV;

namespace State {
class Scene;
}

namespace UI {

// Nancy 10+ inventory popup (replaces the always-visible InventoryBox of
// pre-Nancy-10 games). Driven by the UIIV chunk (popup graphics + slot
// rects + filter buttons) and the INV chunk (per-item descriptions/icons).
class InventoryPopup : public RenderObject {
public:
	InventoryPopup();
	~InventoryPopup() override = default;

	void init() override;
	void handleInput(NancyInput &input);

	bool isOpen() const { return _isVisible; }
	void open();
	void close();

	// The taskbar's inventory button toggles the popup; convenience helper.
	void toggle() { if (_isVisible) close(); else open(); }

	// Re-render the slot grid. Called by Scene whenever the inventory
	// contents change while the popup is open.
	void refreshGrid();

	enum FilterType {
		kFilterAll       = 0,
		kFilterViewable  = 1,
		kFilterPortable  = 2,
	};

	enum WidgetState {
		kStatePressed = 0,
		kStateHover = 1,
		kStateIdle = 2,
		kStateDisabled = 3
	};

private:
	static const uint kSlotsPerPage = 16;
	static const uint kNumFilters = 3;

	void drawBackground();
	void drawSlot(uint slotIndex, int16 itemId);
	void drawFilterTabs();
	void drawFilterTab(uint index, bool drawHover = false);
	void drawFilterCaption();
	void drawCloseButton(WidgetState state);
	void drawScrollbar(WidgetState state);
	void rebuildVisibleList();
	void setActiveFilterIndex(uint index);

	// Apply the current scrollbar position to the page index, clamping
	// to the number of pages required by the active filter.
	void updatePageFromScroll();

	// Returns the on-popup-surface bounding rect of the slider thumb at
	// the current scroll position (in popup-local coords).
	Common::Rect computeSliderRect() const;

	const UIIV *_uiivData = nullptr;
	const INV *_invData = nullptr;

	Graphics::ManagedSurface _overlayImage;     // popup background image
	Graphics::ManagedSurface _itemIcons;        // per-item icon sheet

	bool _closeButtonHovered = false;

	uint _activeFilterIndex = 0;

	// Page index within the active filter (0-based).
	uint _currentPage = 0;

	// Items the player owns that match the active filter, in inventory
	// order. The on-screen grid is a 16-item window into this array.
	Common::Array<int16> _visibleItems;

	// Item ID currently shown in each of the 16 slots (-1 if empty).
	int16 _slotItemIDs[kSlotsPerPage];

	// Slider state. Driven by header.slider.
	float _scrollPos = 0.0f;        // [0, 1]: 0 = top page, 1 = bottom page
	bool _scrollbarDragging = false;
	bool _scrollbarHovered = false;
	int _scrollbarGrabOffset = 0;   // mouse-y minus thumb-top at click time

	bool _filterHovered = false;
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_INVENTORYPOPUP_H
