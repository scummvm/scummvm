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

#ifndef TITANIC_TRUE_TALK_MANAGER_H
#define TITANIC_TRUE_TALK_MANAGER_H

#include "titanic/messages/messages.h"
#include "titanic/support/simple_file.h"
#include "titanic/true_talk/dialogue_file.h"
#include "titanic/true_talk/title_engine.h"
#include "titanic/true_talk/tt_scripts.h"
#include "titanic/true_talk/tt_talker.h"

namespace Titanic {

class CGameManager;
class CTreeItem;
class CViewItem;
class CTrueTalkManager;
class CTrueTalkNPC;

class CTrueTalkManager {
private:
	CGameManager *_gameManager;
	STtitleEngine _titleEngine;
	TTScripts _scripts;
	int _currentCharId;
	CDialogueFile *_dialogueFile;
	int _dialogueId;
	int _field18;
	TTTalkerList _talkers;
private:
	/**
	 * Loads the statics for the class
	 */
	static void loadStatics(SimpleFile *file);

	/**
	 * Saves the statics associated with the class
	 */
	static void saveStatics(SimpleFile *file);

	/**
	 * Loads an NPC from file
	 */
	void loadNPC(SimpleFile *file, int charId);

	/**
	 * Saves the specified NPC to file
	 */
	void saveNPC(SimpleFile *file, int charId) const;

	/**
	 * Gets the script associated with an NPC game object
	 */
	TTNpcScript *getNpcScript(CTrueTalkNPC *npc) const;

	/**
	 * Gets the script associated with the current room
	 */
	TTRoomScript *getRoomScript() const;

	/**
	 * Loads assets for the current character, if it's changed
	 */
	void loadAssets(CTrueTalkNPC *npc, int charId);

	void setDialogue(CTrueTalkNPC *npc, TTRoomScript *roomScript, CViewItem *view);

	/**
	 * Read in text from the dialogue file
	 */
	CString readDialogueString();

	/**
	 * Read in the sound from the dialogue file
	 */
	int readDialogSound();

	/**
	 * Triggers animation for the NPC
	 */
	void triggerNPC(CTrueTalkNPC *npc);

	void setTalker(TTTalker *talker, TTRoomScript *roomScript, CViewItem *view, bool isParrot);
public:
	static int _v1;
	static int _v2;
	static int _v3;
	static bool _v4;
	static bool _v5;
	static int _v6;
	static int _v7;
	static bool _v8;
	static int _v9;
	static bool _v10;
	static int _v11[41];
	static CTrueTalkNPC *_currentNPC;

	static void setFlags(int index, int val);
public:
	/**
	 * Get a specified state value from the currently set NPC
	 */
	static int getStateVal(int stateNum);

	/**
	 * Trigger an NPC action
	 */
	static bool triggerAction(int action, int param);
public:
	CTrueTalkManager(CGameManager *owner);
	~CTrueTalkManager();

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file) const;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file);

	/**
	 * Clear the manager
	 */
	void clear();

	/**
	 * Called when a game is about to be loaded
	 */
	void preLoad();

	/**
	 * Called when loading a game is complete
	 */
	void postLoad() {}

	/**
	 * Called when a game is about to be saved
	 */
	void preSave() {}

	/**
	 * Called when a game has finished being saved
	 */
	void postSave() {}

	/**
	 * Returns the scripts for the manager
	 */
	TTScripts &getScripts() { return _scripts; }

	/**
	 * Remove any completed talkers
	 */
	void removeCompleted();

	void update2();

	/**
	 * Start a TrueTalk conversation
	 */
	void start(CTrueTalkNPC *npc, uint id, CViewItem *view);

	/**
	 * Return a TrueTalk talker/script
	 */
	TTNpcScript *getTalker(const CString &name) const;

	/**
	 * Process player's input
	 */
	void processInput(CTrueTalkNPC *npc, CTextInputMsg *msg, CViewItem *view);
};

} // End of namespace Titanic

#endif /* TITANIC_TRUE_TALK_MANAGER_H */
