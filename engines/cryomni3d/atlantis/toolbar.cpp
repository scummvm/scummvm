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

#include "common/system.h"

#include "cryomni3d/cryomni3d.h"

#include "cryomni3d/atlantis/engine.h"
#include "cryomni3d/atlantis/toolbar.h"

namespace CryOmni3D {
namespace Atlantis {

Atlantis_Toolbar::Atlantis_Toolbar() :
	_sprites(nullptr), _inventory(nullptr), _engine(nullptr),
	_inventoryOffset(0), _inventoryMaxOffset(0),
	_inventoryHovered(uint(-1)), _inventorySelected(uint(-1)),
	_inventoryButtonDragging(false),
	_position(82) {
}

Atlantis_Toolbar::~Atlantis_Toolbar() {
	_bgSurface.free();
	_destSurface.free();
}

void Atlantis_Toolbar::init(const Sprites *sprites, Inventory *inventory, CryOmni3DEngine_Atlantis *engine) {
	_sprites  = sprites;
	_inventory = inventory;
	_engine   = engine;

	_format = g_system->getScreenFormat();
	// 82 rows: matches the height of OBJETS0.SPR, the single inventory-bar
	// background sprite in the original game.  The bar slides up from
	// y=screen.h to y=screen.h-82.
	_bgSurface.create(640, 82, _format);
	_destSurface.create(640, 82, _format);

	// 9 inventory slot rectangles.  Slot centres come straight from
	// atlantis.exe (toolbar setup FUN_0042c2c4): the slot-X table at
	// 0x00611710 is filled with  x = i * 0x46 + 0x2a  →  i * 70 + 42, and the
	// slot-Y table at 0x00611788 settles at 0x1bb = 443 (screen coords).  The
	// toolbar surface is 640 wide at screen y = 480-82 = 398, so the slot
	// centre in toolbar-local coords is (i*70+42, 443-398 = 45).  Confirmed by
	// dynamic dump: 0x611710 = {42,112,182,...}, 0x611788 = {443,443,...}.
	static const int kSlotCenters[kInventorySlots] = {
		42, 112, 182, 252, 322, 392, 462, 532, 602
	};
	const int kSlotHalfW = 25;
	const int kSlotTop   = 25;   // toolbar-local; centre y = 45 → screen 443
	const int kSlotBot   = 65;
	ZoneCallback kSlotCbs[kInventorySlots] = {
		&Atlantis_Toolbar::callbackInventory<0>,
		&Atlantis_Toolbar::callbackInventory<1>,
		&Atlantis_Toolbar::callbackInventory<2>,
		&Atlantis_Toolbar::callbackInventory<3>,
		&Atlantis_Toolbar::callbackInventory<4>,
		&Atlantis_Toolbar::callbackInventory<5>,
		&Atlantis_Toolbar::callbackInventory<6>,
		&Atlantis_Toolbar::callbackInventory<7>,
		&Atlantis_Toolbar::callbackInventory<8>,
	};
	for (uint i = 0; i < kInventorySlots; i++)
		addZone(Common::Rect(kSlotCenters[i] - kSlotHalfW, kSlotTop,
		                     kSlotCenters[i] + kSlotHalfW, kSlotBot),
		        kSlotCbs[i]);
}

void Atlantis_Toolbar::addZone(Common::Rect rect, ZoneCallback callback) {
	Zone zone = { rect, callback, true, false };
	_zones.push_back(zone);
}

void Atlantis_Toolbar::inventoryChanged(uint newPosition) {
	if (newPosition != uint(-1) && newPosition > _inventoryOffset)
		_inventoryOffset = newPosition - (kInventorySlots - 1);
	updateZones();
}

void Atlantis_Toolbar::updateZones() {
	// Compute max scroll offset (how far we can scroll right).  No prev/next
	// buttons on the bar, but inventoryChanged() still pages forward
	// automatically when a new item lands outside the visible window.
	_inventoryMaxOffset = 0;
	for (uint i = kInventorySlots; i < _inventory->size(); i++) {
		if ((*_inventory)[i] != nullptr)
			_inventoryMaxOffset = i - (kInventorySlots - 1);
	}

	if (_inventoryOffset > _inventoryMaxOffset)
		_inventoryOffset = _inventoryMaxOffset;
}

Common::Array<Atlantis_Toolbar::Zone>::const_iterator Atlantis_Toolbar::hitTestZones(
	const Common::Point &mousePos) const {
	// atlantis.exe FUN_0042c098 hit-tests the inventory bar as 9 full-width
	// 70-px columns (slot = mouseX / 70) — not the icon-sized rects — so a
	// click anywhere in a slot's column counts.  Valid while the cursor is
	// inside the bar: screen y >= 409, i.e. toolbar-local y >= 11.  (The Zone
	// rects themselves only position the item icons; see drawToolbar().)
	if (mousePos.y < 11 || mousePos.y >= (int)_destSurface.h)
		return _zones.end();
	int column = mousePos.x / 70;
	if (column < 0 || column >= (int)_zones.size())
		return _zones.end();
	Common::Array<Zone>::const_iterator it = _zones.begin() + column;
	if (it->hidden || !it->callback)
		return _zones.end();
	return it;
}

uint Atlantis_Toolbar::captureEvent(const Common::Point &mousePos, uint dragStatus) {
	Common::Array<Zone>::const_iterator it = hitTestZones(mousePos);
	if (it != _zones.end())
		return (this->*(it->callback))(dragStatus);
	return 0;
}

// -----------------------------------------------------------------------
// Drawing
// -----------------------------------------------------------------------

void Atlantis_Toolbar::addMenuSprite(uint16 w, uint16 h, int16 xoff, int16 yoff,
                                      uint16 transpKey, int tx, int ty, const uint16 *pix,
                                      const uint8 *blend) {
	MenuSprData s;
	s.w = w; s.h = h;
	s.xoff = xoff; s.yoff = yoff;
	s.tx = tx; s.ty = ty;
	s.transpKey = transpKey;
	s.pixels.assign(pix, pix + (uint)w * h);
	s.blend.assign(blend, blend + (uint)w * h);
	_menuSprData.push_back(s);
}

void Atlantis_Toolbar::drawBackground(const Graphics::Surface *original) {
	if (!original)
		return;
	// Capture the bottom 60 rows of the current scene *unmodified*.  Atlantis
	// does not paint a darkened panel under the toolbar -- the SPRMENU sprites
	// composited on top (golden filigree line + options emblem + corner
	// ornaments) are the entirety of the visible toolbar, with the scene
	// showing through everywhere else.  Previous code halved every pixel,
	// which produced a dark band that doesn't match the original game.
	const int srcY = original->h - _bgSurface.h;
	if (srcY < 0)
		return;
	const byte *src = (const byte *)original->getBasePtr(0, srcY);
	byte *dst = (byte *)_bgSurface.getPixels();
	const uint rowBytes = _bgSurface.w * _format.bytesPerPixel;
	for (int y = 0; y < _bgSurface.h; y++)
		memcpy(dst + y * _bgSurface.pitch, src + y * original->pitch, rowBytes);
}

void Atlantis_Toolbar::drawToolbar(const Graphics::Surface *original) {
	if (_position > 82)
		_position = 82;

	if (_position != 0) {
		// Not fully visible: copy the top _position rows from original
		if (original) {
			int srcY = original->h - _destSurface.h;
			Common::Rect srcRct(0, srcY, 640, srcY + _position);
			_destSurface.copyRectToSurface(*original, 0, 0, srcRct);
		}
	}

	if (_position == (uint)_destSurface.h) {
		// Fully hidden
		return;
	}

	// Copy the (unmodified) scene strip into the visible region.
	{
		Common::Rect visRct(0, _position, 640, _destSurface.h);
		_destSurface.copyRectToSurface(_bgSurface, 0, _position, visRct);
	}

	// Draw SPRMENU decorative sprites over the darkened background
	for (const MenuSprData &spr : _menuSprData) {
		int baseX = spr.tx - spr.xoff;
		int baseY = (spr.ty - spr.yoff) + _position;
		for (int y = 0; y < (int)spr.h; y++) {
			int dy = baseY + y;
			if (dy < 0 || dy >= _destSurface.h)
				continue;
			for (int x = 0; x < (int)spr.w; x++) {
				int dx = baseX + x;
				if (dx < 0 || dx >= _destSurface.w)
					continue;
				uint sidx = y * spr.w + x;
				uint16 pix = spr.pixels[sidx];
				if (pix == spr.transpKey)
					continue;
				uint16 *dp = (uint16 *)_destSurface.getBasePtr(dx, dy);
				if (spr.blend[sidx] != kSprNoBlend)
					pix = blendSprPixel565(pix, *dp, spr.blend[sidx]);
				*dp = pix;
			}
		}
	}

	// Draw inventory item icons on top of the OBJETS0 star ornaments.  No
	// per-slot fill/frame -- the bar background already shows the slot
	// position; an empty slot just leaves the star ornament visible.
	for (uint i = 0; i < kInventorySlots; i++) {
		uint invIdx = i + _inventoryOffset;
		Object *obj = (invIdx < _inventory->size()) ? (*_inventory)[invIdx] : nullptr;
		if (obj == nullptr)
			continue;
		// The item currently in hand (taken out to be used) is not shown in
		// the bar — its slot reads as empty, matching atlantis.exe
		// FUN_0042cc98, which skips the held slot while it is on the cursor.
		if (obj == _inventory->selectedObject())
			continue;

		// Slot rect in toolbar-local coords, offset by the slide position.
		// Deliberately NOT clipped to the surface: clipping the rect first
		// would shift its centre once it overlaps an edge, so the icon would
		// lag behind the bar and get trimmed mid-slide.  drawInventoryIcon()
		// clips to the surface itself.
		Common::Rect r = _zones[kZoneInventory0 + i].rect;
		r.translate(0, _position);
		if (r.bottom <= 0 || r.top >= (int)_destSurface.h)
			continue;

		_engine->drawInventoryIcon(_destSurface, obj, r);
	}
}

// -----------------------------------------------------------------------
// Event loop
// -----------------------------------------------------------------------

bool Atlantis_Toolbar::displayToolbar(const Graphics::Surface *original) {
	_engine->setCursor(181 < (int)_sprites->getSpritesCount() ? 181 : 0);

	// Capture the current scene as the (unmodified) toolbar background.
	drawBackground(original);

	// Reset selection state
	_inventorySelected = uint(-1);
	_inventoryHovered  = uint(-1);

	updateZones();

	// Slide toolbar up
	for (_position = _destSurface.h; _position > 0 && !_engine->shouldAbort(); _position--) {
		drawToolbar(original);
		g_system->copyRectToScreen(_destSurface.getPixels(), _destSurface.pitch, 0,
		                           original ? original->h - _destSurface.h : (480 - 82),
		                           _destSurface.w, _destSurface.h);
		g_system->updateScreen();
		g_system->delayMillis(10);
		_engine->pollEvents();
	}

	if (_engine->shouldAbort())
		return false;

	_engine->clearKeys();
	_engine->waitMouseRelease();

	bool itemTaken = handleToolbarEvents(original);

	if (_engine->shouldAbort())
		return false;

	// Slide the toolbar down — skipped when an item was just taken, so the bar
	// clears at once and the carried object reaches the centred navigation
	// crosshair without waiting out the slide.
	if (!itemTaken) {
		for (_position = 0; _position <= (uint)_destSurface.h && !_engine->shouldAbort(); _position++) {
			drawToolbar(original);
			g_system->copyRectToScreen(_destSurface.getPixels(), _destSurface.pitch, 0,
			                           original ? original->h - _destSurface.h : (480 - 82),
			                           _destSurface.w, _destSurface.h);
			g_system->updateScreen();
			g_system->delayMillis(10);
			_engine->pollEvents();
		}
	}

	return false;
}

bool Atlantis_Toolbar::handleToolbarEvents(const Graphics::Surface *original) {
	_inventoryHovered  = uint(-1);
	_inventorySelected = uint(-1);
	_inventoryButtonDragging = false;
	// The in-hand object (selectedObject) is deliberately NOT cleared here:
	// reopening the bar must keep the item you are carrying, so it can be put
	// back only by an explicit action (right-click, or a left-click on an
	// empty part of the bar).

	updateZones();
	drawToolbar(original);
	g_system->copyRectToScreen(_destSurface.getPixels(), _destSurface.pitch, 0,
	                           original ? original->h - _destSurface.h : (480 - 82),
	                           _destSurface.w, _destSurface.h);
	g_system->updateScreen();

	if (_sprites->getSpritesCount() > 181)
		_engine->setCursor(181);

	// Toolbar covers screen.h - toolbar.h .. screen.h, plus a 32px hover
	// buffer above it — i.e. 480 - 82 - 32 = 366.
	const int kInsideY = 480 - (int)_destSurface.h - 32;
	bool mouseInsideToolbar = (_engine->getMousePos().y > kInsideY);
	bool exitToolbar = false;
	bool itemTaken = false;   // set when the loop ends because an item was taken
	bool rightReleasedSinceOpen = false;  // gates the put-back/close right-click

	// Edge-hover scroll throttle: the exe pages one item per frame, which is
	// far too fast at this loop's 10ms cadence, so step at a fixed interval.
	const uint32 kScrollIntervalMs = 110;
	uint32 nextScrollMs = 0;

	while (!exitToolbar) {
		_engine->pollEvents();
		if (_engine->shouldAbort()) {
			exitToolbar = true;
			break;
		}

		// A right-click is acted on only once the button has been released at
		// least once since the bar opened, so the right-click that OPENED the
		// bar is not also consumed as the put-back / close click.
		if (_engine->getCurrentMouseButton() != 2)
			rightReleasedSinceOpen = true;

		// Escape / Space always close the bar.
		if (_engine->checkKeysPressed(2, Common::KEYCODE_ESCAPE, Common::KEYCODE_SPACE)) {
			exitToolbar = true;
			break;
		}
		// Right-click: while carrying an item it drops the item back into its
		// slot (the original uses the right button for this); then — like any
		// right-click — the bar closes.  The slide-down redraws the bar with
		// the item back in place, so it visibly returns before the bar slides
		// away.
		if (_engine->getCurrentMouseButton() == 2 && rightReleasedSinceOpen) {
			_engine->waitMouseRelease();
			if (_inventory->selectedObject() != nullptr) {
				_inventory->setSelectedObject(nullptr);
				_engine->setArrowCursor();   // object leaves the cursor at once
				updateZones();
			}
			exitToolbar = true;
			break;
		}

		Common::Point mousePosInToolbar = _engine->getMousePos();
		mousePosInToolbar -= Common::Point(0, 480 - _destSurface.h);

		DragStatus drag = _engine->getDragStatus();
		bool redrawToolbar = false;
		bool tookItem = false;
		if (captureEvent(mousePosInToolbar, drag)) {
			updateZones();
			redrawToolbar = true;
			// A completed click that leaves an object in hand is a "take".
			// Clicking the held item's own slot is a put-back instead and
			// leaves selectedObject() null, so it is not treated as a take.
			if (drag == kDragStatus_Finished && _inventory->selectedObject() != nullptr)
				tookItem = true;
		} else if (drag == kDragStatus_Pressed) {
			// Left-click on an empty part of the bar: put any in-hand item
			// back into the inventory.
			_inventorySelected = uint(-1);
			_inventory->setSelectedObject(nullptr);
			_engine->setArrowCursor();   // object leaves the cursor at once
			updateZones();
			redrawToolbar = true;
		}

		// Taking an item closes the bar at once: the original (FUN_0042c098)
		// slides the toolbar down as soon as the in-hand object is set.  The
		// slide-down then draws the bar with the taken slot already empty.
		if (tookItem) {
			// Snap the cursor — now carrying the taken item — to the screen
			// centre (matches the original's SetCursorPos(320,240); effective
			// in fullscreen where warpMouse works).  The caller skips the
			// slide-down so navigation resumes — and recentres — at once.
			_engine->setMousePos(Common::Point(g_system->getWidth() / 2, 240));
			itemTaken = true;
			exitToolbar = true;
			break;
		}

		// Edge-hover scrolling (atlantis.exe FUN_0042c8ac): holding the cursor
		// against the right/left screen edge inside the bar pages the visible
		// inventory window.  The exe tests mouse X beyond 0x276 (630) / below
		// 5; here it is throttled to stay usable.
		{
			Common::Point ms = _engine->getMousePos();
			if (ms.y > kInsideY && g_system->getMillis() >= nextScrollMs) {
				if (ms.x > 630 && _inventoryOffset < _inventoryMaxOffset) {
					_inventoryOffset++;
					nextScrollMs = g_system->getMillis() + kScrollIntervalMs;
					redrawToolbar = true;
				} else if (ms.x < 5 && _inventoryOffset > 0) {
					_inventoryOffset--;
					nextScrollMs = g_system->getMillis() + kScrollIntervalMs;
					redrawToolbar = true;
				}
			}
		}

		// Exit when mouse leaves toolbar area
		if (!mouseInsideToolbar) {
			mouseInsideToolbar = (_engine->getMousePos().y > kInsideY);
		} else if (_engine->getMousePos().y <= kInsideY) {
			exitToolbar = true;
			break;
		}

		// Hover inventory items
		if (_inventory->selectedObject() == nullptr) {
			auto zoneIt = hitTestZones(mousePosInToolbar);
			uint zoneId = (uint)(zoneIt - _zones.begin());
			uint invIdx = zoneId + _inventoryOffset;
			bool shouldHover = (zoneId < kInventorySlots &&
			                    invIdx < _inventory->size() &&
			                    (*_inventory)[invIdx] != nullptr);
			if (shouldHover) {
				if (_inventoryHovered != invIdx && (*_inventory)[invIdx]->valid()) {
					_inventoryHovered = invIdx;
					redrawToolbar = true;
				}
			} else if (_inventoryHovered != uint(-1)) {
				_inventoryHovered  = uint(-1);
				_inventorySelected = uint(-1);
				updateZones();
				if (!_inventory->selectedObject() && _sprites->getSpritesCount() > 181)
					_engine->setCursor(181);
				redrawToolbar = true;
			}
			_inventoryButtonDragging = false;
		}

		if (redrawToolbar) {
			drawToolbar(original);
			g_system->copyRectToScreen(_destSurface.getPixels(), _destSurface.pitch, 0,
			                           original ? original->h - _destSurface.h : (480 - 82),
			                           _destSurface.w, _destSurface.h);
		}

		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	return itemTaken;
}

// -----------------------------------------------------------------------
// Zone callbacks
// -----------------------------------------------------------------------

uint Atlantis_Toolbar::callbackInventory(uint invId, uint dragStatus) {
	invId += _inventoryOffset;
	Object *obj = (invId < _inventory->size()) ? (*_inventory)[invId] : nullptr;
	if (obj == nullptr || !obj->valid())
		return 0;

	switch (dragStatus) {
	case kDragStatus_Pressed:
		_inventorySelected = invId;
		if (_sprites->getSpritesCount() > 181)
			_engine->setCursor(181);
		_inventoryButtonDragging = true;
		return 1;
	case kDragStatus_Dragging:
		if (_inventorySelected != invId) {
			_inventorySelected = invId;
			_inventoryButtonDragging = true;
		}
		return 1;
	case kDragStatus_Finished:
		if (obj == _inventory->selectedObject()) {
			// Clicking the slot of the item already in hand puts it back
			// (atlantis.exe FUN_0042c098: curObj == slot item → curObj = 0).
			_inventory->setSelectedObject(nullptr);
			_engine->setArrowCursor();   // object leaves the cursor at once
			_inventorySelected = uint(-1);
			return 1;
		}
		if (_sprites->getSpritesCount() > obj->idSl())
			_engine->setCursor(obj->idSl());
		_inventory->setSelectedObject(obj);
		_inventorySelected = invId;
		return 1;
	default:
		return 0;
	}
}

} // namespace Atlantis
} // namespace CryOmni3D
