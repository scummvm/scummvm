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

#ifndef QUEEN_TALK_H
#define QUEEN_TALK_H

#include "queen/queen.h"

namespace Queen {

class Resource;
class Logic;

class Talk {
	public:

		//! Public interface to run a talk from a file
		static void run(
				const char *filename,
				char *cutawayFilename,
				Logic *logic,
				Resource *resource);

		//! Public interface to speak a sentence
#if 0
		static void run(
				const char *sentence,
				const char *person,
				int noun,
				Logic *logic,
				Resource *resource);
#endif

		//! Read a string from ptr and return new ptr
		static byte *getString(byte *ptr, char *str, int maxLength, int align = 2);

	private:
		//!  Collection of constants used by Talk
		enum {
			MAX_STRING_LENGTH = 255,
			MAX_STRING_SIZE = (MAX_STRING_LENGTH + 1),
			TALK_SELECTED_COUNT = 86
		};

		//! TODO Move this to struct.h later!
		struct TalkSelected {
			int16 hasTalkedTo;
			int16 values[4];
		};

		struct DialogueNode {
			int16 head;
			int16 dialogueNodeValue1;
			int16 gameStateIndex;
			int16 gameStateValue;
		};

		Logic *_logic;
		Resource *_resource;
		
		//! Raw .dog file data (without 20 byte header)
		byte *_fileData;

		//! Number of dialogue levels
		int16 _levelMax;

		//! Unique key for this dialogue
		int16 _uniqueKey;

		//! Used to select voice files
		int16 _talkKey;

		//! Used by findDialogueString
		int16 _pMax;

		//! String data
		byte *_person1Ptr;

		//! Data used if we have talked to the person before
		byte *_person2Ptr;

		//! Data used if we haven't talked to the person before
		byte *_joePtr;

		//! Set to true to quit talking
		bool _quit;

		//! IDs for sentences
		DialogueNode _dialogueTree[18][6];

		//! TODO Move this to the Logic class later!
		TalkSelected _talkSelected[TALK_SELECTED_COUNT];

		//! Greeting from person Joe has talked to before
		char _person2String[MAX_STRING_SIZE];

		int _oldSelectedSentenceIndex;
		int _oldSelectedSentenceValue;

		char _talkString[5][MAX_STRING_SIZE];

		Talk(Logic *logic, Resource *resource);
		~Talk();

		//! Perform talk in file and return a cutaway filename
		void talk(const char *filename, char *cutawayFilename);

		//! Load talk data from .dog file 
		void load(const char *filename);

		//! First things spoken
		void initialTalk();

		//! Find a string in the dialogue tree
		void findDialogueString(byte *ptr, int16 id, char *str);

		//! Speak sentence
		bool speak(const char *sentence, const char *person, const char *voiceFilePrefix);

		//! Get TalkSelected struct for this talk
		TalkSelected *talkSelected();

		//! The sentence will not be displayed again
		void disableSentence(int oldLevel, int selectedSentence);

		//! Select what to say
		int16 selectSentence();

};

} // End of namespace Queen

#endif
