/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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

#ifndef SHERLOCK_TATTOO_WIDGET_INVENTORY_H
#define SHERLOCK_TATTOO_WIDGET_INVENTORY_H

#include "common/scummsys.h"
#include "sherlock/tattoo/widget_base.h"

namespace Sherlock {

class SherlockEngine;

namespace Tattoo {

class WidgetInventory: public WidgetBase {
private:
	int _invVerbMode;
	int _invSelect, _oldInvSelect;
	int _selector, _oldSelector;
	int _dialogTimer;

	void loadInv();
public:
	int _invMode;
public:
	WidgetInventory(SherlockEngine *vm);
	virtual ~WidgetInventory() {}

	void load(int mode);
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
