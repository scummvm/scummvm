/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tetraedge/game/inventory_object.h"

namespace Tetraedge {

InventoryObject::InventoryObject() {
}

void InventoryObject::load(const Common::String &newName) {
	setSizeType(RELATIVE_TO_PARENT);
	setSize(TeVector3f32(1.0f, 1.0f, 1.0f));
	_gui.load("Inventory/InventoryObject.lua");
	addChild(_gui.layoutChecked("object"));
	setName(newName);
	_gui.spriteLayoutChecked("upLayout")->load(spritePath());
	TeButtonLayout *btn = _gui.buttonLayoutChecked("object");
	btn->onMouseClickValidated().add(this, &InventoryObject::onButtonDown);
	// TODO: btn->setDoubleValidationProtectionEnabled(false)
}

Common::String InventoryObject::spritePath() {
	return Common::Path("Inventory/Objects").join(name()).append(".png").toString();
}

bool InventoryObject::onButtonDown() {
	_selectedSignal.call(*this);
	return false;
}


} // end namespace Tetraedge
