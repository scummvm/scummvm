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
#include "engines/stark/resources/resource.h"

namespace Stark {

class Item;
class XRCReadStream;

/**
 * A location layer
 *
 * Layers own the scene items
 */
class Layer : public Resource {
public:
	static const ResourceType::Type TYPE = ResourceType::kLayer;

	enum SubType {
		kLayer2D = 1,
		kLayer3D = 2
	};

	/** Layer factory */
	static Resource *construct(Resource *parent, byte subType, uint16 index, const Common::String &name);

	Layer(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Layer();

	// Resource API
	virtual void readData(XRCReadStream *stream) override;

	/** Obtain the render entry for the background item */
	virtual RenderEntry *getBackgroundRenderEntry() = 0;

	/** Obtain the render entries for all items, including the background */
	virtual RenderEntryArray listRenderEntries() = 0;

protected:
	void printData() override;

	float _field_44;
	uint _field_50;
};

/**
 * A 2D layer
 *
 * 2D layers contain 2D positioned items
 */
class Layer2D : public Layer {
public:
	Layer2D(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Layer2D();

	// Resource API
	void readData(XRCReadStream *stream) override;

	// Layer API
	RenderEntry *getBackgroundRenderEntry() override;
	RenderEntryArray listRenderEntries() override;

protected:
	void printData() override;

	Common::Array<uint32> _itemIndices;
};

/**
 * A 3D layer
 *
 * 3D layers contain 3D positioned items, a camera and a floorfield
 */
class Layer3D : public Layer {
public:
	Layer3D(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Layer3D();

	// Resource API
	void readData(XRCReadStream *stream) override;
	void onAllLoaded() override;

	// Layer API
	RenderEntry *getBackgroundRenderEntry() override;
	RenderEntryArray listRenderEntries() override;

protected:
	void printData() override;

	uint32 _field_54;
	uint32 _maxShadowLength;
	float _nearClipPlane;
	float _farClipPlane;

	Item *_backgroundItem;
	Common::Array<Item *> _items;
};

} // End of namespace Stark

#endif // STARK_RESOURCES_LAYER_H
