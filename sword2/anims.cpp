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

#include <stdio.h>		// for sprintf

#ifdef _SWORD2_DEBUG
#include <string.h>		// for strlen
#endif

#include "stdafx.h"
#include "common/scummsys.h"
#include "driver/driver96.h"
#include "anims.h"
#include "console.h"
#include "controls.h"		// for 'speechSelected' & 'subtitles'
#include "defs.h"
#include "header.h"
#include "interpreter.h"
#include "maketext.h"		// for MakeTextSprite used by FN_play_sequence ultimately
#include "object.h"
#include "protocol.h"
#include "resman.h"
#include "sword2.h"
#include "sync.h"
#include "sound.h"		// for Speech stuff.

// stores resource id of wav to use as lead-out from smacker
uint32 smackerLeadOut = 0;

int32 Animate(int32 *params, uint8 reverse_flag);
int32 Mega_table_animate(int32 *params, uint8 reverse_flag);

int32 FN_anim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 resource id of animation file

	// 0 means normal forward anim
	return Animate(params, 0);
}

int32 FN_reverse_anim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 resource id of animation file

	// 1 means reverse anim
	return Animate(params, 1);
}

int32 FN_mega_table_anim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to animation table

	// 0 means normal forward anim
	return Mega_table_animate(params, 0);
}

int32 FN_reverse_mega_table_anim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to animation table

	// 1 means reverse anim
	return Mega_table_animate(params, 1);
}

int32 Animate(int32 *params, uint8 reverse_flag) {
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

	ob_logic = (Object_logic *) params[0];
	ob_graphic = (Object_graphic *) params[1];
	
	if (ob_logic->looping == 0) {
		// This is the start of the anim - set up the first frame

#ifdef _SWORD2_DEBUG
		// For testing all anims!	(James18apr97)
		// A script loop can send every resource number to the anim
		// function & it will only run the valid ones. See
		// 'testing_routines' object in George's Player Character
		// section of linc

		if (SYSTEM_TESTING_ANIMS) {
			// if the resource number is within range & it's not
			// a null resource

			if (res_man.Res_check_valid(res)) {
				// Open the resource. Can close it immediately.
				// We've got a pointer to the header.
				head = (_standardHeader *) res_man.open(res);
				res_man.close(res);

				// if it's not an animation file
				if (head->fileType != ANIMATION_FILE) {
					// switch off the sprite
					// don't animate - just continue
					// script next cycle
					FN_no_sprite(params + 1);
					return IR_STOP;
				}
			} else {
				// Not a valid resource number. Switch off
				// the sprite. Don't animate - just continue
				// script next cycle.
				FN_no_sprite(params + 1);
				return IR_STOP;
			}

			// switch on the sprite
			FN_sort_sprite(params + 1);
		}
#endif

#ifdef _SWORD2_DEBUG
		// check that we haven't been passed a zero resource number
		if (res == 0)
			Con_fatal_error("Animate: %s (id %d) passed zero anim resource", FetchObjectName(ID), ID);
#endif

		// open anim file
		anim_file = res_man.open(res);

#ifdef _SWORD2_DEBUG
		// check this this resource is actually an animation file!
		head = (_standardHeader *) anim_file;
		if (head->fileType != ANIMATION_FILE)
			Con_fatal_error("Animate: %s (%d) is not an anim!", FetchObjectName(res), res);
#endif

		// point to anim header
		anim_head = FetchAnimHeader(anim_file);

/* #ifdef _SWORD2_DEBUG
		// check there's at least one frame
		if (anim_head->noAnimFrames == 0)
 			Con_fatal_error("Animate: %s (%d) has zero frame count!", FetchObjectName(res), res);
#endif */

		// now running an anim, looping back to this 'FN' call again
		ob_logic->looping = 1;
		ob_graphic->anim_resource = res;

		if (reverse_flag)
			ob_graphic->anim_pc = anim_head->noAnimFrames - 1;
		else
			ob_graphic->anim_pc = 0;
 	} else if (Get_sync()) {
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
		anim_file = res_man.open(ob_graphic->anim_resource);
		anim_head = FetchAnimHeader(anim_file);

		if (reverse_flag)
			ob_graphic->anim_pc--;
		else
			ob_graphic->anim_pc++;
	}

	// check for end of anim

	if (reverse_flag) {
		if (ob_graphic->anim_pc == 0)
			ob_logic->looping = 0;
	} else {
		if (ob_graphic->anim_pc == (int32) (anim_head->noAnimFrames - 1))
			ob_logic->looping = 0;
	}

	// close the anim file
	res_man.close(ob_graphic->anim_resource);

	// check if we want the script to loop back & call this function again
	return ob_logic->looping ? IR_REPEAT : IR_STOP;
}

