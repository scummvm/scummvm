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

#include "engines/icb/floors.h"
#include "engines/icb/speech.h"
#include "engines/icb/fn_routines.h"
#include "engines/icb/session.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/icon_list_manager.h"
#include "engines/icb/sound.h"
#include "engines/icb/mission.h"
#include "engines/icb/sound_lowlevel.h"
#include "engines/icb/res_man.h"
#include "engines/icb/common/ptr_util.h"
#include "engines/icb/sound/music_manager.h"

namespace ICB {

#define TEXT_MAX_WIDTH 300
#define IS_SPEECH_STARTED 1
#define SPEECH_ERROR 0
#define IS_SPEECH_ALREADY_PLAYING 0

uint GetCountReduction() { return 1; }

// This colour is used to display voice over text (normally player's speech colour).
uint8 voice_over_red = VOICE_OVER_DEFAULT_RED;
uint8 voice_over_green = VOICE_OVER_DEFAULT_GREEN;
uint8 voice_over_blue = VOICE_OVER_DEFAULT_BLUE;

mcodeFunctionReturnCodes fn_request_speech(int32 &result, int32 *params) { return (MS->fn_request_speech(result, params)); }
mcodeFunctionReturnCodes fn_add_talker(int32 &result, int32 *params) { return (MS->fn_add_talker(result, params)); }
mcodeFunctionReturnCodes fn_issue_speech_request(int32 &result, int32 *params) { return (MS->fn_issue_speech_request(result, params)); }
mcodeFunctionReturnCodes fn_anon_speech_invite(int32 &result, int32 *params) { return (MS->fn_anon_speech_invite(result, params)); }
mcodeFunctionReturnCodes fn_speak(int32 &result, int32 *params) { return (MS->fn_speak(result, params)); }
mcodeFunctionReturnCodes fn_confirm_requests(int32 &result, int32 *params) { return (MS->fn_confirm_requests(result, params)); }
mcodeFunctionReturnCodes fn_converse(int32 &result, int32 *params) { return (MS->fn_converse(result, params)); }
mcodeFunctionReturnCodes speak_object_face_object(int32 &result, int32 *params) { return (MS->speak_object_face_object(result, params)); }
mcodeFunctionReturnCodes speak_play_generic_anim(int32 &result, int32 *params) { return (MS->speak_play_generic_anim(result, params)); }
mcodeFunctionReturnCodes speak_wait_for_everyone(int32 &result, int32 *params) { return (MS->speak_wait_for_everyone(result, params)); }
mcodeFunctionReturnCodes speak_add_chooser_icon(int32 &result, int32 *params) { return (MS->speak_add_chooser_icon(result, params)); }
mcodeFunctionReturnCodes speak_user_chooser(int32 &result, int32 *params) { return (MS->speak_user_chooser(result, params)); }
mcodeFunctionReturnCodes speak_chosen(int32 &result, int32 *params) { return (MS->speak_chosen(result, params)); }
mcodeFunctionReturnCodes speak_new_menu(int32 &result, int32 *params) { return (MS->speak_new_menu(result, params)); }
mcodeFunctionReturnCodes speak_close_menu(int32 &result, int32 *params) { return (MS->speak_close_menu(result, params)); }
mcodeFunctionReturnCodes speak_menu_still_active(int32 &result, int32 *params) { return (MS->speak_menu_still_active(result, params)); }
mcodeFunctionReturnCodes speak_menu_choices_remain(int32 &result, int32 *params) { return (MS->speak_menu_choices_remain(result, params)); }
mcodeFunctionReturnCodes speak_end_conversation(int32 &result, int32 *params) { return (MS->speak_end_conversation(result, params)); }
mcodeFunctionReturnCodes speak_end_menu(int32 &result, int32 *params) { return (MS->speak_end_menu(result, params)); }
mcodeFunctionReturnCodes speak_add_special_chooser_icon(int32 &result, int32 *params) { return (MS->speak_add_special_chooser_icon(result, params)); }
mcodeFunctionReturnCodes speak_set_custom(int32 &result, int32 *params) { return (MS->speak_set_custom(result, params)); }
mcodeFunctionReturnCodes speak_play_custom_anim(int32 &result, int32 *params) { return (MS->speak_play_custom_anim(result, params)); }
mcodeFunctionReturnCodes fn_get_speech_status(int32 &result, int32 *params) { return (MS->fn_get_speech_status(result, params)); }
mcodeFunctionReturnCodes fn_kill_conversations(int32 &result, int32 *params) { return (MS->fn_kill_conversations(result, params)); }
mcodeFunctionReturnCodes fn_speech_colour(int32 &result, int32 *params) { return (MS->fn_speech_colour(result, params)); }
mcodeFunctionReturnCodes speak_reverse_custom_anim(int32 &result, int32 *params) { return (MS->speak_reverse_custom_anim(result, params)); }
mcodeFunctionReturnCodes speak_preload_custom_anim(int32 &result, int32 *params) { return (MS->speak_preload_custom_anim(result, params)); }
mcodeFunctionReturnCodes fn_set_voice_over_colour(int32 &result, int32 *params) { return (MS->fn_set_voice_over_colour(result, params)); }
mcodeFunctionReturnCodes fn_default_voice_over_colour(int32 &result, int32 *params) { return (MS->fn_default_voice_over_colour(result, params)); }

mcodeFunctionReturnCodes _game_session::fn_get_speech_status(int32 &result, int32 *) {
	// tells us if a converation is already running or not

	result = total_convs;

	if ((cur_id == player.Fetch_player_id()) && (player.player_status == REMORA))
		Fatal_error("fn_get_speech_status - player cant start conversation inside remora!");

	if ((cur_id == player.Fetch_player_id()) && (g_oIconMenu->IsActive()))
		g_oIconMenu->CloseDownIconMenu();

	if ((g_oIconMenu->IsActive()) || (player.player_status == REMORA))
		result = 1;

	if ((result) && (cur_id == player.Fetch_player_id()))
		Tdebug("speech_check.txt", "get status");

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_request_speech(int32 &result, int32 *params) {
	// try to start up a conversation
	// there may be one or several participants who must agree to join the conversation

	// params    0 name of scene script

	const char *scene_script_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	Zdebug("[%s] fn_request_speech [%s]", object->GetName(), scene_script_name);

	// there cannot be any other conversations happening - change to initial spec as it is not used and memory is required
	if (total_convs) {
		if (cur_id == player.Fetch_player_id())
			Tdebug("speech_check.txt", "request");

		return IR_REPEAT; // just wait until other is done
	}

	if (player.player_status == REMORA)
		return IR_REPEAT;

	if ((cur_id == player.Fetch_player_id()) && (g_oIconMenu->IsActive()))
		g_oIconMenu->CloseDownIconMenu();

	if ((g_oIconMenu->IsActive()) || (player.player_status == REMORA))
		return IR_REPEAT;

	// not started yet
	S.state = __PENDING;

	// get the system now - in case another object tries to start a conversation
	total_convs++; // to 1 ;)

	// find the speech script
	// form name of speech script
	sprintf(temp_buf, "scenes::%s", scene_script_name);

	S.script_pc = (char *)scripts->Try_fetch_item_by_name(temp_buf); // run init script

	//	conversation script doesnt exist
	if (!S.script_pc)
		Fatal_error("object [%d] tried to start conversation script [%s] which doesnt exist", cur_id, (const char *const)temp_buf);

	// reset number of subs
	S.total_subscribers = 0; // everyone but us initially

	S.current_subscribers = 0; // reset

	for (uint32 j = 0; j < MAX_coms; j++)
		S.coms[j].active = FALSE8;

	menu_number = 0; // start at menu 0

	result = 0; // means ok

	Set_string(scene_script_name, speech_conv_name); // from, to

	// set player to stand frame
	if (cur_id == player.Fetch_player_id()) {
		L->cur_anim_type = __STAND;
		L->anim_pc = 0;
	}

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_add_talker(int32 &, int32 *params) {
	// a name of a mega to join the conversation is passed
	// we add its id into the list of subscribers

	// params    0   ascii name of object

	uint32 talk_id;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (S.state != __PENDING)
		Fatal_error("fn_add_talker called but in wrong order");

	// convert the ascii name into an object id
	talk_id = objects->Fetch_item_number_by_name(object_name);

	// check for illegal object
	if (talk_id >= total_objects)
		Fatal_error("fn_add_talker finds [%s] is not a real object", object_name);

	if (cur_id == talk_id)
		Fatal_error("[%s] calls fn_add_talker('%s') which isnt necessary and may cause strange lock up effects!", object_name, object_name);

	Zdebug("talk id %d", talk_id);

	if (talk_id != 0xffffffff) {

		if (S.total_subscribers == MAX_people_talking)
			Fatal_error("fn_add_talker(%s) too many people in conversation", object_name);

		S.subscribers_requested[S.total_subscribers] = talk_id;

		//		increase participant count
		S.total_subscribers++;

	} else {
		//		named object doesnt exist which is pretty serious
		Fatal_error("tried to add non existent object [%s] to conversation", object_name);
	}

	// keep going
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_issue_speech_request(int32 &, int32 *) {
	// once everyone required has been added to the subscriber list then we can ask each in turn if they want to join
	uint32 j;

	Zdebug("issue speech request");
	Zdebug(" %d invitees", S.total_subscribers);

	if (S.state != __PENDING)
		Fatal_error("fn_issue_speech_request called but in wrong order");

	//	if   (S.subscribers_requested.GetNoItems())
	if (S.total_subscribers) {
		//		force each to re-run their context
		for (j = 0; j < S.total_subscribers; j++) {
			Zdebug("forcing context rerun for %d", S.subscribers_requested[j]);

			if (S.subscribers_requested[j] >= total_objects)
				Fatal_error("fn_issue_speech_request has illegal object in issue list");

			Force_context_check(S.subscribers_requested[j]);
			Set_objects_conversation_uid(S.subscribers_requested[j], 0); // participant id, conversation uid
		}
	}

	// skip a cycle to allow the results to come back
	return (IR_STOP);
}

mcodeFunctionReturnCodes _game_session::fn_confirm_requests(int32 &result, int32 *params) {
	// did everyone in this conversation subscribe?
	// if yes then record the owner and let the  conversation begin

	Zdebug("fn-confirm-requests");

	if (S.state != __PENDING)
		Fatal_error("fn_confirm_requests called but in wrong order");

	// did everyone subscrive and consequently fn-converse?
	if (S.total_subscribers == S.current_subscribers) {
		// same number accepted as were requested so we're all systems go

		Zdebug(" conversation ok");

		S.state = __PROCESS; // set to script processing mode

		if (S.total_subscribers > MAX_people_talking)
			Fatal_error("fn_confirm_requests finds too many people in conversation");

		S.subscribers_requested[S.total_subscribers] = cur_id;

		// increase participant count
		S.total_subscribers++;

		// set our conversation uid as we are about to become a lowly subscriber
		Set_objects_conversation_uid(cur_id, 0); // participant id, conversation uid

		L->do_not_disturb++; // stop events

		conv_focus = 0; // us and always

		result = TRUE8;

		if (fn_stop_sting(result, params) == IR_REPEAT)
			return IR_REPEAT;
	} else {
		// no
		Zdebug(" conversation not fully subscribed");

		End_conversation(CONV_ID);

		result = FALSE8;
	}

	// and onward
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_anon_speech_invite(int32 &result, int32 *) {
	// general context script check for being a participant in a conversation
	// the idea is that if we are in a conversation then we continue with it regardless

	// just having this in the logic context means no conversation can be interupted by another

	Zdebug("check speech invite");

	if (L->conversation_uid != NO_SPEECH_REQUEST) {
		// yes, a request is pending
		// if not on a floor - perhaps on a ladder or stair - then mask the request
		if (M) {
			if (!floor_def->On_a_floor(M)) {
				Message_box("%d", player.player_status);
				L->conversation_uid = NO_SPEECH_REQUEST;
				result = 0; // no request
				return IR_CONT;
			}
		}

		L->do_not_disturb++; // stop events
		result = 1;
		return IR_CONT;
	}

	// no
	result = 0;
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_kill_conversations(int32 &, int32 *) {
	// current conversations end - allowing a new one to interupt

	// if there are conversations on going them kill em (it)
	if (total_convs)
		End_conversation(CONV_ID);

	return IR_STOP;
}

mcodeFunctionReturnCodes _game_session::fn_converse(int32 &, int32 *) {
	// mega calls this to subscribe to a conversation
	// we will be told when to quit though

	uint32 j;
	int32 result;
	int32 params;
	mcodeFunctionReturnCodes ret;

	Zdebug("fn_converse [%s] - uid %d", object->GetName(), L->conversation_uid);

	if (L->conversation_uid == NO_SPEECH_REQUEST) {
		//		conversation has ended!

		L->do_not_disturb--; // restart events

		//		continue and fall off script letting us re-run our logic context
		return (IR_CONT);
	}

	// re-register
	speech_info[L->conversation_uid].current_subscribers++;

	// if player then update buttons and check for skip speech
	if (cur_id == player.Fetch_player_id()) {
		//		check keys/pads/etc. to see what the user is trying to do
		player.Update_input_state();

		if (((++no_click_zone) > 2)) {
			if ((player.cur_state.IsButtonSet(__INTERACT)) && (!player.interact_lock) && (speech_info[CONV_ID].state == __SAYING)) {
				player.interact_lock = TRUE8;

				speech_info[CONV_ID].count = 0;
				CancelSpeechPlayback();

			} else if (!player.cur_state.IsButtonSet(__INTERACT)) {
				player.interact_lock = FALSE8;
			}
		}
	}

	//	check for instructions
	for (j = 0; j < MAX_coms; j++) // loop to number of commands
		if (speech_info[L->conversation_uid].coms[j].active == TRUE8) // this slot?
			if (speech_info[L->conversation_uid].coms[j].id == cur_id) { // is the command for us
				switch (speech_info[L->conversation_uid].coms[j].command) {
				case __FACE_OBJECT:
					Zdebug("face object [com %d] - [param %d]", j, speech_info[L->conversation_uid].coms[j].param1);

					if (!speech_face_object(speech_info[L->conversation_uid].coms[j].param1)) {
						speech_info[L->conversation_uid].coms[j].active = FALSE8;
					}
					break;

				case __PLAY_GENERIC_ANIM:
					params = MemoryUtil::encodePtr((uint8 *)speech_info[L->conversation_uid].coms[j].str_param1);
					ret = fn_play_generic_anim(result, &params);
					if (ret == IR_CONT) {
						speech_info[L->conversation_uid].coms[j].active = FALSE8;
					}
					break;

				case __PLAY_CUSTOM_ANIM:
					params = MemoryUtil::encodePtr((uint8 *)speech_info[L->conversation_uid].coms[j].str_param1);
					ret = fn_easy_play_custom_anim(result, &params);
					if (ret == IR_CONT) {
						Reset_cur_megas_custom_type();
						speech_info[L->conversation_uid].coms[j].active = FALSE8;
					}
					break;

				case __REVERSE_CUSTOM_ANIM:
					params = MemoryUtil::encodePtr((uint8 *)speech_info[L->conversation_uid].coms[j].str_param1);
					ret = fn_reverse_custom_anim(result, &params);
					if (ret == IR_CONT) {
						Reset_cur_megas_custom_type();
						speech_info[L->conversation_uid].coms[j].active = FALSE8;
					}
					break;

				default:
					Fatal_error("illegal speech com instruction");
					break;
				}

				// do the first pending com - not any further ones which may be queued
				return (IR_REPEAT); // keep returning until we're released
			}

	return (IR_REPEAT); // keep returning until we're released
}

void _game_session::Service_speech() {
	// the system runs all speech scripts

	c_game_object *speech_object;
	uint32 ret;

	// anything going on?
	if (!total_convs)
		return; // none

	// not started yet
	if (S.state == __PENDING)
		return;

	if (speech_info[CONV_ID].total_subscribers > speech_info[CONV_ID].current_subscribers) {
		// oh no, conversation has to end as someones dropped out - perhaps they were shot, blown up or distracted

		// in-case someone is mid sentence
		CancelSpeechPlayback();

		End_conversation(CONV_ID);
		return;
	}

	// reset sub total for next cycles check
	speech_info[CONV_ID].current_subscribers = 0;

	// process the conversation
	switch (speech_info[CONV_ID].state) {
	case __PROCESS: // run the script
		// get the dummy speech object
		speech_object = (c_game_object *)objects->Fetch_item_by_name("scenes");
		cur_id = objects->Fetch_item_number_by_name("scenes");
		L = logic_structs[cur_id];
		I = 0;
		M = 0;

		// run the script
		ret = RunScript(const_cast<const char *&>(speech_info[CONV_ID].script_pc), speech_object);

		if (ret == 1) {
			// speech script has finished
			End_conversation(CONV_ID);
			return;
		}

		break;

	// on psx this waits until the speech is started then sets the timer for stopping it
	case __WAITING_TO_SAY:
	case __SAYING: // someone is talking

		// set state to saying if we're actually playing
		if (IS_SPEECH_STARTED)
			speech_info[CONV_ID].state = __SAYING;

		// count down the timer
		// back into process mode when we're done
		if (!speech_info[CONV_ID].count) { // done
			speech_info[CONV_ID].current_talker = -1; // nobody talking now
			speech_info[CONV_ID].state = __PROCESS;

			// if  this conversation has the focus then kill the text bloc
			if (CONV_ID == conv_focus) {
				// delete    text_bloc;
				text_speech_bloc->please_render = FALSE8;
			}
		} else
			speech_info[CONV_ID].count -= GetCountReduction();

		break;

	default:
		Fatal_error("illegal instruction found in conversation");
		break;
	}
}

mcodeFunctionReturnCodes _game_session::fn_speak(int32 &, int32 *params) {
	// current conversation

	// params    0   ascii name of person
	//			1  ascii text line name

	_TSrtn ret_code;
	uint32 speaker_id;
	PXvector pos;
	// yesno
	bool8 resu = FALSE8;

	// screen pos
	PXvector filmpos;

	char error[] = "text file out of date!";

	char *ascii;

	const char *person_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *text_label = (const char *)MemoryUtil::resolvePtr(params[1]);

	// wait until last line is finished
	if (IS_SPEECH_ALREADY_PLAYING)
		return IR_REPEAT;

	// Make the hash from the label
	uint32 speechHash = HashString(text_label);

	if (!PreloadSpeech(speechHash))
		return IR_REPEAT;

	if (text_speech_bloc->please_render == TRUE8)
		Fatal_error("fn_speak - text block already exists!");

	Zdebug("fn_speak [%s] [%s]", person_name, text_label);

	// work out position using speakers position
	// fetch the speaker
	speaker_id = objects->Fetch_item_number_by_name(person_name);
	if (speaker_id == PX_LINKED_DATA_FILE_ERROR)
		Fatal_error("Unable to find object ID for [%s] in fn_speak()", person_name);

	if (text) {
		// retrieve text line
		ascii = (char *)text->Try_fetch_item_by_name(text_label);

		if (!ascii)
			ascii = error;

		else if (!*(ascii)) // null terminated string
			ascii = error;

	} else
		ascii = error;

	// work out length
	Zdebug("[%s]", ascii);

	{
		_TSparams *text_params;
		// build text block
		text_params = text_speech_bloc->GetParams();
		text_speech_bloc->please_render = TRUE8; // exists to draw

		text_params->textLine = (uint8 *)ascii; // print id instead of real text for now
		// set font
		text_params->fontResource = (const char *)speech_font_one; //&test_font[0];

		// Jake for clustering
		text_params->fontResource_hash = speech_font_one_hash;

		text_params->maxWidth = TEXT_MAX_WIDTH;
		text_params->lineSpacing = 0;
		text_params->charSpacing = 0;
		text_params->errorChecking = 1; // enable error checking during AnalyseSentence

		// Set the colour for the speech.
		if (logic_structs[speaker_id]->mega) {
			// Each mega has their own speech colour.
			SetTextColour(logic_structs[speaker_id]->mega->speech_red, logic_structs[speaker_id]->mega->speech_green, logic_structs[speaker_id]->mega->speech_blue);
		} else {
			// Objects all share one value.
			SetTextColour(voice_over_red, voice_over_green, voice_over_blue);
		}

		ret_code = text_speech_bloc->MakeTextSprite();

		if (ret_code != TS_OK)
			Fatal_error("line [%s] text formating is illegal [%s]", text_label, ascii);

		if (px.display_mode == THREED) {

			// get coords
			if (logic_structs[speaker_id]->image_type == PROP) {
				// has prop got coords?
				if (logic_structs[speaker_id]->prop_coords_set == TRUE8) {
					pos.x = logic_structs[speaker_id]->prop_xyz.x;
					pos.y = logic_structs[speaker_id]->prop_xyz.y; // talks over head rather than from the feet
					pos.z = logic_structs[speaker_id]->prop_xyz.z;

					// setup camera
					PXcamera &camera = GetCamera();

					// compute screen coord

					PXWorldToFilm(pos, camera, resu, filmpos);

					if (!resu)
						Zdebug(" position off film?");

					text_speech_bloc->GetRenderCoords((int32)(filmpos.x + (SCREEN_WIDTH / 2)), (int32)((SCREEN_DEPTH / 2) - (filmpos.y)), PIN_AT_CENTRE_OF_BASE,
					                                  5); // margin
				} else { // prop with no coords prints along bottom of screen
					text_speech_bloc->renderX = 20;
					text_speech_bloc->renderY = 400;
				}
			} else {
				pos.x = logic_structs[speaker_id]->mega->actor_xyz.x;
				pos.y = logic_structs[speaker_id]->mega->actor_xyz.y + 200; // talks over head rather than from the feet
				pos.z = logic_structs[speaker_id]->mega->actor_xyz.z;

				// setup camera
				PXcamera &camera = GetCamera();

				// compute screen coord
				PXWorldToFilm(pos, camera, resu, filmpos);

				if (!resu)
					Zdebug(" position off film?");

				text_speech_bloc->GetRenderCoords((int32)(filmpos.x + (SCREEN_WIDTH / 2)), (int32)((SCREEN_DEPTH / 2) - (filmpos.y)), PIN_AT_CENTRE_OF_BASE,
				                                  5); // margin
			}

		} else { // fixed position in nethack mode
			text_speech_bloc->renderX = 20;
			text_speech_bloc->renderY = 400;
		}

		speech_info[CONV_ID].count = SayLineOfSpeech(speechHash);


		if (!speech_info[CONV_ID].count)
			Fatal_error("Speech xa file is 0 game cycles see, int32");
	}

	no_click_zone = 0; // cant click past for specified period

	// set conv mode
	speech_info[CONV_ID].current_talker = speaker_id;
	speech_info[CONV_ID].state = __WAITING_TO_SAY;

	return (IR_STOP); // drop out
}

mcodeFunctionReturnCodes _game_session::fn_speech_colour(int32 &, int32 *params) {
	Zdebug("fn_speech_colour( %d, %d, %d ) called by object ID %d", (int32)params[0], (int32)params[1], (int32)params[2], cur_id);

	// Must be a mega calling this.
	if (!logic_structs[cur_id]->mega)
		Fatal_error("Non-mega %d called fn_speech_colour()", cur_id);

	// Set the values in the mega's structure.
	logic_structs[cur_id]->mega->speech_red = (uint8)params[0];
	logic_structs[cur_id]->mega->speech_green = (uint8)params[1];
	logic_structs[cur_id]->mega->speech_blue = (uint8)params[2];

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_set_voice_over_colour(int32 &, int32 *params) {
	// This colour is used to display voice over text (normally player's speech colour).
	voice_over_red = (uint8)params[0];
	voice_over_green = (uint8)params[1];
	voice_over_blue = (uint8)params[2];

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_default_voice_over_colour(int32 &, int32 *) {
	// This colour is used to display voice over text (normally player's speech colour).
	voice_over_red = VOICE_OVER_DEFAULT_RED;
	voice_over_green = VOICE_OVER_DEFAULT_GREEN;
	voice_over_blue = VOICE_OVER_DEFAULT_BLUE;

	// Calling script can continue.
	return (IR_CONT);
}

extern int globalCharSpacing;

// This function computes the formatting of a paragraph of Remora text without going as far as making
// the sprite.  This is so the Remora can decide how it is going to format text before it tries to draw
// it.
void _game_session::Format_remora_text(const char *pcText, int32 nLineSpacing, int32 nCharSpacing, uint32 nMaxWidth) {
	_TSrtn eErrorCode;
	_TSparams *psTextParams;

	// Get the pointer to the parameter block that needs to be filled in before the functions of class text_sprite
	// can do their stuff.
	psTextParams = text_bloc->GetParams();

	// Set the parameters required by text_sprite::AnalyseSentence().
	psTextParams->textLine = (uint8 *)const_cast<char *>(pcText);
	psTextParams->fontResource = (const char *)remora_font;
	psTextParams->fontResource_hash = remora_font_hash;
	psTextParams->maxWidth = nMaxWidth;
	psTextParams->lineSpacing = nLineSpacing;
	psTextParams->charSpacing = nCharSpacing + globalCharSpacing;
	psTextParams->errorChecking = 1;

	// Now I can call AnalyseSentence() to work out how the text will be formatted when it is drawn.
	eErrorCode = text_bloc->AnalyseSentence();

	// Check there was no error.
	if (eErrorCode != TS_OK) {
		Fatal_error("text_sprite::AnalyseSentence( '%s' ) failed with code %d in Format_remora_text", pcText, (int32)eErrorCode);
	}
}

void _game_session::Create_remora_text(uint32 x, uint32 y, const char *ascii,
                                       int32 margin, _pin_position pin_pos, int32 lineSpacing, int32 charSpacing,
                                       uint32 maxWidth,
                                       bool8 analysisAlreadyDone,
                                       int32 stopAtLine) {
	bool8 bRemoraLeftFormatting;

	Zdebug("Create_remora_text %d,%d [%s]", x, y, ascii);

	// if we are in focus then build a text bloc
	// build text block
	_TSparams *text_params;
	text_params = text_bloc->GetParams();
	text_params->textLine = (uint8 *)const_cast<char *>(ascii); // print id instead of real text for now
	// set font
	text_params->fontResource = (const char *)remora_font;
	// Jake for clustering
	text_params->fontResource_hash = remora_font_hash;

	text_params->maxWidth = maxWidth;
	text_params->lineSpacing = lineSpacing;
	text_params->charSpacing = charSpacing + globalCharSpacing;
	text_params->errorChecking = 1; // enable error checking during AnalyseSentence

	// See if we need to trigger the special left formatting that only the Remora uses.
	if (pin_pos == PIN_AT_TOP_LEFT)
		bRemoraLeftFormatting = TRUE8;
	else
		bRemoraLeftFormatting = FALSE8;

	int oldFlag = px.speechLineNumbers;

	// Turn off line numbers for non-spoken lines of dialogue
	if (*ascii == TS_NON_SPOKEN_LINE)
		px.speechLineNumbers = 0;

	text_bloc->MakeTextSprite(analysisAlreadyDone, stopAtLine, bRemoraLeftFormatting);
	px.speechLineNumbers = (u_char)oldFlag;
	text_bloc->GetRenderCoords(x, y, pin_pos, margin);

	text_bloc->please_render = TRUE8;
}

void _game_session::Kill_remora_text() {

	if (!text_bloc->please_render)
		Fatal_error("Kill_remora_text - no text block exists!");

	text_bloc->please_render = FALSE8;
}

mcodeFunctionReturnCodes _game_session::speak_object_face_object(int32 &, int32 *params) {
	// first person turns to face the other

	// params    0   ascii name of person to turn
	//			1  ascii name of person to turn to

	uint32 speaker_id;
	uint32 tar_id;
	uint32 com_no = 0;

	const char *speaker_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *target_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	// fetch the speaker
	speaker_id = objects->Fetch_item_number_by_name(speaker_name);

	// fetch target
	tar_id = objects->Fetch_item_number_by_name(target_name);

	// find next com slot
	while ((speech_info[0].coms[com_no].active == TRUE8) && (speech_info[0].coms[com_no].id != speaker_id))
		com_no++;

	speech_info[CONV_ID].coms[com_no].active = TRUE8;

	// set the commands owner id
	speech_info[CONV_ID].coms[com_no].id = speaker_id;

	// set command type
	speech_info[CONV_ID].coms[com_no].command = __FACE_OBJECT;

	// set the target id parameter
	speech_info[CONV_ID].coms[com_no].param1 = tar_id;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::speak_play_generic_anim(int32 &, int32 *params) {
	// object plays a generic animation
	// links to fn_play_generic_anim

	// params    0   ascii name of person to animate
	//			1  ascii name of generic animation

	uint32 speaker_id;
	uint32 com_no = 0;

	const char *person_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	Zdebug("speak_play_generic_anim [%s] to face [%s]", person_name, anim_name);

	// fetch the speaker
	speaker_id = objects->Fetch_item_number_by_name(person_name);

	while ((speech_info[0].coms[com_no].active == TRUE8) && (speech_info[0].coms[com_no].id != speaker_id))
		com_no++;

	if (com_no == MAX_coms)
		Fatal_error("too many speech commands");

	speech_info[CONV_ID].coms[com_no].active = TRUE8;

	// set the commands owner id
	speech_info[CONV_ID].coms[com_no].id = speaker_id;

	// set command type
	speech_info[CONV_ID].coms[com_no].command = __PLAY_GENERIC_ANIM;

	// set the target id parameter
	Set_string(anim_name, speech_info[CONV_ID].coms[com_no].str_param1);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::speak_set_custom(int32 &, int32 *params) {
	// set the custom set for a character

	// params    0   ascii name of person to animate
	//			1  ascii name of custom set

	int32 speaker_id;

	const char *person_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *custom_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	// fetch the speaker
	speaker_id = objects->Fetch_item_number_by_name(person_name);

	if (speaker_id == -1)
		Fatal_error("speak_set_custom cant find object [%s]", person_name);

	Set_string(custom_name, logic_structs[speaker_id]->mega->custom_set, MAX_CUSTOM_NAME_LENGTH);
	logic_structs[speaker_id]->mega->custom = TRUE8;
	logic_structs[speaker_id]->looping = 0; // reset

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::speak_preload_custom_anim(int32 &, int32 *params) {
	// set a custom anim preloading

	// params    0   name of mega
	//			1  name of anim

	int32 speaker_id;

	const char *mega_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	/*const char *anim_name = (const char *)*/ MemoryUtil::resolvePtr(params[1]);

#define II logic_structs[speaker_id]->voxel_info
#define MM logic_structs[speaker_id]->mega

	// fetch the speaker
	speaker_id = objects->Fetch_item_number_by_name(mega_name);

	// build that name
	II->Init_custom_animation(mega_name);

	// now set prebuilt flag
	II->has_custom_path_built = FALSE8;

	// start preloading
	rs_anims->Res_async_open(II->get_anim_name(__NON_GENERIC), II->anim_name_hash[__NON_GENERIC], II->base_path, II->base_path_hash); //

	MM->custom = FALSE8;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::speak_play_custom_anim(int32 &, int32 *params) {
	// object plays a custom animation
	// links to fn_play_custom_anim

	// params    0   ascii name of person to animate
	//			1  ascii name of custom animation

	uint32 speaker_id;
	uint32 com_no = 0;

	const char *speaker_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	// fetch the speaker
	speaker_id = objects->Fetch_item_number_by_name(speaker_name);

	while ((speech_info[0].coms[com_no].active == TRUE8) && (speech_info[0].coms[com_no].id != speaker_id))
		com_no++;

	if (com_no == MAX_coms)
		Fatal_error("too many speech commands");

	speech_info[CONV_ID].coms[com_no].active = TRUE8;

	// set the commands owner id
	speech_info[CONV_ID].coms[com_no].id = speaker_id;

	// set command type
	speech_info[CONV_ID].coms[com_no].command = __PLAY_CUSTOM_ANIM;

	// set the target id parameter
	Set_string(anim_name, speech_info[CONV_ID].coms[com_no].str_param1);

	return (IR_STOP);
}

mcodeFunctionReturnCodes _game_session::speak_reverse_custom_anim(int32 &, int32 *params) {
	// object REVERSE plays a custom animation

	// params    0   ascii name of person to animate
	//			1  ascii name of custom animation

	uint32 speaker_id;
	uint32 com_no = 0;

	const char *speaker_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	// fetch the speaker
	speaker_id = objects->Fetch_item_number_by_name(speaker_name);

	while ((speech_info[0].coms[com_no].active == TRUE8) && (speech_info[0].coms[com_no].id != speaker_id))
		com_no++;

	if (com_no == MAX_coms)
		Fatal_error("too many speech commands");

	speech_info[CONV_ID].coms[com_no].active = TRUE8;

	// set the commands owner id
	speech_info[CONV_ID].coms[com_no].id = speaker_id;

	// set command type
	speech_info[CONV_ID].coms[com_no].command = __REVERSE_CUSTOM_ANIM;

	// set the target id parameter
	Set_string(anim_name, speech_info[CONV_ID].coms[com_no].str_param1);

	return (IR_CONT);
}

void _game_session::Exit_speech(uint32 id) {
	// if we are talking then stop the sample
	// called when this id takes a gun shot
	// will drop out of speech and speech wav will stop playing if this id is currently speaking

	if (logic_structs[id]->conversation_uid != NO_SPEECH_REQUEST) { // is talking
		if (speech_info[CONV_ID].current_talker == (int32)id)
			StopSpeechPlayback(); // target is talking

		//		will drop out
		logic_structs[id]->conversation_uid = NO_SPEECH_REQUEST;
	}
}

void _game_session::End_conversation(uint32 uid) {
	// clean up conversation

	uint32 j;

	// decrease number of active conversations (from 1 to 0)
	total_convs--;

	//	clean up and free the objects
	//	reset everyones conversation_uid if they are still registed as being in this conversation
	if (speech_info[uid].total_subscribers)
		for (j = 0; j < speech_info[uid].total_subscribers; j++) {
			if (speech_info[uid].subscribers_requested[j] == player.Fetch_player_id()) {
				if (logic_structs[player.Fetch_player_id()]->conversation_uid !=
				    NO_SPEECH_REQUEST) { // dont reset player if he never joined - for request failures on ladders, etc.
					player.Pop_player_stat(); // stood or aiming, etc
				}
			}

			if (speech_info[uid].subscribers_requested[j] >= total_objects)
				Fatal_error("End_conversation find illegal id %d - total = %d", speech_info[uid].subscribers_requested[j],
				            speech_info[uid].total_subscribers);

			Set_objects_conversation_uid(speech_info[uid].subscribers_requested[j], NO_SPEECH_REQUEST); // participant id, conversation uid   - release the object
		}

	conv_focus = 0; // no one

	if (text_speech_bloc->please_render == TRUE8) {
		Zdebug("removing text bloc");
		text_speech_bloc->please_render = FALSE8;
	}

	// end music
	int32 r, p[1];

	speak_end_music(r, p);

	// reset list of subscribers
	speech_info[uid].total_subscribers = 0;
}

mcodeFunctionReturnCodes _game_session::speak_wait_for_everyone(int32 &, int32 *) {
	// waits until all pending coms are done

	// are there any coms pending?

	uint32 j;

	for (j = 0; j < MAX_coms; j++)
		if (speech_info[0].coms[j].active == TRUE8)
			return (IR_REPEAT); // yes

	// no
	return (IR_CONT);
}

// ICON_LIST_MANAGER maximum must be this number +2 (inventory + remora)
#define MAX_number_menus 6

char menu_name_list[MAX_number_menus][4];

bool8 choosing[10];
uint32 item_count[10];

mcodeFunctionReturnCodes _game_session::speak_new_menu(int32 &, int32 *) {
	// create a new menu

	menu_number++;
	if (menu_number >= MAX_number_menus) {
		Fatal_error("too many menus MAX %d", MAX_number_menus);
	}

	g_oIconListManager->ResetList(menu_name_list[menu_number]);

	sprintf(menu_name_list[menu_number], "m%02d", menu_number); // create a unique name
	choosing[menu_number] = FALSE8;
	item_count[menu_number] = 0;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::speak_close_menu(int32 &, int32 *) {
	menu_number--;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::speak_add_chooser_icon(int32 &, int32 *params) {
	// add icon to speech menu

	// params    0   ascii name of icon
	const char *icon_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	g_oIconListManager->AddIconToList(menu_name_list[menu_number], icon_name);

	item_count[menu_number]++;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::speak_add_special_chooser_icon(int32 &, int32 *params) {
	// add icon to speech menu
	// these icons dont count as items that must be selected to finish with the menu

	// params    0   ascii name of icon
	const char *icon_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	Zdebug("speak_add_special_chooser_icon [%s]", icon_name);

	g_oIconListManager->AddIconToList(menu_name_list[menu_number], icon_name);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::speak_user_chooser(int32 &, int32 *) {
	// wait for user to choose an option

	_input *state;

	Zdebug("user chooser");

	// check keys/pads/etc. to see what the user is trying to do
	player.Update_input_state();

	state = player.Fetch_input_state();

	// first time in bring up the new menu
	if (choosing[menu_number] == FALSE8) {
		Zdebug("activating menu");

		g_oIconListManager->ActivateIconMenu(menu_name_list[menu_number], FALSE8, FALSE8);
		Zdebug("~activating menu");
		choosing[menu_number] = TRUE8;

		player.Push_control_mode(ACTOR_RELATIVE);
	}

	// safe to do it here as menu has been setup
	g_oIconListManager->CycleInventoryLogic(*state);

	if (g_oIconListManager->ItemHeld()) {
		Zdebug("selected");
		choosing[menu_number] = FALSE8; // reset
		player.interact_lock = TRUE8;

		player.Pop_control_mode();

		return (IR_CONT);
	}

	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::speak_chosen(int32 &result, int32 *params) {
	// is the named icon the one that was chosen

	const char *icon_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (g_oIconListManager->Holding(icon_name)) {
		item_count[menu_number]--;
		g_oIconListManager->RemoveIconFromList(menu_name_list[menu_number], icon_name);
		g_oIconListManager->Drop();
		result = TRUE8;
	} else {
		//		no
		result = FALSE8;
	}

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::speak_menu_still_active(int32 &result, int32 *) {
	// says if menu is still active - i.e user hasnt chosen a quit icon

	result = TRUE8;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::speak_menu_choices_remain(int32 &result, int32 *) {
	// says if menu still has items left to choose

	result = item_count[menu_number];

	Zdebug("speak_menu_choices_remain reports %d options remain", result);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::speak_end_conversation(int32 &, int32 *) {
	// kill current conversation at user request
	// this will be called via Service_speech

	End_conversation(CONV_ID);

	return (IR_STOP);
}

mcodeFunctionReturnCodes _game_session::speak_end_menu(int32 &, int32 *) {
	// end the current menu

	// only destroy it if there are some options left
	if (item_count[menu_number])
		g_oIconListManager->DestroyList(menu_name_list[menu_number]);

	// zero the items left count so that speak_menu_choices_remain will cancel the outer while loop
	item_count[menu_number] = 0;

	return (IR_CONT);
}

// Works out how int32 some text should be displayed to allow adequate time for it to be read.  We might need to
// tinker with the formula in here 'till we get it right.
uint32 Get_reading_time(const char *pcString) {
	uint32 nPos = 0;
	uint32 nSpaceCount = 0;

	// Base it on the wordcount.
	do {
		if (pcString[nPos] == ' ')
			++nSpaceCount;

	} while (pcString[nPos++] != '\0');

	// Fixed 1/2 second plus 1/12 second for each word.
	return (12 + (nSpaceCount * 2));

} // end Get_reading_time()

} // End of namespace ICB
