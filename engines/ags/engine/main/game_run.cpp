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
#include "ags/engine/ac/button.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/character_extras.h"
#include "ags/shared/ac/character_info.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_debug.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/ac/global_region.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/hotspot.h"
#include "ags/shared/ac/keycode.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/overlay.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/room_object.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/device/mouse_w32.h"
#include "ags/engine/gui/animating_gui_button.h"
#include "ags/shared/gui/gui_inv.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/gui/gui_textbox.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/main/update.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/plugins/ags_plugin.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/engine/script/script.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/events.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

static int ShouldStayInWaitMode();

#define UNTIL_ANIMEND   1
#define UNTIL_MOVEEND   2
#define UNTIL_CHARIS0   3
#define UNTIL_NOOVERLAY 4
#define UNTIL_NEGATIVE  5
#define UNTIL_INTIS0    6
#define UNTIL_SHORTIS0  7
#define UNTIL_INTISNEG  8

static void ProperExit() {
	_G(want_exit) = 0;
	_G(proper_exit) = 1;
	quit("||exit!");
}

static void game_loop_check_problems_at_start() {
	if ((_G(in_enters_screen) != 0) & (_G(displayed_room) == _G(starting_room)))
		quit("!A text script run in the Player Enters Screen event caused the\n"
		     "screen to be updated. If you need to use Wait(), do so in After Fadein");
	if ((_G(in_enters_screen) != 0) && (_G(done_es_error) == 0)) {
		debug_script_warn("Wait() was used in Player Enters Screen - use Enters Screen After Fadein instead");
		_G(done_es_error) = 1;
	}
	if (_G(no_blocking_functions))
		quit("!A blocking function was called from within a non-blocking event such as " REP_EXEC_ALWAYS_NAME);
}

static void game_loop_check_new_room() {
	if (_G(in_new_room) == 0) {
		// Run the room and game script repeatedly_execute
		run_function_on_non_blocking_thread(&_GP(repExecAlways));
		setevent(EV_TEXTSCRIPT, TS_REPEAT);
		setevent(EV_RUNEVBLOCK, EVB_ROOM, 0, 6);
	}
	// run this immediately to make sure it gets done before fade-in
	// (player enters screen)
	check_new_room();
}

static void game_loop_do_late_update() {
	if (_G(in_new_room) == 0) {
		// Run the room and game script late_repeatedly_execute
		run_function_on_non_blocking_thread(&_GP(lateRepExecAlways));
	}
}

static int game_loop_check_ground_level_interactions() {
	if ((_GP(play).ground_level_areas_disabled & GLED_INTERACTION) == 0) {
		// check if he's standing on a hotspot
		int hotspotThere = get_hotspot_at(_G(playerchar)->x, _G(playerchar)->y);
		// run Stands on Hotspot event
		setevent(EV_RUNEVBLOCK, EVB_HOTSPOT, hotspotThere, 0);

		// check current region
		int onRegion = GetRegionIDAtRoom(_G(playerchar)->x, _G(playerchar)->y);
		int inRoom = _G(displayed_room);

		if (onRegion != _GP(play).player_on_region) {
			// we need to save this and set _GP(play).player_on_region
			// now, so it's correct going into RunRegionInteraction
			int oldRegion = _GP(play).player_on_region;

			_GP(play).player_on_region = onRegion;
			// Walks Off last region
			if (oldRegion > 0)
				RunRegionInteraction(oldRegion, 2);
			// Walks Onto new region
			if (onRegion > 0)
				RunRegionInteraction(onRegion, 1);
		}
		if (_GP(play).player_on_region > 0)   // player stands on region
			RunRegionInteraction(_GP(play).player_on_region, 0);

		// one of the region interactions sent us to another room
		if (inRoom != _G(displayed_room)) {
			check_new_room();
		}

		// if in a Wait loop which is no longer valid (probably
		// because the Region interaction did a NewRoom), abort
		// the rest of the loop
		if ((_G(restrict_until)) && (!ShouldStayInWaitMode())) {
			// cancel the Rep Exec and Stands on Hotspot events that
			// we just added -- otherwise the event queue gets huge
			_G(numevents) = _G(numEventsAtStartOfFunction);
			return 0;
		}
	} // end if checking ground level interactions

	return RETURN_CONTINUE;
}

