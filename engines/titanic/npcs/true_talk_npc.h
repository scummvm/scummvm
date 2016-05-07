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

#ifndef TITANIC_TRUE_TALK_NPC_H
#define TITANIC_TRUE_TALK_NPC_H

#include "titanic/npcs/character.h"
#include "titanic/messages/messages.h"

namespace Titanic {

enum NpcFlag {
	NPCFLAG_SPEAKING = 1, NPCFLAG_2 = 2, NPCFLAG_4 = 4, NPCFLAG_8 = 8
};

class CViewItem;

class CTrueTalkNPC : public CCharacter {
	DECLARE_MESSAGE_MAP
	bool TextInputMsg(CTextInputMsg *msg);
	bool TrueTalkGetAssetDetailsMsg(CTrueTalkGetAssetDetailsMsg *msg);
	bool DismissBotMsg(CDismissBotMsg *msg);
	bool TrueTalkNotifySpeechStartedMsg(CTrueTalkNotifySpeechStartedMsg *msg);
	bool TrueTalkNotifySpeechEndedMsg(CTrueTalkNotifySpeechEndedMsg *msg);
	bool MovieEndMsg(CMovieEndMsg *msg);
	bool NPCQueueIdleAnimMsg(CNPCQueueIdleAnimMsg *msg);
	bool TimerMsg(CTimerMsg *msg);
	bool NPCPlayAnimationMsg(CNPCPlayAnimationMsg *msg);
protected:
	int _assetNumber;
	CString _assetName;
	int _fieldE4;
	uint _npcFlags;
	int _fieldEC;
	int _fieldF0;
	int _fieldF4;
	int _fieldF8;
	int _speechTimerId;
	int _field100;
	int _field104;
protected:
	void processInput(CTextInputMsg *msg, CViewItem *view);
	void performAction(int val1, int val2);
	
	/**
	 * Start an animation timer
	 */
	int startAnimTimer(const CString &action, uint firstDuration, uint duration);
public:
	CLASSDEF
	CTrueTalkNPC();

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);
};

} // End of namespace Titanic

#endif /* TITANIC_TRUE_TALK_NPC_H */
