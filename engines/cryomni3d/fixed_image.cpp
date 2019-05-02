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

#include "engines/cryomni3d/fixed_image.h"

#include "common/file.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "image/image_decoder.h"

namespace CryOmni3D {

ZonFixedImage::ZonFixedImage(CryOmni3DEngine &engine,
                             Inventory &inventory,
                             const Sprites &sprites,
                             const FixedImageConfiguration *configuration) :
	_engine(engine), _inventory(inventory), _sprites(sprites),
	_configuration(configuration),
	_callback(nullptr), _imageDecoder(nullptr), _imageSurface(nullptr) {
}

ZonFixedImage::~ZonFixedImage() {
	delete _imageDecoder;
}

void ZonFixedImage::run(const CallbackFunctor *callback) {
	_exit = false;
	_zonesMode = kZonesMode_None;

	_callback = callback;

	g_system->showMouse(true);
	while (!_exit) {
		(*_callback)(this);
	}
	_engine.waitMouseRelease();
	g_system->showMouse(false);

	// Deselect object
	_inventory.setSelectedObject(nullptr);

	delete _callback;
	_callback = nullptr;
}

void ZonFixedImage::load(const Common::String &image) {
	_imageSurface = nullptr;
	delete _imageDecoder;
	_imageDecoder = nullptr;

	_imageDecoder = _engine.loadHLZ(image);
	if (!_imageDecoder) {
		error("Can't display fixed image");
	}
	_imageSurface = _imageDecoder->getSurface();

	loadZones(image);
#if 0
	// This is not correct but to debug zones I think it's OK
	Graphics::Surface *tmpSurf = (Graphics::Surface *) _imageSurface;
	for (Common::Array<Zone>::const_iterator it = _zones.begin(); it != _zones.end(); it++) {
		Common::Rect tmp = it->rect;
		tmpSurf->frameRect(tmp, 244);
	}
#endif

	_zonesMode = kZonesMode_Standard;
	_refreshCursor = true;

	display();

	// WORKAROUND: Wait for release after displaying the fixed image to avoid handling events due to mouse being pressed
	// There is this bug in game
	// Don't display cursor to prevent displaying an invalid cursor
	g_system->showMouse(false);
	g_system->updateScreen();
	_engine.waitMouseRelease();
	g_system->showMouse(true);
}

void ZonFixedImage::display() const {
	_engine.setupPalette(_imageDecoder->getPalette(), _imageDecoder->getPaletteStartIndex(),
	                     _imageDecoder->getPaletteColorCount());

	g_system->copyRectToScreen(_imageSurface->getPixels(), _imageSurface->pitch, 0, 0,
	                           _imageSurface->w, _imageSurface->h);
	g_system->updateScreen();
}

void ZonFixedImage::loadZones(const Common::String &image) {
	_zones.clear();

	Common::String fname(_engine.prepareFileName(image, "zon"));

	Common::File zonFile;
	if (!zonFile.open(fname)) {
		error("Can't open ZON file '%s'", fname.c_str());
	}

	int32 zonesNumber = zonFile.size() / 26;
	_zones.reserve(zonesNumber);

	_highLeftId = -1;
	_highRightId = -1;

	int leftSeen = 0x7fffffff; // MAX_INT
	int rightSeen = 0;
	Common::Array<Zone>::size_type index = 0;

	while (zonesNumber > 0) {
		Zone zone;
		zone.rect.left = zonFile.readSint16BE();
		zone.rect.top = zonFile.readSint16BE();
		zone.rect.right = zonFile.readSint16BE();
		zone.rect.bottom = zonFile.readSint16BE();
		zone.spriteId = zonFile.readSint16BE();
		zone.cursorId = _sprites.revMapSpriteId(zone.spriteId);
		zone.valid = true;
		zonFile.skip(16);

		_zones.push_back(zone);

		if (zone.cursorId == _configuration->spriteHigh) {
			if (leftSeen > zone.rect.right) {
				// The right side is at the leftest seen
				leftSeen = zone.rect.right;
				_highLeftId = index;
			}
			if (rightSeen < zone.rect.left) {
				// The left side is at the rightest seen
				rightSeen = zone.rect.left;
				_highRightId = index;
			}
		}

		zonesNumber--;
		index++;
	}
}

Common::Point ZonFixedImage::getZoneCenter(unsigned int zoneId) const {
	if (zoneId >= _zones.size()) {
		error("Invalid zoneId %u/%u", zoneId, _zones.size());
	}
	const Common::Rect &rect = _zones[zoneId].rect;

	return Common::Point((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);
}

void ZonFixedImage::manage() {
	_currentZone = -1;
	_zoneLow = false;
	_zoneHigh = false;
	_zoneHighLeft = false;
	_zoneHighRight = false;
	_zoneLeft = false;
	_zoneRight = false;
	_zoneQuestion = false;
	_zoneListen = false;
	_zoneSee = false;
	_zoneUse = false;
	_zoneSpeak = false;
	_usedObject = nullptr;

	// Force poll events even when we must refresh the cursor
	if (!_engine.pollEvents() && !_refreshCursor) {
		g_system->updateScreen();
		// TODO: countdown even when no events
		return;
	}
	_refreshCursor = false;

	// Feed the key for the caller
	_key = _engine.getNextKey();
	Common::Point mousePos = _engine.getMousePos();

	if (_key == Common::KEYCODE_ESCAPE) {
		_exit = true;
		return;
	} else if (_engine.shouldAbort()) {
		_exit = true;
		return;
	}

	if (_key == Common::KEYCODE_SPACE ||
	        _engine.getCurrentMouseButton() == 2 ||
	        mousePos.y > _configuration->toolbarTriggerY) {
		bool mustRedraw = _engine.displayToolbar(_imageSurface);
		// We just came back from toolbar: check if an object is selected and go into object mode
		if (_inventory.selectedObject()) {
			_zonesMode = kZonesMode_Object;
		}
		if (mustRedraw) {
			display();
		}
		// Return without any event to redo the loop and force refresh
		_refreshCursor = true;
		return;
	}

	Common::Array<Zone>::iterator zoneIt;
	for (zoneIt = _zones.begin(); zoneIt != _zones.end(); zoneIt++) {
		if (zoneIt->valid && zoneIt->rect.contains(mousePos)) {
			break;
		}
	}

	if (zoneIt != _zones.end()) {
		_currentZone = zoneIt - _zones.begin();
	} else {
		_currentZone = -1;
	}

	if (_zonesMode == kZonesMode_Standard) {
		if (zoneIt != _zones.end()) {
			_engine.setCursor(zoneIt->cursorId);
			if (_engine.getCurrentMouseButton() == 1) {
				handleMouseZones(zoneIt);
			}
		} else {
			_engine.setCursor(_configuration->spriteNothing);
		}
	} else if (_zonesMode == kZonesMode_Object) {
		Object *selectedObj = _inventory.selectedObject();
		if (!selectedObj) {
			// Normally useless but we never know
			_engine.setCursor(_configuration->spriteNothing);
		} else if (zoneIt != _zones.end()) {
			_engine.setCursor(selectedObj->idSA());
			if (_engine.getDragStatus() == kDragStatus_Finished) {
				// Just clicked, store the event and go back to standard mode
				_usedObject = selectedObj;
				_zonesMode = kZonesMode_Standard;
				// We changed mode: need to refresh
				_refreshCursor = true;
			}
		} else {
			_engine.setCursor(selectedObj->idSl());
		}

	}

	// TODO: handle countdown
	g_system->updateScreen();
}

void ZonFixedImage::handleMouseZones(const Common::Array<Zone>::const_iterator &currentZone) {
	if (currentZone->cursorId == _configuration->spriteLow) {
		_zoneLow = true;
	} else if (currentZone->cursorId == _configuration->spriteHigh) {
		Common::Array<Zone>::size_type id = currentZone - _zones.begin();
		if (id == _highLeftId) {
			_zoneHighLeft = true;
		} else if (id == _highRightId) {
			_zoneHighRight = true;
		} else {
			_zoneHigh = true;
		}
	} else if (currentZone->cursorId == _configuration->spriteLeft) {
		_zoneLeft = true;
	} else if (currentZone->cursorId == _configuration->spriteRight) {
		_zoneRight = true;
	} else if (currentZone->cursorId == _configuration->spriteQuestion) {
		_zoneQuestion = true;
	} else if (currentZone->cursorId == _configuration->spriteListen) {
		_zoneListen = true;
	} else if (currentZone->cursorId == _configuration->spriteSee) {
		_zoneSee = true;
	} else if (currentZone->cursorId == _configuration->spriteUse) {
		_zoneUse = true;
	} else if (currentZone->cursorId == _configuration->spriteSpeak) {
		_zoneSpeak = true;
	} else {
		error("Invalid cursor ID: %d in ImgFix", currentZone->cursorId);
	}
}

} // End of namespace CryOmni3D
