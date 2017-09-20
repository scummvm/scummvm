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

#include "titanic/game/head_smash_lever.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CHeadSmashLever, CBackground)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(FrameMsg)
	ON_MESSAGE(LoadSuccessMsg)
END_MESSAGE_MAP()

CHeadSmashLever::CHeadSmashLever() : CBackground(),
		_enabled(false), _fieldE4(false), _ticks(0) {}

void CHeadSmashLever::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_enabled, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeNumberLine(_ticks, indent);

	CBackground::save(file, indent);
}

void CHeadSmashLever::load(SimpleFile *file) {
	file->readNumber();
	_enabled = file->readNumber();
	_fieldE4 = file->readNumber();
	_ticks = file->readNumber();

	CBackground::load(file);
}

bool CHeadSmashLever::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_enabled) {
		playMovie(0, 14, 0);
		playSound(TRANSLATE("z#54.wav", "z#585.wav"));
		int soundHandle = playSound(TRANSLATE("z#45.wav", "z#576.wav"));
		queueSound(TRANSLATE("z#49.wav", "z#580.wav"), soundHandle);
		_ticks = getTicksCount();
		_fieldE4 = true;
	} else {
		playMovie(0);
		playSound(TRANSLATE("z#56.wav", "z#587.wav"));
	}

	return true;
}

bool CHeadSmashLever::ActMsg(CActMsg *msg) {
	if (msg->_action == "EnableObject")
		_enabled = true;
	else if (msg->_action == "DisableObject")
		_enabled = false;

	return true;
}

bool CHeadSmashLever::FrameMsg(CFrameMsg *msg) {
	if (_fieldE4 && msg->_ticks > (_ticks + 750)) {
		CActMsg actMsg1("CreatorsChamber.Node 1.S");
		actMsg1.execute("MoveToCreators");
		CActMsg actMsg2("PlayToEnd");
		actMsg2.execute("SmashingStatue");

		playSound(TRANSLATE("b#16.wav", "b#100.wav"));
		_fieldE4 = false;
	}

	return true;
}

bool CHeadSmashLever::LoadSuccessMsg(CLoadSuccessMsg *msg) {
	if (_fieldE4)
		_ticks = getTicksCount();

	return true;
}

} // End of namespace Titanic
