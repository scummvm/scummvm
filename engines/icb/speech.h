/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_SPEECH_H
#define ICB_SPEECH_H

#include "engines/icb/common/px_array.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/common/px_assert.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/string_vest.h"

namespace ICB {

// this sits in the _mega structure for conversation_uid - means no current conversation
#define NO_SPEECH_REQUEST 424242

// goes in conversation_owners when a conversation is removed and its slot freed
#define DEAD_CONVERSATION 424242

enum __speech_state {
	__PENDING, // conversation is being set up
	__PROCESS, // ready for next speech instruction
	__WAITING_TO_SAY, // specific to psx really, this means the disc is spinning towards its destination, just keep going until it's finished!
	__SAYING // text is up on screen
};

enum __coms { __FACE_OBJECT, __PLAY_GENERIC_ANIM, __PLAY_CUSTOM_ANIM, __REVERSE_CUSTOM_ANIM };

class __conv_command {
public:

	uint32 id; // object id

	uint32 param1;
	//	pxString str_param1;
	char str_param1[ENGINE_STRING_LEN];

	__coms command;

	bool8 active;
	uint8 padding1;
	uint8 padding2;
	uint8 padding3;
};

#define MAX_people_talking 3
#define MAX_coms 3

class __conversation {
      public:
	__conversation() {
		for (int i = 0; i < MAX_people_talking; i++)
			subscribers_requested[i] = 0;

		total_subscribers = 0;
		current_subscribers = 0;

		current_talker = (int32)-1;
		count = 0;
	}

	uint32 subscribers_requested[MAX_people_talking];
	uint32 total_subscribers; // the number of people in the conversation
	uint32 current_subscribers; // number of people still in at the end of the logic cycle - we can therefore tell if any have dropped out

	__conv_command coms[MAX_coms];

	char *script_pc;

	int32 current_talker; // id of person talking
	uint32 count; // used to count down current text
	__speech_state state; // conversation state
};

// Works out how int32 some text should be displayed to allow adequate time for it to be read.  We might need to
// tinker with the formula in here 'till we get it right.
extern uint32 Get_reading_time(const char *pcString);

// Moved these from speech_psx.h, because the PC needs to be able to alter text colours now for the
// Remora's text.
void SetTextColour(uint8 r, uint8 g, uint8 b);

// This colour is used to display voice over text (normally player's speech colour).
extern uint8 voice_over_red;
extern uint8 voice_over_green;
extern uint8 voice_over_blue;

// These defaults are set to the player's text colour.
#define VOICE_OVER_DEFAULT_RED 255
#define VOICE_OVER_DEFAULT_GREEN 245
#define VOICE_OVER_DEFAULT_BLUE 100

} // End of namespace ICB

#endif
