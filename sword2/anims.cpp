/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

// ---------------------------------------------------------------------------
// A more intelligent version of the old ANIMS.C
// All this stuff by James
// DON'T TOUCH!
// ---------------------------------------------------------------------------

#include "common/stdafx.h"
#include "common/file.h"
#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/interpreter.h"

namespace Sword2 {

int32 Logic::fnAnim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 resource id of animation file

	// 0 means normal forward anim
	return animate(params, false);
}

int32 Logic::fnReverseAnim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 resource id of animation file

	// 1 means reverse anim
	return animate(params, true);
}

int32 Logic::fnMegaTableAnim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to animation table

	// 0 means normal forward anim
	return megaTableAnimate(params, false);
}

int32 Logic::fnReverseMegaTableAnim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to animation table

	// 1 means reverse anim
	return megaTableAnimate(params, true);
}

int32 Logic::animate(int32 *params, bool reverse) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 resource id of animation file

 	Object_logic *ob_logic;
 	Object_graphic *ob_graphic;
	uint8 *anim_file;
	_animHeader *anim_head;
 	int32 res = params[2];

#ifdef _SWORD2_DEBUG
	// for animation testing & checking for correct file type
	_standardHeader	*head;
#endif

	// read the main parameters

	ob_logic = (Object_logic *) _vm->_memory->intToPtr(params[0]);
	ob_graphic = (Object_graphic *) _vm->_memory->intToPtr(params[1]);
	
	if (ob_logic->looping == 0) {
		// This is the start of the anim - set up the first frame

#ifdef _SWORD2_DEBUG
		// For testing all anims!
		// A script loop can send every resource number to the anim
		// function & it will only run the valid ones. See
		// 'testing_routines' object in George's Player Character
		// section of linc

		if (SYSTEM_TESTING_ANIMS) {
			// if the resource number is within range & it's not
			// a null resource

			if (_resman->Res_check_valid(res)) {
				// Open the resource. Can close it immediately.
				// We've got a pointer to the header.
				head = (_standardHeader *) _resman->openResource(res);
				_resman->closeResource(res);

				// if it's not an animation file
				if (head->fileType != ANIMATION_FILE) {
					// switch off the sprite
					// don't animate - just continue
					// script next cycle
					fnNoSprite(params + 1);
					return IR_STOP;
				}
			} else {
				// Not a valid resource number. Switch off
				// the sprite. Don't animate - just continue
				// script next cycle.
				fnNoSprite(params + 1);
				return IR_STOP;
			}

			// switch on the sprite
			fnSortSprite(params + 1);
		}
#endif

#ifdef _SWORD2_DEBUG
		// check that we haven't been passed a zero resource number
		if (res == 0)
			error("animate: %s (id %d) passed zero anim resource", _vm->fetchObjectName(ID), ID);
#endif

		// open anim file
		anim_file = _vm->_resman->openResource(res);

#ifdef _SWORD2_DEBUG
		// check this this resource is actually an animation file!
		head = (_standardHeader *) anim_file;
		if (head->fileType != ANIMATION_FILE)
			error("animate: %s (%d) is not an anim!", _vm->fetchObjectName(res), res);
#endif

		// point to anim header
		anim_head = _vm->fetchAnimHeader(anim_file);

/* #ifdef _SWORD2_DEBUG
		// check there's at least one frame
		if (anim_head->noAnimFrames == 0)
 			error("animate: %s (%d) has zero frame count!", _vm->fetchObjectName(res), res);
#endif */

		// now running an anim, looping back to this 'FN' call again
		ob_logic->looping = 1;
		ob_graphic->anim_resource = res;

		if (reverse)
			ob_graphic->anim_pc = anim_head->noAnimFrames - 1;
		else
			ob_graphic->anim_pc = 0;
 	} else if (getSync()) {
		// We've received a sync - return to script immediately
		debug(5, "**sync stopped %d**", ID);

		// If sync received, anim finishes right now (remaining on
		// last frame). Quit animation, but continue script.
		ob_logic->looping = 0;
		return IR_CONT;
	} else {
		// Not first frame, and no sync received - set up the next
		// frame of the anim.

		// open anim file and point to anim header
		anim_file = _vm->_resman->openResource(ob_graphic->anim_resource);
		anim_head = _vm->fetchAnimHeader(anim_file);

		if (reverse)
			ob_graphic->anim_pc--;
		else
			ob_graphic->anim_pc++;
	}

	// check for end of anim

	if (reverse) {
		if (ob_graphic->anim_pc == 0)
			ob_logic->looping = 0;
	} else {
		if (ob_graphic->anim_pc == (int32) (anim_head->noAnimFrames - 1))
			ob_logic->looping = 0;
	}

	// close the anim file
	_vm->_resman->closeResource(ob_graphic->anim_resource);

	// check if we want the script to loop back & call this function again
	return ob_logic->looping ? IR_REPEAT : IR_STOP;
}

