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

#ifndef PETKA_BASE_H
#define PETKA_BASE_H

#include "common/array.h"

namespace Petka {

enum Opcode {
	kZero = 0,
	kUse = 1,
	kSetPos = 2,
	kGoTo = 3,
	kLook = 4,
	kSay = 5,
	kTake = 6,
	kQMUse = 7,
	kVasiliyIvanovich = 8,
	kWalk = 9,
	kTalk = 10,
	kEnd = 11,
	kSetAnimation = 12,
	kForceMove = 13,
	kSet = 14,
	kShow = 15,
	kHide = 16,
	kDialog = 17,
	kZBuffer = 18,
	kTotalInit = 19,
	kAnimate = 20,
	kStatus = 21,
	kAddInv = 22,
	kDelInv = 23,
	kStop = 24,
	kCursor = 25,
	kObjectUse = 26,
	kActive = 27,
	kSaid = 28,
	kSetSeq = 29,
	kEndSeq = 30,
	kCheck = 31,
	kIf = 32,
	kDescription = 33,
	kHalf = 34,
	kWalked = 35,
	kWalkTo = 36,
	kWalkVich = 37,
	kInitBG = 38,
	kUserMsg = 39,
	kSystem = 40,
	kSetZBuffer = 41, // ??? Reserved1
	kContinue = 42,
	kMap = 43,
	kPassive = 44,
	kNoMap = 45,
	kSetInv = 46,
	kBGsFX = 47,
	kMusic = 48,
	kImage = 49,
	kStand = 50,
	kOn = 51,
	kOff = 52,
	kPlay = 53,
	kLeaveBG = 54,
	kShake = 55,
	kSP = 56,
	kRandom = 57,
	kJump = 58,
	kJumpVich = 59,
	kPart = 60,
	kChapter = 61,
	kAvi = 62,
	kToMap = 63 // ??? MessageNumber
};

class QMessageObject;

struct QMessage {
	QMessage() {}
	QMessage(uint16 _objId, uint16 _opcode, uint16 _arg1, int16 _arg2, int16 _arg3, QMessageObject *_sender, int _unk) {
		this->objId = _objId;
		this->opcode = _opcode;
		this->arg1 = _arg1;
		this->arg2 = _arg2;
		this->arg3 = _arg3;
		this->sender = _sender;
		this->unk = _unk;
	}

	uint16 objId;
	uint16 opcode;
	uint16 arg1;
	int16 arg2;
	int16 arg3;
	QMessageObject *sender;
	int32 unk;
};

struct QReaction {
	uint16 opcode;
	int8 status;
	int16 senderId;
	Common::Array<QMessage> messages;
};

} // End of namespace Petka

#endif
