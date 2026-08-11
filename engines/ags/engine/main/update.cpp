/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//
// Game update procedure
//

#include "ags/shared/ac/common.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/character_extras.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_state.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/lip_sync.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/room_object.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/main/update.h"
#include "ags/engine/ac/screen_overlay.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/ac/view_frame.h"
#include "ags/engine/ac/walkable_area.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/ac/timer.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/ac/move_list.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;


// Optionally fixes target position, when one axis is left to move along.
// This is done only for backwards compatibility now.
// Uses generic parameters.
static void movelist_handle_targetfix(const fixed &xpermove, const fixed &ypermove, int &targety) {
	// Old comment about ancient behavior:
	// if the X-movement has finished, and the Y-per-move is < 1, finish
	// This can cause jump at the end, but without it the character will
	// walk on the spot for a while if the Y-per-move is for example 0.2
	//    if ((ypermove & 0xfffff000) == 0) cmls.doneflag|=2;
	//    int ypmm=(ypermove >> 16) & 0x0000ffff;

	// NEW 2.15 SR-1 plan: if X-movement has finished, and Y-per-move is < 1,
	// allow it to finish more easily by moving target zone
	// NOTE: interesting fact: this fix was also done for the strictly vertical
	// move, probably because of the logical mistake in condition.

	int tfix = 3;
	// 2.70: if the X permove is also <=1, don't skip as far
	if (((xpermove & 0xffff0000) == 0xffff0000) ||
		((xpermove & 0xffff0000) == 0x00000000))
		tfix = 2;

	// 2.61 RC1: correct this to work with > -1 as well as < 1
	if (ypermove == 0) {
	}
	// Y per move is < 1, so finish the move
	else if ((ypermove & 0xffff0000) == 0)
		targety -= tfix;
	// Y per move is -1 exactly, don't snap to finish
	else if ((ypermove & 0xffffffff) == 0xffff0000) {
	}
	// Y per move is > -1, so finish the move
	else if ((ypermove & 0xffff0000) == 0xffff0000)
		targety += tfix;
}

// Handle remaining move along a single axis; uses generic parameters.
static void movelist_handle_remainer(const fixed xpermove, const fixed ypermove,
									 const int xdistance, const float step_length, fixed &fin_ymove, float &fin_from_part) {
	// Walk along the remaining axis with the full walking speed
	assert(xpermove != 0 && ypermove != 0 && step_length >= 0.f);
	fin_ymove = ypermove > 0 ? ftofix(step_length) : -ftofix(step_length);
	fin_from_part = (float)xdistance / fixtof(xpermove);
	assert(fin_from_part >= 0);
}

// Handle remaining move fixup, but only if necessary
static void movelist_handle_remainer(MoveList &m) {
	assert(m.numstage > 0);
	const fixed xpermove = m.xpermove[m.onstage];
	const fixed ypermove = m.ypermove[m.onstage];
	const Point target = m.pos[m.onstage + 1];
	// Apply remainer to movelists where LONGER axis was completed, and SHORTER remains
	if ((xpermove != 0) && (ypermove != 0)) {
		if ((m.doneflag & kMoveListDone_XY) == kMoveListDone_X && (abs(ypermove) < abs(xpermove)))
			movelist_handle_remainer(xpermove, ypermove, target.X - m.from.X,
									 m.GetStepLength(), m.fin_move, m.fin_from_part);
		else if ((m.doneflag & kMoveListDone_XY) == kMoveListDone_Y && (abs(xpermove) < abs(ypermove)))
			movelist_handle_remainer(ypermove, xpermove, target.Y - m.from.Y,
									 m.GetStepLength(), m.fin_move, m.fin_from_part);
	}
}

// Test if move completed, returns if just completed
static bool movelist_handle_donemove(const uint8_t testflag, const fixed xpermove, const int targetx, uint8_t &doneflag, int &xps) {
	if ((doneflag & testflag) != 0)
		return false; // already done before

	if (((xpermove > 0) && (xps >= targetx)) || ((xpermove < 0) && (xps <= targetx))) {
		doneflag |= testflag;
		xps = targetx; // snap to the target (in case run over)
		// Comment about old engine behavior:
		// if the Y is almost there too, finish it
		// this is new in v2.40
		// removed in 2.70
		/*if (abs(yps - targety) <= 2)
			yps = targety;*/
	} else if (xpermove == 0) {
		doneflag |= testflag;
	}
	return (doneflag & testflag) != 0;
}

