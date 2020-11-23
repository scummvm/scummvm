/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

//
// Game update procedure
//

//include <math.h>
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/characterextras.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/lipsync.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/roomobject.h"
#include "ags/engine/ac/roomstatus.h"
#include "ags/engine/main/mainheader.h"
#include "ags/engine/main/update.h"
#include "ags/engine/ac/screenoverlay.h"
#include "ags/engine/ac/viewframe.h"
#include "ags/engine/ac/walkablearea.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/graphicsdriver.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/ac/timer.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/ac/movelist.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

extern MoveList *mls;
extern RoomStatus *croom;
extern GameSetupStruct game;
extern GameState play;
extern RoomStruct thisroom;
extern RoomObject *objs;
extern ViewStruct *views;
extern int our_eip;
extern CharacterInfo *playerchar;
extern CharacterExtras *charextra;
extern CharacterInfo *facetalkchar;
extern int face_talking, facetalkview, facetalkwait, facetalkframe;
extern int facetalkloop, facetalkrepeat, facetalkAllowBlink;
extern int facetalkBlinkLoop;
extern bool facetalk_qfg4_override_placement_x, facetalk_qfg4_override_placement_y;
extern SpeechLipSyncLine *splipsync;
extern int numLipLines, curLipLine, curLipLinePhoneme;
extern int is_text_overlay;
extern IGraphicsDriver *gfxDriver;

int do_movelist_move(short *mlnum, int *xx, int *yy) {
	int need_to_fix_sprite = 0;
	if (mlnum[0] < 1) quit("movelist_move: attempted to move on a non-exist movelist");
	MoveList *cmls;
	cmls = &mls[mlnum[0]];
	fixed xpermove = cmls->xpermove[cmls->onstage], ypermove = cmls->ypermove[cmls->onstage];

	short targetx = short((cmls->pos[cmls->onstage + 1] >> 16) & 0x00ffff);
	short targety = short(cmls->pos[cmls->onstage + 1] & 0x00ffff);
	int xps = xx[0], yps = yy[0];
	if (cmls->doneflag & 1) {
		// if the X-movement has finished, and the Y-per-move is < 1, finish
		// This can cause jump at the end, but without it the character will
		// walk on the spot for a while if the Y-per-move is for example 0.2
//    if ((ypermove & 0xfffff000) == 0) cmls->doneflag|=2;
//    int ypmm=(ypermove >> 16) & 0x0000ffff;

		// NEW 2.15 SR-1 plan: if X-movement has finished, and Y-per-move is < 1,
		// allow it to finish more easily by moving target zone

		int adjAmnt = 3;
		// 2.70: if the X permove is also <=1, don't do the skipping
		if (((xpermove & 0xffff0000) == 0xffff0000) ||
			((xpermove & 0xffff0000) == 0x00000000))
			adjAmnt = 2;

		// 2.61 RC1: correct this to work with > -1 as well as < 1
		if (ypermove == 0) {
		}
		// Y per move is < 1, so finish the move
		else if ((ypermove & 0xffff0000) == 0)
			targety -= adjAmnt;
		// Y per move is -1 exactly, don't snap to finish
		else if (ypermove == 0xffff0000) {
		}
		// Y per move is > -1, so finish the move
		else if ((ypermove & 0xffff0000) == 0xffff0000)
			targety += adjAmnt;
	} else xps = cmls->fromx + (int)(fixtof(xpermove) * (float)cmls->onpart);

	if (cmls->doneflag & 2) {
		// Y-movement has finished

		int adjAmnt = 3;

		// if the Y permove is also <=1, don't skip as far
		if (((ypermove & 0xffff0000) == 0xffff0000) ||
			((ypermove & 0xffff0000) == 0x00000000))
			adjAmnt = 2;

		if (xpermove == 0) {
		}
		// Y per move is < 1, so finish the move
		else if ((xpermove & 0xffff0000) == 0)
			targetx -= adjAmnt;
		// X per move is -1 exactly, don't snap to finish
		else if (xpermove == 0xffff0000) {
		}
		// X per move is > -1, so finish the move
		else if ((xpermove & 0xffff0000) == 0xffff0000)
			targetx += adjAmnt;

		/*    int xpmm=(xpermove >> 16) & 0x0000ffff;
		//    if ((xpmm==0) | (xpmm==0xffff)) cmls->doneflag|=1;
			if (xpmm==0) cmls->doneflag|=1;*/
	} else yps = cmls->fromy + (int)(fixtof(ypermove) * (float)cmls->onpart);
	// check if finished horizontal movement
	if (((xpermove > 0) && (xps >= targetx)) ||
		((xpermove < 0) && (xps <= targetx))) {
		cmls->doneflag |= 1;
		xps = targetx;
		// if the Y is almost there too, finish it
		// this is new in v2.40
		// removed in 2.70
		/*if (abs(yps - targety) <= 2)
		  yps = targety;*/
	} else if (xpermove == 0)
		cmls->doneflag |= 1;
	// check if finished vertical movement
	if ((ypermove > 0) &(yps >= targety)) {
		cmls->doneflag |= 2;
		yps = targety;
	} else if ((ypermove < 0) & (yps <= targety)) {
		cmls->doneflag |= 2;
		yps = targety;
	} else if (ypermove == 0)
		cmls->doneflag |= 2;

	if ((cmls->doneflag & 0x03) == 3) {
		// this stage is done, go on to the next stage
		// signed shorts to ensure that numbers like -20 do not become 65515
		cmls->fromx = (signed short)((cmls->pos[cmls->onstage + 1] >> 16) & 0x000ffff);
		cmls->fromy = (signed short)(cmls->pos[cmls->onstage + 1] & 0x000ffff);
		if ((cmls->fromx > 65000) || (cmls->fromy > 65000))
			quit("do_movelist: int to short rounding error");

		cmls->onstage++;
		cmls->onpart = -1;
		cmls->doneflag &= 0xf0;
		cmls->lastx = -1;
		if (cmls->onstage < cmls->numstage) {
			xps = cmls->fromx;
			yps = cmls->fromy;
		}
		if (cmls->onstage >= cmls->numstage - 1) { // last stage is just dest pos
			cmls->numstage = 0;
			mlnum[0] = 0;
			need_to_fix_sprite = 1;
		} else need_to_fix_sprite = 2;
	}
	cmls->onpart++;
	xx[0] = xps;
	yy[0] = yps;
	return need_to_fix_sprite;
}