static void lock_mouse_on_click() {
	if (_GP(usetup).mouse_auto_lock && _GP(scsystem).windowed)
		_GP(mouse).TryLockToWindow();
}

static void toggle_mouse_lock() {
	if (_GP(scsystem).windowed) {
		if (_GP(mouse).IsLockedToWindow())
			_GP(mouse).UnlockFromWindow();
		else
			_GP(mouse).TryLockToWindow();
	}
}

// Runs default mouse button handling
static void check_mouse_controls() {
	int mongu = -1;

	mongu = gui_on_mouse_move();

	_G(mouse_on_iface) = mongu;
	if ((_G(ifacepopped) >= 0) && (_G(mousey) >= _GP(guis)[_G(ifacepopped)].Y + _GP(guis)[_G(ifacepopped)].Height))
		remove_popup_interface(_G(ifacepopped));

	// check mouse clicks on GUIs
	if ((_G(wasbutdown) > 0) && (ags_misbuttondown(_G(wasbutdown) - 1))) {
		gui_on_mouse_hold(_G(wasongui), _G(wasbutdown));
	} else if ((_G(wasbutdown) > 0) && (!ags_misbuttondown(_G(wasbutdown) - 1))) {
		gui_on_mouse_up(_G(wasongui), _G(wasbutdown));
		_G(wasbutdown) = 0;
	}

	int mbut = MouseNone;
	int mwheelz = 0;
	if (run_service_mb_controls(mbut, mwheelz) && mbut >= 0) {

		check_skip_cutscene_mclick(mbut);

		if (_GP(play).fast_forward || _GP(play).IsIgnoringInput()) { /* do nothing if skipping cutscene or input disabled */
		} else if ((_GP(play).wait_counter != 0) && (_GP(play).key_skip_wait & SKIP_MOUSECLICK) != 0) {
			_GP(play).SetWaitSkipResult(SKIP_MOUSECLICK, mbut);
		} else if (_GP(play).text_overlay_on > 0) {
			if (_GP(play).cant_skip_speech & SKIP_MOUSECLICK) {
				remove_screen_overlay(_GP(play).text_overlay_on);
				_GP(play).SetWaitSkipResult(SKIP_MOUSECLICK, mbut);
			}
		} else if (!IsInterfaceEnabled());  // blocking cutscene, ignore mouse
		else if (pl_run_plugin_hooks(AGSE_MOUSECLICK, mbut + 1)) {
			// plugin took the click
			debug_script_log("Plugin handled mouse button %d", mbut + 1);
		} else if (mongu >= 0) {
			if (_G(wasbutdown) == 0) {
				gui_on_mouse_down(mongu, mbut + 1);
			}
			_G(wasongui) = mongu;
			_G(wasbutdown) = mbut + 1;
		} else setevent(EV_TEXTSCRIPT, TS_MCLICK, mbut + 1);
		//    else RunTextScriptIParam(_G(gameinst),"on_mouse_click",aa+1);
	}

	if (mwheelz < 0)
		setevent(EV_TEXTSCRIPT, TS_MCLICK, 9);
	else if (mwheelz > 0)
		setevent(EV_TEXTSCRIPT, TS_MCLICK, 8);
}



// Special flags to OR saved SDL_Keymod flags with:
// Mod key combination already fired (wait until full mod release)
#define KEY_MODS_FIRED      0x80000000

int cur_key_mods = 0;
int old_key_mod = 0; // for saving previous key mods

