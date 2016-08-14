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
#include "titanic/true_talk/tt_talker.h"

namespace Titanic {

enum NpcFlag {
	NPCFLAG_SPEAKING = 1, NPCFLAG_2 = 2, NPCFLAG_4 = 4, NPCFLAG_8 = 8,
	NPCFLAG_10000 = 0x10000, NPCFLAG_20000 = 0x20000,
	NPCFLAG_40000 = 0x40000, NPCFLAG_80000 = 0x80000,
	NPCFLAG_100000 = 0x100000, NPCFLAG_200000 = 0x200000,
	NPCFLAG_400000 = 0x400000, NPCFLAG_800000 = 0x800000,
	NPCFLAG_1000000 = 0x1000000, NPCFLAG_2000000 = 0x2000000,
	NPCFLAG_4000000 = 0x4000000, NPCFLAG_8000000 = 0x8000000
};

class CViewItem;

class CTrueTalkNPC : public CCharacter {
	DECLARE_MESSAGE_MAP;
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
	uint _soundId;
	int _fieldF0;
	int _fieldF4;
	int _fieldF8;
	int _speechTimerId;
	int _field100;
	int _field104;
protected:
	void processInput(CTextInputMsg *msg, CViewItem *view);

	/**
	 * Stop an animation timer
	 */
	void stopAnimTimer(int id);

	/**
	 * Perform an action
	 */
	void performAction(bool startTalking, CViewItem *view = nullptr);
public:
	CLASSDEF;
	CTrueTalkNPC();

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent);

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);

	/**
	 * Set the view for the NPC
	 */
	void setView(CViewItem *view);

	/**
	 * Start the talker in the given view
	 */
	void startTalker(CViewItem *view);
};

} // End of namespace Titanic

#endif /* TITANIC_TRUE_TALK_NPC_H */
