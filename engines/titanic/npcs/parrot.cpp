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

#include "titanic/npcs/parrot.h"
#include "titanic/core/project_item.h"
#include "titanic/carry/carry.h"
#include "titanic/titanic.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CParrot, CTrueTalkNPC)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(TrueTalkTriggerActionMsg)
	ON_MESSAGE(MouseDragStartMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(ParrotSpeakMsg)
	ON_MESSAGE(NPCPlayTalkingAnimationMsg)
	ON_MESSAGE(NPCPlayIdleAnimationMsg)
	ON_MESSAGE(FrameMsg)
	ON_MESSAGE(MovieFrameMsg)
	ON_MESSAGE(PutParrotBackMsg)
	ON_MESSAGE(PreEnterViewMsg)
	ON_MESSAGE(PanningAwayFromParrotMsg)
	ON_MESSAGE(LeaveRoomMsg)
END_MESSAGE_MAP()

int CParrot::_v1;
int CParrot::_v2;
int CParrot::_v3;
int CParrot::_v4;
int CParrot::_v5;

CParrot::CParrot() : CTrueTalkNPC() {
	_field108 = 0;
	_string2 = "CarryParrot";
	_field118 = 1;
	_field11C = 25;
	_field120 = 0;
	_field124 = 73;
	_field128 = 58;
	_field12C = 0;
	_field130 = 0;
	_field134 = nullptr;
	_field138 = 851;
	_field13C = 851;
	_field140 = 265;
	_field144 = 274;
	_field148 = 726;
	_field14C = 730;
	_field150 = 510;
	_field154 = 570;
	_field158 = 569;
	_field15C = 689;
	_field160 = 690;
	_field164 = 725;
	_field168 = 375;
	_field16C = 508;
	_field170 = 363;
	_field174 = 375;
	_field178 = 303;
	_field17C = 313;
	_field180 = 279;
	_field184 = 302;
	_field188 = 260;
	_field18C = 264;
	_field190 = 315;
	_field194 = 327;
	_field198 = 330;
	_field19C = 360;
	_field1A0 = 175;
	_field1A4 = 259;
	_field1A8 = 175;
	_field1AC = 175;
	_field1B0 = 162;
	_field1B4 = 175;
	_field1B8 = 150;
	_field1BC = 162;
	_field1C0 = 135;
	_field1C4 = 150;
	_field1C8 = 95;
	_field1CC = 135;
	_field1D0 = 76;
	_field1D4 = 95;
	_field1D8 = 55;
	_field1DC = 76;
	_field1E0 = 30;
	_field1E4 = 55;
	_field1E8 = 0;
	_field1EC = 30;

	_assetName = "z454.dlg";
	_assetNumber = 0x13880;
}

void CParrot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_assetNumber, indent);
	
	file->writeQuotedLine(_assetName, indent);
	file->writeNumberLine(_field108, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_v2, indent);
	file->writeNumberLine(_v3, indent);
	
	file->writeQuotedLine(_string2, indent);
	file->writeNumberLine(_field118, indent);
	file->writeNumberLine(_field11C, indent);
	file->writeNumberLine(_field120, indent);
	file->writeNumberLine(_field124, indent);
	file->writeNumberLine(_field128, indent);
	file->writeNumberLine(_field12C, indent);
	file->writeNumberLine(_field130, indent);
	file->writeNumberLine(_v4, indent);
	file->writeNumberLine(_v5, indent);

	CTrueTalkNPC::save(file, indent);
}

void CParrot::load(SimpleFile *file) {
	file->readNumber();
	_assetNumber = file->readNumber();

	_assetName = file->readString();
	_field108 = file->readNumber();
	_v1 = file->readNumber();
	_v2 = file->readNumber();
	_v3 = file->readNumber();

	_string2 = file->readString();
	_field118 = file->readNumber();
	_field11C = file->readNumber();
	_field120 = file->readNumber();
	_field124 = file->readNumber();
	_field128 = file->readNumber();
	_field12C = file->readNumber();
	_field130 = file->readNumber();
	_v4 = file->readNumber();
	_v5 = file->readNumber();

	CTrueTalkNPC::load(file);
}

