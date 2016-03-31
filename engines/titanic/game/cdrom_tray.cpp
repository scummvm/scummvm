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

#include "titanic/game/cdrom_tray.h"

namespace Titanic {

CCDROMTray::CCDROMTray() : CGameObject(), _state(0) {
}

void CCDROMTray::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_state, indent);
	file->writeQuotedLine(_string1, indent);

	CGameObject::save(file, indent);
}

void CCDROMTray::load(SimpleFile *file) {
	file->readNumber();
	_state = file->readNumber();
	_string1 = file->readString();

	CGameObject::load(file);
}

bool CCDROMTray::handleMessage(CActMsg &msg) {
	// TODO
	return true;
}

bool CCDROMTray::handleMessage(CMovieEndMsg &msg) {
	// TODO
	return true;
}

bool CCDROMTray::handleMessage(CStatusChangeMsg &msg) {
	// TODO
	return true;
}

} // End of namespace Titanic
