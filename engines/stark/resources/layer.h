/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_RESOURCES_LAYER_H
#define STARK_RESOURCES_LAYER_H

#include "common/array.h"
#include "common/str.h"

#include "engines/stark/gfx/renderentry.h"
#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

class Item;

/**
 * A location layer
 *
 * Layers own the scene items
 */
class Layer : public Object {
public:
	static const Type::ResourceType TYPE = Type::kLayer;

	enum SubType {
		kLayer2D = 1,
		kLayer3D = 2
	};

	/** Layer factory */
	static Object *construct(Object *parent, byte subType, uint16 index, const Common::String &name);

	Layer(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Layer();

	// Resource API
	virtual void readData(Formats::XRCReadStream *stream) override;
	virtual void saveLoad(ResourceSerializer *serializer) override;
	virtual void saveLoadCurrent(ResourceSerializer *serializer) override;

	/** Obtain the render entries for all items, including the background */
	virtual Gfx::RenderEntryArray listRenderEntries() = 0;

	/** Obtain a list of render entries for all the lights in the layer */
	Gfx::LightEntryArray listLightEntries();

	/** Scroll the layer to the specified position */
	void setScrollPosition(const Common::Point &position);

	/** Get the current scroll for this layer */
	Common::Point getScroll() const;

	/** Set the current scroll for this layer */
	void setScroll(const Common::Point &scroll);

	/** Enable the layer */
	void enable(bool enabled);

	/** Is the layer enabled? Disabled layers are not drawn. */
	bool isEnabled() const;

protected:
	void printData() override;

	Common::Point _scroll;
	float _scrollScale; // Used for the parallax effect
	bool _enabled;
};

/**
 * A 2D layer
 *
 * 2D layers contain 2D positioned items
 */
class Layer2D : public Layer {
public:
	Layer2D(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Layer2D();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onEnterLocation() override;
	void onExitLocation() override;

	// Layer API
	Gfx::RenderEntryArray listRenderEntries() override;

protected:
	void printData() override;

	Common::Array<uint32> _itemIndices;
	Common::Array<Item *> _items;
};

/**
 * A 3D layer
 *
 * 3D layers contain 3D positioned items, a camera and a floorfield
 */
class Layer3D : public Layer {
public:
	Layer3D(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Layer3D();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onAllLoaded() override;
	void onEnterLocation() override;

	// Layer API
	Gfx::RenderEntryArray listRenderEntries() override;

	/** Obtain the render entry for the background item */
	Gfx::RenderEntry *getBackgroundRenderEntry();

protected:
	void printData() override;

	bool _shouldRenderShadows;
	uint32 _maxShadowLength;
	float _nearClipPlane;
	float _farClipPlane;

	Item *_backgroundItem;
	Common::Array<Item *> _items;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_LAYER_H
