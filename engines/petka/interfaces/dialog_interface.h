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

#ifndef PETKA_DIALOG_INTERFACE_H
#define PETKA_DIALOG_INTERFACE_H

#include "common/str.h"

namespace Petka {

enum DialogState {
	kPlaying = 1,
	kMenu = 2,
	kIdle = 3
};

class Sound;
class QMessageObject;
class BigDialogue;
class QSystem;
struct QReaction;

class DialogInterface {
public:
	DialogInterface();
	~DialogInterface();

	void start(uint id, QMessageObject *sender);
	void next(int choice);

	void startUserMsg(uint16 arg);
	void endUserMsg();

	bool isActive();

	Sound *findSound();

	void setSender(QMessageObject *sender);
	void setReaction(QReaction *reaction);

	void fixCursor();

private:
	void onPlayOpcode(int prevTalkerId);
	void onMenuOpcode();
	void onEndOpcode();
	void onUserMsgOpcode();

	void removeSound();

	void sendMsg(uint16 opcode);
	void setPhrase(const Common::U32String *text);
	void playSound(const Common::String &name);

	void initCursor();
	void restoreCursor();

private:
	BigDialogue *_dialog;
	QSystem *_qsys;
	bool _isUserMsg;
	bool _afterUserMsg;
	bool _firstTime;
	int _id;
	DialogState _state;
	Common::String _soundName;
	QMessageObject *_talker;
	QMessageObject *_sender;
	QReaction *_reaction;
	int16 _savedCursorActType;
	int16 _savedCursorId;
	bool _wasCursorShown;
	bool _wasCursorAnim;
};

} // End of namespace Petka

#endif
