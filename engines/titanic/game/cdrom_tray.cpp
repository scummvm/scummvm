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

#include "titanic/core/room_item.h"
#include "titanic/game/cdrom_tray.h"
#include "titanic/messages/messages.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CCDROMTray, CGameObject)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(StatusChangeMsg)
END_MESSAGE_MAP()


CCDROMTray::CCDROMTray() : CGameObject(), _isOpened(false) {
}

void CCDROMTray::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_isOpened, indent);
	file->writeQuotedLine(_insertedCD, indent);

	CGameObject::save(file, indent);
}

void CCDROMTray::load(SimpleFile *file) {
	file->readNumber();
	_isOpened = file->readNumber();
	_insertedCD = file->readString();

	CGameObject::load(file);
}

bool CCDROMTray::ActMsg(CActMsg *msg) {
	if (msg->_action == "ClickedOn") {
		if (_isOpened) {
			// Closing the tray
			if (_insertedCD == "None") {
				// No CD in tray
				playMovie(55, 65, 0);
				playSound(TRANSLATE("a#35.wav", "a#30.wav"), 50, 0, 0);
				_isOpened = false;
			} else {
				// Ejecting tray with CD
				CTreeItem *cdrom = getRoom()->findByName(_insertedCD);
				if (cdrom) {
					CActMsg actMsg("Ejected");
					actMsg.execute(cdrom);
				}

				_insertedCD = "None";
				loadFrame(52);
			}
		} else if (_insertedCD == "None") {
			// Opening tray with no CD
			playMovie(44, 54, 0);
			playSound(TRANSLATE("a#34.wav", "a#29.wav"), 50, 0, 0);
			_isOpened = true;
		} else if (_insertedCD == "newCD1" || _insertedCD == "newCD2") {
			// Opening tray with standard CD
			playMovie(22, 32, 0);
			playSound(TRANSLATE("a#34.wav", "a#29.wav"), 50, 0, 0);
			_isOpened = true;
		} else if (_insertedCD == "newSTCD") {
			// Opening tray with Starship Titanic CD
			playMovie(0, 10, 0);
			playSound(TRANSLATE("a#34.wav", "a#29.wav"), 50, 0, 0);
			_isOpened = true;
		}
	} else if (_isOpened) {
		if (msg->_action == "newCD1" || msg->_action == "newCD2") {
			// Standard CD dropped on CDROM Tray
			playMovie(33, 43, MOVIE_NOTIFY_OBJECT);
			playSound(TRANSLATE("a#35.wav", "a#30.wav"), 50, 0, 0);
		} else if (msg->_action == "newSTCD") {
			// Starship Titanic CD dropped on CDROM Tray
			disableMouse();
			playMovie(11, 21, MOVIE_NOTIFY_OBJECT);
			playSound(TRANSLATE("a#35.wav", "a#30.wav"), 50, 0, 0);
		} else {
			return true;
		}

		_insertedCD = msg->_action;
		_isOpened = false;
	}

	return true;
}

bool CCDROMTray::MovieEndMsg(CMovieEndMsg *msg) {
	CTreeItem *screen = getRoom()->findByName("newScreen");

	if (screen) {
		CActMsg actMsg(_insertedCD);
		actMsg.execute(screen);
	}

	return true;
}

bool CCDROMTray::StatusChangeMsg(CStatusChangeMsg *msg) {
	msg->_success = _isOpened;
	return true;
}

} // End of namespace Titanic