int32 Logic::megaTableAnimate(int32 *params, bool reverse) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to animation table

 	Object_logic *ob_logic;
 	Object_mega *ob_mega;
	uint32 *anim_table;
	int32 pars[5];

	// if this is the start of the anim, read the anim table to get the
	// appropriate anim resource

	ob_logic = (Object_logic *) _vm->_memory->intToPtr(params[0]);

	if (ob_logic->looping == 0) {
	 	ob_mega = (Object_mega *) _vm->_memory->intToPtr(params[2]);
		anim_table = (uint32 *) _vm->_memory->intToPtr(params[3]);

		// appropriate anim resource is in 'table[direction]'
		pars[2] = anim_table[ob_mega->current_dir];
	}

	// set up the rest of the parameters for fnAnim()

	pars[0] = params[0];
	pars[1] = params[1];

	// pars[2] only needed setting at the start of the anim

	// call animate() with these params
	return animate(pars, reverse);
}

int32 Logic::fnSetFrame(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 resource id of animation file
	//		2 frame flag (0=first 1=last)

 	Object_graphic *ob_graphic;
	uint8 *anim_file;
	_animHeader *anim_head;
	int32 res = params[1];

#ifdef _SWORD2_DEBUG
	// for checking for correct file type
	_standardHeader	*head;
#endif

#ifdef _SWORD2_DEBUG
	// check that we haven't been passed a zero resource number
	if (res == 0)
		error("fnSetFrame: %s (id %d) passed zero anim resource", _vm->fetchObjectName(ID), ID);
#endif

	// open the resource (& check it's valid)

	anim_file = _vm->_resman->openResource(res);

#ifdef _SWORD2_DEBUG
	// check this this resource is actually an animation file!
	head = (_standardHeader *) anim_file;
	if (head->fileType != ANIMATION_FILE)
		error("fnSetFrame: %s (%d) is not an anim!", _vm->fetchObjectName(res), res);
#endif

	// set up pointer to the animation header
	anim_head = _vm->fetchAnimHeader(anim_file);

/* #ifdef _SWORD2_DEBUG
	// check there's at least one frame
	if (anim_head->noAnimFrames == 0)
		error("fnSetFrame: %s (%d) has zero frame count!", _vm->fetchObjectName(res), res);
#endif */

	// set up anim resource in graphic object

	ob_graphic = (Object_graphic *) _vm->_memory->intToPtr(params[0]);
	ob_graphic->anim_resource = res;

	if (params[2])
		ob_graphic->anim_pc = anim_head->noAnimFrames - 1;
	else
		ob_graphic->anim_pc = 0;

	// Close the anim file and drop out of script

	_vm->_resman->closeResource(ob_graphic->anim_resource);
	return IR_CONT;
}

int32 Logic::fnNoSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) _vm->_memory->intToPtr(params[0]);

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0xffff0000;
	ob_graphic->type |= NO_SPRITE;

	// continue script
	return IR_CONT;
}

int32 Logic::fnBackPar0Sprite(int32 *params) {
	// params:	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) _vm->_memory->intToPtr(params[0]);

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0xffff0000;
	ob_graphic->type |= BGP0_SPRITE;

	// continue script
	return IR_CONT;
}

int32 Logic::fnBackPar1Sprite(int32 *params) {
	// params:	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) _vm->_memory->intToPtr(params[0]);

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0xffff0000;
	ob_graphic->type |= BGP1_SPRITE;

	// continue script
	return IR_CONT;
}

