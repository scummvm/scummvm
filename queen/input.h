/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef INPUT_H
#define INPUT_H

#include "queen/defs.h"
#include "queen/verb.h"
#include "common/scummsys.h"

class OSystem;

namespace Queen {

class Input {

	public:

		typedef void (*keyPressedCallback)(void *refCon, int key);

		//! Adjust here to change delays!
		enum {
			DELAY_SHORT  =  10,
			DELAY_NORMAL = 100
		};

		enum {
			MOUSE_LBUTTON = 1,
			MOUSE_RBUTTON = 2
		};

		Input(Language language, OSystem *system);

		//! calls the other delay() with a value adjusted depending on _fastMode
		void delay();

		//! moved QueenEngine::delay() here
		void delay(uint amount);

		//! convert input to verb
		int checkKeys();

		//! use instead of KEYVERB=0
		void clearKeyVerb()  { _keyVerb = Verb(VERB_NONE); }

		void canQuit(bool cq)             { _canQuit = cq; }

		bool cutawayRunning() const       { return _cutawayRunning; }
		void cutawayRunning(bool running) { _cutawayRunning = running; }

		bool cutawayQuit() const  { return _cutawayQuit; }
		void cutawayQuitReset()   { _cutawayQuit = false; }

		void dialogueRunning(bool running) { _dialogueRunning = running; }

		bool talkQuit() const { return _talkQuit; }
		void talkQuitReset()  { _talkQuit = false; }

		bool quickSave() const { return _quickSave; }
		void quickSaveReset()  { _quickSave = false; }
		bool quickLoad() const { return _quickLoad; }
		void quickLoadReset()  { _quickLoad = false; }

		bool fastMode() const { return _fastMode; }
		void fastMode(bool fm)	{ _fastMode = fm; }

		Verb keyVerb() const { return _keyVerb; }

		int mousePosX() const { return _mouse_x; }
		int mousePosY() const { return _mouse_y; }

		int mouseButton() const { return _mouseButton; }
		void clearMouseButton() { _mouseButton = 0; }

		bool waitForNumber(int &i, keyPressedCallback callback, void *refCon);
		bool waitForCharacter(char &c);

	private:

		enum KeyCode {
			KEY_SPACE = ' ',
			KEY_COMMA = ',',
			KEY_DOT   = '.',
			
			KEY_DIGIT_1 = '1',
			KEY_DIGIT_2 = '2',
			KEY_DIGIT_3 = '3',
			KEY_DIGIT_4 = '4',

			KEY_ESCAPE    = 27,
			KEY_RETURN    = 13,
			KEY_BACKSPACE = 8,
			
			KEY_F1 = 282,
			KEY_F11 = KEY_F1 + 10,
			KEY_F12
		};

		enum 
		{
			LANGUAGE_COUNT = 6
		};
		
		//! Used to get keyboard and mouse events
		OSystem *_system;

		//! Some cutaways require update() run faster
		bool _fastMode;

		//! The current verb received from keyboard
		Verb _keyVerb;          // KEYVERB

		//! set if a cutaway is running
		bool _cutawayRunning;   // CUTON

		//! set this if we can quit
		bool _canQuit;	// inverse of CANTQUIT

		//! moved Cutaway::_quit here
		bool _cutawayQuit;          // cutawayQuit

		//! set if a dialogue is running
		bool _dialogueRunning;

		//! moved Talk::_quit here 
		bool _talkQuit;         // TALKQUIT

		//! Set if quicksave requested
		bool _quickSave;

		//! Set if quickload requested
		bool _quickLoad;

		//! Set by delay();
		int _inKey;

		//! Set by delay();
		int _mouse_x, _mouse_y;

		//! Set by delay();
		int _mouseButton;

		//! Command keys for current language
		const char* _currentCommandKeys;

		//! Command keys for all languages
		static const char* _commandKeys[LANGUAGE_COUNT];
};

} // End of namespace Queen

#endif
