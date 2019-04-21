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

#include "titanic/game/cell_point_button.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CCellPointButton, CBackground)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

CCellPointButton::CCellPointButton() : CBackground() {
	_unused1 = 0;
	_unused2 = 0;
	_unused3 = 0;
	_unused4 = 0;
	_regionNum = 0;
	_unused5 = 0;
	_unused6 = 0;
	_unused7 = 0;
	_unused8 = 0;
	_unused9 = 0;
	_unused10 = 1;
}

void CCellPointButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_unused1, indent);
	file->writeNumberLine(_unused2, indent);
	file->writeNumberLine(_unused3, indent);
	file->writeNumberLine(_unused4, indent);
	file->writeNumberLine(_regionNum, indent);
	file->writeNumberLine(_unused5, indent);
	file->writeNumberLine(_unused6, indent);
	file->writeNumberLine(_unused7, indent);
	file->writeNumberLine(_unused8, indent);
	file->writeNumberLine(_unused9, indent);
	file->writeNumberLine(_unused10, indent);
	file->writeQuotedLine(_npcName, indent);
	file->writeNumberLine(_dialNum, indent);

	CBackground::save(file, indent);
}

void CCellPointButton::load(SimpleFile *file) {
	file->readNumber();
	_unused1 = file->readNumber();
	_unused2 = file->readNumber();
	_unused3 = file->readNumber();
	_unused4 = file->readNumber();
	_regionNum = file->readNumber();
	_unused5 = file->readNumber();
	_unused6 = file->readNumber();
	_unused7 = file->readNumber();
	_unused8 = file->readNumber();
	_unused9 = file->readNumber();
	_unused10 = file->readNumber();
	_npcName = file->readString();
	_dialNum = file->readNumber();

	CBackground::load(file);
}

bool CCellPointButton::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (getRandomNumber(2) == 0) {
		CParrotSpeakMsg speakMsg("Cellpoints", _npcName);
		speakMsg.execute("PerchedParrot");
	}

	playMovie(0);
	_regionNum = _regionNum ? 0 : 1;
	playSound(TRANSLATE("z#425.wav", "z#170.wav"));
	talkSetDialRegion(_npcName, _dialNum, _regionNum);

	return true;
}

bool CCellPointButton::EnterViewMsg(CEnterViewMsg *msg) {
	_regionNum = talkGetDialRegion(_npcName, _dialNum);
	return true;
}

} // End of namespace Titanic
