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

#include "titanic/game/missiveomat.h"
#include "titanic/core/room_item.h"
#include "titanic/titanic.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMissiveOMat, CGameObject)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(KeyCharMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(MissiveOMatActionMsg)
	ON_MESSAGE(LeaveViewMsg)
END_MESSAGE_MAP()

CMissiveOMat::CMissiveOMat() : CGameObject(), _mode(MMODE_USERNAME),
		_totalMessages(0), _messageNum(0), _account(NO_ACCOUNT) {
	// Load data for the messages, their from and to names
	loadArray(_welcomeMessages, "TEXT/MISSIVEOMAT/WELCOME", 3);
	loadArray(_messages, "TEXT/MISSIVEOMAT/MESSAGES", 58);
	loadArray(_from, "TEXT/MISSIVEOMAT/FROM", 58);
	loadArray(_to, "TEXT/MISSIVEOMAT/TO", 58);
}

void CMissiveOMat::loadArray(CString *arr, const CString &resName, int count) {
	Common::SeekableReadStream *s = g_vm->_filesManager->getResource(resName);
	for (int idx = 0; idx < count; ++idx)
		arr[idx] = readStringFromStream(s);
	delete s;
}

void CMissiveOMat::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_mode, indent);
	file->writeNumberLine(_totalMessages, indent);
	file->writeNumberLine(_messageNum, indent);
	file->writeQuotedLine(_username, indent);
	file->writeQuotedLine(_password, indent);
	file->writeNumberLine(_account, indent);

	CGameObject::save(file, indent);
}

void CMissiveOMat::load(SimpleFile *file) {
	file->readNumber();
	_mode = (MissiveOMatMode)file->readNumber();
	_totalMessages = file->readNumber();
	_messageNum = file->readNumber();
	_username = file->readString();
	_password = file->readString();
	_account = (MissiveOMatAccount)file->readNumber();

	CGameObject::load(file);
}

bool CMissiveOMat::EnterViewMsg(CEnterViewMsg *msg) {
	CMissiveOMatActionMsg actionMsg(9);
	actionMsg.execute(this);
	return true;
}

bool CMissiveOMat::KeyCharMsg(CKeyCharMsg *msg) {
	CTreeItem *loginControl = findRoom()->findByName("MissiveOMat Login Control");
	CTreeItem *welcome = findRoom()->findByName("MissiveOMat Welcome");
	CTreeItem *scrollUp = findRoom()->findByName("MissiveOMat ScrollUp Button");
	CEditControlMsg editMsg;

	switch (_mode) {
	case MMODE_USERNAME:
		playSound("z#228.wav");
		editMsg._mode = 6;
		editMsg._param = msg->_key;
		editMsg.execute(loginControl);

		if (editMsg._param == 1000) {
			editMsg._mode = 3;
			editMsg.execute(loginControl);

			_username = editMsg._text;
			_username.toLowercase();

			if (!_username.empty()) {
				loadFrame(2);
				_mode = MMODE_PASSWORD;

				editMsg._mode = 1;
				editMsg.execute(loginControl);
				editMsg._mode = 10;
				editMsg._param = 24;
				editMsg.execute(loginControl);
			}
		}
		break;

	case MMODE_PASSWORD:
		playSound("z#228.wav");
		editMsg._mode = 6;
		editMsg._param = msg->_key;
		editMsg.execute(loginControl);

		if (editMsg._param == 1000) {
			_password = editMsg._text;
			_password.toLowercase();

			if (_username == "droot scraliontis") {
				_username = "scraliontis";
			} else if (_username == "antar brobostigon") {
				_username = "brobostigon";
			} else if (_username == "colin") {
				_username = "leovinus";
			}

			bool validFlag = false;
			if (_username == "leovinus" && _password == "other") {
				validFlag = true;
				_account = LEOVINUS;
			} else if (_username == "scraliontis" && _password == "this") {
				validFlag = true;
				_account = SCRALIONTIS;
			} else if (_username == "brobostigon" && _password == "that") {
				validFlag = true;
				_account = BROBOSTIGON;
			}

			if (validFlag) {
				_mode = MMODE_4;
				loadFrame(4);
				editMsg._mode = 1;
				editMsg.execute(loginControl);

				getTextCursor()->hide();
				editMsg._mode = 13;
				editMsg.execute(loginControl);

				editMsg._mode = 12;
				editMsg.execute(welcome);

				editMsg._mode = 2;
				editMsg._text = _welcomeMessages[_account];
				editMsg.execute(welcome);

				editMsg._mode = 12;
				editMsg._text = "MissiveOMat OK Button";
				editMsg.execute(welcome);
				editMsg.execute(scrollUp);
			} else {
				_mode = MMODE_3;
				loadFrame(3);
				addTimer(1500);

				editMsg._mode = 1;
				editMsg.execute(loginControl);

				getTextCursor()->hide();
			}
		}
		break;

	default:
		break;
	}

	return true;
}