bool CParrot::ActMsg(CActMsg *msg) {
	if (msg->_action == "PistaccioEaten") {
		CActMsg actMsg("NutsEaten");
		actMsg.execute("Ear2");
	} else if (msg->_action == "Chicken") {
		// Nothing to do
	} else if (msg->_action == "CarryParrotLeftView") {
		if (!_v2) {
			_v1 = 0;
			CStatusChangeMsg statusMsg;
			statusMsg._newStatus = 1;
			statusMsg.execute("PerchCoreHolder");
		}
	} else if (msg->_action == "StartChickenDrag") {
		if (!_v4) {
			stopMovie();
			startTalking(this, 280275, findView());
			_field12C = 0;
		}
	} else if (msg->_action == "EnteringFromTOW" &&
			(_v4 == 0 || _v4 == 2)) {
		if (_v2) {
			_v2 = 2;
		} else {
			setVisible(true);
			CTreeItem *cageBar = getRoot()->findByName("CageBar");
			detach();
			attach(cageBar);

			_v4 = 0;
			CActMsg actMsg1("OpenNow");
			actMsg1.execute("ParrotCage");
			CActMsg actMsg2("GainParrot");
			actMsg2.execute("ParrotLobbyController");
		}
	}

	return true;
}

bool CParrot::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (!(_npcFlags & NPCFLAG_2000000) && _field100 <= 0) {
		CTrueTalkTriggerActionMsg triggerMsg(280250, 280250, 1);
		triggerMsg.execute(this);
	}

	return true;
}

bool CParrot::MovieEndMsg(CMovieEndMsg *msg) {
	// TODO
	return true;
}

bool CParrot::EnterViewMsg(CEnterViewMsg *msg) {
	static const char *const NAMES[] = {
		"Talking0", "Talking1", "Talking2", "Talking3", "Talking4",
		"Talking5", "Talking6", "Talking7", nullptr
	};

	if (!_v4) {
		setPosition(Point(_field124, _bounds.top));
		_field118 = 1;
		_npcFlags &= ~(NPCFLAG_10000  |  NPCFLAG_20000  |  NPCFLAG_40000  |  NPCFLAG_80000  |  NPCFLAG_100000  |  NPCFLAG_200000  |  NPCFLAG_400000);
		loadFrame(0);
		endTalking(this, true, findView());

		if (_field100 > 0) {
			playRandomClip(NAMES, MOVIE_NOTIFY_OBJECT);
		} else {
			startTalking(this, 280258, findView());
		}

		petSetArea(PET_CONVERSATION);
		_field12C = 0;
		_npcFlags |= NPCFLAG_4;
	}

	return true;
}

bool CParrot::TrueTalkTriggerActionMsg(CTrueTalkTriggerActionMsg *msg) {
	if (_v4) {
		CViewItem *view = msg->_param2 ? findView() : nullptr;
		startTalking(this, msg->_action, view);
	}

	return true;
}

bool CParrot::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (_field118 && !_v4 && checkPoint(msg->_mousePos, false, true)) {
		setVisible(false);
		CRoomItem *room = findRoom();

		moveUnder(room);
		startTalking(this, 280129);
		performAction(true);

		CCarry *item = static_cast<CCarry *>(getRoot()->findByName(_string2));
		if (item) {
			item->_fieldE0 = 1;
			CPassOnDragStartMsg passMsg;
			passMsg._mousePos = msg->_mousePos;
			passMsg.execute(item);
			msg->_dragItem = item;

			CActMsg actMsg("LoseParrot");
			actMsg.execute("ParrotLobbyController");
		}
	}

	return true;
}

bool CParrot::LeaveViewMsg(CLeaveViewMsg *msg) {
	performAction(true);
	_npcFlags &= ~NPCFLAG_4;

	return true;
}

bool CParrot::ParrotSpeakMsg(CParrotSpeakMsg *msg) {
	// TODO
	return true;
}

bool CParrot::NPCPlayTalkingAnimationMsg(CNPCPlayTalkingAnimationMsg *msg) {
	const char *const NAMES[] = {
		"Talking0", "Talking1", "Talking2", "Talking3", "Talking4",
		"Talking5", "Talking6", "Talking7", nullptr
	};

	if (!(_npcFlags & (NPCFLAG_10000 | NPCFLAG_20000 | NPCFLAG_40000 | NPCFLAG_80000 | NPCFLAG_100000 | NPCFLAG_200000 | NPCFLAG_400000))
			&& _visible && !_v4) {
		if (!compareViewNameTo("ParrotLobby.Node 1.N"))
			msg->_names = NAMES;
	}

	return true;
}

