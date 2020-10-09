/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "engines/grim/costume.h"
#include "engines/grim/resource.h"
#include "engines/grim/colormap.h"

#include "engines/grim/costume/colormap_component.h"

namespace Grim {

ColormapComponent::ColormapComponent(Component *p, int parentID, const char *filename, tag32 t) :
		Component(p, parentID, filename, t) {
	_cmap = g_resourceloader->getColormap(_name);

	// Set the colormap here in the ctor and not in init()!
	if (p)
		p->setColormap(_cmap);
}

void ColormapComponent::init() {
	if (!_parent)
		warning("No parent to apply colormap object on. CMap: %s, Costume: %s",
				_cmap->getFilename().c_str(), _cost->getFilename().c_str());
}

} // end of namespace Grim