// Runs service key controls, returns false if service key combinations were handled
// and no more processing required, otherwise returns true and provides current keycode and key shifts.
bool run_service_key_controls(KeyInput &out_key) {
	bool handled = false;
	const bool key_valid = ags_keyevent_ready();
	const Common::Event key_evt = key_valid ? ags_get_next_keyevent() : Common::Event();
	const bool is_only_mod_key = key_evt.type == Common::EVENT_KEYDOWN ?
		is_mod_key(key_evt.kbd.keycode) : false;

	// Following section is for testing for pushed and released mod-keys.
	// A bit of explanation: some service actions may require combination of
	// mod-keys, for example [Ctrl + Alt] toggles mouse lock in window.
	// Here comes a problem: other actions may also use [Ctrl + Alt] mods in
	// combination with a third key: e.g. [Ctrl + Alt + V] displays engine info.
	// For this reason we cannot simply test for pressed Ctrl and Alt here,
	// but we must wait until player *releases at least one mod key* of this combo,
	// while no third key was pressed.
	// In other words, such action should only trigger if:
	// * if combination of held down mod-keys was gathered,
	// * if no other key was pressed meanwhile,
	// * if at least one of those gathered mod-keys was released.
	//
	// TODO: maybe split this mod handling into sep procedure and make it easier to use (not that it's used alot)?

	// First, check mods
	const int cur_mod = make_merged_mod(key_evt.kbd.flags);

	// If shifts combination have already triggered an action, then do nothing
	// until new shifts are empty, in which case reset saved shifts
	if (old_key_mod & KEY_MODS_FIRED) {
		if (cur_mod == 0)
			old_key_mod = 0;
	} else {
		// If any non-mod key is pressed, add fired flag to indicate that
		// this is no longer a pure mod keys combination
		if (key_valid && !is_only_mod_key) {
			old_key_mod = cur_mod | KEY_MODS_FIRED;
		}
		// If all the previously registered mods are still pressed,
		// then simply resave new mods state.
		else if ((old_key_mod & cur_mod) == old_key_mod) {
			old_key_mod = cur_mod;
		}
		// Otherwise some of the mods were released, then run key combo action
		// and set KEY_MODS_FIRED flag to prevent multiple execution
		else if (old_key_mod) {
			// Toggle mouse lock on Ctrl + Alt
			if (old_key_mod == (Common::KBD_CTRL | Common::KBD_ALT)) {
				toggle_mouse_lock();
				handled = true;
			}
			old_key_mod |= KEY_MODS_FIRED;
		}
	}
	cur_key_mods = cur_mod;

	if (!key_valid)
		return false; // if there was no key press, finish after handling current mod state
	if (is_only_mod_key || handled)
		return false; // rest of engine currently does not use pressed mod keys
	// change this when it's no longer true (but be mindful about key-skipping!)

	KeyInput ki = ags_keycode_from_scummvm(key_evt);
	eAGSKeyCode agskey = ki.Key;
	if (agskey == eAGSKeyCodeNone)
		return false; // should skip this key event

	// LAlt or RAlt + Enter/Return
	if ((cur_mod == Common::KBD_ALT) && agskey == eAGSKeyCodeReturn) {
		engine_try_switch_windowed_gfxmode();
		return false;
	}

	// Alt+X, abort (but only once game is loaded)
	if ((_G(displayed_room) >= 0) && (_GP(play).abort_key > 0 && agskey == _GP(play).abort_key)) {
		_G(check_dynamic_sprites_at_exit) = 0;
		quit("!|");
	}

	// debug console
	if ((agskey == '`') && (_GP(play).debug_mode > 0)) {
		_G(display_console) = !_G(display_console);
		return false;
	}

	if ((agskey == eAGSKeyCodeCtrlE) && (_G(display_fps) == kFPS_Forced)) {
		// if --fps paramter is used, Ctrl+E will max out frame rate
		setTimerFps(isTimerFpsMaxed() ? _G(frames_per_second) : 1000);
		return false;
	}

	if ((agskey == eAGSKeyCodeCtrlD) && (_GP(play).debug_mode > 0)) {
		// ctrl+D - show info
		char infobuf[900];
		int ff;
		// MACPORT FIX 9/6/5: added last %s
		sprintf(infobuf, "In room %d %s[Player at %d, %d (view %d, loop %d, frame %d)%s%s%s",
		        _G(displayed_room), (_G(noWalkBehindsAtAll) ? "(has no walk-behinds)" : ""), _G(playerchar)->x, _G(playerchar)->y,
		        _G(playerchar)->view + 1, _G(playerchar)->loop, _G(playerchar)->frame,
		        (IsGamePaused() == 0) ? "" : "[Game paused.",
		        (_GP(play).ground_level_areas_disabled == 0) ? "" : "[Ground areas disabled.",
		        (IsInterfaceEnabled() == 0) ? "[Game in Wait state" : "");
		for (ff = 0; ff < _G(croom)->numobj; ff++) {
			if (ff >= 8) break; // buffer not big enough for more than 7
			sprintf(&infobuf[strlen(infobuf)],
			        "[Object %d: (%d,%d) size (%d x %d) on:%d moving:%s animating:%d slot:%d trnsp:%d clkble:%d",
			        ff, _G(objs)[ff].x, _G(objs)[ff].y,
			        (_GP(spriteset)[_G(objs)[ff].num] != nullptr) ? _GP(game).SpriteInfos[_G(objs)[ff].num].Width : 0,
			        (_GP(spriteset)[_G(objs)[ff].num] != nullptr) ? _GP(game).SpriteInfos[_G(objs)[ff].num].Height : 0,
			        _G(objs)[ff].on,
			        (_G(objs)[ff].moving > 0) ? "yes" : "no", _G(objs)[ff].cycling,
			        _G(objs)[ff].num, _G(objs)[ff].transparent,
			        ((_G(objs)[ff].flags & OBJF_NOINTERACT) != 0) ? 0 : 1);
		}
		Display(infobuf);
		int chd = _GP(game).playercharacter;
		char bigbuffer[STD_BUFFER_SIZE] = "CHARACTERS IN THIS ROOM:[";
		for (ff = 0; ff < _GP(game).numcharacters; ff++) {
			if (_GP(game).chars[ff].room != _G(displayed_room)) continue;
			if (strlen(bigbuffer) > 430) {
				strcat(bigbuffer, "and more...");
				Display(bigbuffer);
				strcpy(bigbuffer, "CHARACTERS IN THIS ROOM (cont'd):[");
			}
			chd = ff;
			sprintf(&bigbuffer[strlen(bigbuffer)],
			        "%s (view/loop/frm:%d,%d,%d  x/y/z:%d,%d,%d  idleview:%d,time:%d,left:%d walk:%d anim:%d follow:%d flags:%X wait:%d zoom:%d)[",
			        _GP(game).chars[chd].scrname, _GP(game).chars[chd].view + 1, _GP(game).chars[chd].loop, _GP(game).chars[chd].frame,
			        _GP(game).chars[chd].x, _GP(game).chars[chd].y, _GP(game).chars[chd].z,
			        _GP(game).chars[chd].idleview, _GP(game).chars[chd].idletime, _GP(game).chars[chd].idleleft,
			        _GP(game).chars[chd].walking, _GP(game).chars[chd].animating, _GP(game).chars[chd].following,
			        _GP(game).chars[chd].flags, _GP(game).chars[chd].wait, _G(charextra)[chd].zoom);
		}
		Display(bigbuffer);
		return false;
	}

	if (((agskey == eAGSKeyCodeCtrlV) && (cur_key_mods & Common::KBD_ALT) != 0)
	        && (_GP(play).wait_counter < 1) && (_GP(play).text_overlay_on == 0) && (_G(restrict_until) == 0)) {
		// make sure we can't interrupt a Wait()
		// and desync the music to cutscene
		_GP(play).debug_mode++;
		script_debug(1, 0);
		_GP(play).debug_mode--;
		return false;
	}

	// No service operation triggered? return active keypress and mods to caller
	out_key = ki;
	return true;
}