void update_script_timers() {
	if (play.gscript_timer > 0) play.gscript_timer--;
	for (int aa = 0; aa < MAX_TIMERS; aa++) {
		if (play.script_timers[aa] > 1) play.script_timers[aa]--;
	}
}

void update_cycling_views() {
	// update graphics for object if cycling view
	for (int aa = 0; aa < croom->numobj; aa++) {

		RoomObject *obj = &objs[aa];

		obj->UpdateCyclingView();
	}
}

void update_shadow_areas() {
	// shadow areas
	int onwalkarea = get_walkable_area_at_character(game.playercharacter);
	if (onwalkarea < 0);
	else if (playerchar->flags & CHF_FIXVIEW);
	else {
		onwalkarea = thisroom.WalkAreas[onwalkarea].Light;
		if (onwalkarea > 0) playerchar->view = onwalkarea - 1;
		else if (thisroom.Options.PlayerView == 0) playerchar->view = playerchar->defview;
		else playerchar->view = thisroom.Options.PlayerView - 1;
	}
}

void update_character_move_and_anim(int &numSheep, int *followingAsSheep) {
	// move & animate characters
	for (int aa = 0; aa < game.numcharacters; aa++) {
		if (game.chars[aa].on != 1) continue;

		CharacterInfo *chi = &game.chars[aa];
		CharacterExtras *chex = &charextra[aa];

		chi->UpdateMoveAndAnim(aa, chex, numSheep, followingAsSheep);
	}
}

void update_following_exactly_characters(int &numSheep, int *followingAsSheep) {
	// update location of all following_exactly characters
	for (int aa = 0; aa < numSheep; aa++) {
		CharacterInfo *chi = &game.chars[followingAsSheep[aa]];

		chi->UpdateFollowingExactlyCharacter();
	}
}

void update_overlay_timers() {
	// update overlay timers
	for (size_t i = 0; i < screenover.size();) {
		if (screenover[i].timeout > 0) {
			screenover[i].timeout--;
			if (screenover[i].timeout == 0) {
				remove_screen_overlay_index(i);
				continue;
			}
		}
		i++;
	}
}

