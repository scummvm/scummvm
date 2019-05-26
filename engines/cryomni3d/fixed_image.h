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

#ifndef CRYOMNI3D_FIXED_IMAGE_H
#define CRYOMNI3D_FIXED_IMAGE_H

#include "common/func.h"

#include "cryomni3d/cryomni3d.h"
#include "cryomni3d/objects.h"

namespace Graphics {
struct Surface;
}

namespace CryOmni3D {

struct FixedImageConfiguration {
	uint spriteNothing;
	uint spriteLow;
	uint spriteHigh;
	uint spriteLeft;
	uint spriteRight;
	uint spriteQuestion;
	uint spriteListen;
	uint spriteSee;
	uint spriteUse;
	uint spriteSpeak;

	int16 toolbarTriggerY;
};

class ZonFixedImage {
public:
	typedef Common::Functor1<ZonFixedImage *, void> CallbackFunctor;
	enum ZonesMode {
		kZonesMode_None = 0,
		kZonesMode_Standard,
		kZonesMode_Object
	};

	/* These functions are used in main engine code */
	ZonFixedImage(CryOmni3DEngine &engine, Inventory &inventory, const Sprites &sprites,
	              const FixedImageConfiguration *configuration);
	~ZonFixedImage();

	void run(const CallbackFunctor *callback);

	/* This function is used to refresh image after various events */
	void display() const;

	/* These functions and attributes are used in image handler */
	void load(const Common::String &image, const char *zone = nullptr);
	void manage();
	void updateSurface(const Graphics::Surface *newSurface);
	const Graphics::Surface *surface() const { return _imageSurface; }
	void changeCallback(CallbackFunctor *callback) { delete _callback; _callback = callback; }
	Common::Point getZoneCenter(uint zoneId) const;
	void disableZone(uint zoneId) { _zones[zoneId].valid = false; }

	ZonesMode _zonesMode;

	/* These attributes are read by the image handler to check what action player did */
	uint _currentZone;
	bool _exit;
	bool _zoneLow;
	bool _zoneHigh;
	bool _zoneHighLeft;
	bool _zoneHighRight;
	bool _zoneLeft;
	bool _zoneRight;
	bool _zoneQuestion;
	bool _zoneListen;
	bool _zoneSee;
	bool _zoneUse;
	bool _zoneSpeak;
	Object *_usedObject;
	Common::KeyState _key;

protected:
	const CallbackFunctor *_callback;
	CryOmni3DEngine &_engine;
	Inventory &_inventory;
	const Sprites &_sprites;

	struct Zone {
		Common::Rect rect;
		/* ZON file stores the index in the sprite */
		uint16 spriteId;
		uint16 cursorId;
		bool valid;
	};

	void loadZones(const Common::String &image);
	void handleMouseZones(const Common::Array<Zone>::const_iterator &currentZone);

	Image::ImageDecoder *_imageDecoder;
	const Graphics::Surface *_imageSurface;

	Common::Array<Zone> _zones;
	Common::Array<Zone>::size_type _highLeftId;
	Common::Array<Zone>::size_type _highRightId;

	const FixedImageConfiguration *_configuration;

	bool _refreshCursor;
};

} // End of namespace CryOmni3D

#endif