int do_movelist_move(short &mslot, int &pos_x, int &pos_y) {
	// TODO: find out why movelist 0 is not being used
	assert(mslot >= 1);
	if (mslot < 1)
		return 0;

	int need_to_fix_sprite = 0; // TODO: find out what this value means and refactor
	MoveList &cmls = _GP(mls)[mslot];
	const fixed xpermove = cmls.xpermove[cmls.onstage];
	const fixed ypermove = cmls.ypermove[cmls.onstage];
	const fixed fin_move = cmls.fin_move;
	const float main_onpart = (cmls.fin_from_part > 0.f) ? cmls.fin_from_part : cmls.onpart;
	const float fin_onpart = cmls.onpart - main_onpart;
	Point target = cmls.pos[cmls.onstage + 1];
	int xps = pos_x, yps = pos_y;

	// Old-style optional move target fixup
	if (_G(loaded_game_file_version) < kGameVersion_361) {
		if ((ypermove != 0) && (cmls.doneflag & kMoveListDone_X) != 0) { // X-move has finished, handle the Y-move remainer
			movelist_handle_targetfix(xpermove, ypermove, target.Y);
		} else if ((xpermove != 0) && (cmls.doneflag & kMoveListDone_Y) != 0) { // Y-move has finished, handle the X-move remainer
			movelist_handle_targetfix(xpermove, ypermove, target.Y);
		}
	}

	// Calculate next positions, as required
	if ((cmls.doneflag & kMoveListDone_X) == 0) {
		xps = cmls.from.X + (int)(fixtof(xpermove) * main_onpart) + (int)(fixtof(fin_move) * fin_onpart);
	}
	if ((cmls.doneflag & kMoveListDone_Y) == 0) {
		yps = cmls.from.Y + (int)(fixtof(ypermove) * main_onpart) + (int)(fixtof(fin_move) * fin_onpart);
	}

	// Check if finished either horizontal or vertical movement;
	// if any was finished just now, then also handle remainer fixup
	bool done_now = movelist_handle_donemove(kMoveListDone_X, xpermove, target.X, cmls.doneflag, xps);
	done_now |= movelist_handle_donemove(kMoveListDone_Y, ypermove, target.Y, cmls.doneflag, yps);
	if (done_now)
		movelist_handle_remainer(cmls);

	// Handle end of move stage
	if ((cmls.doneflag & kMoveListDone_XY) == kMoveListDone_XY) {
		// this stage is done, go on to the next stage
		cmls.from = cmls.pos[cmls.onstage + 1];
		cmls.onstage++;
		cmls.onpart = -1.f;
		cmls.fin_from_part = 0.f;
		cmls.fin_move = 0;
		cmls.doneflag = 0;
		if (cmls.onstage < cmls.numstage) {
			xps = cmls.from.X;
			yps = cmls.from.Y;
		}

		if (cmls.onstage >= cmls.numstage - 1) { // last stage is just dest pos
			cmls.numstage = 0;
			mslot = 0;
			need_to_fix_sprite = 1; // TODO: find out what this means
		} else {
			need_to_fix_sprite = 2; // TODO: find out what this means
		}
	}

	// Make a step along the current vector and return
	cmls.onpart += 1.f;
	pos_x = xps;
	pos_y = yps;
	return need_to_fix_sprite;
}

void restore_movelists() {
	// Recalculate move remainer fixups, where necessary
	for (auto &m : _GP(mls)) {
		if (m.numstage > 0)
			movelist_handle_remainer(m);
	}
}

void update_script_timers() {
	if (_GP(play).gscript_timer > 0) _GP(play).gscript_timer--;
	for (int aa = 0; aa < MAX_TIMERS; aa++) {
		if (_GP(play).script_timers[aa] > 1) _GP(play).script_timers[aa]--;
	}
}

void update_cycling_views() {
	// update graphics for object if cycling view
	for (uint32_t  i = 0; i < _G(croom)->numobj; ++i) {
		_G(objs)[i].UpdateCyclingView(i);
	}
}

