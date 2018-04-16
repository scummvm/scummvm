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

#include "titanic/gfx/move_object_button.h"
#include "titanic/core/project_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMoveObjectButton, CSTButton)
	ON_MESSAGE(MouseButtonUpMsg)
END_MESSAGE_MAP()

CMoveObjectButton::CMoveObjectButton() : CSTButton(), _field11C(1) {
}

void CMoveObjectButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writePoint(_pos1, indent);
	file->writeNumberLine(_field11C, indent);

	CSTButton::save(file, indent);
}

void CMoveObjectButton::load(SimpleFile *file) {
	file->readNumber();
	_pos1 = file->readPoint();
	_field11C = file->readNumber();

	CSTButton::load(file);
}

bool CMoveObjectButton::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	CGameObject *obj = dynamic_cast<CGameObject *>(getRoot()->findByName(_actionTarget));
	if (obj) {
		obj->petAddToInventory();
		obj->setVisible(_field11C);
	}

	return true;
}

} // End of namespace Titanic
