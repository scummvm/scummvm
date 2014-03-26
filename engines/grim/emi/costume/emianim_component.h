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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GRIM_EMI_ANIM_COMPONENT_H
#define GRIM_EMI_ANIM_COMPONENT_H

#include "engines/grim/costume/component.h"

// This is mostly stubbed for testing the animation loading at the moment.

namespace Grim {

class AnimationEmi;

class EMIAnimComponent : public Component {
public:
	EMIAnimComponent(Component *parent, int parentID, const char *filename, Component *prevComponent, tag32 tag);
	~EMIAnimComponent();
	void init();
	void setKey(int);
	int update(uint time);
	void reset();
	void draw();

private:
	bool _hierShared;
	bool _looping;
	bool _active;
	Component *_parentModel;
	AnimationEmi *_obj;
};

} // end of namespace Grim

#endif
