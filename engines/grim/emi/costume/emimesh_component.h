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

#ifndef GRIM_EMI_MESH_COMPONENT_H
#define GRIM_EMI_MESH_COMPONENT_H

#include "engines/grim/costume/component.h"

namespace Grim {

class EMICostume;
class EMIModel;

class EMIMeshComponent : public Component {
public:
	EMIMeshComponent(Component *parent, int parentID, const char *filename, Component *prevComponent, tag32 tag, EMICostume *costume);
	~EMIMeshComponent();
	void init() override;
	int update(uint time) override;
	void reset() override;
	void draw() override;
	void getBoundingBox(int *x1, int *y1, int *x2, int *y2) const;

public:
	EMICostume *_costume;
	bool _hierShared;
	Common::List<EMIMeshComponent*> _children;
	EMIMeshComponent *_parentModel;
	EMIModel *_obj;
	bool _hasComplained; // Temporary fix for warning-spam.
};

} // end of namespace Grim

#endif