void update_speech_and_messages() {
	bool is_voice_playing = false;
	if (play.speech_has_voice) {
		AudioChannelsLock lock;
		auto *ch = lock.GetChannel(SCHAN_SPEECH);
		is_voice_playing = ch && ch->is_playing();
	}
	// determine if speech text should be removed
	if (play.messagetime >= 0) {
		play.messagetime--;
		// extend life of text if the voice hasn't finished yet
		if (play.speech_has_voice && !play.speech_in_post_state) {
			if ((is_voice_playing) && (play.fast_forward == 0)) {
				if (play.messagetime <= 1)
					play.messagetime = 1;
			} else // if the voice has finished, remove the speech
				play.messagetime = 0;
		}

		if (play.messagetime < 1 && play.speech_display_post_time_ms > 0 &&
			play.fast_forward == 0) {
			if (!play.speech_in_post_state) {
				play.messagetime = ::lround(play.speech_display_post_time_ms * get_current_fps() / 1000.0f);
			}
			play.speech_in_post_state = !play.speech_in_post_state;
		}

		if (play.messagetime < 1) {
			if (play.fast_forward > 0) {
				remove_screen_overlay(OVER_TEXTMSG);
			} else if (play.cant_skip_speech & SKIP_AUTOTIMER) {
				remove_screen_overlay(OVER_TEXTMSG);
				play.SetIgnoreInput(play.ignore_user_input_after_text_timeout_ms);
			}
		}
	}
}

