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

#include "titanic/npcs/true_talk_npc.h"
#include "titanic/core/view_item.h"
#include "titanic/debugger.h"
#include "titanic/game_manager.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CTrueTalkNPC, CCharacter)
	ON_MESSAGE(TextInputMsg)
	ON_MESSAGE(TrueTalkGetAssetDetailsMsg)
	ON_MESSAGE(DismissBotMsg)
	ON_MESSAGE(TrueTalkNotifySpeechStartedMsg)
	ON_MESSAGE(TrueTalkNotifySpeechEndedMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(NPCQueueIdleAnimMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(NPCPlayAnimationMsg)
END_MESSAGE_MAP()

CTrueTalkNPC::CTrueTalkNPC() : _assetName("z451.dlg"),
	_assetNumber(0x11170), _fieldE4(0), _npcFlags(0), _speechDuration(0), _startTicks(0),
	_fieldF4(0), _fieldF8(0), _speechTimerId(0), _speechCounter(0), _field104(0) {
}

void CTrueTalkNPC::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_assetNumber, indent);
	file->writeQuotedLine(_assetName, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeNumberLine(_npcFlags, indent);
	file->writeNumberLine(_speechDuration, indent);
	file->writeNumberLine(_startTicks, indent);
	file->writeNumberLine(_fieldF4, indent);
	file->writeNumberLine(_fieldF8, indent);
	file->writeNumberLine(_speechTimerId, indent);
	file->writeNumberLine(_speechCounter, indent);
	file->writeNumberLine(_field104, indent);

	CCharacter::save(file, indent);
}

void CTrueTalkNPC::load(SimpleFile *file) {
	file->readNumber();
	_assetNumber = file->readNumber();
	_assetName = file->readString();
	_fieldE4 = file->readNumber();
	_npcFlags = file->readNumber();
	_speechDuration = file->readNumber();
	_startTicks = file->readNumber();
	_fieldF4 = file->readNumber();
	_fieldF8 = file->readNumber();
	_speechTimerId = file->readNumber();
	_speechCounter = file->readNumber();
	_field104 = file->readNumber();

	CCharacter::load(file);
}

bool CTrueTalkNPC::TextInputMsg(CTextInputMsg *msg) {
	processInput(msg, _field104 ? findView() : nullptr);
	return true;
}

bool CTrueTalkNPC::TrueTalkGetAssetDetailsMsg(CTrueTalkGetAssetDetailsMsg *msg) {
	msg->_filename = _assetName;
	msg->_numValue = _assetNumber;
	return true;
}

bool CTrueTalkNPC::DismissBotMsg(CDismissBotMsg *msg) {
	performAction(1, 0);
	return true;
}

bool CTrueTalkNPC::TrueTalkNotifySpeechStartedMsg(CTrueTalkNotifySpeechStartedMsg *msg) {
	debugC(DEBUG_DETAILED, kDebugScripts, "%s TrueTalkNotifySpeechStartedMsg flags=%x dialogueId=%d",
		getName().c_str(), _npcFlags, msg->_dialogueId);

	_npcFlags |= NPCFLAG_SPEAKING;
	++_speechCounter;

	if (!(_npcFlags & NPCFLAG_DOORBOT_IN_HOME)) {
		// Stop any previous animation
		if (_speechTimerId)
			stopAnimTimer(_speechTimerId);
		_speechTimerId = 0;

		_speechDuration = msg->_speechDuration;
		_startTicks = getTicksCount();

		if (!hasActiveMovie() || (_npcFlags & NPCFLAG_IDLING)) {
			_npcFlags &= ~NPCFLAG_IDLING;
			stopMovie();

			CNPCPlayTalkingAnimationMsg msg1(_speechDuration, 0, nullptr);
			msg1.execute(this);

			if (msg1._names) {
				CNPCPlayAnimationMsg msg2(msg1._names, msg1._speechDuration);
				msg2.execute(this);
			}
		}
	}

	return true;
}

bool CTrueTalkNPC::TrueTalkNotifySpeechEndedMsg(CTrueTalkNotifySpeechEndedMsg *msg) {
	debugC(DEBUG_DETAILED, kDebugScripts, "%s TrueTalkNotifySpeechEndedMsg flags=%x dialogueId=%d", getName().c_str(), _npcFlags, msg->_dialogueId);

	if (!getGameManager())
		return false;

	_npcFlags &= ~NPCFLAG_SPEAKING;
	--_speechCounter;
	_speechDuration = 0;

	if (!(_npcFlags & NPCFLAG_DOORBOT_IN_HOME)) {
		CNPCPlayTalkingAnimationMsg msg1(0, 2, nullptr);
		msg1.execute(this);
		CNPCQueueIdleAnimMsg msg2;
		msg2.execute(this);
	}

	return true;
}

bool CTrueTalkNPC::MovieEndMsg(CMovieEndMsg *msg) {
	if (_npcFlags & NPCFLAG_IDLING) {
		_npcFlags &= ~NPCFLAG_IDLING;
		CNPCQueueIdleAnimMsg idleMsg;
		idleMsg.execute(this);
		return true;
	} else if (!(_npcFlags & NPCFLAG_SPEAKING)) {
		return false;
	}

	int diff = getTicksCount() - _startTicks;
	int ticks = MAX((int)_speechDuration - diff, 0);
	CNPCPlayTalkingAnimationMsg msg1(ticks, ticks > 1000 ? 1 : 2, 0);
	msg1.execute(this);

	if (msg1._names) {
		CNPCPlayAnimationMsg msg2(msg1._names, ticks);
		msg2.execute(this);
	}

	return true;
}

bool CTrueTalkNPC::NPCQueueIdleAnimMsg(CNPCQueueIdleAnimMsg *msg) {
	int rndVal = getRandomNumber(_fieldF8 - 1) - (_fieldF8 / 2);
	_speechTimerId = startAnimTimer("NPCIdleAnim", _fieldF4 + rndVal, 0);

	return true;
}

bool CTrueTalkNPC::TimerMsg(CTimerMsg *msg) {
	if (_npcFlags & NPCFLAG_START_IDLING) {
		if (_speechCounter > 0)
			return false;

		CNPCPlayIdleAnimationMsg idleMsg;
		if (idleMsg.execute(this)) {
			if (idleMsg._names) {
				CNPCPlayAnimationMsg animMsg(idleMsg._names, 0);
				animMsg.execute(this);
			}

			_npcFlags |= NPCFLAG_IDLING;
		}
	}

	_speechTimerId = 0;
	return true;
}

bool CTrueTalkNPC::NPCPlayAnimationMsg(CNPCPlayAnimationMsg *msg) {
//	const char *const *nameP = msg->_names;
	int count;
	for (count = 0; msg->_names[count]; ++count)
		;

	if (msg->_maxDuration) {
		// Randomly pick a clip that's less than the allowed maximum
		int tries = 10, index;
		do {
			index = getRandomNumber(count - 1);
		} while (getClipDuration(msg->_names[index]) > msg->_maxDuration && --tries);

		if (!tries) {
			// Sequentially go through the clips to find any below the maximum
			index = 0;
			for (int idx = 0; idx < count; ++idx) {
				if (getClipDuration(msg->_names[idx]) < msg->_maxDuration) {
					index = idx;
					break;
				}
			}
		}

		playClip(msg->_names[index], MOVIE_NOTIFY_OBJECT);
	} else {
		playClip(msg->_names[getRandomNumber(count - 1)], MOVIE_NOTIFY_OBJECT);
	}

	return true;
}

void CTrueTalkNPC::processInput(CTextInputMsg *msg, CViewItem *view) {
	CTrueTalkManager *talkManager = getGameManager()->getTalkManager();
	if (talkManager)
		talkManager->processInput(this, msg, view);
}

void CTrueTalkNPC::setView(CViewItem *view) {
	CTrueTalkManager *talkManager = getGameManager()->getTalkManager();
	if (talkManager)
		talkManager->start3(this, view);
}

void CTrueTalkNPC::startTalker(CViewItem *view) {
	CGameManager *gameManager = getGameManager();
	if (gameManager)
		gameManager->getTalkManager()->start4(this, view);
}

void CTrueTalkNPC::performAction(bool startTalkingFlag, CViewItem *destView) {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->resetActiveNPC();

	if (startTalkingFlag)
		startTalker(destView);

	if (pet)
		pet->convResetNPC();
}

} // End of namespace Titanic
