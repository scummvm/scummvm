/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "talk.h"
#include "graphics.h"

namespace Queen {

/*
	 Functions needed:

	 Data needed:

 */

void Talk::run(
		const char *filename, 
		char *cutawayFilename,
		Graphics *graphics,
		Logic *logic,
		Resource *resource) {
	Talk *talk = new Talk(graphics, logic, resource);
	talk->talk(filename, cutawayFilename);
	delete talk;
}

Talk::Talk(
		Graphics *graphics,
		Logic *logic,
		Resource *resource) 
: _graphics(graphics), _logic(logic), _resource(resource), _fileData(NULL), _quit(false) {

	//! TODO Move this to the Logic class later!
	memset(_talkSelected, 0, sizeof(_talkSelected));
}

Talk::~Talk() {
	delete[] _fileData;
}



void Talk::talk(const char *filename, char *cutawayFilename) {
	_oldSelectedSentenceIndex = 0;
	_oldSelectedSentenceValue = 0;

	debug(0, "----- talk(\"%s\") -----", filename);
	
	cutawayFilename[0] = '\0';
	load(filename);

	char personName[MAX_STRING_SIZE];
	// XXX SET_PERSON_DATA(N,NAMEstr,0);
	int bobNum = 1; // XXX P_BNUM;
	// XXX strcpy(PERstr,P_NAMEstr);
	personName[0] = '\0';

	int16 oldLevel = 0;
	bool personWalking = false;		// OWALK in talk.c

	// Lines 828-846 in talk.c
	for (int i = 1; i <= 4; i++) {
		if (talkSelected()->values[i-1] > 0) {
			// This option has been redefined so display new dialogue option
			_dialogueTree[1][i].head = talkSelected()->values[i-1];
		}
		else if (talkSelected()->values[i-1] == -1) {

			// Already selected so don't redisplay
			if (_dialogueTree[1][i].gameStateIndex >= 0) {
				_dialogueTree[1][i].head = -1;
				_dialogueTree[1][i].dialogueNodeValue1 = -1;
				_dialogueTree[1][i].gameStateIndex = -1;
				_dialogueTree[1][i].gameStateValue = -1;
			}
		}
	}

	initialTalk();

	// Lines 906-? in talk.c
	// XXX drawmouseflag=1;
	int16 level=1, retval=0;
	int16 head = _dialogueTree[level][0].head;

	// TODO: split this loop in several functions
	while(retval != -1) {
		// debug(0, "retval = %i", retval);
		
		char otherVoiceFilePrefix    [MAX_STRING_SIZE];
		char joeVoiceFilePrefix   [5][MAX_STRING_SIZE];

		_talkString[0][0] = '\0';

		if(talkSelected()->hasTalkedTo == 1 && head == 1)
			strcpy(_talkString[0], _person2String);
		else
			findDialogueString(_person1Ptr, head, _talkString[0]);

		if(talkSelected()->hasTalkedTo == 1 && head == 1)
			sprintf(otherVoiceFilePrefix, "%2dXXXXP", _talkKey);
		else
			sprintf(otherVoiceFilePrefix, "%2d%4xP", _talkKey, head);

		if (_talkString[0][0] == '\0' && retval > 1) {
			findDialogueString(_person1Ptr, retval, _talkString[0]);
			sprintf(otherVoiceFilePrefix,"%2d%4xP", _talkKey, retval);
		}

		// Joe dialogue

		for (int i = 1; i <= 4; i++) {
			findDialogueString(_joePtr, _dialogueTree[level][i].head, _talkString[i]);

			int16 index = _dialogueTree[level][i].gameStateIndex;

			if (index < 0 && _logic->gameState(abs(index)) != _dialogueTree[level][i].gameStateValue)
				_talkString[i][0] = '\0';

			sprintf(joeVoiceFilePrefix[i], "%2d%4xJ", _talkKey, _dialogueTree[level][i].head);
		}

		// Check to see if(all the dialogue options have been selected.
		// if this is the case, and the last one left is the exit option,
		// then automatically set S to that and exit.

		int choicesLeft = 0;
		int selectedSentence = 0;

		for (int i = 1; i <= 4; i++) {
			if (_talkString[i][0] != '\0') {
				choicesLeft++;
				selectedSentence = i;
			}
		}

		// debug(0, "choicesLeft = %i", choicesLeft);

		if (1 == choicesLeft) {
			// Automatically run the final dialogue option
			if (speak(_talkString[0], personName, otherVoiceFilePrefix))
				personWalking = true;

			if (_quit)
				break;

			speak(_talkString[selectedSentence], personName, joeVoiceFilePrefix[selectedSentence]);
		}
		else {
			if (bobNum > 0) {
				speak(_talkString[0], personName, otherVoiceFilePrefix);
				selectedSentence = selectSentence();
			}
			else {
				warning("bobBum is %i", bobNum);
				selectedSentence = 0;
			}
		}

		if (_quit)
			break;

		retval   = _dialogueTree[level][selectedSentence].dialogueNodeValue1;
		head     = _dialogueTree[level][selectedSentence].head;
		oldLevel = level;
		level    = 0;

		// Set LEVEL to the selected child in dialogue tree
		
		for (int i = 1; i <= _levelMax; i++)
			if (_dialogueTree[i][0].head == head)
				level = i;

		if (0 == level) {
			// No new level has been selected, so lets set LEVEL to the
			// tree path pointed to by the RETVAL

			for (int i = 1; i <= _levelMax; i++)
				for (int j = 0; j <= 5; j++)
					if (_dialogueTree[i][j].head == retval)
						level = i;

			disableSentence(oldLevel, selectedSentence);
		}
		else { // 0 != level
			// Check to see if Person Return value is positive, if it is, then
			// change the selected dialogue option to the Return value

			if (_dialogueTree[level][0].dialogueNodeValue1 > 0) {
				if (1 == oldLevel) {
						_oldSelectedSentenceIndex = selectedSentence;
						_oldSelectedSentenceValue = talkSelected()->values[selectedSentence-1];
						talkSelected()->values[selectedSentence-1] = _dialogueTree[level][0].dialogueNodeValue1;
				}

				_dialogueTree[oldLevel][selectedSentence].head = _dialogueTree[level][0].dialogueNodeValue1;
				_dialogueTree[level][0].dialogueNodeValue1 = -1;
			}
			else {
				disableSentence(oldLevel, selectedSentence);
			}
		} 
		
		// Check selected person to see if any Gamestates need setting

		int16 index = _dialogueTree[level][0].gameStateIndex;

		if (index > 0)
			_logic->gameState(index, _dialogueTree[level][0].gameStateValue);

		// if the selected dialogue line has a POSITIVE game state value
		// then set gamestate to Value = TALK(OLDLEVEL,S,3)
		
		index = _dialogueTree[oldLevel][selectedSentence].gameStateIndex;
		if (index > 0)
			_logic->gameState(index, _dialogueTree[oldLevel][selectedSentence].gameStateValue);


		// if(RETVAL = -1, then before we exit, check to see if(person
		// has something final to say!

		if (-1 == retval) {
			findDialogueString(_person1Ptr, head, _talkString[0]);
			if (_talkString[0][0] != '\0') {
				sprintf(otherVoiceFilePrefix, "%2d%4xP", _talkKey, head);
				if (speak(_talkString[0], personName, otherVoiceFilePrefix))
					personWalking = true;
			}
		}
	}
}
		
void Talk::disableSentence(int oldLevel, int selectedSentence) {
	// Mark off selected option

	if (1 == oldLevel) {
		if (_dialogueTree[oldLevel][selectedSentence].dialogueNodeValue1 != -1) {
			// Make sure choice is not exit option
			_oldSelectedSentenceIndex = selectedSentence;
			_oldSelectedSentenceValue = talkSelected()->values[selectedSentence-1];
			talkSelected()->values[selectedSentence-1] = -1;
		}
	}

	// Cancel selected dialogue line, so that its no longer displayed

	_dialogueTree[oldLevel][selectedSentence].head = -1;
	_dialogueTree[oldLevel][selectedSentence].dialogueNodeValue1 = -1;
}

void Talk::findDialogueString(byte *ptr, int16 id, char *str) {
	str[0] = '\0';

	for (int i = 1; i <= _pMax; i++) {
		ptr += 2;
		int16 currentId = (int16)READ_BE_UINT16(ptr); ptr += 2;
		if (id == currentId) {
			ptr = getString(ptr, str, MAX_STRING_LENGTH, 4);
			//debug(0, "Found string with ID %i: '%s'", id, str);
			break;
		}
		else
			ptr = getString(ptr, NULL, MAX_STRING_LENGTH, 4);
	}

	if (str[0] == '\0')
		warning("Failed to find string with ID %i", id);
}

void Talk::load(const char *filename) {
	byte *ptr = _fileData = _resource->loadFile(filename, 20);
	if (!_fileData) {
		error("Failed to load resource data file '%s'", filename);
	}

	bool canQuit;

	//
	// Load talk header
	//

	_levelMax = (int16)READ_BE_UINT16(ptr); ptr += 2;

	//debug(0, "levelMax = %i", _levelMax);

	if (_levelMax < 0) {
		_levelMax = -_levelMax;
		canQuit = false;
	}
	else
		canQuit = true;

	_uniqueKey            = (int16)READ_BE_UINT16(ptr); ptr += 2;
	_talkKey              = (int16)READ_BE_UINT16(ptr); ptr += 2;
	/*int16 jMax            =*/ (int16)READ_BE_UINT16(ptr); ptr += 2;
	_pMax                 = (int16)READ_BE_UINT16(ptr); ptr += 2;
	/*int16 gameState1      =*/ (int16)READ_BE_UINT16(ptr); ptr += 2;
	/*int16 testValue1      =*/ (int16)READ_BE_UINT16(ptr); ptr += 2;
	/*int16 itemToInsert1   =*/ (int16)READ_BE_UINT16(ptr); ptr += 2;
	/*int16 gameState2      =*/ (int16)READ_BE_UINT16(ptr); ptr += 2;
	/*int16 testValue2      =*/ (int16)READ_BE_UINT16(ptr); ptr += 2;
	/*int16 itemToInsert2   =*/ (int16)READ_BE_UINT16(ptr); ptr += 2;

	//debug(0, "uniqueKey = %i", _uniqueKey);
	//debug(0, "talkKey   = %i", _talkKey);

	_person1Ptr      = _fileData + READ_BE_UINT16(ptr); ptr += 2;
	/*byte *cutawayPtr = _fileData + READ_BE_UINT16(ptr);*/ ptr += 2;
	_person2Ptr      = _fileData + READ_BE_UINT16(ptr); ptr += 2;

	if (ptr != (_fileData + 28))
		error("ptr != (_fileData + 28))");
	
	byte *dataPtr    = _fileData + 32;
	_joePtr          = dataPtr + _levelMax * 96;
	
	//
	// Load dialogue tree
	//

	ptr = dataPtr;

	for (int i = 1; i <= _levelMax; i++)
		for (int j = 0; j <= 5; j++) {
			ptr += 2;
			_dialogueTree[i][j].head = (int16)READ_BE_UINT16(ptr); ptr += 2;
			ptr += 2;
			_dialogueTree[i][j].dialogueNodeValue1 = (int16)READ_BE_UINT16(ptr); ptr += 2;
			ptr += 2;
			_dialogueTree[i][j].gameStateIndex = (int16)READ_BE_UINT16(ptr); ptr += 2;
			ptr += 2;
			_dialogueTree[i][j].gameStateValue = (int16)READ_BE_UINT16(ptr); ptr += 2;
		}
}

void Talk::initialTalk() {
	// Lines 848-903 in talk.c 

	byte *ptr = _joePtr + 2;
	
	uint16 hasString = READ_BE_UINT16(ptr); ptr += 2;

	char joeString[MAX_STRING_SIZE];
	if (hasString) {
		ptr = getString(ptr, joeString, MAX_STRING_LENGTH);
		//debug(0, "joeString = '%s'", joeString);
	}
	else
		joeString[0] = '\0';

	ptr = _person2Ptr;
	ptr = getString(ptr, _person2String, MAX_STRING_LENGTH);
	//debug(0, "person2String = '%s'", _person2String);

	char joe2String[MAX_STRING_SIZE];
	ptr = getString(ptr, joe2String, MAX_STRING_LENGTH);
	//debug(0, "joe2String = '%s'", joe2String);

	if (talkSelected()->hasTalkedTo == 0) {
		
		// Not yet talked to this person
		
		if (joeString[0] != '0') {
			char voiceFilePrefix[MAX_STRING_SIZE];
			sprintf(voiceFilePrefix, "%2dSSSSJ", _talkKey);
			speak(joeString, "JOE", voiceFilePrefix);
		}
	}
	else {
		// Already spoken to them, choose second response
		
		if (joe2String[0] != '0') {
			char voiceFilePrefix[MAX_STRING_SIZE];
			sprintf(voiceFilePrefix, "%2dSSSSJ", _talkKey);
			speak(joe2String, "JOE", voiceFilePrefix);
		}

	}

}

bool Talk::speak(const char *sentence, const char *person, const char *voiceFilePrefix) {
	debug(0, "Sentence '%s' is said by person '%s' and voice files with prefix '%s' played",
			sentence, person, voiceFilePrefix);
	return false; // XXX
}

byte *Talk::getString(byte *ptr, char *str, int maxLength, int align) {
	int length = *ptr;
	ptr++;

	if (length > maxLength) {
		error("String too long. Length = %i, maxLength = %i, str = '%*s'",
				length, maxLength, length, (const char*)ptr);
	}
	else if (length) {
		if (str)
			memcpy(str, (const char*)ptr, length);
		ptr += length;

		while ((int)ptr % align)
			ptr++;
	}

	if (str)
		str[length] = '\0';

	return ptr;
}

Talk::TalkSelected *Talk::talkSelected() {
	return _talkSelected + _uniqueKey;
}

int Talk::splitOption(const char *str, char optionText[5][MAX_STRING_SIZE]) {

	//debug(0, "splitOption(\"%s\")", str);

	// Check to see if option fits on one line, and exit early

	/* XXX if (_logic->language() == ENGLISH || textWidth(str) <= MAX_TEXT_WIDTH)*/ {
		strcpy(optionText[0], str);
		return 1;
	}

	abort();

	// Split up multiple line option at closest space character
	// int optionLines = 0;
}

static char *removeStar(char *str) {
	
	// The remove_star function in talk.c uses a static variable, but this
	// modifies the string instead, so the caller should use a copy of the
	// string.

	char *p = strchr(str, '*');
	if (p)
		*p = '\0';

	return str;
}

static void text(int x, int y, const char *str) {
	
	// This function is just for debugging, the caller should really use some
	// method in Queen::Graphics.
	
	debug(0, "Write '%s' at (%i,%i)", str, x, y);
}

int16 Talk::selectSentence() {
	// Function TALK_BOB (lines 577-739) in talk.c
	int selectedSentence = 0;

	int scrollX = 0; 	// XXX: global variable
	int startOption = 1;
	int optionLines = 0;
	char optionText[5][MAX_STRING_SIZE];

	// These bobs are up and down arrows

	BobSlot *bob1 = _graphics->bob(SENTENCE_BOB_1);
	BobSlot *bob2 = _graphics->bob(SENTENCE_BOB_2);

	bob1->x         = 303 + 8 + scrollX;
	bob1->y         = 150 + 1;
	bob1->frameNum  = 3;
	bob1->box.y2    = 199;
	bob1->active    = false;

	bob2->x         = 303 + scrollX;
	bob2->y         = 175;
	bob2->frameNum  = 4;
	bob2->box.y2    = 199;
	bob2->active    = false;

	bool rezone = true;

	while (rezone) {
		rezone = false;

		// Set zones for UP/DOWN text arrows when not English version
		// XXX ClearZones(1);

		if (_logic->language() != ENGLISH) {
			// XXX SetZone(1,5,MAXTEXTLEN+1, 0,319,24);
			// XXX SetZone(1,6,MAXTEXTLEN+1,25,319,49);
		}

		// blanktexts(151,199);

		int sentenceCount = 0;
		int yOffset = 1;

		for (int i = startOption; i <= 4; i++) {
			// XXX TALK_ZONE[I] = 0;
			if (_talkString[i][0] != '\0') {
				sentenceCount++;
				
				char temp[MAX_STRING_SIZE];
				strcpy(temp, _talkString[i]);
				optionLines = splitOption(removeStar(temp), optionText);

				if (yOffset < 5)
					/* XXX SetZone(
							1, 
							I, 
							0, 
							(yofs * 10) - PUSHUP, 
							(VersionStr[1] =='E') ? 319 : MAX_TEXT_WIDTH,
							10 * optionLines + (yOffset * 10) - PUSHUP) */;

				for (int j = 0; j < optionLines; j++) {
					if (yOffset < 5)
						text((j == 0) ? 0 : 24, 150 - PUSHUP + yOffset * 10, optionText[j]);
					yOffset++;
				}
				
			// XXX TALK_ZONE[i] = sentenceCount;
			}
		}

		yOffset--;

		// Up and down dialogue arrows

		if (_logic->language() != ENGLISH) {
			bob1->active = (startOption > 1);
			bob2->active = (yOffset > 4);
		}

		// XXX KEYVERB=0;
		if (sentenceCount > 0) {
			// XXX CH=0, S=0, OLDS=0;
      // XXX while(CH==0) 

			if (_quit)
				break;

			// XXX update();

			// XXX manage input 
		}
	}


	// XXX Begin debug stuff
	// debug(0, "----- Select a sentence of these -----");
	for (int i = 1; i <= 4; i++) {
		if (_talkString[i][0] != '\0') {
			// XXX debug(0, "%i: %s", i, _talkString[i]);
			if (!selectedSentence)
				selectedSentence = i;
		}
	}
	// XXX End debug stuff

	bob1->active = false;
	bob2->active = false;

	debug(0, "Selected sentence %i", selectedSentence);
	return selectedSentence;
}

} // End of namespace Queen