bool run_service_mb_controls(int &mbut, int &mwheelz) {
	int mb = ags_mgetbutton();
	int mz = ags_check_mouse_wheel();
	if (mb == MouseNone && mz == 0)
		return false;
	lock_mouse_on_click(); // do not claim
	mbut = mb;
	mwheelz = mz;
	return true;
}

// Runs default keyboard handling
static void check_keyboard_controls() {
	// First check for service engine's combinations (mouse lock, display mode switch, and so forth)
	KeyInput ki;
	if (!run_service_key_controls(ki)) {
		return;
	}
	eAGSKeyCode kgn = ki.Key;
	// Then, check cutscene skip
	check_skip_cutscene_keypress(kgn);
	if (_GP(play).fast_forward) {
		return;
	}
	if (_GP(play).IsIgnoringInput()) {
		return;
	}
	// Now check for in-game controls
	if (pl_run_plugin_hooks(AGSE_KEYPRESS, kgn)) {
		// plugin took the keypress
		debug_script_log("Keypress code %d taken by plugin", kgn);
		return;
	}

	// skip speech if desired by Speech.SkipStyle
	if ((_GP(play).text_overlay_on > 0) && (_GP(play).cant_skip_speech & SKIP_KEYPRESS)) {
		// only allow a key to remove the overlay if the icon bar isn't up
		if (IsGamePaused() == 0) {
			// check if it requires a specific keypress
			if ((_GP(play).skip_speech_specific_key > 0) &&
				(kgn != _GP(play).skip_speech_specific_key)) {
			} else {
				remove_screen_overlay(_GP(play).text_overlay_on);
				_GP(play).SetWaitSkipResult(SKIP_KEYPRESS, kgn);
			}
		}

		return;
	}

	if ((_GP(play).wait_counter != 0) && (_GP(play).key_skip_wait & SKIP_KEYPRESS) != 0) {
		_GP(play).SetWaitSkipResult(SKIP_KEYPRESS, kgn);
		return;
	}

	if (_G(inside_script)) {
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
	        && !_G(all_buttons_disabled)) {
		for (int guiIndex = 0; guiIndex < _GP(game).numgui; guiIndex++) {
			auto &gui = _GP(guis)[guiIndex];

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

				guitex->OnKeyPress(ki);

				if (guitex->IsActivated) {
					guitex->IsActivated = false;
					setevent(EV_IFACECLICK, guiIndex, controlIndex, 1);
				}
			}
		}
	}

	if (!keywasprocessed) {
		int sckey = AGSKeyToScriptKey(kgn);
		debug_script_log("Running on_key_press keycode %d", sckey);
		setevent(EV_TEXTSCRIPT, TS_KEYPRESS, sckey);
	}

	// RunTextScriptIParam(_G(gameinst),"on_key_press",kgn);
}