// Updates the view of the player character
void update_player_view() {
	if (_G(playerchar)->flags & CHF_FIXVIEW)
		return; // view is locked

	int onwalkarea = get_walkable_area_at_character(_GP(game).playercharacter);
	if (onwalkarea < 0)
		return; // error?

	int areaview = _GP(thisroom).WalkAreas[onwalkarea].PlayerView;
	if (areaview > 0)
		_G(playerchar)->view = areaview - 1; // convert to 0-based id
	else if (_GP(thisroom).Options.PlayerView > 0)
		_G(playerchar)->view = _GP(thisroom).Options.PlayerView - 1; // convert to 0-based id
	else
		_G(playerchar)->view = _G(playerchar)->defview;
}

void update_character_move_and_anim(std::vector<int> &followingAsSheep) {
	// move & animate characters
	for (int aa = 0; aa < _GP(game).numcharacters; aa++) {
		if (_GP(game).chars[aa].on != 1) continue;

		CharacterInfo *chi = &_GP(game).chars[aa];
		CharacterExtras *chex = &_GP(charextra)[aa];

		chi->UpdateMoveAndAnim(aa, chex, followingAsSheep);
	}
}

void update_following_exactly_characters(const std::vector<int> &followingAsSheep) {
	// update location of all following_exactly characters
	for (size_t i = 0; i < followingAsSheep.size(); ++i) {
		CharacterInfo *chi = &_GP(game).chars[followingAsSheep[i]];

		chi->UpdateFollowingExactlyCharacter();
	}
}

void update_overlay_timers() {
	// update overlay timers
	auto &overs = get_overlays();
	for (auto &over : overs) {
		if (over.timeout > 0) {
			over.timeout--;
			if (over.timeout == 0) {
				remove_screen_overlay(over.type);
			}
		}
	}
}

void update_speech_and_messages() {
	bool is_voice_playing = false;
	if (_GP(play).speech_has_voice) {
		auto *ch = AudioChans::GetChannel(SCHAN_SPEECH);
		is_voice_playing = ch && ch->is_playing();
	}
	// determine if speech text should be removed
	if (_GP(play).messagetime >= 0) {
		_GP(play).messagetime--;
		// extend life of text if the voice hasn't finished yet
		if (_GP(play).speech_has_voice && !_GP(play).speech_in_post_state) {
			if ((is_voice_playing) && (_GP(play).fast_forward == 0)) {
				if (_GP(play).messagetime <= 1)
					_GP(play).messagetime = 1;
			} else  // if the voice has finished, remove the speech
				_GP(play).messagetime = 0;
		}

		// Enter speech post-state: optionally increase final waiting time
		if (!_GP(play).speech_in_post_state && (_GP(play).fast_forward == 0) && (_GP(play).messagetime < 1)) {
			_GP(play).speech_in_post_state = true;
			if (_GP(play).speech_display_post_time_ms > 0) {
				_GP(play).messagetime = ::lround(_GP(play).speech_display_post_time_ms * get_game_fps() / 1000.0f);
			}
		}

		if (_GP(play).messagetime < 1) {
			if (_GP(play).fast_forward > 0) {
				remove_screen_overlay(_GP(play).text_overlay_on);
				_GP(play).SetWaitSkipResult(SKIP_AUTOTIMER);
			} else if (_GP(play).speech_skip_style & SKIP_AUTOTIMER) {
				remove_screen_overlay(_GP(play).text_overlay_on);
				_GP(play).SetWaitSkipResult(SKIP_AUTOTIMER);
				_GP(play).SetIgnoreInput(_GP(play).ignore_user_input_after_text_timeout_ms);
			}
		}
	}
}

