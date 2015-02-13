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

#ifndef STARK_RESOURCES_LOCATION_H
#define STARK_RESOURCES_LOCATION_H

#include "common/rect.h"
#include "common/str.h"

#include "engines/stark/gfx/renderentry.h"
#include "engines/stark/resources/object.h"

namespace Stark {

class XRCReadStream;

namespace Resources {

class Layer;

/**
 * A location is a scene of the game
 *
 * Locations contain layers. The game engine retrieves the list of renderable
 * items from the current location.
 */
class Location : public Object {
public:
	static const Type::ResourceType TYPE = Type::kLocation;

	Location(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Location();

	// Resource API
	void onAllLoaded() override;

	/** Does the location have a 3D layer ? */
	bool has3DLayer();

	/** Obtain a list of render entries for all the items in the location */
	RenderEntryArray listRenderEntries();

	/** Initialize scrolling from Camera data */
	void initScroll(const Common::Point &maxScroll);

	/** Obtain the current scroll position */
	Common::Point getScrollPosition() const;

	/** Scroll the location to the specified position if possible */
	void setScrollPosition(const Common::Point &position);

protected:
	void printData() override;

private:
	Common::Array<Layer *> _layers;

	bool _canScroll;
	Common::Point _scroll;
	Common::Point _maxScroll;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_LOCATION_H
