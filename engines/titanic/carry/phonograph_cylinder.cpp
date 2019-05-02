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

#include "titanic/carry/phonograph_cylinder.h"
#include "titanic/game/phonograph.h"
#include "titanic/sound/music_room.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPhonographCylinder, CCarry)
	ON_MESSAGE(UseWithOtherMsg)
	ON_MESSAGE(QueryCylinderMsg)
	ON_MESSAGE(RecordOntoCylinderMsg)
	ON_MESSAGE(SetMusicControlsMsg)
	ON_MESSAGE(ErasePhonographCylinderMsg)
END_MESSAGE_MAP()

CPhonographCylinder::CPhonographCylinder() : CCarry(),
		_bellsMuteControl(false), _bellsPitchControl(0),
		_bellsSpeedControl(0), _bellsDirectionControl(false),
		_bellsInversionControl(false), _snakeMuteControl(false),
		_snakeSpeedControl(0), _snakePitchControl(0),
		_snakeInversionControl(false), _snakeDirectionControl(false),
		_pianoMuteControl(false), _pianoSpeedControl(0),
		_pianoPitchControl(0), _pianoInversionControl(false),
		_pianoDirectionControl(false), _bassMuteControl(false),
		_bassSpeedControl(0), _bassPitchControl(0),
		_bassInversionControl(false) {
}

void CPhonographCylinder::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_itemName, indent);
	file->writeNumberLine(_bellsMuteControl, indent);
	file->writeNumberLine(_bellsPitchControl, indent);
	file->writeNumberLine(_bellsSpeedControl, indent);
	file->writeNumberLine(_bellsDirectionControl, indent);
	file->writeNumberLine(_bellsInversionControl, indent);
	file->writeNumberLine(_snakeMuteControl, indent);
	file->writeNumberLine(_snakeSpeedControl, indent);
	file->writeNumberLine(_snakePitchControl, indent);
	file->writeNumberLine(_snakeInversionControl, indent);
	file->writeNumberLine(_snakeDirectionControl, indent);
	file->writeNumberLine(_pianoMuteControl, indent);
	file->writeNumberLine(_pianoSpeedControl, indent);
	file->writeNumberLine(_pianoPitchControl, indent);
	file->writeNumberLine(_pianoInversionControl, indent);
	file->writeNumberLine(_pianoDirectionControl, indent);
	file->writeNumberLine(_bassMuteControl, indent);
	file->writeNumberLine(_bassSpeedControl, indent);
	file->writeNumberLine(_bassPitchControl, indent);
	file->writeNumberLine(_bassInversionControl, indent);
	file->writeNumberLine(_bassDirectionControl, indent);

	CCarry::save(file, indent);
}

void CPhonographCylinder::load(SimpleFile *file) {
	file->readNumber();
	_itemName = file->readString();
	_bellsMuteControl = file->readNumber();
	_bellsPitchControl = file->readNumber();
	_bellsSpeedControl = file->readNumber();
	_bellsDirectionControl = file->readNumber();
	_bellsInversionControl = file->readNumber();
	_snakeMuteControl = file->readNumber();
	_snakeSpeedControl = file->readNumber();
	_snakePitchControl = file->readNumber();
	_snakeInversionControl = file->readNumber();
	_snakeDirectionControl = file->readNumber();
	_pianoMuteControl = file->readNumber();
	_pianoSpeedControl = file->readNumber();
	_pianoPitchControl = file->readNumber();
	_pianoInversionControl = file->readNumber();
	_pianoDirectionControl = file->readNumber();
	_bassMuteControl = file->readNumber();
	_bassSpeedControl = file->readNumber();
	_bassPitchControl = file->readNumber();
	_bassInversionControl = file->readNumber();
	_bassDirectionControl = file->readNumber();

	CCarry::load(file);
}

bool CPhonographCylinder::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	CPhonograph *phonograph = dynamic_cast<CPhonograph *>(msg->_other);
	if (phonograph) {
		// Below is redundant, since original never actually executes message
		CSetVarMsg varMsg("m_RecordStatus", 1);
		return true;
	} else {
		return CCarry::UseWithOtherMsg(msg);
	}
}

bool CPhonographCylinder::QueryCylinderMsg(CQueryCylinderMsg *msg) {
	msg->_name = _itemName;
	return true;
}

