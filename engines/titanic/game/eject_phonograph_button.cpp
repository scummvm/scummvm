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

#include "titanic/game/eject_phonograph_button.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CEjectPhonographButton, CBackground)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(CylinderHolderReadyMsg)
END_MESSAGE_MAP()

void CEjectPhonographButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_ejected, indent);
	file->writeNumberLine(_readyFlag, indent);
	file->writeQuotedLine(_soundName, indent);
	file->writeQuotedLine(_readySoundName, indent);

	CBackground::save(file, indent);
}

void CEjectPhonographButton::load(SimpleFile *file) {
	file->readNumber();
	_ejected = file->readNumber();
	_readyFlag = file->readNumber();
	_soundName = file->readString();
	_readySoundName = file->readString();

	CBackground::load(file);
}

bool CEjectPhonographButton::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	CQueryPhonographState queryMsg;
	queryMsg.execute(getParent(), nullptr, MSGFLAG_SCAN);

	if (!_ejected && !queryMsg._value) {
		loadFrame(1);
		playSound(_soundName);
		_readyFlag = true;

		CEjectCylinderMsg ejectMsg;
		ejectMsg.execute(getParent(), nullptr, MSGFLAG_SCAN);
		_ejected = true;
	}

	return true;
}

bool CEjectPhonographButton::CylinderHolderReadyMsg(CCylinderHolderReadyMsg *msg) {
	if (_readyFlag) {
		loadFrame(0);
		playSound(_readySoundName);
		_readyFlag = 0;
	}

	_ejected = false;
	return true;
}

} // End of namespace Titanic
