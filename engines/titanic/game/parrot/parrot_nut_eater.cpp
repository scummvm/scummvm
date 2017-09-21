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

#include "titanic/game/parrot/parrot_nut_eater.h"
#include "titanic/core/room_item.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CParrotNutEater, CGameObject)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(ReplaceBowlAndNutsMsg)
	ON_MESSAGE(NutPuzzleMsg)
	ON_MESSAGE(MovieFrameMsg)
END_MESSAGE_MAP()

CParrotNutEater::CParrotNutEater() : CGameObject(), _fieldBC(0),
		_fieldC0(69), _fieldC4(132), _fieldC8(0), _fieldCC(68) {
}

void CParrotNutEater::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);

	CGameObject::save(file, indent);
}

void CParrotNutEater::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();

	CGameObject::load(file);
}

bool CParrotNutEater::MovieEndMsg(CMovieEndMsg *msg) {
	setVisible(false);
	CNutPuzzleMsg nutMsg("NutsGone");
	nutMsg.execute(getRoom(), nullptr, MSGFLAG_SCAN);

	playSound(TRANSLATE("z#47.wav", "z#578.wav"));
	return true;
}

bool CParrotNutEater::ReplaceBowlAndNutsMsg(CReplaceBowlAndNutsMsg *msg) {
	setVisible(false);
	return true;
}

bool CParrotNutEater::NutPuzzleMsg(CNutPuzzleMsg *msg) {
	if (msg->_action == "Jiggle") {
		setVisible(true);
		playMovie(MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		movieEvent(68);
		movieEvent(132);
		playSound(TRANSLATE("z#215.wav", "z#6.wav"));

		CTrueTalkTriggerActionMsg triggerMsg;
		triggerMsg._param1 = triggerMsg._param2 = 0;
		triggerMsg.execute("PerchedParrot");
	}

	return true;
}

bool CParrotNutEater::MovieFrameMsg(CMovieFrameMsg *msg) {
	CProximity prox(Audio::Mixer::kSpeechSoundType);

	switch (msg->_frameNumber) {
	case 68:
		playSound(TRANSLATE("z#214.wav", "z#5.wav"), prox);
		break;
	case 132:
		playSound(TRANSLATE("z#216.wav", "z#7.wav"), prox);
		break;
	default:
		break;
	}

	return true;
}

} // End of namespace Titanic
