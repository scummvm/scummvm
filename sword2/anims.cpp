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

//-------------------------------------------------------------------------------------------------------------
// A more intelligent version of the old ANIMS.C
// All this stuff by James
// DON'T TOUCH!
//-------------------------------------------------------------------------------------------------------------

//#include <io.h>		// for access()
#include <stdio.h>	// for sprintf
#include <string.h>

#include "stdafx.h"
#include "common/scummsys.h"
#include "driver/driver96.h"
#include "anims.h"
#include "build_display.h"	// for DisplayMsg()
#include "console.h"
#include "controls.h"	// for 'speechSelected' & 'subtitles'
#include "debug.h"
#include "defs.h"
#include "header.h"
#include "interpreter.h"
#include "layers.h"	// for 'this_screen' structure - for restoring palette in FN_play_sequence
#include "maketext.h"	// for MakeTextSprite used by FN_play_sequence ultimately
#include "object.h"
#include "protocol.h"
#include "resman.h"
#include "sword2.h"		// for Close_game()
#include "sync.h"
#include "sound.h"		// for Speech stuff.

//-------------------------------------------------------------------------------------------------------------
uint32 smackerLeadOut=0;	// stores resource id of wav to use as lead-out from smacker

//-------------------------------------------------------------------------------------------------------------
// local function prototypes

int32 Animate(int32 *params, uint8 reverse_flag);
int32 Mega_table_animate(int32 *params, uint8 reverse_flag);