bool CParrot::NPCPlayIdleAnimationMsg(CNPCPlayIdleAnimationMsg *msg) {
	const char *const NAMES[] = {
		"Idle0", "Idle1", "Peck At Feet", "Peck At Feet Left"
		"Peck At Feet Right", nullptr
	};

	if (!(_npcFlags & (NPCFLAG_10000 | NPCFLAG_20000 | NPCFLAG_40000 | NPCFLAG_80000 | NPCFLAG_100000 | NPCFLAG_200000 | NPCFLAG_400000))
			&& _visible && !_v4 && !compareViewNameTo("ParrotLobby.Node 1.N")) {
		CGameObject *dragItem = getDraggingObject();
		if (!dragItem || dragItem->getName() == "Chicken") {
			if (!_v5 || g_vm->getRandomNumber(3) != 0) {
				if (g_vm->getRandomNumber(1)) {
					startTalking(this, 280267, findView());
				} else {
					msg->_names = NAMES;
				}
			} else {
				int id = -1;
				switch (stateGet38()) {
				case 0:
					id = 280107;
					break;
				case 1:
					id = 280106;
					break;
				case 2:
					id = 280115;
					break;
				case 3:
					id = 280114;
					break;
				case 4:
					id = 280113;
					break;
				case 5:
					id = 280112;
					break;
				case 6:
					id = 280111;
					break;
				case 7:
					id = 280110;
					break;
				case 8:
					id = 280109;
					break;
				case 9:
					id = 280108;
					break;
				case 10:
					id = 280105;
					break;
				case 11:
					id = 280000;
					break;
				default:
					break;
				}

				if (id != -1)
					startTalking(this, id, findView());

				CActMsg actMsg("FlashCore");
				actMsg.execute("PerchCoreHolder");
			}
		}
	}

	return true;
}

bool CParrot::FrameMsg(CFrameMsg *msg) {
	if (compareViewNameTo("ParrotLobby.Node 1.N"))
		return false;
	if (_v4)
		return true;

	Point pt = getMousePos();
	CGameObject *dragObject = getDraggingObject();
	int xp = _bounds.left + _bounds.width() / 2;
//; edi=xp, ebp=dragObject
	if ((_npcFlags & NPCFLAG_400000) && !hasActiveMovie()) {
		_field128 = xp - (_field124 + _bounds.width() / 2);

		if (xp < 64) {
			if (_field134) {
				CActMsg actMsg("PanAwayFromParrot");
				actMsg.execute(_field134);
			}

			_npcFlags &= ~(NPCFLAG_10000 | NPCFLAG_20000 | NPCFLAG_40000 
				| NPCFLAG_80000 | NPCFLAG_100000 | NPCFLAG_200000 | NPCFLAG_400000);
			return true;
		}
	}

	bool chickenFlag = dragObject && dragObject->compareTo("Chicken");

	if (_npcFlags & NPCFLAG_1000000) {
		if (!chickenFlag || pt.x > 70 || pt.y < 90 || pt.y > 280) {
			stopMovie();
			loadFrame(0);
			setPosition(Point(-90, _bounds.top));
		}
	} else {
		if (!chickenFlag)
			return false;
	}

	_field128 = CLIP((int)pt.x, 230, 480);
	if ((_npcFlags & NPCFLAG_10000) || hasActiveMovie())
		return true;

	if (_field128 > 64) {
		_npcFlags |= NPCFLAG_10000 | NPCFLAG_20000;

		// TODO
	}

	return true;
}

bool CParrot::MovieFrameMsg(CMovieFrameMsg *msg) {
	// TODO
	return true;
}

bool CParrot::PutParrotBackMsg(CPutParrotBackMsg *msg) {
	// TODO
	return true;
}

bool CParrot::PreEnterViewMsg(CPreEnterViewMsg *msg) {
	// TODO
	return true;
}

bool CParrot::PanningAwayFromParrotMsg(CPanningAwayFromParrotMsg *msg) {
	// TODO
	return true;
}

bool CParrot::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	// TODO
	return true;
}

} // End of namespace Titanic