// update sierra-style speech
void update_sierra_speech() {
	int voice_pos_ms = -1;
	if (_GP(play).speech_has_voice) {
		auto *ch = AudioChans::GetChannel(SCHAN_SPEECH);
		voice_pos_ms = ch ? ch->get_pos_ms() : -1;
	}
	if ((_G(face_talking) >= 0) && (_GP(play).fast_forward == 0)) {
		int updatedFrame = 0;

		if ((_G(facetalkchar)->blinkview > 0) && (_G(facetalkAllowBlink))) {
			if (_G(facetalkchar)->blinktimer > 0) {
				// countdown to playing blink anim
				_G(facetalkchar)->blinktimer--;
				if (_G(facetalkchar)->blinktimer == 0) {
					_G(facetalkchar)->blinkframe = 0;
					_G(facetalkchar)->blinktimer = -1;
					updatedFrame = 2;
				}
			} else if (_G(facetalkchar)->blinktimer < 0) {
				// currently playing blink anim
				if (_G(facetalkchar)->blinktimer < ((0 - 6) - _GP(views)[_G(facetalkchar)->blinkview].loops[_G(facetalkBlinkLoop)].frames[_G(facetalkchar)->blinkframe].speed)) {
					// time to advance to next frame
					_G(facetalkchar)->blinktimer = -1;
					_G(facetalkchar)->blinkframe++;
					updatedFrame = 2;
					if (_G(facetalkchar)->blinkframe >= _GP(views)[_G(facetalkchar)->blinkview].loops[_G(facetalkBlinkLoop)].numFrames) {
						_G(facetalkchar)->blinkframe = 0;
						_G(facetalkchar)->blinktimer = _G(facetalkchar)->blinkinterval;
					}
				} else
					_G(facetalkchar)->blinktimer--;
			}

		}

		if (_G(curLipLine) >= 0) {
			// check voice lip sync
			if (_G(curLipLinePhoneme) >= _GP(splipsync)[_G(curLipLine)].numPhonemes) {
				// the lip-sync has finished, so just stay idle
			} else {
				while ((_G(curLipLinePhoneme) < _GP(splipsync)[_G(curLipLine)].numPhonemes) &&
				        ((_G(curLipLinePhoneme) < 0) || (voice_pos_ms >= _GP(splipsync)[_G(curLipLine)].endtimeoffs[_G(curLipLinePhoneme)]))) {
					_G(curLipLinePhoneme)++;
					if (_G(curLipLinePhoneme) >= _GP(splipsync)[_G(curLipLine)].numPhonemes)
						_G(facetalkframe) = _GP(game).default_lipsync_frame;
					else
						_G(facetalkframe) = _GP(splipsync)[_G(curLipLine)].frame[_G(curLipLinePhoneme)];

					if (_G(facetalkframe) >= _GP(views)[_G(facetalkview)].loops[_G(facetalkloop)].numFrames)
						_G(facetalkframe) = 0;

					updatedFrame |= 1;
				}
			}
		} else if (_G(facetalkwait) > 0) _G(facetalkwait)--;
		// don't animate if the speech has finished
		else if ((_GP(play).messagetime < 1) && (_G(facetalkframe) == 0) &&
		         // if _GP(play).close_mouth_speech_time = 0, this means animation should play till
		         // the speech ends; but this should not work in voice mode, and also if the
		         // speech is in the "post" state
		         (_GP(play).speech_has_voice || _GP(play).speech_in_post_state || _GP(play).close_mouth_speech_time > 0))
			;
		else {
			// Close mouth at end of sentence: if speech has entered the "post" state,
			// or if this is a text only mode and close_mouth_speech_time is set
			if (_GP(play).speech_in_post_state ||
			        (!_GP(play).speech_has_voice &&
			         (_GP(play).messagetime < _GP(play).close_mouth_speech_time) &&
			         (_GP(play).close_mouth_speech_time > 0))) {
				_G(facetalkframe) = 0;
				_G(facetalkwait) = _GP(play).messagetime;
			} else if ((_GP(game).options[OPT_LIPSYNCTEXT]) && (_G(facetalkrepeat) > 0)) {
				// lip-sync speech (and not a thought)
				_G(facetalkwait) = update_lip_sync(_G(facetalkview), _G(facetalkloop), &_G(facetalkframe));
				// It is actually displayed for _G(facetalkwait)+1 loops
				// (because when it's 1, it gets --'d then wait for next time)
				_G(facetalkwait)--;
			} else {
				// normal non-lip-sync
				_G(facetalkframe)++;
				if ((_G(facetalkframe) >= _GP(views)[_G(facetalkview)].loops[_G(facetalkloop)].numFrames) ||
				        (!_GP(play).speech_has_voice && (_GP(play).messagetime < 1) && (_GP(play).close_mouth_speech_time > 0))) {

					if ((_G(facetalkframe) >= _GP(views)[_G(facetalkview)].loops[_G(facetalkloop)].numFrames) &&
					        (_GP(views)[_G(facetalkview)].loops[_G(facetalkloop)].RunNextLoop())) {
						_G(facetalkloop)++;
					} else {
						_G(facetalkloop) = 0;
					}
					_G(facetalkframe) = 0;
					if (!_G(facetalkrepeat))
						_G(facetalkwait) = 999999;
				}
				if ((_G(facetalkframe) != 0) || (_G(facetalkrepeat) == 1))
					_G(facetalkwait) = _GP(views)[_G(facetalkview)].loops[_G(facetalkloop)].frames[_G(facetalkframe)].speed + GetCharacterSpeechAnimationDelay(_G(facetalkchar));
			}
			updatedFrame |= 1;
		}

		// _G(is_text_overlay) might be 0 if it was only just destroyed this loop
		if ((updatedFrame) && (_GP(play).text_overlay_on > 0)) {

			const auto &talking_chex = _GP(charextra)[_G(facetalkchar)->index_id];
			const int frame_vol = talking_chex.GetFrameSoundVolume(_G(facetalkchar));
			if (updatedFrame & 1)
				CheckViewFrame(_G(facetalkview), _G(facetalkloop), _G(facetalkframe), frame_vol);
			if (updatedFrame & 2)
				CheckViewFrame(_G(facetalkchar)->blinkview, _G(facetalkBlinkLoop), _G(facetalkchar)->blinkframe, frame_vol);

			int thisPic = _GP(views)[_G(facetalkview)].loops[_G(facetalkloop)].frames[_G(facetalkframe)].pic;
			int view_frame_x = 0;
			int view_frame_y = 0;

			auto *face_over = get_overlay(_G(face_talking));
			assert(face_over != nullptr);
			Bitmap *frame_pic = _GP(spriteset)[face_over->GetSpriteNum()];
			if (_GP(game).options[OPT_SPEECHTYPE] == 3) {
				// QFG4-style fullscreen dialog
				if (_G(facetalk_qfg4_override_placement_x)) {
					view_frame_x = _GP(play).speech_portrait_x;
				}
				if (_G(facetalk_qfg4_override_placement_y)) {
					view_frame_y = _GP(play).speech_portrait_y;
				} else {
					view_frame_y = (frame_pic->GetHeight() / 2) - (_GP(game).SpriteInfos[thisPic].Height / 2);
				}
				frame_pic->Clear(0);
			} else {
				frame_pic->ClearTransparent();
			}

			const ViewFrame *face_vf = &_GP(views)[_G(facetalkview)].loops[_G(facetalkloop)].frames[_G(facetalkframe)];
			bool face_has_alpha = (_GP(game).SpriteInfos[face_vf->pic].Flags & SPF_ALPHACHANNEL) != 0;
			DrawViewFrame(frame_pic, face_vf, view_frame_x, view_frame_y);

			if ((_G(facetalkchar)->blinkview > 0) && (_G(facetalkchar)->blinktimer < 0)) {
				ViewFrame *blink_vf = &_GP(views)[_G(facetalkchar)->blinkview].loops[_G(facetalkBlinkLoop)].frames[_G(facetalkchar)->blinkframe];
				face_has_alpha |= (_GP(game).SpriteInfos[blink_vf->pic].Flags & SPF_ALPHACHANNEL) != 0;
				// draw the blinking sprite on top
				DrawViewFrame(frame_pic, blink_vf, view_frame_x, view_frame_y, face_has_alpha);
			}

			face_over->SetAlphaChannel(face_has_alpha);
			face_over->MarkChanged();
		}  // end if updatedFrame
	}
}

// update_stuff: moves and animates objects, executes repeat scripts, and
// the like.
void update_stuff() {

	set_our_eip(20);

	update_script_timers();

	update_cycling_views();

	set_our_eip(21);

	update_player_view();

	set_our_eip(22);

	std::vector<int> followingAsSheep;

	update_character_move_and_anim(followingAsSheep);

	update_following_exactly_characters(followingAsSheep);

	set_our_eip(23);

	update_overlay_timers();

	update_speech_and_messages();

	set_our_eip(24);

	update_sierra_speech();

	set_our_eip(25);
}

} // namespace AGS3
