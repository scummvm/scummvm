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
// Game loop
//

#include "ags/lib/std/limits.h"
#include "ags/lib/std/chrono.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/characterextras.h"
#include "ags/shared/ac/characterinfo.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/gamesetup.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/global_debug.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/ac/global_region.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/hotspot.h"
#include "ags/engine/ac/keycode.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/roomobject.h"
#include "ags/engine/ac/roomstatus.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/gui/guiinv.h"
#include "ags/shared/gui/guimain.h"
#include "ags/shared/gui/guitextbox.h"
#include "ags/engine/main/mainheader.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/main/update.h"
#include "ags/engine/plugin/agsplugin.h"
#include "ags/engine/plugin/plugin_engine.h"
#include "ags/engine/script/script.h"
#include "ags/shared/ac/spritecache.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/engine/ac/timer.h"
#include "ags/engine/ac/keycode.h"
#include "ags/lib/allegro/keyboard.h"
#include "ags/engine/globals.h"
#include "ags/events.h"

namespace AGS3 {

using namespace AGS::Shared;

extern AnimatingGUIButton animbuts[MAX_ANIMATING_BUTTONS];
extern int numAnimButs;
extern int mouse_on_iface;   // mouse cursor is over this interface
extern int ifacepopped;
extern int is_text_overlay;
extern int proper_exit, our_eip;
extern int displayed_room, starting_room, in_new_room, new_room_was;
extern GameSetupStruct game;
extern RoomStruct thisroom;
extern int game_paused;
extern int getloctype_index;
extern int in_enters_screen, done_es_error;
extern int in_leaves_screen;
extern int inside_script, in_graph_script;
extern int no_blocking_functions;
extern CharacterInfo *playerchar;
extern GameState play;
extern int mouse_ifacebut_xoffs, mouse_ifacebut_yoffs;
extern int cur_mode;
extern RoomObject *objs;
extern char noWalkBehindsAtAll;
extern RoomStatus *croom;
extern CharacterExtras *charextra;
extern SpriteCache spriteset;
extern int cur_mode, cur_cursor;

// Checks if user interface should remain disabled for now
static int ShouldStayInWaitMode();

static int numEventsAtStartOfFunction;
static uint32 t1 = 0; // AGS_Clock::now();  // timer for FPS // ... 't1'... how very appropriate.. :)

#define UNTIL_ANIMEND   1
#define UNTIL_MOVEEND   2
#define UNTIL_CHARIS0   3
#define UNTIL_NOOVERLAY 4
#define UNTIL_NEGATIVE  5
#define UNTIL_INTIS0    6
#define UNTIL_SHORTIS0  7
#define UNTIL_INTISNEG  8

// Following 3 parameters instruct the engine to run game loops until
// certain condition is not fullfilled.
static int restrict_until = 0;
static int user_disabled_for = 0;
static const void *user_disabled_data = nullptr;

unsigned int loopcounter = 0;
static unsigned int lastcounter = 0;

static void ProperExit() {
	_G(want_exit) = 0;
	proper_exit = 1;
	quit("||exit!");
}

static void game_loop_check_problems_at_start() {
	if ((in_enters_screen != 0) & (displayed_room == starting_room))
		quit("!A text script run in the Player Enters Screen event caused the\n"
			"screen to be updated. If you need to use Wait(), do so in After Fadein");
	if ((in_enters_screen != 0) && (done_es_error == 0)) {
		debug_script_warn("Wait() was used in Player Enters Screen - use Enters Screen After Fadein instead");
		done_es_error = 1;
	}
	if (no_blocking_functions)
		quit("!A blocking function was called from within a non-blocking event such as " REP_EXEC_ALWAYS_NAME);
}

static void game_loop_check_new_room() {
	if (in_new_room == 0) {
		// Run the room and game script repeatedly_execute
		run_function_on_non_blocking_thread(&repExecAlways);
		setevent(EV_TEXTSCRIPT, TS_REPEAT);
		setevent(EV_RUNEVBLOCK, EVB_ROOM, 0, 6);
	}
	// run this immediately to make sure it gets done before fade-in
	// (player enters screen)
	check_new_room();
}

static void game_loop_do_late_update() {
	if (in_new_room == 0) {
		// Run the room and game script late_repeatedly_execute
		run_function_on_non_blocking_thread(&lateRepExecAlways);
	}
}

static int game_loop_check_ground_level_interactions() {
	if ((play.ground_level_areas_disabled & GLED_INTERACTION) == 0) {
		// check if he's standing on a hotspot
		int hotspotThere = get_hotspot_at(playerchar->x, playerchar->y);
		// run Stands on Hotspot event
		setevent(EV_RUNEVBLOCK, EVB_HOTSPOT, hotspotThere, 0);

		// check current region
		int onRegion = GetRegionIDAtRoom(playerchar->x, playerchar->y);
		int inRoom = displayed_room;

		if (onRegion != play.player_on_region) {
			// we need to save this and set play.player_on_region
			// now, so it's correct going into RunRegionInteraction
			int oldRegion = play.player_on_region;

			play.player_on_region = onRegion;
			// Walks Off last region
			if (oldRegion > 0)
				RunRegionInteraction(oldRegion, 2);
			// Walks Onto new region
			if (onRegion > 0)
				RunRegionInteraction(onRegion, 1);
		}
		if (play.player_on_region > 0)   // player stands on region
			RunRegionInteraction(play.player_on_region, 0);

		// one of the region interactions sent us to another room
		if (inRoom != displayed_room) {
			check_new_room();
		}

		// if in a Wait loop which is no longer valid (probably
		// because the Region interaction did a NewRoom), abort
		// the rest of the loop
		if ((restrict_until) && (!ShouldStayInWaitMode())) {
			// cancel the Rep Exec and Stands on Hotspot events that
			// we just added -- otherwise the event queue gets huge
			numevents = numEventsAtStartOfFunction;
			return 0;
		}
	} // end if checking ground level interactions

	return RETURN_CONTINUE;
}

static void lock_mouse_on_click() {
	if (usetup.mouse_auto_lock && scsystem.windowed)
		Mouse::TryLockToWindow();
}

static void toggle_mouse_lock() {
	if (scsystem.windowed) {
		if (Mouse::IsLockedToWindow())
			Mouse::UnlockFromWindow();
		else
			Mouse::TryLockToWindow();
	}
}

// Runs default mouse button handling
static void check_mouse_controls() {
	int mongu = -1;

	mongu = gui_on_mouse_move();

	mouse_on_iface = mongu;
	if ((ifacepopped >= 0) && (_G(mousey) >= guis[ifacepopped].Y + guis[ifacepopped].Height))
		remove_popup_interface(ifacepopped);

	// check mouse clicks on GUIs
	static int wasbutdown = 0, wasongui = 0;

	if ((wasbutdown > 0) && (ags_misbuttondown(wasbutdown - 1))) {
		gui_on_mouse_hold(wasongui, wasbutdown);
	} else if ((wasbutdown > 0) && (!ags_misbuttondown(wasbutdown - 1))) {
		gui_on_mouse_up(wasongui, wasbutdown);
		wasbutdown = 0;
	}

	int mbut = NONE;
	int mwheelz = 0;
	if (run_service_mb_controls(mbut, mwheelz) && mbut >= 0) {

		check_skip_cutscene_mclick(mbut);

		if (play.fast_forward || play.IsIgnoringInput()) {
			/* do nothing if skipping cutscene or input disabled */
		} else if ((play.wait_counter != 0) && (play.key_skip_wait & SKIP_MOUSECLICK) != 0) {
			play.wait_counter = 0;
			play.wait_skipped_by = SKIP_MOUSECLICK;
			play.wait_skipped_by_data = mbut;
		} else if (is_text_overlay > 0) {
			if (play.cant_skip_speech & SKIP_MOUSECLICK)
				remove_screen_overlay(OVER_TEXTMSG);
		} else if (!IsInterfaceEnabled()); // blocking cutscene, ignore mouse
		else if (pl_run_plugin_hooks(AGSE_MOUSECLICK, mbut + 1)) {
			// plugin took the click
			debug_script_log("Plugin handled mouse button %d", mbut + 1);
		} else if (mongu >= 0) {
			if (wasbutdown == 0) {
				gui_on_mouse_down(mongu, mbut + 1);
			}
			wasongui = mongu;
			wasbutdown = mbut + 1;
		} else setevent(EV_TEXTSCRIPT, TS_MCLICK, mbut + 1);
		//    else RunTextScriptIParam(gameinst,"on_mouse_click",aa+1);
	}

	if (mwheelz < 0)
		setevent(EV_TEXTSCRIPT, TS_MCLICK, 9);
	else if (mwheelz > 0)
		setevent(EV_TEXTSCRIPT, TS_MCLICK, 8);
}

// Returns current key modifiers;
// NOTE: annoyingly enough, on Windows (not sure about other platforms)
// Allegro API's 'key_shifts' variable seem to be always one step behind real
// situation: if first modifier gets pressed, 'key_shifts' will be zero,
// when second modifier gets pressed it will only contain first one, and so on.
static int get_active_shifts() {
	return ::AGS::g_events->getModifierFlags();
}

// Special flags to OR saved shift flags with:
// Shifts key combination already fired (wait until full shifts release)
#define KEY_SHIFTS_FIRED      0x80000000

// Runs service key controls, returns false if service key combinations were handled
// and no more processing required, otherwise returns true and provides current keycode and key shifts.
bool run_service_key_controls(int &kgn) {
	// check keypresses
	static int old_key_shifts = 0; // for saving shift modes

	bool handled = false;
	int kbhit_res = ags_kbhit();
	// First, check shifts
	const int act_shifts = get_active_shifts();
	// If shifts combination have already triggered an action, then do nothing
	// until new shifts are empty, in which case reset saved shifts
	if (old_key_shifts & KEY_SHIFTS_FIRED) {
		if (act_shifts == 0)
			old_key_shifts = 0;
	} else {
		// If any non-shift key is pressed, add fired flag to indicate that
		// this is no longer a pure shifts key combination
		if (kbhit_res) {
			old_key_shifts = act_shifts | KEY_SHIFTS_FIRED;
		}
		// If all the previously registered shifts are still pressed,
		// then simply resave new shift state.
		else if ((old_key_shifts & act_shifts) == old_key_shifts) {
			old_key_shifts = act_shifts;
		}
		// Otherwise some of the shifts were released, then run key combo action
		// and set KEY_COMBO_FIRED flag to prevent multiple execution
		else if (old_key_shifts) {
			// Toggle mouse lock on Ctrl + Alt
			if (old_key_shifts == (KB_ALT_FLAG | KB_CTRL_FLAG)) {
				toggle_mouse_lock();
				handled = true;
			}
			old_key_shifts |= KEY_SHIFTS_FIRED;
		}
	}

	if (!kbhit_res || handled)
		return false;

	int keycode = ags_getch();
	// NS: I'm still not sure why we read a second key.
	// Perhaps it's of a time when we read the keyboard one byte at a time?
	// if (keycode == 0)
	//     keycode = ags_getch() + AGS_EXT_KEY_SHIFT;
	if (keycode == 0)
		return false;

	// LAlt or RAlt + Enter
	// NOTE: for some reason LAlt + Enter produces same code as F9
	if (act_shifts == KB_ALT_FLAG && (
			(keycode == eAGSKeyCodeF9 && !::AGS::g_events->isKeyPressed(KEY_F9)) ||
			keycode == eAGSKeyCodeReturn)
	) {
		engine_try_switch_windowed_gfxmode();
		return false;
	}

	// No service operation triggered? return active keypress and shifts to caller
	kgn = keycode;
	return true;
}

bool run_service_mb_controls(int &mbut, int &mwheelz) {
	int mb = ags_mgetbutton();
	int mz = ags_check_mouse_wheel();
	if (mb == NONE && mz == 0)
		return false;
	lock_mouse_on_click(); // do not claim
	mbut = mb;
	mwheelz = mz;
	return true;
}

// Runs default keyboard handling
static void check_keyboard_controls() {
	// First check for service engine's combinations (mouse lock, display mode switch, and so forth)
	int kgn;
	if (!run_service_key_controls(kgn)) {
		return;
	}
	// Then, check cutscene skip
	check_skip_cutscene_keypress(kgn);
	if (play.fast_forward) {
		return;
	}
	if (play.IsIgnoringInput()) {
		return;
	}
	// Now check for in-game controls
	if (pl_run_plugin_hooks(AGSE_KEYPRESS, kgn)) {
		// plugin took the keypress
		debug_script_log("Keypress code %d taken by plugin", kgn);
		return;
	}

	// debug console
	if ((kgn == '`') && (play.debug_mode > 0)) {
		display_console = !display_console;
		return;
	}

	// skip speech if desired by Speech.SkipStyle
	if ((is_text_overlay > 0) && (play.cant_skip_speech & SKIP_KEYPRESS)) {
		// only allow a key to remove the overlay if the icon bar isn't up
		if (IsGamePaused() == 0) {
			// check if it requires a specific keypress
			if ((play.skip_speech_specific_key > 0) &&
				(kgn != play.skip_speech_specific_key)) {
			} else
				remove_screen_overlay(OVER_TEXTMSG);
		}

		return;
	}

	if ((play.wait_counter != 0) && (play.key_skip_wait & SKIP_KEYPRESS) != 0) {
		play.wait_counter = 0;
		play.wait_skipped_by = SKIP_KEYPRESS;
		play.wait_skipped_by_data = kgn;
		debug_script_log("Keypress code %d ignored - in Wait", kgn);
		return;
	}

	if ((kgn == eAGSKeyCodeCtrlE) && (display_fps == kFPS_Forced)) {
		// if --fps paramter is used, Ctrl+E will max out frame rate
		setTimerFps(isTimerFpsMaxed() ? frames_per_second : 1000);
		return;
	}

	if ((kgn == eAGSKeyCodeCtrlD) && (play.debug_mode > 0)) {
		// ctrl+D - show info
		char infobuf[900];
		int ff;
		// MACPORT FIX 9/6/5: added last %s
		sprintf(infobuf, "In room %d %s[Player at %d, %d (view %d, loop %d, frame %d)%s%s%s",
			displayed_room, (noWalkBehindsAtAll ? "(has no walk-behinds)" : ""), playerchar->x, playerchar->y,
			playerchar->view + 1, playerchar->loop, playerchar->frame,
			(IsGamePaused() == 0) ? "" : "[Game paused.",
			(play.ground_level_areas_disabled == 0) ? "" : "[Ground areas disabled.",
			(IsInterfaceEnabled() == 0) ? "[Game in Wait state" : "");
		for (ff = 0; ff < croom->numobj; ff++) {
			if (ff >= 8) break; // buffer not big enough for more than 7
			sprintf(&infobuf[strlen(infobuf)],
				"[Object %d: (%d,%d) size (%d x %d) on:%d moving:%s animating:%d slot:%d trnsp:%d clkble:%d",
				ff, objs[ff].x, objs[ff].y,
				(spriteset[objs[ff].num] != nullptr) ? game.SpriteInfos[objs[ff].num].Width : 0,
				(spriteset[objs[ff].num] != nullptr) ? game.SpriteInfos[objs[ff].num].Height : 0,
				objs[ff].on,
				(objs[ff].moving > 0) ? "yes" : "no", objs[ff].cycling,
				objs[ff].num, objs[ff].transparent,
				((objs[ff].flags & OBJF_NOINTERACT) != 0) ? 0 : 1);
		}
		Display(infobuf);
		int chd = game.playercharacter;
		char bigbuffer[STD_BUFFER_SIZE] = "CHARACTERS IN THIS ROOM:[";
		for (ff = 0; ff < game.numcharacters; ff++) {
			if (game.chars[ff].room != displayed_room) continue;
			if (strlen(bigbuffer) > 430) {
				strcat(bigbuffer, "and more...");
				Display(bigbuffer);
				strcpy(bigbuffer, "CHARACTERS IN THIS ROOM (cont'd):[");
			}
			chd = ff;
			sprintf(&bigbuffer[strlen(bigbuffer)],
				"%s (view/loop/frm:%d,%d,%d  x/y/z:%d,%d,%d  idleview:%d,time:%d,left:%d walk:%d anim:%d follow:%d flags:%X wait:%d zoom:%d)[",
				game.chars[chd].scrname, game.chars[chd].view + 1, game.chars[chd].loop, game.chars[chd].frame,
				game.chars[chd].x, game.chars[chd].y, game.chars[chd].z,
				game.chars[chd].idleview, game.chars[chd].idletime, game.chars[chd].idleleft,
				game.chars[chd].walking, game.chars[chd].animating, game.chars[chd].following,
				game.chars[chd].flags, game.chars[chd].wait, charextra[chd].zoom);
		}
		Display(bigbuffer);

		return;
	}

	// if (kgn == key_ctrl_u) {
	//     play.debug_mode++;
	//     script_debug(5,0);
	//     play.debug_mode--;
	//     return;
	// }

	if (kgn == eAGSKeyCodeAltV && (::AGS::g_events->getModifierFlags() & KB_CTRL_FLAG)
			&& (play.wait_counter < 1) && (is_text_overlay == 0) && (restrict_until == 0)) {
		// make sure we can't interrupt a Wait()
		// and desync the music to cutscene
		play.debug_mode++;
		script_debug(1, 0);
		play.debug_mode--;

		return;
	}

	if (inside_script) {
		// Don't queue up another keypress if it can't be run instantly
		debug_script_log("Keypress %d ignored (game blocked)", kgn);
		return;
	}

	int keywasprocessed = 0;

	// determine if a GUI Text Box should steal the click
	// it should do if a displayable character (32-255) is
	// pressed, but exclude control characters (<32) and
	// extended keys (eg. up/down arrow; 256+)
	if ((((kgn >= 32) && (kgn <= 255) && (kgn != '[')) || (kgn == eAGSKeyCodeReturn) || (kgn == eAGSKeyCodeBackspace))
		&& !all_buttons_disabled) {
		for (int guiIndex = 0; guiIndex < game.numgui; guiIndex++) {
			auto &gui = guis[guiIndex];

			if (!gui.IsDisplayed()) continue;

			for (int controlIndex = 0; controlIndex < gui.GetControlCount(); controlIndex++) {
				// not a text box, ignore it
				if (gui.GetControlType(controlIndex) != kGUITextBox) {
					continue;
				}

				auto *guitex = static_cast<GUITextBox *>(gui.GetControl(controlIndex));
				if (guitex == nullptr) {
					continue;
				}

				// if the text box is disabled, it cannot accept keypresses
				if (!guitex->IsEnabled()) {
					continue;
				}
				if (!guitex->IsVisible()) {
					continue;
				}

				keywasprocessed = 1;

				guitex->OnKeyPress(kgn);

				if (guitex->IsActivated) {
					guitex->IsActivated = false;
					setevent(EV_IFACECLICK, guiIndex, controlIndex, 1);
				}
			}
		}
	}

	if (!keywasprocessed) {
		kgn = GetKeyForKeyPressCb(kgn);
		debug_script_log("Running on_key_press keycode %d", kgn);
		setevent(EV_TEXTSCRIPT, TS_KEYPRESS, kgn);
	}

	// RunTextScriptIParam(gameinst,"on_key_press",kgn);
}

// check_controls: checks mouse & keyboard interface
static void check_controls() {
	our_eip = 1007;

	check_mouse_controls();
	check_keyboard_controls();
}

static void check_room_edges(int numevents_was) {
	if ((IsInterfaceEnabled()) && (IsGamePaused() == 0) &&
		(in_new_room == 0) && (new_room_was == 0)) {
		// Only allow walking off edges if not in wait mode, and
		// if not in Player Enters Screen (allow walking in from off-screen)
		int edgesActivated[4] = { 0, 0, 0, 0 };
		// Only do it if nothing else has happened (eg. mouseclick)
		if ((numevents == numevents_was) &&
			((play.ground_level_areas_disabled & GLED_INTERACTION) == 0)) {

			if (playerchar->x <= thisroom.Edges.Left)
				edgesActivated[0] = 1;
			else if (playerchar->x >= thisroom.Edges.Right)
				edgesActivated[1] = 1;
			if (playerchar->y >= thisroom.Edges.Bottom)
				edgesActivated[2] = 1;
			else if (playerchar->y <= thisroom.Edges.Top)
				edgesActivated[3] = 1;

			if ((play.entered_edge >= 0) && (play.entered_edge <= 3)) {
				// once the player is no longer outside the edge, forget the stored edge
				if (edgesActivated[play.entered_edge] == 0)
					play.entered_edge = -10;
				// if we are walking in from off-screen, don't activate edges
				else
					edgesActivated[play.entered_edge] = 0;
			}

			for (int ii = 0; ii < 4; ii++) {
				if (edgesActivated[ii])
					setevent(EV_RUNEVBLOCK, EVB_ROOM, 0, ii);
			}
		}
	}
	our_eip = 1008;

}

static void game_loop_check_controls(bool checkControls) {
	// don't let the player do anything before the screen fades in
	if ((in_new_room == 0) && (checkControls)) {
		int inRoom = displayed_room;
		int numevents_was = numevents;
		check_controls();
		check_room_edges(numevents_was);
		// If an inventory interaction changed the room
		if (inRoom != displayed_room)
			check_new_room();
	}
}

static void game_loop_do_update() {
	if (debug_flags & DBG_NOUPDATE);
	else if (game_paused == 0) update_stuff();
}

static void game_loop_update_animated_buttons() {
	// update animating GUI buttons
	// this bit isn't in update_stuff because it always needs to
	// happen, even when the game is paused
	for (int aa = 0; aa < numAnimButs; aa++) {
		if (UpdateAnimatingButton(aa)) {
			StopButtonAnimation(aa);
			aa--;
		}
	}
}

static void game_loop_do_render_and_check_mouse(IDriverDependantBitmap *extraBitmap, int extraX, int extraY) {
	if (!play.fast_forward) {
		int mwasatx = _G(mousex), mwasaty = _G(mousey);

		// Only do this if we are not skipping a cutscene
		render_graphics(extraBitmap, extraX, extraY);

		// Check Mouse Moves Over Hotspot event
		// TODO: move this out of render related function? find out why we remember mwasatx and mwasaty before render
		// TODO: do not use static variables!
		// TODO: if we support rotation then we also need to compare full transform!
		if (displayed_room < 0)
			return;
		auto view = play.GetRoomViewportAt(_G(mousex), _G(mousey));
		auto cam = view ? view->GetCamera() : nullptr;
		if (cam) {
			// NOTE: all cameras are in same room right now, so their positions are in same coordinate system;
			// therefore we may use this as an indication that mouse is over different camera too.
			static int offsetxWas = -1000, offsetyWas = -1000;
			int offsetx = cam->GetRect().Left;
			int offsety = cam->GetRect().Top;

			if (((mwasatx != _G(mousex)) || (mwasaty != _G(mousey)) ||
				(offsetxWas != offsetx) || (offsetyWas != offsety))) {
				// mouse moves over hotspot
				if (__GetLocationType(game_to_data_coord(_G(mousex)), game_to_data_coord(_G(mousey)), 1) == LOCTYPE_HOTSPOT) {
					int onhs = getloctype_index;

					setevent(EV_RUNEVBLOCK, EVB_HOTSPOT, onhs, 6);
				}
			}

			offsetxWas = offsetx;
			offsetyWas = offsety;
		} // camera found under mouse
	}
}

static void game_loop_update_events() {
	new_room_was = in_new_room;
	if (in_new_room > 0)
		setevent(EV_FADEIN, 0, 0, 0);
	in_new_room = 0;
	update_events();
	if ((new_room_was > 0) && (in_new_room == 0)) {
		// if in a new room, and the room wasn't just changed again in update_events,
		// then queue the Enters Screen scripts
		// run these next time round, when it's faded in
		if (new_room_was == 2) // first time enters screen
			setevent(EV_RUNEVBLOCK, EVB_ROOM, 0, 4);
		if (new_room_was != 3) // enters screen after fadein
			setevent(EV_RUNEVBLOCK, EVB_ROOM, 0, 7);
	}
}

static void game_loop_update_background_animation() {
	if (play.bg_anim_delay > 0) play.bg_anim_delay--;
	else if (play.bg_frame_locked);
	else {
		play.bg_anim_delay = play.anim_background_speed;
		play.bg_frame++;
		if ((size_t)play.bg_frame >= thisroom.BgFrameCount)
			play.bg_frame = 0;
		if (thisroom.BgFrameCount >= 2) {
			// get the new frame's palette
			on_background_frame_change();
		}
	}
}

static void game_loop_update_loop_counter() {
	loopcounter++;

	if (play.wait_counter > 0) play.wait_counter--;
	if (play.shakesc_length > 0) play.shakesc_length--;

	if (loopcounter % 5 == 0) {
		update_ambient_sound_vol();
		update_directional_sound_vol();
	}
}

static void game_loop_update_fps() {
	auto t2 = AGS_Clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
	auto frames = loopcounter - lastcounter;

	if (duration >= std::chrono::milliseconds(1000) && frames > 0) {
		fps = 1000.0f * frames / duration.count();
		t1 = t2;
		lastcounter = loopcounter;
	}
}

float get_current_fps() {
	// if we have maxed out framerate then return the frame rate we're seeing instead
	// fps must be greater that 0 or some timings will take forever.
	if (isTimerFpsMaxed() && fps > 0.0f) {
		return fps;
	}
	return frames_per_second;
}

void set_loop_counter(unsigned int new_counter) {
	loopcounter = new_counter;
	t1 = AGS_Clock::now();
	lastcounter = loopcounter;
	fps = std::numeric_limits<float>::quiet_NaN();
}

void UpdateGameOnce(bool checkControls, IDriverDependantBitmap *extraBitmap, int extraX, int extraY) {

	int res;

	update_polled_mp3();

	numEventsAtStartOfFunction = numevents;

	if (_G(want_exit)) {
		ProperExit();
	}

	ccNotifyScriptStillAlive();
	our_eip = 1;

	game_loop_check_problems_at_start();

	// if we're not fading in, don't count the fadeouts
	if ((play.no_hicolor_fadein) && (game.options[OPT_FADETYPE] == FADE_NORMAL))
		play.screen_is_faded_out = 0;

	our_eip = 1014;

	update_gui_disabled_status();

	our_eip = 1004;

	game_loop_check_new_room();

	our_eip = 1005;

	res = game_loop_check_ground_level_interactions();
	if (res != RETURN_CONTINUE) {
		return;
	}

	mouse_on_iface = -1;

	check_debug_keys();

	game_loop_check_controls(checkControls);

	our_eip = 2;

	game_loop_do_update();

	game_loop_update_animated_buttons();

	game_loop_do_late_update();

	update_audio_system_on_game_loop();

	game_loop_do_render_and_check_mouse(extraBitmap, extraX, extraY);

	our_eip = 6;

	game_loop_update_events();

	our_eip = 7;

	//    if (ags_mgetbutton()>NONE) break;
	update_polled_stuff_if_runtime();

	game_loop_update_background_animation();

	game_loop_update_loop_counter();

	// Immediately start the next frame if we are skipping a cutscene
	if (play.fast_forward)
		return;

	our_eip = 72;

	game_loop_update_fps();

	update_polled_stuff_if_runtime();

	WaitForNextFrame();
}

static void UpdateMouseOverLocation() {
	// Call GetLocationName - it will internally force a GUI refresh
	// if the result it returns has changed from last time
	char tempo[STD_BUFFER_SIZE];
	GetLocationName(game_to_data_coord(_G(mousex)), game_to_data_coord(_G(mousey)), tempo);

	if ((play.get_loc_name_save_cursor >= 0) &&
		(play.get_loc_name_save_cursor != play.get_loc_name_last_time) &&
		(mouse_on_iface < 0) && (ifacepopped < 0)) {
		// we have saved the cursor, but the mouse location has changed
		// and it's time to restore it
		play.get_loc_name_save_cursor = -1;
		set_cursor_mode(play.restore_cursor_mode_to);

		if (cur_mode == play.restore_cursor_mode_to) {
			// make sure it changed -- the new mode might have been disabled
			// in which case don't change the image
			set_mouse_cursor(play.restore_cursor_image_to);
		}
		debug_script_log("Restore mouse to mode %d cursor %d", play.restore_cursor_mode_to, play.restore_cursor_image_to);
	}
}

// Checks if user interface should remain disabled for now
static int ShouldStayInWaitMode() {
	if (restrict_until == 0)
		quit("end_wait_loop called but game not in loop_until state");
	int retval = restrict_until;

	if (restrict_until == UNTIL_MOVEEND) {
		const short *wkptr = (const short *)user_disabled_data;
		if (wkptr[0] < 1) retval = 0;
	} else if (restrict_until == UNTIL_CHARIS0) {
		const char *chptr = (const char *)user_disabled_data;
		if (chptr[0] == 0) retval = 0;
	} else if (restrict_until == UNTIL_NEGATIVE) {
		const short *wkptr = (const short *)user_disabled_data;
		if (wkptr[0] < 0) retval = 0;
	} else if (restrict_until == UNTIL_INTISNEG) {
		const int *wkptr = (const int *)user_disabled_data;
		if (wkptr[0] < 0) retval = 0;
	} else if (restrict_until == UNTIL_NOOVERLAY) {
		if (is_text_overlay < 1) retval = 0;
	} else if (restrict_until == UNTIL_INTIS0) {
		const int *wkptr = (const int *)user_disabled_data;
		if (wkptr[0] == 0) retval = 0;
	} else if (restrict_until == UNTIL_SHORTIS0) {
		const short *wkptr = (const short *)user_disabled_data;
		if (wkptr[0] == 0) retval = 0;
	} else quit("loop_until: unknown until event");

	return retval;
}

static int UpdateWaitMode() {
	if (restrict_until == 0) {
		return RETURN_CONTINUE;
	}

	restrict_until = ShouldStayInWaitMode();
	our_eip = 77;

	if (restrict_until != 0) {
		return RETURN_CONTINUE;
	}

	auto was_disabled_for = user_disabled_for;

	set_default_cursor();
	guis_need_update = 1;
	play.disabled_user_interface--;
	user_disabled_for = 0;

	switch (was_disabled_for) {
		// case FOR_ANIMATION:
		//     run_animation((FullAnimation*)user_disabled_data2,user_disabled_data3);
		//     break;
	case FOR_EXITLOOP:
		return -1;
	case FOR_SCRIPT:
		quit("err: for_script obsolete (v2.1 and earlier only)");
		break;
	default:
		quit("Unknown user_disabled_for in end restrict_until");
	}

	// we shouldn't get here.
	return RETURN_CONTINUE;
}

// Run single game iteration; calls UpdateGameOnce() internally
static int GameTick() {
	if (displayed_room < 0)
		quit("!A blocking function was called before the first room has been loaded");

	UpdateGameOnce(true);
	UpdateMouseOverLocation();

	our_eip = 76;

	int res = UpdateWaitMode();
	if (res == RETURN_CONTINUE) {
		return 0;    // continue looping
	}
	return res;
}

static void SetupLoopParameters(int untilwhat, const void *udata) {
	play.disabled_user_interface++;
	guis_need_update = 1;
	// Only change the mouse cursor if it hasn't been specifically changed first
	// (or if it's speech, always change it)
	if (((cur_cursor == cur_mode) || (untilwhat == UNTIL_NOOVERLAY)) &&
		(cur_mode != CURS_WAIT))
		set_mouse_cursor(CURS_WAIT);

	restrict_until = untilwhat;
	user_disabled_data = udata;
	user_disabled_for = FOR_EXITLOOP;
}

// This function is called from lot of various functions
// in the game core, character, room object etc
static void GameLoopUntilEvent(int untilwhat, const void *daaa) {
	// blocking cutscene - end skipping
	EndSkippingUntilCharStops();

	// this function can get called in a nested context, so
	// remember the state of these vars in case a higher level
	// call needs them
	auto cached_restrict_until = restrict_until;
	auto cached_user_disabled_data = user_disabled_data;
	auto cached_user_disabled_for = user_disabled_for;

	SetupLoopParameters(untilwhat, daaa);
	while (GameTick() == 0 && !_G(abort_engine)) {
	}

	our_eip = 78;

	restrict_until = cached_restrict_until;
	user_disabled_data = cached_user_disabled_data;
	user_disabled_for = cached_user_disabled_for;
}

void GameLoopUntilValueIsZero(const char *value) {
	GameLoopUntilEvent(UNTIL_CHARIS0, value);
}

void GameLoopUntilValueIsZero(const short *value) {
	GameLoopUntilEvent(UNTIL_SHORTIS0, value);
}

void GameLoopUntilValueIsZero(const int *value) {
	GameLoopUntilEvent(UNTIL_INTIS0, value);
}

void GameLoopUntilValueIsZeroOrLess(const short *value) {
	GameLoopUntilEvent(UNTIL_MOVEEND, value);
}

void GameLoopUntilValueIsNegative(const short *value) {
	GameLoopUntilEvent(UNTIL_NEGATIVE, value);
}

void GameLoopUntilValueIsNegative(const int *value) {
	GameLoopUntilEvent(UNTIL_INTISNEG, value);
}

void GameLoopUntilNotMoving(const short *move) {
	GameLoopUntilEvent(UNTIL_MOVEEND, move);
}

void GameLoopUntilNoOverlay() {
	GameLoopUntilEvent(UNTIL_NOOVERLAY, 0);
}


extern unsigned int load_new_game;
void RunGameUntilAborted() {
	// skip ticks to account for time spent starting game.
	skipMissedTicks();

	while (!_G(abort_engine)) {
		GameTick();

		if (load_new_game) {
			RunAGSGame(nullptr, load_new_game, 0);
			load_new_game = 0;
		}
	}
}

void update_polled_stuff_if_runtime() {
	if (_G(want_exit)) {
		_G(want_exit) = 0;
		quit("||exit!");
	}

	update_polled_mp3();

	if (_G(editor_debugging_initialized))
		check_for_messages_from_editor();
}

} // namespace AGS3
