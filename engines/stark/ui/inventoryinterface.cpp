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

#include "engines/stark/ui/inventoryinterface.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/resources/anim.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"

#include "engines/stark/visual/image.h"

namespace Stark {

InventoryInterface::InventoryInterface() {
	StaticProvider *staticProvider = StarkServices::instance().staticProvider;
	_backgroundTexture = staticProvider->getUIItem(StaticProvider::kInventoryBg);
	_position = Common::Point(40, 50);
}

void InventoryInterface::render() {
	Gfx::Driver *gfx = StarkServices::instance().gfx;
	gfx->setScreenViewport(false);

	_backgroundTexture->getVisual()->get<VisualImageXMG>()->render(_position);
}

bool InventoryInterface::containsPoint(Common::Point point) {
	Common::Rect r;
	r.left = _position.x;
	r.top = _position.y;
	r.setWidth(_backgroundTexture->getVisual()->get<VisualImageXMG>()->getWidth());
	r.setHeight(_backgroundTexture->getVisual()->get<VisualImageXMG>()->getHeight());
	return r.contains(point);
}

} // End of namespace Stark
