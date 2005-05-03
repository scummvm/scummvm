/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2005 The ScummVM project
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
#include "sword2/build_display.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/resman.h"
#include "sword2/sound.h"
#include "sword2/driver/animation.h"

namespace Sword2 {

int32 Logic::animate(int32 *params, bool reverse) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 resource id of animation file

 	ObjectLogic *ob_logic = (ObjectLogic *) decodePtr(params[0]);
 	ObjectGraphic *ob_graphic = (ObjectGraphic *) decodePtr(params[1]);
	byte *anim_file;
	AnimHeader *anim_head;
 	int32 res = params[2];

	if (ob_logic->looping == 0) {
		StandardHeader *head;

		// This is the start of the anim - set up the first frame

		// For testing all anims!
		// A script loop can send every resource number to the anim
		// function & it will only run the valid ones. See
		// 'testing_routines' object in George's Player Character
		// section of linc

		if (_scriptVars[SYSTEM_TESTING_ANIMS]) {
			// if the resource number is within range & it's not
			// a null resource

			if (_vm->_resman->checkValid(res)) {
				// Open the resource. Can close it immediately.
				// We've got a pointer to the header.
				head = (StandardHeader *) _vm->_resman->openResource(res);
				_vm->_resman->closeResource(res);

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

		assert(res);

		// open anim file
		anim_file = _vm->_resman->openResource(res);

		head = (StandardHeader *) anim_file;
		assert(head->fileType == ANIMATION_FILE);

		// point to anim header
		anim_head = _vm->fetchAnimHeader(anim_file);

		// now running an anim, looping back to this 'FN' call again
		ob_logic->looping = 1;
		ob_graphic->anim_resource = res;

		if (reverse)
			ob_graphic->anim_pc = anim_head->noAnimFrames - 1;
		else
			ob_graphic->anim_pc = 0;
 	} else if (getSync() != -1) {
		// We've received a sync - return to script immediately
		debug(5, "**sync stopped %d**", _scriptVars[ID]);

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

	int32 pars[3];

	// Set up the parameters for animate().

	pars[0] = params[0];
	pars[1] = params[1];

	// If this is the start of the anim, read the anim table to get the
	// appropriate anim resource

 	ObjectLogic *ob_logic = (ObjectLogic *) decodePtr(params[0]);

	if (ob_logic->looping == 0) {
	 	ObjectMega *ob_mega = (ObjectMega *) decodePtr(params[2]);
		uint32 *anim_table = (uint32 *) decodePtr(params[3]);

		// appropriate anim resource is in 'table[direction]'
		pars[2] = anim_table[ob_mega->current_dir];
	}

	return animate(pars, reverse);
}

void Logic::setSpriteStatus(uint32 sprite, uint32 type) {
	ObjectGraphic *ob_graphic = (ObjectGraphic *) decodePtr(sprite);

	// Remove the previous status, but don't affect the shading upper-word
	ob_graphic->type = (ob_graphic->type & 0xffff0000) | type;
}

void Logic::setSpriteShading(uint32 sprite, uint32 type) {
	ObjectGraphic *ob_graphic = (ObjectGraphic *) decodePtr(sprite);

	// Remove the previous shading, but don't affect the status lower-word.
	// Note that drivers may still shade mega frames automatically, even
	// when not sent 'RDSPR_SHADOW'.
	ob_graphic->type = (ob_graphic->type & 0x0000ffff) | type;
}

void Logic::createSequenceSpeech(MovieTextObject *sequenceText[]) {
	uint32 line;
 	FrameHeader *frame;
 	uint32 local_text;
	uint32 text_res;
	byte *text;
	uint32 wavId;	// ie. offical text number (actor text number)
	bool speechRunning;

	// for each sequence text line that's been logged
	for (line = 0; line < _sequenceTextLines; line++) {
		// allocate this structure
		sequenceText[line] = new MovieTextObject;

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
			_sequenceTextList[line].speechBufferSize = _vm->_sound->preFetchCompSpeech(wavId, &_sequenceTextList[line].speech_mem);
			if (_sequenceTextList[line].speechBufferSize) {
				// ok, we've got speech!
				speechRunning = true;
			}
		}

		// if we want subtitles, or speech failed to load

		if (_vm->getSubtitles() || !speechRunning) {
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
	// MovieTextObject's
	sequenceText[_sequenceTextLines] = NULL;

	for (line = 0; line < _sequenceTextLines; line++) {
		// if we've made a text sprite for this line...

		if (_sequenceTextList[line].text_mem) {
			// now fill out the SpriteInfo structure in the
			// MovieTextObjectStructure

			frame = (FrameHeader *) _sequenceTextList[line].text_mem;

			sequenceText[line]->textSprite = new SpriteInfo;

			// center text at bottom of screen
			sequenceText[line]->textSprite->x = 320 - frame->width / 2;
			sequenceText[line]->textSprite->y = 440 - frame->height;
			sequenceText[line]->textSprite->w = frame->width;
			sequenceText[line]->textSprite->h = frame->height;
			sequenceText[line]->textSprite->type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION;
			sequenceText[line]->textSprite->data = _sequenceTextList[line].text_mem + sizeof(FrameHeader);
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

void Logic::clearSequenceSpeech(MovieTextObject *sequenceText[]) {
	for (uint i = 0; i < _sequenceTextLines; i++) {
		// free up the memory used by this MovieTextObject
		delete sequenceText[i];

		// free up the mem block containing this text sprite
		if (_sequenceTextList[i].text_mem)
			free(_sequenceTextList[i].text_mem);

		// free up the mem block containing this speech sample
		if (_sequenceTextList[i].speech_mem)
			free(_sequenceTextList[i].speech_mem);
	}

	// IMPORTANT! Reset the line count ready for the next sequence!
	_sequenceTextLines = 0;
}

} // End of namespace Sword2