// update sierra-style speech
void update_sierra_speech() {
	int voice_pos_ms = -1;
	if (play.speech_has_voice) {
		AudioChannelsLock lock;
		auto *ch = lock.GetChannel(SCHAN_SPEECH);
		voice_pos_ms = ch ? ch->get_pos_ms() : -1;
	}
	if ((face_talking >= 0) && (play.fast_forward == 0)) {
		int updatedFrame = 0;

		if ((facetalkchar->blinkview > 0) && (facetalkAllowBlink)) {
			if (facetalkchar->blinktimer > 0) {
				// countdown to playing blink anim
				facetalkchar->blinktimer--;
				if (facetalkchar->blinktimer == 0) {
					facetalkchar->blinkframe = 0;
					facetalkchar->blinktimer = -1;
					updatedFrame = 2;
				}
			} else if (facetalkchar->blinktimer < 0) {
				// currently playing blink anim
				if (facetalkchar->blinktimer < ((0 - 6) - views[facetalkchar->blinkview].loops[facetalkBlinkLoop].frames[facetalkchar->blinkframe].speed)) {
					// time to advance to next frame
					facetalkchar->blinktimer = -1;
					facetalkchar->blinkframe++;
					updatedFrame = 2;
					if (facetalkchar->blinkframe >= views[facetalkchar->blinkview].loops[facetalkBlinkLoop].numFrames) {
						facetalkchar->blinkframe = 0;
						facetalkchar->blinktimer = facetalkchar->blinkinterval;
					}
				} else
					facetalkchar->blinktimer--;
			}

		}

		if (curLipLine >= 0) {
			// check voice lip sync
			if (curLipLinePhoneme >= splipsync[curLipLine].numPhonemes) {
				// the lip-sync has finished, so just stay idle
			} else {
				while ((curLipLinePhoneme < splipsync[curLipLine].numPhonemes) &&
					((curLipLinePhoneme < 0) || (voice_pos_ms >= splipsync[curLipLine].endtimeoffs[curLipLinePhoneme]))) {
					curLipLinePhoneme++;
					if (curLipLinePhoneme >= splipsync[curLipLine].numPhonemes)
						facetalkframe = game.default_lipsync_frame;
					else
						facetalkframe = splipsync[curLipLine].frame[curLipLinePhoneme];

					if (facetalkframe >= views[facetalkview].loops[facetalkloop].numFrames)
						facetalkframe = 0;

					updatedFrame |= 1;
				}
			}
		} else if (facetalkwait > 0) facetalkwait--;
		// don't animate if the speech has finished
		else if ((play.messagetime < 1) && (facetalkframe == 0) &&
			// if play.close_mouth_speech_time = 0, this means animation should play till
			// the speech ends; but this should not work in voice mode, and also if the
			// speech is in the "post" state
			(play.speech_has_voice || play.speech_in_post_state || play.close_mouth_speech_time > 0))
			;
		else {
			// Close mouth at end of sentence: if speech has entered the "post" state,
			// or if this is a text only mode and close_mouth_speech_time is set
			if (play.speech_in_post_state ||
				(!play.speech_has_voice &&
				(play.messagetime < play.close_mouth_speech_time) &&
					(play.close_mouth_speech_time > 0))) {
				facetalkframe = 0;
				facetalkwait = play.messagetime;
			} else if ((game.options[OPT_LIPSYNCTEXT]) && (facetalkrepeat > 0)) {
				// lip-sync speech (and not a thought)
				facetalkwait = update_lip_sync(facetalkview, facetalkloop, &facetalkframe);
				// It is actually displayed for facetalkwait+1 loops
				// (because when it's 1, it gets --'d then wait for next time)
				facetalkwait--;
			} else {
				// normal non-lip-sync
				facetalkframe++;
				if ((facetalkframe >= views[facetalkview].loops[facetalkloop].numFrames) ||
					(!play.speech_has_voice && (play.messagetime < 1) && (play.close_mouth_speech_time > 0))) {

					if ((facetalkframe >= views[facetalkview].loops[facetalkloop].numFrames) &&
						(views[facetalkview].loops[facetalkloop].RunNextLoop())) {
						facetalkloop++;
					} else {
						facetalkloop = 0;
					}
					facetalkframe = 0;
					if (!facetalkrepeat)
						facetalkwait = 999999;
				}
				if ((facetalkframe != 0) || (facetalkrepeat == 1))
					facetalkwait = views[facetalkview].loops[facetalkloop].frames[facetalkframe].speed + GetCharacterSpeechAnimationDelay(facetalkchar);
			}
			updatedFrame |= 1;
		}

		// is_text_overlay might be 0 if it was only just destroyed this loop
		if ((updatedFrame) && (is_text_overlay > 0)) {

			if (updatedFrame & 1)
				CheckViewFrame(facetalkview, facetalkloop, facetalkframe);
			if (updatedFrame & 2)
				CheckViewFrame(facetalkchar->blinkview, facetalkBlinkLoop, facetalkchar->blinkframe);

			int thisPic = views[facetalkview].loops[facetalkloop].frames[facetalkframe].pic;
			int view_frame_x = 0;
			int view_frame_y = 0;

			if (game.options[OPT_SPEECHTYPE] == 3) {
				// QFG4-style fullscreen dialog
				if (facetalk_qfg4_override_placement_x) {
					view_frame_x = play.speech_portrait_x;
				}
				if (facetalk_qfg4_override_placement_y) {
					view_frame_y = play.speech_portrait_y;
				} else {
					view_frame_y = (screenover[face_talking].pic->GetHeight() / 2) - (game.SpriteInfos[thisPic].Height / 2);
				}
				screenover[face_talking].pic->Clear(0);
			} else {
				screenover[face_talking].pic->ClearTransparent();
			}

			Bitmap *frame_pic = screenover[face_talking].pic;
			const ViewFrame *face_vf = &views[facetalkview].loops[facetalkloop].frames[facetalkframe];
			bool face_has_alpha = (game.SpriteInfos[face_vf->pic].Flags & SPF_ALPHACHANNEL) != 0;
			DrawViewFrame(frame_pic, face_vf, view_frame_x, view_frame_y);

			if ((facetalkchar->blinkview > 0) && (facetalkchar->blinktimer < 0)) {
				ViewFrame *blink_vf = &views[facetalkchar->blinkview].loops[facetalkBlinkLoop].frames[facetalkchar->blinkframe];
				face_has_alpha |= (game.SpriteInfos[blink_vf->pic].Flags & SPF_ALPHACHANNEL) != 0;
				// draw the blinking sprite on top
				DrawViewFrame(frame_pic, blink_vf, view_frame_x, view_frame_y, face_has_alpha);
			}

			gfxDriver->UpdateDDBFromBitmap(screenover[face_talking].bmp, screenover[face_talking].pic, face_has_alpha);
		}  // end if updatedFrame
	}
}

// update_stuff: moves and animates objects, executes repeat scripts, and
// the like.
void update_stuff() {

	our_eip = 20;

	update_script_timers();

	update_cycling_views();

	our_eip = 21;

	update_shadow_areas();

	our_eip = 22;

	int numSheep = 0;
	int followingAsSheep[MAX_SHEEP];

	update_character_move_and_anim(numSheep, followingAsSheep);

	update_following_exactly_characters(numSheep, followingAsSheep);

	our_eip = 23;

	update_overlay_timers();

	update_speech_and_messages();

	our_eip = 24;

	update_sierra_speech();

	our_eip = 25;
}

} // namespace AGS3