int32 Logic::fnBackSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) _vm->_memory->intToPtr(params[0]);

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0xffff0000;
	ob_graphic->type |= BACK_SPRITE;

	// continue script
	return IR_CONT;
}

int32 Logic::fnSortSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) _vm->_memory->intToPtr(params[0]);

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0xffff0000;
	ob_graphic->type |= SORT_SPRITE;

	// continue script
	return IR_CONT;
}

int32 Logic::fnForeSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) _vm->_memory->intToPtr(params[0]);

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0xffff0000;
	ob_graphic->type |= FORE_SPRITE;

	// continue script
	return IR_CONT;
}

int32 Logic::fnForePar0Sprite(int32 *params) {
	// params:	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) _vm->_memory->intToPtr(params[0]);

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0xffff0000;
	ob_graphic->type |= FGP0_SPRITE;

	// continue script
	return IR_CONT;
}

int32 Logic::fnForePar1Sprite(int32 *params) {
	// params:	0 pointer to object's graphic structure

	Object_graphic	*ob_graphic = (Object_graphic *) _vm->_memory->intToPtr(params[0]);

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0xffff0000;
	ob_graphic->type |= FGP1_SPRITE;

	// continue script
	return IR_CONT;
}

int32 Logic::fnShadedSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure

	Object_graphic	*ob_graphic = (Object_graphic *) _vm->_memory->intToPtr(params[0]);

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0x0000ffff;
	ob_graphic->type |= SHADED_SPRITE;

	// note that drivers may still shade mega frames automatically, even
	// when not sent 'RDSPR_SHADOW'

	// continue script
	return IR_CONT;
}

int32 Logic::fnUnshadedSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure

	Object_graphic	*ob_graphic = (Object_graphic *) _vm->_memory->intToPtr(params[0]);

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0x0000ffff;
	ob_graphic->type |= UNSHADED_SPRITE;

	// continue script
	return IR_CONT;
}

// Notes on PlaySmacker()

// 1st param is filename of sequence file
// 2nd param is a pointer to a null-terminated array of pointers to
// _movieTextObject structures

//int32 PlaySmacker(char *filename, _movieTextObject *textObjects[]);

//	typedef struct {
//		uint16 startFrame;
//		uint16 endFrame;
//		_spriteInfo *textSprite;
//		_wavHeader *speech;
//	} _movieTextObject;

int32 Logic::fnAddSequenceText(int32 *params) {
	// params:	0 text number
	//		1 frame number to start the text displaying
	//		2 frame number to stop the text dispalying

	assert(_sequenceTextLines < MAX_SEQUENCE_TEXT_LINES);

	_sequenceTextList[_sequenceTextLines].textNumber = params[0];
	_sequenceTextList[_sequenceTextLines].startFrame = params[1];
	_sequenceTextList[_sequenceTextLines].endFrame = (uint16) params[2];
	_sequenceTextLines++;

	// continue script
	return IR_CONT;
}

