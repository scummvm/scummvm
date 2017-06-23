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

#include "titanic/game/maitred/maitred_prod_receptor.h"
#include "titanic/npcs/maitre_d.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMaitreDProdReceptor, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MouseMoveMsg)
	ON_MESSAGE(ProdMaitreDMsg)
	ON_MESSAGE(DisableMaitreDProdReceptor)
END_MESSAGE_MAP()

void CMaitreDProdReceptor::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeNumberLine(_counter, indent);
	file->writeNumberLine(_fieldC4, indent);

	CGameObject::save(file, indent);
}

void CMaitreDProdReceptor::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_counter = file->readNumber();
	_fieldC4 = file->readNumber();

	CGameObject::load(file);
}

bool CMaitreDProdReceptor::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_fieldBC == 2 && static_cast<CGameObject *>(getParent())->hasActiveMovie()) {
		return false;
	} else {
		CProdMaitreDMsg prodMsg(126);
		prodMsg.execute(this);
		return true;
	}
}

bool CMaitreDProdReceptor::MouseMoveMsg(CMouseMoveMsg *msg) {
	if (_fieldBC == 2 && static_cast<CGameObject *>(getParent())->hasActiveMovie())
		return false;
	else if (++_counter < 20)
		return true;

	_counter = 0;
	CProdMaitreDMsg prodMsg(126);

	if (isEquals("Stick"))
		prodMsg._value = 121;
	else if (isEquals("Hammer"))
		prodMsg._value = 122;
	else if (isEquals("Lemon"))
		prodMsg._value = 123;
	else if (isEquals("Chicken"))
		prodMsg._value = 124;
	else if (isEquals("Perch"))
		prodMsg._value = 125;

	CMaitreD *maitreD = dynamic_cast<CMaitreD *>(findRoomObject("MaitreD"));
	if (maitreD && maitreD->_speechCounter == 0)
		prodMsg.execute(this);

	return true;
}

bool CMaitreDProdReceptor::ProdMaitreDMsg(CProdMaitreDMsg *msg) {
	if (_fieldC4) {
		CMaitreD *maitreD = static_cast<CMaitreD *>(findRoomObject("MaitreD"));
		if (maitreD->_speechCounter == 0) {
			CViewItem *view = findView();
			startTalking(maitreD, msg->_value, view);

			switch (_fieldBC) {
			case 1:
				startTalking(maitreD, 128, view);
				break;
			case 2:
				startTalking(maitreD, 129, view);
				break;
			case 3:
				startTalking(maitreD, 127, view);
				break;
			default:
				startTalking(maitreD, 130, view);
				break;
			}
		}
	}

	return true;
}

bool CMaitreDProdReceptor::DisableMaitreDProdReceptor(CDisableMaitreDProdReceptor *msg) {
	_fieldC4 = 0;
	return true;
}

} // End of namespace Titanic
