/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2006 The ScummVM project
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
 * $URL$
 * $Id$
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
#include "sword2/router.h"
#include "sword2/sound.h"
#include "sword2/animation.h"

namespace Sword2 {

int Router::doAnimate(byte *ob_logic, byte *ob_graph, int32 animRes, bool reverse) {
	AnimHeader anim_head;
	byte *anim_file;

	ObjectLogic obLogic(ob_logic);
	ObjectGraphic obGraph(ob_graph);

	if (obLogic.getLooping() == 0) {
		byte *ptr;

		// This is the start of the anim - set up the first frame

		// For testing all anims!
		// A script loop can send every resource number to the anim
		// function & it will only run the valid ones. See
		// 'testing_routines' object in George's Player Character
		// section of linc

		if (_vm->_logic->readVar(SYSTEM_TESTING_ANIMS)) {
			if (!_vm->_resman->checkValid(animRes)) {
				// Not a valid resource number. Switch off
				// the sprite. Don't animate - just continue
				// script next cycle.
				setSpriteStatus(ob_graph, NO_SPRITE);
				return IR_STOP;
			}

			ptr = _vm->_resman->openResource(animRes);

			// if it's not an animation file
			if (_vm->_resman->fetchType(animRes) != ANIMATION_FILE) {
				_vm->_resman->closeResource(animRes);

				// switch off the sprite
				// don't animate - just continue
				// script next cycle
				setSpriteStatus(ob_graph, NO_SPRITE);
				return IR_STOP;
			}

			_vm->_resman->closeResource(animRes);

			// switch on the sprite
			setSpriteStatus(ob_graph, SORT_SPRITE);
		}

		assert(animRes);

		// open anim file
		anim_file = _vm->_resman->openResource(animRes);

		assert(_vm->_resman->fetchType(animRes) == ANIMATION_FILE);

		// point to anim header
		anim_head.read(_vm->fetchAnimHeader(anim_file));

		// now running an anim, looping back to this call again
		obLogic.setLooping(1);
		obGraph.setAnimResource(animRes);

		if (reverse)
			obGraph.setAnimPc(anim_head.noAnimFrames - 1);
		else
			obGraph.setAnimPc(0);
	} else if (_vm->_logic->getSync() != -1) {
		// We've received a sync - return to script immediately
		debug(5, "**sync stopped %d**", _vm->_logic->readVar(ID));

		// If sync received, anim finishes right now (remaining on
		// last frame). Quit animation, but continue script.
		obLogic.setLooping(0);
		return IR_CONT;
	} else {
		// Not first frame, and no sync received - set up the next
		// frame of the anim.

		// open anim file and point to anim header
		anim_file = _vm->_resman->openResource(obGraph.getAnimResource());
		anim_head.read(_vm->fetchAnimHeader(anim_file));

		if (reverse)
			obGraph.setAnimPc(obGraph.getAnimPc() - 1);
		else
			obGraph.setAnimPc(obGraph.getAnimPc() + 1);
	}

	// check for end of anim

	if (reverse) {
		if (obGraph.getAnimPc() == 0)
			obLogic.setLooping(0);
	} else {
		if (obGraph.getAnimPc() == anim_head.noAnimFrames - 1)
			obLogic.setLooping(0);
	}

	// close the anim file
	_vm->_resman->closeResource(obGraph.getAnimResource());

	// check if we want the script to loop back & call this function again
	return obLogic.getLooping() ? IR_REPEAT : IR_STOP;
}

int Router::megaTableAnimate(byte *ob_logic, byte *ob_graph, byte *ob_mega, byte *animTable, bool reverse) {
	int32 animRes = 0;

	// If this is the start of the anim, read the anim table to get the
	// appropriate anim resource

	ObjectLogic obLogic(ob_logic);

	if (obLogic.getLooping() == 0) {
		ObjectMega obMega(ob_mega);

		// Appropriate anim resource is in 'table[direction]'
		animRes = READ_LE_UINT32(animTable + 4 * obMega.getCurDir());
	}

	return doAnimate(ob_logic, ob_graph, animRes, reverse);
}

void Router::setSpriteStatus(byte *ob_graph, uint32 type) {
	ObjectGraphic obGraph(ob_graph);

	// Remove the previous status, but don't affect the shading upper-word
	obGraph.setType((obGraph.getType() & 0xffff0000) | type);
}

void Router::setSpriteShading(byte *ob_graph, uint32 type) {
	ObjectGraphic obGraph(ob_graph);

	// Remove the previous shading, but don't affect the status lower-word.
	// Note that mega frames may still be shaded automatically, even when
	// not sent 'RDSPR_SHADOW'.
	obGraph.setType((obGraph.getType() & 0x0000ffff) | type);
}

void Logic::createSequenceSpeech(MovieTextObject *sequenceText[]) {
	uint32 line;
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
		wavId = (int32)READ_LE_UINT16(text);

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
			FrameHeader frame;

			frame.read(_sequenceTextList[line].text_mem);

			sequenceText[line]->textSprite = new SpriteInfo;

			// center text at bottom of screen
			sequenceText[line]->textSprite->x = 320 - frame.width / 2;
			sequenceText[line]->textSprite->y = 440 - frame.height;
			sequenceText[line]->textSprite->w = frame.width;
			sequenceText[line]->textSprite->h = frame.height;
			sequenceText[line]->textSprite->type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION;
			sequenceText[line]->textSprite->data = _sequenceTextList[line].text_mem + FrameHeader::size();
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
