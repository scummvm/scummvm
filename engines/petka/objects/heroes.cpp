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

#include "petka/petka.h"
#include "petka/q_manager.h"
#include "petka/q_system.h"
#include "petka/flc.h"
#include "petka/objects/heroes.h"

namespace Petka {

QObjectPetka::QObjectPetka() {
	_field7C = 1;
	_reaction = nullptr;
	_sender = nullptr;
	_isPetka = true;
	_isWalking = false;
	_x = 574;
	_y = 44;
	_z = 200;
	_surfId  = -5;
	_surfH = 0;
	_surfW = 0;
}

void QObjectPetka::processMessage(const QMessage &arg) {
	QMessage msg = arg;
	if (msg.opcode == kImage) {
		msg.opcode = kSet;
		_imageId = msg.arg1;
	}
	if (msg.opcode == kSaid || msg.opcode == kStand) {
		msg.opcode = kSet;
		msg.arg1 = _imageId + 1;
		msg.arg2 = 1;
	}
	if (msg.opcode == kSet || msg.opcode == kPlay) {
		_field7C = msg.arg2 == _imageId || msg.opcode == kPlay;
	}
	if (msg.opcode != kWalk) {
		if (msg.opcode == kWalked && _heroReaction) {
			processSavedReaction(&_heroReaction, _sender);
		}
		QMessageObject::processMessage(msg);
		if (msg.opcode == kSet || msg.opcode == kPlay) {
			initSurface();
			if (!g_vm->getQSystem()->_isIniting) {
				setPos(_x_, _y_);
			}
		}
	}
}

void QObjectPetka::initSurface() {
	QManager *resMgr = g_vm->resMgr();
	FlicDecoder *flc = resMgr->loadFlic(_resourceId);
	resMgr->removeResource(_surfId);
	resMgr->findOrCreateSurface(_surfId, flc->getWidth(), flc->getHeight());
	_surfW = flc->getWidth() * _field98;
	_surfH = flc->getHeight() * _field98;
}

QObjectChapayev::QObjectChapayev() {
	_x = 477;
	_y = 350;
	_surfId = -6;
	_isPetka = false;
}

}