void Logic::createSequenceSpeech(_movieTextObject *sequenceText[]) {
	uint32 line;
 	_frameHeader *frame;
 	uint32 local_text;
	uint32 text_res;
	uint8 *text;
	uint32 wavId;	// ie. offical text number (actor text number)
	bool speechRunning;
 	char speechFile[256];

	// for each sequence text line that's been logged
	for (line = 0; line < _sequenceTextLines; line++) {
		// allocate this structure
		sequenceText[line] = new _movieTextObject;

		sequenceText[line]->startFrame = _sequenceTextList[line].startFrame;
		sequenceText[line]->endFrame = _sequenceTextList[line].endFrame;

		// pull out the text line to get the official text number
		// (for wav id)

  		text_res = _sequenceTextList[line].textNumber / SIZE;
		local_text = _sequenceTextList[line].textNumber & 0xffff;

		// open text resource & get the line
		text = _vm->fetchTextLine(_vm->_resman->openResource(text_res), local_text);
		wavId = (int32) READ_LE_UINT16(text);

		// now ok to close the text file
		_vm->_resman->closeResource(text_res);

		// 1st word of text line is the official line number
		debug(5,"(%d) SEQUENCE TEXT: %s", READ_LE_UINT16(text), text + 2);

		// is it to be speech or subtitles or both?
		// assume speech is not running until know otherwise

		speechRunning = false;
		_sequenceTextList[line].speech_mem = NULL;
		sequenceText[line]->speech = NULL;

		if (!_vm->_sound->isSpeechMute()) {
			// speech is selected, so try that first

			// set up path to speech cluster
			// first checking if we have speech1.clu or
			// speech2.clu in current directory (for translators
			// to test)

			File fp;

			sprintf(speechFile, "speech%d.clu", _vm->_resman->whichCd());
			if (fp.open(speechFile))
				fp.close();
			else
				strcpy(speechFile, "speech.clu");

			_sequenceTextList[line].speechBufferSize = _vm->_sound->preFetchCompSpeech((char *) speechFile, wavId, &_sequenceTextList[line].speech_mem);
			if (_sequenceTextList[line].speechBufferSize) {
				// ok, we've got speech!
				speechRunning = true;
			}
		}

		// if we want subtitles, or speech failed to load

		if (_vm->_gui->_subtitles || !speechRunning) {
			// open text resource & get the line
			text = _vm->fetchTextLine(_vm->_resman->openResource(text_res), local_text);
			// make the sprite
			// 'text+2' to skip the first 2 bytes which form the
			// line reference number

			// NB. The mem block containing the text sprite is
			// currently FLOATING!

			// When rendering text over a sequence we need a
			// different colour for the border.

			_sequenceTextList[line].text_mem = _vm->_fontRenderer->makeTextSprite(text + 2, 600, 255, _vm->_speechFontId, 1);

			// ok to close the text resource now
			_vm->_resman->closeResource(text_res);
		} else {
			_sequenceTextList[line].text_mem = NULL;
			sequenceText[line]->textSprite = NULL;
		}
	}

	// for drivers: NULL-terminate the array of pointers to
	// _movieTextObject's
	sequenceText[_sequenceTextLines] = NULL;

  	// now lock all the memory blocks containing text sprites & speech
	// samples and set up the pointers to them, etc, for the drivers

	for (line = 0; line < _sequenceTextLines; line++) {
		// if we've made a text sprite for this line...

		if (_sequenceTextList[line].text_mem) {
			_vm->_memory->lockMemory(_sequenceTextList[line].text_mem);

			// now fill out the _spriteInfo structure in the
			// _movieTextObjectStructure

			frame = (_frameHeader *) _sequenceTextList[line].text_mem->ad;

			sequenceText[line]->textSprite = new _spriteInfo;

			// center text at bottom of screen
			sequenceText[line]->textSprite->x = 320 - frame->width / 2;
			sequenceText[line]->textSprite->y = 440 - frame->height;
			sequenceText[line]->textSprite->w = frame->width;
			sequenceText[line]->textSprite->h = frame->height;
			sequenceText[line]->textSprite->type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION;
			sequenceText[line]->textSprite->data = _sequenceTextList[line].text_mem->ad + sizeof(_frameHeader);
		}

		// if we've loaded a speech sample for this line...

 		if (_sequenceTextList[line].speech_mem) {
			// for drivers: set up pointer to decompressed wav in
			// memory

			sequenceText[line]->speechBufferSize = _sequenceTextList[line].speechBufferSize;
			sequenceText[line]->speech = _sequenceTextList[line].speech_mem;
		}
	}
}

void Logic::clearSequenceSpeech(_movieTextObject *sequenceText[]) {
	uint32 line;

	for (line = 0; line < _sequenceTextLines; line++) {
		// free up the memory used by this _movieTextObject
		delete sequenceText[line];

		// free up the mem block containing this text sprite
		if (_sequenceTextList[line].text_mem)
			_vm->_memory->freeMemory(_sequenceTextList[line].text_mem);

		// free up the mem block containing this speech sample
		if (_sequenceTextList[line].speech_mem)
			free(_sequenceTextList[line].speech_mem);
	}

	// IMPORTANT! Reset the line count ready for the next sequence!
	_sequenceTextLines = 0;
}