//-------------------------------------------------------------------------------------------------------------
int32 FN_anim(int32 *params)
{
	// params:	0 pointer to object's logic structure
	//			1 pointer to object's graphic structure
	//			2 resource id of animation file

	return Animate(params,0);	// 0 means normal forward anim
}
//-------------------------------------------------------------------------------------------------------------
int32 FN_reverse_anim(int32 *params)
{
	// params:	0 pointer to object's logic structure
	//			1 pointer to object's graphic structure
	//			2 resource id of animation file

	return Animate(params,1);	// 1 means reverse anim
}
//-------------------------------------------------------------------------------------------------------------
int32 FN_mega_table_anim(int32 *params)
{
	// params:	0 pointer to object's logic structure
	//			1 pointer to object's graphic structure
	//			2 pointer to object's mega structure
	//			3 pointer to animation table

	return Mega_table_animate(params,0);	// 0 means normal forward anim
}
//-------------------------------------------------------------------------------------------------------------
int32 FN_reverse_mega_table_anim(int32 *params)
{
	// params:	0 pointer to object's logic structure
	//			1 pointer to object's graphic structure
	//			2 pointer to object's mega structure
	//			3 pointer to animation table

	return Mega_table_animate(params,1);	// 1 means reverse anim
}
//-------------------------------------------------------------------------------------------------------------
int32 Animate(int32 *params, uint8 reverse_flag)
{
	// params:	0 pointer to object's logic structure
	//			1 pointer to object's graphic structure
	//			2 resource id of animation file

 	Object_logic	*ob_logic;
 	Object_graphic	*ob_graphic;
	uint8			*anim_file;
	_animHeader		*anim_head;
 	int32	res = params[2];

	#ifdef _SWORD2_DEBUG
	_standardHeader	*head;	// for animation testing & checking for correct file type
	#endif

	//----------------------------------------------------------------------------------------
	// read the main parameters

	ob_logic	= (Object_logic *) params[0];	// param 0 is pointer to normal structure
	ob_graphic	= (Object_graphic *) params[1];	// param 1 is pointer to graphic structure
	
	//----------------------------------------------------------------------------------------
	// if this is the start of the anim, set up the first frame

	if (ob_logic->looping==0)
	{
		//-----------------------------------------------------------
		#ifdef _SWORD2_DEBUG
		// For testing all anims!	(James18apr97)
		// A script loop can send every resource number to the anim function
		// & it will only run the valid ones
		// See 'testing_routines' object in George's Player Character section of linc
		if (SYSTEM_TESTING_ANIMS)
		{
			if (res_man.Res_check_valid(res))	// if the resource number is within range & it's not a null resource
			{
				head = (_standardHeader*) res_man.Res_open(res);	// open the resource
				res_man.Res_close(res);								// can close immediately - we've got a pointer to the header

				if (head->fileType!=ANIMATION_FILE)					// if it's not an animation file
				{
					FN_no_sprite(params+1);							// switch off the sprite
					return(IR_STOP);								// don't animate - just continue script next cycle
				}
			}
			else
			{														// not a valid resource number
				FN_no_sprite(params+1);								// switch off the sprite
				return(IR_STOP);									// don't animate - just continue script next cycle
			}

			FN_sort_sprite(params+1);								// switch on the sprite
		}
		#endif
		//-----------------------------------------------------------

		//---------------------
		#ifdef _SWORD2_DEBUG
		// check that we haven't been passed a zero resource number
		if (res==0)
			Con_fatal_error("Animate: %s (id %d) passed zero anim resource (%s line %u)", FetchObjectName(ID), ID, __FILE__, __LINE__);
		#endif
  		//---------------------

		anim_file = res_man.Res_open(res);	// open anim file

		//---------------------
		#ifdef _SWORD2_DEBUG
		// check this this resource is actually an animation file!
		head = (_standardHeader*) anim_file;
		if (head->fileType!=ANIMATION_FILE)	// if it's not an animation file
			Con_fatal_error("Animate: %s (%d) is not an anim! (%s line %u)", FetchObjectName(res), res, __FILE__, __LINE__);
		#endif
		//---------------------

		anim_head = FetchAnimHeader( anim_file );					// point to anim header

		//---------------------
		/*
		#ifdef _SWORD2_DEBUG
		// check there's at least one frame
		if (anim_head->noAnimFrames==0)
 			Con_fatal_error("Animate: %s (%d) has zero frame count! (%s line %u)", FetchObjectName(res), res, __FILE__, __LINE__);
		#endif
		*/
		//---------------------

		ob_logic->looping		  = 1;			// now running an anim, looping back to this 'FN' call again
		ob_graphic->anim_resource = res;	// param 2 is id of animation resource

		if (reverse_flag)	// if a reverse anim
			ob_graphic->anim_pc = anim_head->noAnimFrames-1;		// start on last frame
		else				// forward anim
			ob_graphic->anim_pc = 0;								// start on first frame
 	}
 	//-------------------------------------------------------------------------------------------------------
	// otherwise, if we've received a sync, return to script immediately

	else if (Get_sync())			// returns sync value if one has been sent to current 'id', otherwise 0
	{
//		Zdebug("**sync stopped %d**", ID);
		ob_logic->looping = 0;		// if sync received, anim finishes right now (remaining on last frame)
		return(IR_CONT);			// quit anim but continue script
	}
 	//-------------------------------------------------------------------------------------------------------
	// otherwise (not first frame & not received a sync), set up the next frame of the anim
	else
	{
		anim_file = res_man.Res_open(ob_graphic->anim_resource);	// open anim file
		anim_head = FetchAnimHeader( anim_file );					// point to anim header

		if (reverse_flag)					// if a reverse anim
			ob_graphic->anim_pc--;			// decrement the anim frame number
		else								// normal forward anim
			ob_graphic->anim_pc++;			// increment the anim frame number
	}
   	//-------------------------------------------------------------------------------------------------------
	// check for end of anim

	if (reverse_flag)					// if a reverse anim
	{
		if (ob_graphic->anim_pc == 0)	// reached the first frame of the anim
			ob_logic->looping = 0;		// anim finishes on this frame
	}
	else								// normal forward anim
	{
		if (ob_graphic->anim_pc == (int32)(anim_head->noAnimFrames-1))	// reached the last frame of the anim
			ob_logic->looping = 0;		// anim finishes on this frame
	}
   	//-------------------------------------------------------------------------------------------------------
	// close the anim file

	res_man.Res_close(ob_graphic->anim_resource);	// close anim file

	//-------------------------------------------------------------------------------------------------------
	// check if we want the script to loop back & call this function again

	if (ob_logic->looping)
		return(IR_REPEAT);	// drop out of script, but call this function again next cycle
	else
		return(IR_STOP);	// drop out of script

	//-------------------------------------------------------------------------------------------------------
}
//-------------------------------------------------------------------------------------------------------------
int32 Mega_table_animate(int32 *params, uint8 reverse_flag)
{
	// params:	0 pointer to object's logic structure
	//			1 pointer to object's graphic structure
	//			2 pointer to object's mega structure
	//			3 pointer to animation table

 	Object_logic	*ob_logic;
 	Object_mega		*ob_mega;
	uint32			*anim_table;
	int32			pars[5];

	//----------------------------------------------------------------------------------------
	// if this is the start of the anim, read the anim table to get the appropriate anim resource

	ob_logic = (Object_logic *) params[0];			// param 0 is pointer to logic structure

	if (ob_logic->looping==0)
	{
	 	ob_mega = (Object_mega *) params[2];		// param 2 is pointer to mega structure

		anim_table = (uint32*)params[3];
		pars[2] = anim_table[ob_mega->current_dir];	// appropriate anim resource is in 'table[direction]'
	}

	//-------------------------------------------------------------------------------------------------------
	// set up the rest of the parameters for FN_anim()

	pars[0] = params[0];
	pars[1] = params[1];
	// pars[2] only needed setting at the start of the anim

	//-------------------------------------------------------------------------------------------------------

	return Animate(pars, reverse_flag);		// call Animate() with these params
}
//-------------------------------------------------------------------------------------------------------------
int32 FN_set_frame(int32 *params)
{
	// params:	0 pointer to object's graphic structure
	//			1 resource id of animation file
	//			2 frame flag (0=first 1=last)

 	Object_graphic	*ob_graphic;
	uint8			*anim_file;
	_animHeader		*anim_head;
	int32	res = params[1];

	#ifdef _SWORD2_DEBUG
	_standardHeader	*head;	// for checking for correct file type
	#endif

	//---------------------
	#ifdef _SWORD2_DEBUG
	// check that we haven't been passed a zero resource number
	if (res==0)
		Con_fatal_error("FN_set_frame: %s (id %d) passed zero anim resource (%s line %u)", FetchObjectName(ID), ID, __FILE__, __LINE__);
	#endif
	//---------------------

	//----------------------------------------------------------------------------------------
	// open the resource (& check it's valid)

	anim_file = res_man.Res_open(res);	// open anim file

	//---------------------
	#ifdef _SWORD2_DEBUG
	// check this this resource is actually an animation file!
	head = (_standardHeader*) anim_file;
	if (head->fileType!=ANIMATION_FILE)	// if it's not an animation file
		Con_fatal_error("FN_set_frame: %s (%d) is not an anim! (%s line %u)", FetchObjectName(res), res, __FILE__, __LINE__);
	#endif
	//---------------------

	anim_head = FetchAnimHeader( anim_file );	// set up pointer to the animation header

	//---------------------
	/*
	#ifdef _SWORD2_DEBUG
	// check there's at least one frame
	if (anim_head->noAnimFrames==0)
		Con_fatal_error("FN_set_frame: %s (%d) has zero frame count! (%s line %u)", FetchObjectName(res), res, __FILE__, __LINE__);
	#endif
	*/
	//---------------------

	//----------------------------------------------------------------------------------------
	// set up anim resource in graphic object

	ob_graphic	= (Object_graphic *) params[0];	// param 0 is pointer to the object's graphic structure

	ob_graphic->anim_resource = res;		// param 2 is id of animation resource

	//----------------------------------------------------------------------------------------

	if (params[2])	// frame flag is non-zero
		ob_graphic->anim_pc = anim_head->noAnimFrames-1;	// last frame
	else			// frame flag is 0
		ob_graphic->anim_pc = 0;							// first frame

 	//-------------------------------------------------------------------------------------------------------
	// close the anim file

	res_man.Res_close(ob_graphic->anim_resource);	// close anim file

	//-------------------------------------------------------------------------------------------------------

	return(IR_CONT);	//	drop out of script
}