bool CPhonographCylinder::RecordOntoCylinderMsg(CRecordOntoCylinderMsg *msg) {
	_itemName = "STMusic";

	CQueryMusicControlSettingMsg queryMsg;
	queryMsg.execute("Bells Mute Control");
	_bellsMuteControl = queryMsg._value;
	queryMsg.execute("Bells Pitch Control");
	_bellsPitchControl = queryMsg._value;
	queryMsg.execute("Bells Speed Control");
	_bellsSpeedControl = queryMsg._value;
	queryMsg.execute("Bells Direction Control");
	_bellsDirectionControl = queryMsg._value;
	queryMsg.execute("Bells Inversion Control");
	_bellsInversionControl = queryMsg._value;
	queryMsg.execute("Snake Mute Control");
	_snakeMuteControl = queryMsg._value;
	queryMsg.execute("Snake Speed Control");
	_snakeSpeedControl = queryMsg._value;
	queryMsg.execute("Snake Pitch Control");
	_snakePitchControl = queryMsg._value;
	queryMsg.execute("Snake Inversion Control");
	_snakeInversionControl = queryMsg._value;
	queryMsg.execute("Snake Direction Control");
	_snakeDirectionControl = queryMsg._value;
	queryMsg.execute("Piano Mute Control");
	_pianoMuteControl = queryMsg._value;
	queryMsg.execute("Piano Speed Control");
	_pianoSpeedControl = queryMsg._value;
	queryMsg.execute("Piano Pitch Control");
	_pianoPitchControl = queryMsg._value;
	queryMsg.execute("Piano Inversion Control");
	_pianoInversionControl = queryMsg._value;
	queryMsg.execute("Piano Direction Control");
	_pianoDirectionControl = queryMsg._value;
	queryMsg.execute("Bass Mute Control");
	_bassMuteControl = queryMsg._value;
	queryMsg.execute("Bass Speed Control");
	_bassSpeedControl = queryMsg._value;
	queryMsg.execute("Bass Pitch Control");
	_bassPitchControl = queryMsg._value;
	queryMsg.execute("Bass Inversion Control");
	_bassInversionControl = queryMsg._value;
	queryMsg.execute("Bass Direction Control");
	_bassDirectionControl = queryMsg._value;

	return true;
}

bool CPhonographCylinder::SetMusicControlsMsg(CSetMusicControlsMsg *msg) {
	if (!_itemName.hasPrefix("STMusic"))
		return true;

	CMusicRoom *musicRoom = getMusicRoom();
	musicRoom->setMuteControl(BELLS, _bellsMuteControl);
	musicRoom->setPitchControl(BELLS, _bellsPitchControl);
	musicRoom->setSpeedControl(BELLS, _bellsSpeedControl);
	musicRoom->setInversionControl(BELLS, _bellsInversionControl);
	musicRoom->setDirectionControl(BELLS, _bellsDirectionControl);

	musicRoom->setMuteControl(SNAKE, _snakeMuteControl);
	musicRoom->setPitchControl(SNAKE, _snakePitchControl);
	musicRoom->setSpeedControl(SNAKE, _snakeSpeedControl);
	musicRoom->setInversionControl(SNAKE, _snakeInversionControl);
	musicRoom->setDirectionControl(SNAKE, _snakeDirectionControl);

	musicRoom->setMuteControl(PIANO, _pianoMuteControl);
	musicRoom->setPitchControl(PIANO, _pianoPitchControl);
	musicRoom->setSpeedControl(PIANO, _pianoSpeedControl);
	musicRoom->setInversionControl(PIANO, _pianoInversionControl);
	musicRoom->setDirectionControl(PIANO, _pianoDirectionControl);

	musicRoom->setMuteControl(BASS, _bassMuteControl);
	musicRoom->setPitchControl(BASS, _bassPitchControl);
	musicRoom->setSpeedControl(BASS, _bassSpeedControl);
	musicRoom->setInversionControl(BASS, _bassInversionControl);
	musicRoom->setDirectionControl(BASS, _bassDirectionControl);

	return true;
}

bool CPhonographCylinder::ErasePhonographCylinderMsg(CErasePhonographCylinderMsg *msg) {
	_itemName.clear();
	return true;
}

} // End of namespace Titanic