// check_controls: checks mouse & keyboard interface
static void check_controls() {
	_G(our_eip) = 1007;

	sys_evt_process_pending();

	check_mouse_controls();
	check_keyboard_controls();
}

static void check_room_edges(int numevents_was) {
	if ((IsInterfaceEnabled()) && (IsGamePaused() == 0) &&
	        (_G(in_new_room) == 0) && (_G(new_room_was) == 0)) {
		// Only allow walking off edges if not in wait mode, and
		// if not in Player Enters Screen (allow walking in from off-screen)
		int edgesActivated[4] = { 0, 0, 0, 0 };
		// Only do it if nothing else has happened (eg. mouseclick)
		if ((_G(numevents) == numevents_was) &&
		        ((_GP(play).ground_level_areas_disabled & GLED_INTERACTION) == 0)) {

			if (_G(playerchar)->x <= _GP(thisroom).Edges.Left)
				edgesActivated[0] = 1;
			else if (_G(playerchar)->x >= _GP(thisroom).Edges.Right)
				edgesActivated[1] = 1;
			if (_G(playerchar)->y >= _GP(thisroom).Edges.Bottom)
				edgesActivated[2] = 1;
			else if (_G(playerchar)->y <= _GP(thisroom).Edges.Top)
				edgesActivated[3] = 1;

			if ((_GP(play).entered_edge >= 0) && (_GP(play).entered_edge <= 3)) {
				// once the player is no longer outside the edge, forget the stored edge
				if (edgesActivated[_GP(play).entered_edge] == 0)
					_GP(play).entered_edge = -10;
				// if we are walking in from off-screen, don't activate edges
				else
					edgesActivated[_GP(play).entered_edge] = 0;
			}

			for (int ii = 0; ii < 4; ii++) {
				if (edgesActivated[ii])
					setevent(EV_RUNEVBLOCK, EVB_ROOM, 0, ii);
			}
		}
	}
	_G(our_eip) = 1008;

}

static void game_loop_check_controls(bool checkControls) {
	// don't let the player do anything before the screen fades in
	if ((_G(in_new_room) == 0) && (checkControls)) {
		int inRoom = _G(displayed_room);
		int numevents_was = _G(numevents);
		check_controls();
		check_room_edges(numevents_was);

		if (_G(abort_engine))
			return;

		// If an inventory interaction changed the room
		if (inRoom != _G(displayed_room))
			check_new_room();
	}
}

static void game_loop_do_update() {
	if (_G(debug_flags) & DBG_NOUPDATE);
	else if (_G(game_paused) == 0) update_stuff();
}