//---------------------------------------------------------------------------------------------------------------------
int32 FN_no_sprite(int32 *params)	
{
	// params	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	ob_graphic->type &= 0xffff0000;		// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type |= NO_SPRITE;

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_back_par0_sprite(int32 *params)	
{
	// params	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	ob_graphic->type &= 0xffff0000;		// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type |= BGP0_SPRITE;

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_back_par1_sprite(int32 *params)	
{
	// params	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	ob_graphic->type &= 0xffff0000;		// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type |= BGP1_SPRITE;

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_back_sprite(int32 *params)	
{
	// params	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	ob_graphic->type &= 0xffff0000;		// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type |= BACK_SPRITE;

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_sort_sprite(int32 *params)	
{
	// params	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	ob_graphic->type &= 0xffff0000;		// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type |= SORT_SPRITE;

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_fore_sprite(int32 *params)	
{
	// params	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	ob_graphic->type &= 0xffff0000;		// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type |= FORE_SPRITE;

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_fore_par0_sprite(int32 *params)	
{
	// params	0 pointer to object's graphic structure

 	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	ob_graphic->type &= 0xffff0000;		// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type |= FGP0_SPRITE;

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_fore_par1_sprite(int32 *params)	
{
	// params	0 pointer to object's graphic structure

	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	ob_graphic->type &= 0xffff0000;		// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type |= FGP1_SPRITE;

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_shaded_sprite(int32 *params)	
{
	// params	0 pointer to object's graphic structure

	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	ob_graphic->type &= 0x0000ffff;		// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type |= SHADED_SPRITE;

	// note that drivers may still shade mega frames automatically, even when not sent 'RDSPR_SHADOW'

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_unshaded_sprite(int32 *params)	
{
	// params	0 pointer to object's graphic structure

	Object_graphic	*ob_graphic = (Object_graphic *) params[0];

	ob_graphic->type &= 0x0000ffff;		// remove previous status (but don't affect the shading upper-word)
	ob_graphic->type |= UNSHADED_SPRITE;

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
// Notes on PlaySmacker()

// 1st param is filename of sequence file
// 2nd param is a pointer to a null-terminated array of pointers to _movieTextObject structures


//int32 PlaySmacker(char *filename, _movieTextObject *textObjects[]);

//	typedef struct
//	{
//		uint16 startFrame;
//		uint16 endFrame;
//		_spriteInfo *textSprite;
//		_wavHeader *speech;
//	} _movieTextObject;

//---------------------------------------------------------------------------------------------------------------------
// FOR TEXT LINES IN SEQUENCE PLAYER (James22may97)

#define MAX_SEQUENCE_TEXT_LINES 15

typedef struct
{
	uint32	textNumber;
	uint16	startFrame;
	uint16	endFrame;
	mem		*text_mem;
	mem		*speech_mem;
}	_sequenceTextInfo;

static _sequenceTextInfo sequence_text_list[MAX_SEQUENCE_TEXT_LINES];
uint32 sequenceTextLines=0;		// keeps count of number of text lines to disaply during the sequence

//-------------------------------------------------------------------------------------------------------------

int32 FN_add_sequence_text(int32 *params)	// (James22may97)
{
//	params	0	text number
//			1	frame number to start the text displaying
//			2	frame number to stop the text dispalying

	#ifdef _SWORD2_DEBUG
	if (sequenceTextLines == MAX_SEQUENCE_TEXT_LINES)
		Con_fatal_error("FN_add_sequence_text ran out of lines (%s line %u)",__FILE__,__LINE__);
	#endif

	sequence_text_list[sequenceTextLines].textNumber = params[0];
	sequence_text_list[sequenceTextLines].startFrame = params[1];
	sequence_text_list[sequenceTextLines].endFrame	= (uint16) params[2];
	sequenceTextLines++;

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------

// speech sample code added by James on 16july97
void CreateSequenceSpeech(_movieTextObject *sequenceText[])	// (James23may97)
{
	uint32	line;
 	_frameHeader *frame;
 	uint32	local_text;
	uint32	text_res;
	uint8	*text;
	int16	wavId;	// ie. offical text number (actor text number)
	uint8	speechRunning;
 	char	speechFile[256];
	int32	wavSize;


	for (line=0; line < sequenceTextLines; line++)	// for each sequence text line that's been logged
	{
		sequenceText[line] = new _movieTextObject;	// allocate this structure

		sequenceText[line]->startFrame	= sequence_text_list[line].startFrame;
		sequenceText[line]->endFrame	= sequence_text_list[line].endFrame;

 		//-----------------------------------------------------------
		// pull out the text line to get the official text number (for wav id)

  		text_res	= sequence_text_list[line].textNumber/SIZE;
		local_text	= sequence_text_list[line].textNumber&0xffff;

		text = FetchTextLine( res_man.Res_open(text_res), local_text );	// open text resource & get the line
		memcpy(&wavId, text, 2);	// this works on PSX & PC

		res_man.Res_close(text_res);	// now ok to close the text file

		//--------------------------------------
		// Write to walkthrough file (zebug0.txt)
		#ifdef _SWORD2_DEBUG
		Zdebug(0,"(%d) SEQUENCE TEXT: %s", *(uint16*)text, text+2);	// 1st word of text line is the official line number
		#endif
		//--------------------------------------
		// is it to be speech or subtitles or both?

		speechRunning=0;	// assume not running until know otherwise
		sequence_text_list[line].speech_mem = NULL;
		sequenceText[line]->speech			= NULL;

		if (speechSelected)		// speech is selected, so try that first
		{
			//------------------------------
			// set up path to speech cluster
			// first checking if we have speech1.clu or speech2.clu in current directory (for translators to test)

			if (g_sword2->_gameId == GID_SWORD2_DEMO) {
				strcpy(speechFile,"SPEECH.CLU");
			} else {

				#ifdef _SWORD2_DEBUG
					if ((res_man.WhichCd()==1) && (!access("speech1.clu",0)))	// if 0 ie. if it's there
					{
						strcpy(speechFile,"speech1.clu");
					}
					else if ((res_man.WhichCd()==2) && (!access("speech2.clu",0)))	// if 0 ie. if it's there
					{
						strcpy(speechFile,"speech2.clu");
					}
					else
					#endif	// _SWORD2_DEBUG
					{
						strcpy(speechFile,res_man.GetCdPath());
						strcat(speechFile,"CLUSTERS\\SPEECH.CLU");
					}
			}
			//------------------------------

			wavSize = g_sword2->_sound->GetCompSpeechSize(speechFile, wavId);		// returns size of decompressed wav, or 0 if wav not found
			if (wavSize)	// if we've got the wav
			{
				// allocate memory for speech buffer
				sequence_text_list[line].speech_mem = Twalloc( wavSize, MEM_locked, UID_temp );	// last param is an optional id for type of mem block

				if (sequence_text_list[line].speech_mem)	// if mem allocated ok (should be fine, but worth checking)
				{
					if (g_sword2->_sound->PreFetchCompSpeech(speechFile, wavId, sequence_text_list[line].speech_mem->ad) == RD_OK)	// Load speech & decompress to our buffer
					{
						Float_mem (sequence_text_list[line].speech_mem);	// now float this buffer so we can make space for the next text sprites and/or speech samples
						speechRunning=1;	// ok, we've got speech!
					}
					else	// whoops, sample didn't load & decompress for some reason...
					{
						Free_mem (sequence_text_list[line].speech_mem);	// may as well free up this speech buffer now, rather than in ClearSequenceSpeech();
						sequence_text_list[line].speech_mem = NULL;		// so we know that it's free'd
					}
				}
			}
		}

		if (subtitles || (speechRunning==0))	// if we want subtitles, or speech failed to load
		{
			text = FetchTextLine( res_man.Res_open(text_res), local_text );	// open text resource & get the line
			// mem* MakeTextSprite( uint8 *sentence, uint16 maxWidth, uint8 pen, uint32 fontRes );
			sequence_text_list[line].text_mem = MakeTextSprite( text+2, 600, 255, speech_font_id );	// make the sprite
			// 'text+2' to skip the first 2 bytes which form the line reference number
			// NB. The mem block containing the text sprite is currently FLOATING!

			res_man.Res_close(text_res);	// ok to close the text resource now
		}
		else
		{
			sequence_text_list[line].text_mem	= NULL;
			sequenceText[line]->textSprite		= NULL;
		}
 		//--------------------------------------
	}

	sequenceText[sequenceTextLines] = NULL;	// for drivers: NULL-terminate the array of pointers to _movieTextObject's

	//---------------------------------------
  	// now lock all the memory blocks containing text sprites & speech samples
	// and set up the pointers to them, etc, for the drivers

	for (line=0; line < sequenceTextLines; line++)
	{
		// text sprites:
		if (sequence_text_list[line].text_mem)	// if we've made a text sprite for this line...
		{
			Lock_mem (sequence_text_list[line].text_mem);
			// now fill out the _spriteInfo structure in the _movieTextObjectStructure

			frame = (_frameHeader*) sequence_text_list[line].text_mem->ad;

			sequenceText[line]->textSprite = new _spriteInfo;

			sequenceText[line]->textSprite->x			= 320 - frame->width/2;	// centred
			sequenceText[line]->textSprite->y			= 440 - frame->height;	// at bottom of screen
			sequenceText[line]->textSprite->w			= frame->width;
			sequenceText[line]->textSprite->h			= frame->height;
			sequenceText[line]->textSprite->scale		= 0;
			sequenceText[line]->textSprite->scaledWidth	= 0;
			sequenceText[line]->textSprite->scaledHeight= 0;
			sequenceText[line]->textSprite->type		= RDSPR_DISPLAYALIGN+RDSPR_TRANS+RDSPR_NOCOMPRESSION;
			sequenceText[line]->textSprite->blend		= 0;
			sequenceText[line]->textSprite->data		= sequence_text_list[line].text_mem->ad+sizeof(_frameHeader);
			sequenceText[line]->textSprite->colourTable	= 0;
		}

		// speech samples:
 		if (sequence_text_list[line].speech_mem)	// if we've loaded a speech sample for this line...
		{
			Lock_mem (sequence_text_list[line].speech_mem);
			sequenceText[line]->speech = (_wavHeader *)sequence_text_list[line].speech_mem->ad;	// for drivers: set up pointer to decompressed wav in memory
		}
	}
	//---------------------------------------
}
//---------------------------------------------------------------------------------------------------------------------

// speech sample code added by James on 16july97
void ClearSequenceSpeech(_movieTextObject *textSprites[])	// (James27may97)
{
	uint32	line;

	for (line=0; line < sequenceTextLines; line++)
	{
		delete (textSprites[line]);		// free up the memory used by this _movieTextObject

		if (sequence_text_list[line].text_mem)
			Free_mem (sequence_text_list[line].text_mem);	// free up the mem block containing this text sprite

		if (sequence_text_list[line].speech_mem)
			Free_mem (sequence_text_list[line].speech_mem);	// free up the mem block containing this speech sample
	}

	sequenceTextLines=0;	// IMPORTANT! Reset the line count ready for the next sequence!
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_smacker_lead_in(int32 *params)	// James(21july97)
{
	uint8 *leadIn;
	uint32 rv;
#ifdef _SWORD2_DEBUG
	_standardHeader *header;
#endif


	leadIn = res_man.Res_open(params[0]);

	//-----------------------------------------
	#ifdef _SWORD2_DEBUG
	header = (_standardHeader*)leadIn;
	if (header->fileType != WAV_FILE)
		Con_fatal_error("FN_smacker_lead_in() given invalid resource (%s line %u)",__FILE__,__LINE__);
	#endif
	//-----------------------------------------

	leadIn += sizeof(_standardHeader);
	rv = g_sword2->_sound->PlayFx( 0, leadIn, 0, 0, RDSE_FXLEADIN );		// wav data gets copied to sound memory

	//-----------------------------------------
	#ifdef _SWORD2_DEBUG
	if (rv)
		Zdebug("SFX ERROR: PlayFx() returned %.8x (%s line %u)", rv, __FILE__, __LINE__);
	#endif
	//-----------------------------------------

	res_man.Res_close(params[0]);

	FN_stop_music(NULL);	// fade out any music that is currently playing (James22july97)

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_smacker_lead_out(int32 *params)	// James(21july97)
{
	smackerLeadOut = params[0];	// ready for use in FN_play_sequence

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------

int32 FN_play_sequence(int32 *params)	// James(09apr97)
{
	// params	0 pointer to null-terminated ascii filename
	// params	1 number of frames in the sequence, used for PSX.

	char	filename[30];
 	uint32	rv;	// drivers return value
	_movieTextObject *sequenceSpeechArray[MAX_SEQUENCE_TEXT_LINES+1];
	uint8 *leadOut = NULL;
#ifdef _SWORD2_DEBUG
	_standardHeader *header;
#endif

#ifdef _MUTE_SMACKERS
	uint32 musicMuteStatus;
#endif

 	//----------------------------------
	// In the case where smackers are crashing but the rest of the game is fine,
	// the "Skip Smackers" executable will display a message giving the smacker
	// file name rather than actually playing it.
	// Then the user can switch tasks & view the smacker using the stand-alone player!
	// This has got to be the biggest fudge in the history of computer games.

	#ifdef _SKIP_SMACKERS
		uint8	message[30];

		sprintf((char*)message,"SKIPPING SMACKER: \"%s.smk\"", (char *)params[0]);
		DisplayMsg(message, 3);	// 3 is duration in seconds
		RemoveMsg();
		sequenceTextLines=0;	// IMPORTANT - clear this so it doesn't overflow!
		return(IR_CONT);		//	continue script now; don't play smacker!
	#endif
	//----------------------------------
	// Another attempt to prevent the smacker crash
	// This time muting the music during the smacker
	// - in case that's what's causing the crash

	#ifdef _MUTE_SMACKERS
		musicMuteStatus = IsMusicMute();	// keep note of what mute status was to start with
		MuteMusic(1);	// mute the music - we'll set it back to 'musicMuteStatus' later
	#endif
	//----------------------------------

	Zdebug("FN_play_sequence(\"%s\");", params[0]);

	//--------------------------------------------------
	// check that the name paseed from script is 8 chars or less
	#ifdef _SWORD2_DEBUG
	if (strlen((char *)params[0]) > 8)
		Con_fatal_error("Sequence filename too long (%s line %u)",__FILE__,__LINE__);
	#endif
	//--------------------------------------------------
	// add the appropriate file extension & play it

	if (g_sword2->_gameId == GID_SWORD2_DEMO)
		sprintf(filename,"%s.smk", (char *)params[0]);
	else
		sprintf(filename,"%sSMACKS\\%s.smk", res_man.GetCdPath(), (char *)params[0]);

	//--------------------------------------
	// Write to walkthrough file (zebug0.txt)

	#ifdef _SWORD2_DEBUG
 	Zdebug(0,"PLAYING SEQUENCE \"%s\"", filename);
	#endif
   	//--------------------------------------
	// now create the text sprites, if any (James27may97)

	if (sequenceTextLines)	// if we have some text to accompany this sequence
		CreateSequenceSpeech(sequenceSpeechArray);

 	//--------------------------------------
	// open the lead-out music resource, if there is one

	if (smackerLeadOut)
	{
		leadOut = res_man.Res_open(smackerLeadOut);

	 	//---------------------------
		#ifdef _SWORD2_DEBUG
		header = (_standardHeader*)leadOut;
		if (header->fileType != WAV_FILE)
			Con_fatal_error("FN_smacker_lead_out() given invalid resource (%s line %u)",__FILE__,__LINE__);
		#endif
 	 	//---------------------------

		leadOut += sizeof(_standardHeader);
	}

 	//--------------------------------------
	// play the smacker

	FN_stop_music(NULL);	// don't want to carry on streaming game music when smacker starts!
	g_sword2->_sound->PauseFxForSequence();	// pause sfx during sequence, except the one used for lead-in music

	if (sequenceTextLines)	// if we have some text to accompany this sequence
		rv = PlaySmacker(filename, sequenceSpeechArray, leadOut);
	else
		rv = PlaySmacker(filename, NULL, leadOut);

/* we don't have this call - khalek
	if (ServiceWindows() == RDERR_APPCLOSED)	// if we pressed Ctrl-Q during the smacker
	{
		Close_game();	//close engine systems down
		RestoreDisplay();
		CloseAppWindow();
		exit(0);	//quit the game
	}
*/

	g_sword2->_sound->UnpauseFx();	// unpause sound fx again, in case we're staying in same location

 	//--------------------------------------
	// close the lead-out music resource

 	if (smackerLeadOut)
	{
		res_man.Res_close(smackerLeadOut);
		smackerLeadOut=0;
	}
	//--------------------------
	// check the error return-value
	#ifdef _SWORD2_DEBUG
	if (rv)
		Zdebug("PlaySmacker(\"%s\") returned 0x%.8x", filename, rv);
	#endif
	//--------------------------
	// now clear the text sprites, if any (James27may97)

	if (sequenceTextLines)	// if we have some text/speech to accompany this sequence
		ClearSequenceSpeech(sequenceSpeechArray);

	//--------------------------
	// now clear the screen in case the Sequence was quitted (using ESC) rather than fading down to black

	EraseBackBuffer();				// for hardware rendering
	EraseSoftwareScreenBuffer();	// for software rendering
	FlipScreens();					// to get the new blank screen visible

 	//--------------------------------------------------
	// zero the entire palette in case we're about to fade up!

	_palEntry	 pal[256];

	memset(pal, 0, 256*sizeof(_palEntry));
	SetPalette(0, 256, (uint8 *) pal, RDPAL_INSTANT);
 	//--------------------------------------------------

  	Zdebug("FN_play_sequence FINISHED");

 	//--------------------------------------------------
	#ifdef _MUTE_SMACKERS
		MuteMusic(musicMuteStatus);	// set mute status back to what it was before the sequence
	#endif
	//----------------------------------


	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
