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

#include "titanic/game/credits.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CCredits, CGameObject)
	ON_MESSAGE(SignalObject)
	ON_MESSAGE(TimerMsg)
END_MESSAGE_MAP()

CCredits::CCredits() : CGameObject(), _fieldBC(-1), _fieldC0(1) {
}

void CCredits::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeNumberLine(_fieldC0, indent);

	CGameObject::save(file, indent);
}

void CCredits::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_fieldC0 = file->readNumber();

	CGameObject::load(file);
}

bool CCredits::SignalObject(CSignalObject *msg) {
	petHide();
	disableMouse();
	addTimer(50);
	return true;
}

bool CCredits::TimerMsg(CTimerMsg *msg) {
	stopAmbientSound(true, -1);
	setVisible(true);
	loadSound(TRANSLATE("a#16.wav", "a#11.wav"));
	loadSound(TRANSLATE("a#24.wav", "a#19.wav"));

	if (playCutscene(0, 18)) {
		playAmbientSound(TRANSLATE("a#16.wav", "a#11.wav"), VOL_NORMAL, false, false, 0);
		if (playCutscene(19, 642)) {
			playSound(TRANSLATE("a#24.wav", "a#19.wav"));
			playCutscene(643, 750);
		}
	}

	COpeningCreditsMsg creditsMsg;
	creditsMsg.execute("Service Elevator Entity");
	changeView("EmbLobby.Node 6.S");

	setVisible(false);
	petShow();
	enableMouse();
	stopAmbientSound(true, -1);
	return true;
}

} // End of namespace Titanic
