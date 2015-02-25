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

#ifndef STARK_INVENTORYINTERFACE_H
#define STARK_INVENTORYINTERFACE_H

#include "engines/stark/gfx/renderentry.h"

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/array.h"

namespace Stark {

class VisualImageXMG;
class ClickText;

namespace Resources {
class Anim;
class Item;
}

namespace Gfx {
class Texture;
}

class InventoryInterface {
	Resources::Anim *_backgroundTexture;
	Common::Point _position;
	Common::Array<Resources::Item*> _items;
	Gfx::RenderEntryArray _renderEntries;
public:
	InventoryInterface();
	virtual ~InventoryInterface() {}
	void render();
	void update();
	bool containsPoint(Common::Point point);
	Common::String getMouseHintAtPosition(Common::Point point);
	Gfx::RenderEntryArray getRenderEntries() { return _renderEntries; }
};

} // End of namespace Stark

#endif // STARK_DIALOG_INTERFACE_H