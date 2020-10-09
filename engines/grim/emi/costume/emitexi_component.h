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

#ifndef GRIM_EMI_TEXI_COMPONENT_H
#define GRIM_EMI_TEXI_COMPONENT_H

#include "engines/grim/costume/component.h"
#include "engines/grim/material.h"
#include "engines/grim/resource.h"
#include "engines/grim/model.h"

namespace Grim {

class EMITexiComponent : public Component {
public:
	EMITexiComponent(Component *parent, int parentID, const char *filename, Component *prevComponent, tag32 tag);
	~EMITexiComponent();

	void init() override;
	int update(uint time) override;
	void reset() override;
	void draw() override;
	void setKey(int k) override;

private:
	Material *_mat;
};

} // end of namespace Grim

#endif
