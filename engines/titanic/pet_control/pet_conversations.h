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

#ifndef TITANIC_PET_CONVERSATIONS_H
#define TITANIC_PET_CONVERSATIONS_H

#include "titanic/pet_control/pet_section.h"
#include "titanic/gfx/text_control.h"
#include "titanic/pet_control/pet_gfx_element.h"
#include "titanic/true_talk/true_talk_manager.h"

namespace Titanic {

#define TOTAL_DIALS 3

class CPetConversations : public CPetSection {
private:
	CPetGfxElement _scrollUp;
	CPetGfxElement _scrollDown;
	CPetGfxElement _dialBackground;
	CPetGfxElement _dials[TOTAL_DIALS];
	uint _npcLevels[TOTAL_DIALS];
	CPetGfxElement _val4;
	CPetGfxElement _val5;
	CPetGfxElement _indent;
	Rect _rect1;
	CPetGfxElement _doorBot;
	CPetGfxElement _bellBot;
	CPetGfxElement _splitter;
	CPetGfxElement _npcIcons[9];
	int _npcNum;
	CTextControl _log;
	CTextControl _textInput;
	bool _logChanged;
	int _field418;
	CString _npcName;
private:
	/**
	 * Sets up the control
	 */
	bool setupControl(CPetControl *petControl);

	/**
	 * Scroll up the conversation log
	 */
	void scrollUp();

	/**
	 * Scroll down the conversation log
	 */
	void scrollDown();

	/**
	 * Scroll up one page in the conversation log
	 */
	void scrollUpPage();

	/**
	 * Scroll down one page in the conversation log
	 */
	void scrollDownPage();

	/**
	 * Scroll to the top of the conversation log
	 */
	void scrollToTop();

	/**
	 * Scroll to the bottom of the conversation log
	 */
	void scrollToBottom();

	/**
	 * Check whether an NPC can be summoned
	 */
	int canSummonBot(const CString &name);

	/**
	 * Summon an NPC
	 */
	void summonBot(const CString &name);

	/**
	 * Get the TrueTalk script associated with a given NPC
	 */
	TTnpcScript *getNPCScript(const CString &name) const;

	/**
	 * Handle a keypress
	 */
	bool handleKey(const Common::KeyState &keyState);

	/**
	 * Handles an entered text line
	 */
	void textLineEntered(const CString &textLine);

	/**
	 * Updates one of the dials with data from a given NPC
	 */
	void updateDial(uint dialNum, const CString &npcName);

	/**
	 * Get a dial level
	 */
	uint getDialLevel(uint dialNum, TTnpcScript *script, bool flag = true);

	/**
	 * Called when the dial for an NPC is being changed
	 */
	void npcDialChange(uint dialNum, uint oldLevel, uint newLevel);
public:
	CPetConversations();
	~CPetConversations() override {}

	/**
	 * Sets up the section
	 */
	bool setup(CPetControl *petControl) override;

	/**
	 * Reset the section
	 */
	bool reset() override;

	/**
	 * Draw the section
	 */
	void draw(CScreenManager *screenManager) override;

	/**
	 * Get the bounds for the section
	 */
	Rect getBounds() const override;

	/**
	 * Returns true if the object is in a valid state
	 */
	bool isValid(CPetControl *petControl) override;

	/**
	 * Following are handlers for the various messages that the PET can
	 * pass onto the currently active section/area
	 */
	bool MouseButtonDownMsg(CMouseButtonDownMsg *msg) override;
	bool MouseButtonUpMsg(CMouseButtonUpMsg *msg) override;
	bool MouseDoubleClickMsg(CMouseDoubleClickMsg *msg) override;
	bool MouseWheelMsg(CMouseWheelMsg *msg) override;
	bool KeyCharMsg(CKeyCharMsg *msg) override;
	bool VirtualKeyCharMsg(CVirtualKeyCharMsg *msg) override;

	/**
	 * Display a message
	 */
	void displayMessage(const CString &msg) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file, int param) override;

	/**
	 * Called after a game has been loaded
	 */
	void postLoad() override;
	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Called when a section is switched to
	 */
	void enter(PetArea oldArea) override;

	/**
	 * Called when a section is being left, to switch to another area
	 */
	void leave() override;

	/**
	 * Called when a previously set up PET timer expires
	 */
	void timerExpired(int val) override;

	/**
	 * Display a title for an NPC
	 */
	void displayNPCName(CGameObject *npc) override;

	/**
	 * Sets the NPC to use
	 */
	void setNPC(const CString &name) override;

	/**
	 * Resets the active NPC
	 */
	void resetNPC() override;

	/**
	 * Show the text cursor
	 */
	void showCursor() override;

	/**
	 * Hide the text cursor
	 */
	void hideCursor() override;

	/**
	 * Set the active NPC
	 */
	void setActiveNPC(const CString &name);

	/**
	 * Resets the dials with the data for the currently active NPC
	 */
	void resetDials();

	/**
	 * Reset the dials with those for a given NPC
	 */
	void resetDials(const CString &name);

	/**
	 * Reset the dials to the '0' position
	 */
	void resetDials0();

	/**
	 * Adds a line to the log
	 */
	void addLine(const CString &line);

	/**
	 * Starts the NPC timer
	 */
	void startNPCTimer();

	/**
	 * Stops the NPC timer
	 */
	void stopNPCTimer();
};

} // End of namespace Titanic

#endif /* TITANIC_PET_CONVERSATIONS_H */