bool CMissiveOMat::TimerMsg(CTimerMsg *msg) {
	if (_mode == 3) {
		CTreeItem *loginControl = findRoom()->findByName("MissiveOMat Login Control");
		CEditControlMsg editMsg;
		editMsg._mode = 10;
		editMsg._param = 8;
		editMsg.execute(loginControl);
	}

	return true;
}

bool CMissiveOMat::MissiveOMatActionMsg(CMissiveOMatActionMsg *msg) {
	CTreeItem *welcome = findByName("MissiveOMat Welcome");

	switch (msg->_action) {
	case MESSAGE_SHOW: {
		CTreeItem *btnOk = findRoom()->findByName("MissiveOMat OK Button");
		CTreeItem *btnNext = findRoom()->findByName("MissiveOMat Next Button");
		CTreeItem *btnPrev = findRoom()->findByName("MissiveOMat Prev Button");
		CTreeItem *btnLogout = findRoom()->findByName("MissiveOMat Logout Button");

		_mode = MMODE_5;
		CVisibleMsg visibleMsg;
		visibleMsg._visible = false;
		visibleMsg.execute(btnOk);
		visibleMsg._visible = true;
		visibleMsg.execute(btnNext);
		visibleMsg.execute(btnPrev);
		visibleMsg.execute(btnLogout);

		_messageNum = 0;
		_totalMessages = 0;
		CString *strP = &_messages[_account * 19];
		for (_totalMessages = 0; !strP->empty(); ++strP, ++_totalMessages)
			;

		CMissiveOMatActionMsg actionMsg;
		actionMsg._action = REDRAW_MESSAGE;
		actionMsg.execute(this);
		break;
	}

	case NEXT_MESSAGE:
		if (_messageNum < (_totalMessages - 1)) {
			++_messageNum;
			CMissiveOMatActionMsg actionMsg;
			actionMsg._action = REDRAW_MESSAGE;
			actionMsg.execute(this);
		}
		break;

	case PRIOR_MESSAGE:
		if (_messageNum > 0) {
			--_messageNum;
			CMissiveOMatActionMsg actionMsg;
			actionMsg._action = REDRAW_MESSAGE;
			actionMsg.execute(this);
		}
		break;

	case MESSAGE_5: {
		CMissiveOMatActionMsg actionMsg;
		actionMsg._action = MESSAGE_9;
		actionMsg.execute(this);
		break;
	}

	case MESSAGE_DOWN:
		if (welcome)
			scrollTextDown();
		break;

	case MESSAGE_UP:
		if (welcome)
			scrollTextUp();
		break;

	case REDRAW_MESSAGE:
		if (welcome) {
			CString str = CString::format(
				"Missive %d of %d.\nFrom: %s\nTo: %s\n\n%s\n",
				_messageNum + 1, _totalMessages, _from[_messageNum].c_str(),
				_to[_messageNum].c_str(), _messages[_messageNum].c_str());

			setText(str);
		}
		break;

	case MESSAGE_9: {
		loadFrame(1);
		_mode = MMODE_USERNAME;
		_account = NO_ACCOUNT;

		static const char *const WIDGETS[7] = {
			"MissiveOMat Login Control", "MissiveOMat OK Button",
			"MissiveOMat Next Button", "MissiveOMat Prev Button",
			"MissiveOMat Logout Button", "MissiveOMat ScrollDown Button",
			"MissiveOMat ScrollUp Button"
		};
		CEditControlMsg editMsg;

		for (int idx = 0; idx < 7; ++idx) {
			editMsg._mode = 0;
			editMsg._param = 12;
			editMsg.execute(WIDGETS[idx]);
			editMsg._mode = 1;
			editMsg.execute(WIDGETS[idx]);
			editMsg._mode = 13;
			editMsg.execute(WIDGETS[idx]);
		}

		editMsg._mode = 12;
		editMsg.execute("MissiveOMat Login Control");
		editMsg._mode = 10;
		editMsg._param = 8;
		editMsg.execute("MissiveOMat Login Control");
		editMsg._mode = 8;
		editMsg.execute("MissiveOMat Login Control");

		_username.clear();
		_password.clear();
		break;
	}

	default:
		break;
	}

	return true;
}

bool CMissiveOMat::LeaveViewMsg(CLeaveViewMsg *msg) {
	CEditControlMsg editMsg;
	editMsg._mode = 9;
	editMsg.execute("MissiveOMat Login Control");
	petShowCursor();

	return true;
}

} // End of namespace Titanic
