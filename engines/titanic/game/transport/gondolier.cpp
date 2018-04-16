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

#include "titanic/game/transport/gondolier.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CGondolier, CTransport)
	ON_MESSAGE(StatusChangeMsg)
END_MESSAGE_MAP()

int CGondolier::_v1;
int CGondolier::_v2;

void CGondolier::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_v2, indent);
	CTransport::save(file, indent);
}

void CGondolier::load(SimpleFile *file) {
	file->readNumber();
	_v1 = file->readNumber();
	_v2 = file->readNumber();
	CTransport::load(file);
}

bool CGondolier::StatusChangeMsg(CStatusChangeMsg *msg) {
	CShowTextMsg textMsg(GONDOLIERS_FIRST_CLASS_ONLY);
	textMsg.execute("PET");
	return true;
}

} // End of namespace Titanic
