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

#ifndef GRIM_ANIM_COMPONENT_H
#define GRIM_ANIM_COMPONENT_H

#include "engines/grim/costume/component.h"

namespace Grim {

// This is used in Grim demo only
class AnimComponent : public Component {
public:
	AnimComponent(Component *parent, int parentID, const char *filename, tag32 tag);
	void setKey(int val) override;
	void reset() override;

private:
	bool _created;
	bool _overlay;
};

} // end of namespace Grim

#endif