static void game_loop_update_animated_buttons() {
	// update animating GUI buttons
	// this bit isn't in update_stuff because it always needs to
	// happen, even when the game is paused
	for (int aa = 0; aa < _G(numAnimButs); aa++) {
		if (UpdateAnimatingButton(aa)) {
			StopButtonAnimation(aa);
			aa--;
		}
	}
}

static void game_loop_do_render_and_check_mouse(IDriverDependantBitmap *extraBitmap, int extraX, int extraY) {
	if (!_GP(play).fast_forward) {
		int mwasatx = _G(mousex), mwasaty = _G(mousey);

		// Only do this if we are not skipping a cutscene
		render_graphics(extraBitmap, extraX, extraY);

		// Check Mouse Moves Over Hotspot event
		// TODO: move this out of render related function? find out why we remember mwasatx and mwasaty before render
		// TODO: do not use static variables!
		// TODO: if we support rotation then we also need to compare full transform!
		if (_G(displayed_room) < 0)
			return;
		auto view = _GP(play).GetRoomViewportAt(_G(mousex), _G(mousey));
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
					int onhs = _G(getloctype_index);

					setevent(EV_RUNEVBLOCK, EVB_HOTSPOT, onhs, 6);
				}
			}

			offsetxWas = offsetx;
			offsetyWas = offsety;
		} // camera found under mouse
	}
}

static void game_loop_update_events() {
	_G(new_room_was) = _G(in_new_room);
	if (_G(in_new_room) > 0)
		setevent(EV_FADEIN, 0, 0, 0);
	_G(in_new_room) = 0;
	update_events();
	if (!_G(abort_engine) && (_G(new_room_was) > 0) && (_G(in_new_room) == 0)) {
		// if in a new room, and the room wasn't just changed again in update_events,
		// then queue the Enters Screen scripts
		// run these next time round, when it's faded in
		if (_G(new_room_was) == 2)  // first time enters screen
			setevent(EV_RUNEVBLOCK, EVB_ROOM, 0, 4);
		if (_G(new_room_was) != 3)   // enters screen after fadein
			setevent(EV_RUNEVBLOCK, EVB_ROOM, 0, 7);
	}
}

static void game_loop_update_background_animation() {
	if (_GP(play).bg_anim_delay > 0) _GP(play).bg_anim_delay--;
	else if (_GP(play).bg_frame_locked);
	else {
		_GP(play).bg_anim_delay = _GP(play).anim_background_speed;
		_GP(play).bg_frame++;
		if ((size_t)_GP(play).bg_frame >= _GP(thisroom).BgFrameCount)
			_GP(play).bg_frame = 0;
		if (_GP(thisroom).BgFrameCount >= 2) {
			// get the new frame's palette
			on_background_frame_change();
		}
	}
}

static void game_loop_update_loop_counter() {
	_G(loopcounter)++;

	if (_GP(play).wait_counter > 0) _GP(play).wait_counter--;
	if (_GP(play).shakesc_length > 0) _GP(play).shakesc_length--;

	if (_G(loopcounter) % 5 == 0) {
		update_ambient_sound_vol();
		update_directional_sound_vol();
	}
}

static void game_loop_update_fps() {
	auto t2 = AGS_Clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - _G(t1));
	auto frames = _G(loopcounter) - _G(lastcounter);

	if (duration >= std::chrono::milliseconds(1000) && frames > 0) {
		_G(fps) = 1000.0f * frames / duration.count();
		_G(t1) = t2;
		_G(lastcounter) = _G(loopcounter);
	}
}

float get_current_fps() {
	// if we have maxed out framerate then return the frame rate we're seeing instead
	// fps must be greater that 0 or some timings will take forever.
	if (isTimerFpsMaxed() && _G(fps) > 0.0f) {
		return _G(fps);
	}
	return _G(frames_per_second);
}

void set_loop_counter(unsigned int new_counter) {
	_G(loopcounter) = new_counter;
	_G(t1) = AGS_Clock::now();
	_G(lastcounter) = _G(loopcounter);
	_G(fps) = std::numeric_limits<float>::quiet_undefined();
}