int32 Mega_table_animate(int32 *params, uint8 reverse_flag) {
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

	ob_logic = (Object_logic *) params[0];

	if (ob_logic->looping == 0) {
	 	ob_mega = (Object_mega *) params[2];
		anim_table = (uint32 *) params[3];

		// appropriate anim resource is in 'table[direction]'
		pars[2] = anim_table[ob_mega->current_dir];
	}

	// set up the rest of the parameters for FN_anim()

	pars[0] = params[0];
	pars[1] = params[1];

	// pars[2] only needed setting at the start of the anim

	// call Animate() with these params
	return Animate(pars, reverse_flag);
}

int32 FN_set_frame(int32 *params) {
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
		Con_fatal_error("FN_set_frame: %s (id %d) passed zero anim resource", FetchObjectName(ID), ID);
#endif

	// open the resource (& check it's valid)

	anim_file = res_man.open(res);

#ifdef _SWORD2_DEBUG
	// check this this resource is actually an animation file!
	head = (_standardHeader *) anim_file;
	if (head->fileType != ANIMATION_FILE)
		Con_fatal_error("FN_set_frame: %s (%d) is not an anim!", FetchObjectName(res), res);
#endif

	// set up pointer to the animation header
	anim_head = FetchAnimHeader(anim_file);

/* #ifdef _SWORD2_DEBUG
	// check there's at least one frame
	if (anim_head->noAnimFrames == 0)
		Con_fatal_error("FN_set_frame: %s (%d) has zero frame count!", FetchObjectName(res), res);
#endif */

	// set up anim resource in graphic object

	ob_graphic = (Object_graphic *) params[0];
	ob_graphic->anim_resource = res;

	if (params[2])
		ob_graphic->anim_pc = anim_head->noAnimFrames - 1;
	else
		ob_graphic->anim_pc = 0;

	// Close the anim file and drop out of script

	res_man.close(ob_graphic->anim_resource);
	return IR_CONT;
}

int32 FN_no_sprite(int32 *params) {
	// params	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0xffff0000;
	ob_graphic->type |= NO_SPRITE;

	// continue script
	return IR_CONT;
}

int32 FN_back_par0_sprite(int32 *params) {
	// params	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0xffff0000;
	ob_graphic->type |= BGP0_SPRITE;

	// continue script
	return IR_CONT;
}

int32 FN_back_par1_sprite(int32 *params) {
	// params	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0xffff0000;
	ob_graphic->type |= BGP1_SPRITE;

	// continue script
	return IR_CONT;
}

int32 FN_back_sprite(int32 *params) {
	// params	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0xffff0000;
	ob_graphic->type |= BACK_SPRITE;

	// continue script
	return IR_CONT;
}

int32 FN_sort_sprite(int32 *params) {
	// params	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0xffff0000;
	ob_graphic->type |= SORT_SPRITE;

	// continue script
	return IR_CONT;
}

int32 FN_fore_sprite(int32 *params) {
	// params	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0xffff0000;
	ob_graphic->type |= FORE_SPRITE;

	// continue script
	return IR_CONT;
}

int32 FN_fore_par0_sprite(int32 *params) {
	// params	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0xffff0000;
	ob_graphic->type |= FGP0_SPRITE;

	// continue script
	return IR_CONT;
}

int32 FN_fore_par1_sprite(int32 *params) {
	// params	0 pointer to object's graphic structure

	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0xffff0000;
	ob_graphic->type |= FGP1_SPRITE;

	// continue script
	return IR_CONT;
}

int32 FN_shaded_sprite(int32 *params) {
	// params	0 pointer to object's graphic structure

	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type &= 0x0000ffff;
	ob_graphic->type |= SHADED_SPRITE;

	// note that drivers may still shade mega frames automatically, even
	// when not sent 'RDSPR_SHADOW'

	// continue script
	return IR_CONT;
}