int32 Logic::fnSmackerLeadIn(int32 *params) {
	uint8 *leadIn;
	uint32 rv;
#ifdef _SWORD2_DEBUG
	_standardHeader *header;
#endif

	// params:	0 id of lead-in music

	leadIn = _vm->_resman->openResource(params[0]);

#ifdef _SWORD2_DEBUG
	header = (_standardHeader *) leadIn;
	if (header->fileType != WAV_FILE)
		error("fnSmackerLeadIn() given invalid resource");
#endif

	leadIn += sizeof(_standardHeader);
	// wav data gets copied to sound memory
	rv = _vm->_sound->playFx(0, leadIn, 0, 0, RDSE_FXLEADIN);

	if (rv)
		debug(5, "SFX ERROR: playFx() returned %.8x", rv);

	_vm->_resman->closeResource(params[0]);

	// fade out any music that is currently playing
	fnStopMusic(NULL);

	// continue script
	return IR_CONT;
}

int32 Logic::fnSmackerLeadOut(int32 *params) {
	// params:	0 id of lead-out music

	// ready for use in fnPlaySequence
	_smackerLeadOut = params[0];

	// continue script
	return IR_CONT;
}

int32 Logic::fnPlaySequence(int32 *params) {
	// params:	0 pointer to null-terminated ascii filename
	// 		1 number of frames in the sequence, used for PSX.

	char filename[30];
 	uint32 rv;
	_movieTextObject *sequenceSpeechArray[MAX_SEQUENCE_TEXT_LINES + 1];
	uint8 *leadOut = NULL;

#ifdef _SWORD2_DEBUG
	_standardHeader *header;
#endif

	// The original code had some #ifdef blocks for skipping or muting the
	// cutscenes - fondly described as "the biggest fudge in the history
	// of computer games" - but at the very least we want to show the
	// cutscene subtitles, so I removed them.

	debug(5, "fnPlaySequence(\"%s\");", (const char *) _vm->_memory->intToPtr(params[0]));

#ifdef _SWORD2_DEBUG
	// check that the name paseed from script is 8 chars or less
	if (strlen((const char *) _vm->_memory->intToPtr(params[0])) > 8)
		error("Sequence filename too long");
#endif

	// add the appropriate file extension & play it

	sprintf(filename, "%s.smk", (const char *) _vm->_memory->intToPtr(params[0]));

	// Write to walkthrough file (zebug0.txt)
 	debug(5, "PLAYING SEQUENCE \"%s\"", filename);

	// now create the text sprites, if any

	if (_sequenceTextLines)
		createSequenceSpeech(sequenceSpeechArray);

	// open the lead-out music resource, if there is one

	if (_smackerLeadOut) {
		leadOut = _vm->_resman->openResource(_smackerLeadOut);

#ifdef _SWORD2_DEBUG
		header = (_standardHeader *) leadOut;
		if (header->fileType != WAV_FILE)
			error("fnSmackerLeadOut() given invalid resource");
#endif

		leadOut += sizeof(_standardHeader);
	}

	// play the smacker

	// don't want to carry on streaming game music when smacker starts!
	fnStopMusic(NULL);

	// pause sfx during sequence, except the one used for lead-in music
	_vm->_sound->pauseFxForSequence();

	MoviePlayer player(_vm);

	if (_sequenceTextLines && !(_vm->_features & GF_DEMO))
		rv = player.play(filename, sequenceSpeechArray, leadOut);
	else
		rv = player.play(filename, NULL, leadOut);

	// unpause sound fx again, in case we're staying in same location
	_vm->_sound->unpauseFx();

	// close the lead-out music resource

	if (_smackerLeadOut) {
		_vm->_resman->closeResource(_smackerLeadOut);
		_smackerLeadOut = 0;
	}

	// check the error return-value
	if (rv)
		debug(5, "MoviePlayer.play(\"%s\") returned 0x%.8x", filename, rv);

	// now clear the text sprites, if any

	if (_sequenceTextLines)
		clearSequenceSpeech(sequenceSpeechArray);

	// now clear the screen in case the Sequence was quitted (using ESC)
	// rather than fading down to black

	_vm->_graphics->clearScene();

	// zero the entire palette in case we're about to fade up!

	_palEntry pal[256];

	memset(pal, 0, 256 * sizeof(_palEntry));
	_vm->_graphics->setPalette(0, 256, (uint8 *) pal, RDPAL_INSTANT);

	debug(5, "fnPlaySequence FINISHED");

	// continue script
	return IR_CONT;
}

} // End of namespace Sword2