void UpdateGameOnce(bool checkControls, IDriverDependantBitmap *extraBitmap, int extraX, int extraY) {

	int res;

	sys_evt_process_pending();

	_G(numEventsAtStartOfFunction) = _G(numevents);

	if (_G(want_exit)) {
		ProperExit();
	}

	ccNotifyScriptStillAlive();
	_G(our_eip) = 1;

	game_loop_check_problems_at_start();

	// if we're not fading in, don't count the fadeouts
	if ((_GP(play).no_hicolor_fadein) && (_GP(game).options[OPT_FADETYPE] == FADE_NORMAL))
		_GP(play).screen_is_faded_out = 0;

	_G(our_eip) = 1014;

	update_gui_disabled_status();

	_G(our_eip) = 1004;

	game_loop_check_new_room();
	if (_G(abort_engine))
		return;

	_G(our_eip) = 1005;

	res = game_loop_check_ground_level_interactions();
	if (res != RETURN_CONTINUE) {
		return;
	}

	_G(mouse_on_iface) = -1;

	check_debug_keys();

	game_loop_check_controls(checkControls);

	if (_G(abort_engine))
		return;

	_G(our_eip) = 2;

	game_loop_do_update();

	game_loop_update_animated_buttons();

	game_loop_do_late_update();

	update_audio_system_on_game_loop();

	game_loop_do_render_and_check_mouse(extraBitmap, extraX, extraY);

	_G(our_eip) = 6;

	game_loop_update_events();

	if (_G(abort_engine))
		return;

	_G(our_eip) = 7;

	update_polled_stuff_if_runtime();
	if (_G(abort_engine))
		return;

	game_loop_update_background_animation();

	game_loop_update_loop_counter();

	// Immediately start the next frame if we are skipping a cutscene
	if (_GP(play).fast_forward)
		return;

	_G(our_eip) = 72;

	game_loop_update_fps();

	update_polled_stuff_if_runtime();
	if (_G(abort_engine))
		return;

	WaitForNextFrame();
}

static void UpdateMouseOverLocation() {
	// Call GetLocationName - it will internally force a GUI refresh
	// if the result it returns has changed from last time
	char tempo[STD_BUFFER_SIZE];
	GetLocationName(game_to_data_coord(_G(mousex)), game_to_data_coord(_G(mousey)), tempo);

	if ((_GP(play).get_loc_name_save_cursor >= 0) &&
	        (_GP(play).get_loc_name_save_cursor != _GP(play).get_loc_name_last_time) &&
	        (_G(mouse_on_iface) < 0) && (_G(ifacepopped) < 0)) {
		// we have saved the cursor, but the mouse location has changed
		// and it's time to restore it
		_GP(play).get_loc_name_save_cursor = -1;
		set_cursor_mode(_GP(play).restore_cursor_mode_to);

		if (_G(cur_mode) == _GP(play).restore_cursor_mode_to) {
			// make sure it changed -- the new mode might have been disabled
			// in which case don't change the image
			set_mouse_cursor(_GP(play).restore_cursor_image_to);
		}
		debug_script_log("Restore mouse to mode %d cursor %d", _GP(play).restore_cursor_mode_to, _GP(play).restore_cursor_image_to);
	}
}

// Checks if user interface should remain disabled for now
static int ShouldStayInWaitMode() {
	if (_G(restrict_until) == 0)
		quit("end_wait_loop called but game not in loop_until state");
	int retval = _G(restrict_until);

	if (_G(restrict_until) == UNTIL_MOVEEND) {
		const int16 *wkptr = (const int16 *)_G(user_disabled_data);
		if (wkptr[0] < 1) retval = 0;
	} else if (_G(restrict_until) == UNTIL_CHARIS0) {
		const char *chptr = (const char *)_G(user_disabled_data);
		if (chptr[0] == 0) retval = 0;
	} else if (_G(restrict_until) == UNTIL_NEGATIVE) {
		const int16 *wkptr = (const int16 *)_G(user_disabled_data);
		if (wkptr[0] < 0) retval = 0;
	} else if (_G(restrict_until) == UNTIL_INTISNEG) {
		const int *wkptr = (const int *)_G(user_disabled_data);
		if (wkptr[0] < 0) retval = 0;
	} else if (_G(restrict_until) == UNTIL_NOOVERLAY) {
		if (_GP(play).text_overlay_on == 0) retval = 0;
	} else if (_G(restrict_until) == UNTIL_INTIS0) {
		const int *wkptr = (const int *)_G(user_disabled_data);
		if (wkptr[0] == 0) retval = 0;
	} else if (_G(restrict_until) == UNTIL_SHORTIS0) {
		const int16 *wkptr = (const int16 *)_G(user_disabled_data);
		if (wkptr[0] == 0) retval = 0;
	} else quit("loop_until: unknown until event");

	return retval;
}