int32 FN_unshaded_sprite(int32 *params) {
	// params	0 pointer to object's graphic structure

	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

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

// FIXME: At the moment, the subtitle sprites and voice-overs are pre-cached.
// The code would be much simpler if we simply stored the text/speech ids and
// decoded them while the movie is playing. On the other hand, it might slow
// down the video playback too much. We'll need to investigate...

//int32 PlaySmacker(char *filename, _movieTextObject *textObjects[]);

//	typedef struct {
//		uint16 startFrame;
//		uint16 endFrame;
//		_spriteInfo *textSprite;
//		_wavHeader *speech;
//	} _movieTextObject;

// FOR TEXT LINES IN SEQUENCE PLAYER

#define MAX_SEQUENCE_TEXT_LINES 15

typedef struct {
	uint32 textNumber;
	uint16 startFrame;
	uint16 endFrame;
	mem *text_mem;
	uint32 speechBufferSize;
	uint16 *speech_mem;
} _sequenceTextInfo;

// keeps count of number of text lines to disaply during the sequence
uint32 sequenceTextLines = 0;

static _sequenceTextInfo sequence_text_list[MAX_SEQUENCE_TEXT_LINES];

int32 FN_add_sequence_text(int32 *params) {
//	params	0	text number
//		1	frame number to start the text displaying
//		2	frame number to stop the text dispalying

#ifdef _SWORD2_DEBUG
	if (sequenceTextLines == MAX_SEQUENCE_TEXT_LINES)
		Con_fatal_error("FN_add_sequence_text ran out of lines");
#endif

	sequence_text_list[sequenceTextLines].textNumber = params[0];
	sequence_text_list[sequenceTextLines].startFrame = params[1];
	sequence_text_list[sequenceTextLines].endFrame = (uint16) params[2];
	sequenceTextLines++;

	// continue script
	return IR_CONT;
}

void CreateSequenceSpeech(_movieTextObject *sequenceText[]) {
	uint32 line;
 	_frameHeader *frame;
 	uint32 local_text;
	uint32 text_res;
	uint8 *text;
	uint32 wavId;	// ie. offical text number (actor text number)
	uint8 speechRunning;
 	char speechFile[256];

	// for each sequence text line that's been logged
	for (line = 0; line < sequenceTextLines; line++) {
		// allocate this structure
		sequenceText[line] = new _movieTextObject;

		sequenceText[line]->startFrame = sequence_text_list[line].startFrame;
		sequenceText[line]->endFrame = sequence_text_list[line].endFrame;

		// pull out the text line to get the official text number
		// (for wav id)

  		text_res = sequence_text_list[line].textNumber / SIZE;
		local_text = sequence_text_list[line].textNumber & 0xffff;

		// open text resource & get the line
		text = FetchTextLine(res_man.open(text_res), local_text);
		wavId = (int32) READ_LE_UINT16(text);

		// now ok to close the text file
		res_man.close(text_res);

		// 1st word of text line is the official line number
		debug(5,"(%d) SEQUENCE TEXT: %s", *(uint16 *) text, text + 2);

		// is it to be speech or subtitles or both?
		// assume speech is not running until know otherwise

		speechRunning = 0;
		sequence_text_list[line].speech_mem = NULL;
		sequenceText[line]->speech = NULL;

		if (speechSelected) {
			// speech is selected, so try that first

			// set up path to speech cluster
			// first checking if we have speech1.clu or
			// speech2.clu in current directory (for translators
			// to test)

			File fp;

			sprintf(speechFile, "speech%d.clu", res_man.whichCd());
			if (fp.open(speechFile))
				fp.close();
			else
				strcpy(speechFile, "speech.clu");

			sequence_text_list[line].speechBufferSize = g_sound->PreFetchCompSpeech((char *) speechFile, wavId, &sequence_text_list[line].speech_mem);
			if (sequence_text_list[line].speechBufferSize) {
				// ok, we've got speech!
				speechRunning = 1;
			}
		}

		// if we want subtitles, or speech failed to load

		if (subtitles || !speechRunning) {
			// open text resource & get the line
			text = FetchTextLine(res_man.open(text_res), local_text);
			// make the sprite
			// 'text+2' to skip the first 2 bytes which form the
			// line reference number

			// NB. The mem block containing the text sprite is
			// currently FLOATING!

			sequence_text_list[line].text_mem = MakeTextSprite(text + 2, 600, 255, speech_font_id);

			// ok to close the text resource now
			res_man.close(text_res);
		} else {
			sequence_text_list[line].text_mem = NULL;
			sequenceText[line]->textSprite = NULL;
		}
	}

	// for drivers: NULL-terminate the array of pointers to
	// _movieTextObject's
	sequenceText[sequenceTextLines] = NULL;

  	// now lock all the memory blocks containing text sprites & speech
	// samples and set up the pointers to them, etc, for the drivers

	for (line = 0; line < sequenceTextLines; line++) {
		// if we've made a text sprite for this line...

		if (sequence_text_list[line].text_mem) {
			Lock_mem(sequence_text_list[line].text_mem);

			// now fill out the _spriteInfo structure in the
			// _movieTextObjectStructure

			frame = (_frameHeader *) sequence_text_list[line].text_mem->ad;

			sequenceText[line]->textSprite = new _spriteInfo;

			// center text at bottom of screen
			sequenceText[line]->textSprite->x = 320 - frame->width / 2;
			sequenceText[line]->textSprite->y = 440 - frame->height;
			sequenceText[line]->textSprite->w = frame->width;
			sequenceText[line]->textSprite->h = frame->height;
			sequenceText[line]->textSprite->type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION;
			sequenceText[line]->textSprite->data = sequence_text_list[line].text_mem->ad + sizeof(_frameHeader);
		}

		// if we've loaded a speech sample for this line...

 		if (sequence_text_list[line].speech_mem) {
			// for drivers: set up pointer to decompressed wav in
			// memory

			sequenceText[line]->speechBufferSize = sequence_text_list[line].speechBufferSize;
			sequenceText[line]->speech = sequence_text_list[line].speech_mem;
		}
	}
}

void ClearSequenceSpeech(_movieTextObject *textSprites[]) {
	uint32 line;

	for (line = 0; line < sequenceTextLines; line++) {
		// free up the memory used by this _movieTextObject
		delete textSprites[line];

		// free up the mem block containing this text sprite
		if (sequence_text_list[line].text_mem)
			Free_mem(sequence_text_list[line].text_mem);

		// free up the mem block containing this speech sample
		if (sequence_text_list[line].speech_mem)
			free(sequence_text_list[line].speech_mem);
	}

	// IMPORTANT! Reset the line count ready for the next sequence!
	sequenceTextLines = 0;
}

int32 FN_smacker_lead_in(int32 *params) {
	uint8 *leadIn;
	uint32 rv;
#ifdef _SWORD2_DEBUG
	_standardHeader *header;
#endif

	leadIn = res_man.open(params[0]);

#ifdef _SWORD2_DEBUG
	header = (_standardHeader *) leadIn;
	if (header->fileType != WAV_FILE)
		Con_fatal_error("FN_smacker_lead_in() given invalid resource");
#endif

	leadIn += sizeof(_standardHeader);
	// wav data gets copied to sound memory
	rv = g_sound->PlayFx(0, leadIn, 0, 0, RDSE_FXLEADIN);

	if (rv)
		debug(5, "SFX ERROR: PlayFx() returned %.8x", rv);

	res_man.close(params[0]);

	// fade out any music that is currently playing (James22july97)
	FN_stop_music(NULL);

	// continue script
	return IR_CONT;
}

int32 FN_smacker_lead_out(int32 *params) {
	// ready for use in FN_play_sequence
	smackerLeadOut = params[0];

	// continue script
	return IR_CONT;
}

int32 FN_play_sequence(int32 *params) {
	// params	0 pointer to null-terminated ascii filename
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

	debug(5, "FN_play_sequence(\"%s\");", params[0]);

#ifdef _SWORD2_DEBUG
	// check that the name paseed from script is 8 chars or less
	if (strlen((char *) params[0]) > 8)
		Con_fatal_error("Sequence filename too long");
#endif

	// add the appropriate file extension & play it

	sprintf(filename, "%s.smk", (char *) params[0]);

	// Write to walkthrough file (zebug0.txt)
 	debug(5, "PLAYING SEQUENCE \"%s\"", filename);

	// now create the text sprites, if any (James27may97)

	if (sequenceTextLines)
		CreateSequenceSpeech(sequenceSpeechArray);

	// open the lead-out music resource, if there is one

	if (smackerLeadOut) {
		leadOut = res_man.open(smackerLeadOut);

#ifdef _SWORD2_DEBUG
		header = (_standardHeader *)leadOut;
		if (header->fileType != WAV_FILE)
			error("FN_smacker_lead_out() given invalid resource");
#endif

		leadOut += sizeof(_standardHeader);
	}

	// play the smacker

	// don't want to carry on streaming game music when smacker starts!
	FN_stop_music(NULL);

	// pause sfx during sequence, except the one used for lead-in music
	g_sound->PauseFxForSequence();

	if (sequenceTextLines && g_sword2->_gameId == GID_SWORD2)
		rv = PlaySmacker(filename, sequenceSpeechArray, leadOut);
	else
		rv = PlaySmacker(filename, NULL, leadOut);

	// unpause sound fx again, in case we're staying in same location
	g_sound->UnpauseFx();

	// close the lead-out music resource

 	if (smackerLeadOut) {
		res_man.close(smackerLeadOut);
		smackerLeadOut = 0;
	}

	// check the error return-value
	if (rv)
		debug(5, "PlaySmacker(\"%s\") returned 0x%.8x", filename, rv);

	// now clear the text sprites, if any (James27may97)

	if (sequenceTextLines)
		ClearSequenceSpeech(sequenceSpeechArray);

	// now clear the screen in case the Sequence was quitted (using ESC)
	// rather than fading down to black

	EraseBackBuffer();

	// zero the entire palette in case we're about to fade up!

	_palEntry  pal[256];

	memset(pal, 0, 256 * sizeof(_palEntry));
	BS2_SetPalette(0, 256, (uint8 *) pal, RDPAL_INSTANT);

	debug(5, "FN_play_sequence FINISHED");

	// continue script
	return IR_CONT;
}