static int UpdateWaitMode() {
	if (_G(restrict_until) == 0) {
		return RETURN_CONTINUE;
	}

	_G(restrict_until) = ShouldStayInWaitMode();
	_G(our_eip) = 77;

	if (_G(restrict_until) != 0) {
		return RETURN_CONTINUE;
	}

	auto was_disabled_for = _G(user_disabled_for);

	set_default_cursor();
	if (_G(gui_disabled_style) != GUIDIS_UNCHANGED) { // If GUI looks change when disabled, then update them all
		GUI::MarkAllGUIForUpdate();
	}
	_GP(play).disabled_user_interface--;
	_G(user_disabled_for) = 0;

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
		quit("Unknown _G(user_disabled_for) in end _G(restrict_until)");
	}

	// we shouldn't get here.
	return RETURN_CONTINUE;
}

// Run single game iteration; calls UpdateGameOnce() internally
static int GameTick() {
	if (_G(displayed_room) < 0)
		quit("!A blocking function was called before the first room has been loaded");

	UpdateGameOnce(true);

	if (_G(abort_engine))
		return -1;

	UpdateMouseOverLocation();

	_G(our_eip) = 76;

	int res = UpdateWaitMode();
	if (res == RETURN_CONTINUE) {
		return 0;
	} // continue looping
	return res;
}

static void SetupLoopParameters(int untilwhat, const void *udata) {
	_GP(play).disabled_user_interface++;
	if (_G(gui_disabled_style) != GUIDIS_UNCHANGED) { // If GUI looks change when disabled, then update them all
		GUI::MarkAllGUIForUpdate();
	}
	// Only change the mouse cursor if it hasn't been specifically changed first
	// (or if it's speech, always change it)
	if (((_G(cur_cursor) == _G(cur_mode)) || (untilwhat == UNTIL_NOOVERLAY)) &&
	        (_G(cur_mode) != CURS_WAIT))
		set_mouse_cursor(CURS_WAIT);

	_G(restrict_until) = untilwhat;
	_G(user_disabled_data) = udata;
	_G(user_disabled_for) = FOR_EXITLOOP;
}

// This function is called from lot of various functions
// in the game core, character, room object etc
static void GameLoopUntilEvent(int untilwhat, const void *daaa) {
	// blocking cutscene - end skipping
	EndSkippingUntilCharStops();

	// this function can get called in a nested context, so
	// remember the state of these vars in case a higher level
	// call needs them
	auto cached_restrict_until = _G(restrict_until);
	auto cached_user_disabled_data = _G(user_disabled_data);
	auto cached_user_disabled_for = _G(user_disabled_for);

	SetupLoopParameters(untilwhat, daaa);
	while (GameTick() == 0 && !_G(abort_engine)) {}

	_G(our_eip) = 78;

	_G(restrict_until) = cached_restrict_until;
	_G(user_disabled_data) = cached_user_disabled_data;
	_G(user_disabled_for) = cached_user_disabled_for;
}

void GameLoopUntilValueIsZero(const int8 *value) {
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



void RunGameUntilAborted() {
	// skip ticks to account for time spent starting _GP(game).
	skipMissedTicks();

	while (!_G(abort_engine)) {
		GameTick();

		if (_G(load_new_game)) {
			RunAGSGame(nullptr, _G(load_new_game), 0);
			_G(load_new_game) = 0;
		}
	}
}

void update_polled_stuff_if_runtime() {
	::AGS::g_events->pollEvents();

	if (_G(want_exit)) {
		_G(want_exit) = 0;
		quit("||exit!");

	} else if (_G(editor_debugging_initialized))
		check_for_messages_from_editor();
}

} // namespace AGS3
