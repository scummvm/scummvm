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

#include "ags/engine/ac/event.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/draw.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_room.h"
#include "ags/engine/ac/global_screen.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/screen.h"
#include "ags/engine/main/game_run.h"
#include "ags/shared/script/cc_common.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/plugins/ags_plugin.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/engine/script/script.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/ddb.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/ac/timer.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

int run_claimable_event(const char *tsname, bool includeRoom, int numParams, const RuntimeScriptValue *params, bool *eventWasClaimed) {
	*eventWasClaimed = true;
	// Run the room script function, and if it is not claimed,
	// then run the main one
	// We need to remember the eventClaimed variable's state, in case
	// this is a nested event
	int eventClaimedOldValue = _G(eventClaimed);
	_G(eventClaimed) = EVENT_INPROGRESS;
	int toret;

	if (includeRoom && _G(roominst)) {
		toret = RunScriptFunction(_G(roominst), tsname, numParams, params);
		if (_G(abort_engine))
			return -1;

		if (_G(eventClaimed) == EVENT_CLAIMED) {
			_G(eventClaimed) = eventClaimedOldValue;
			return toret;
		}
	}

	// run script modules
	for (auto &module_inst : _GP(moduleInst)) {
		toret = RunScriptFunction(module_inst, tsname, numParams, params);

		if (_G(eventClaimed) == EVENT_CLAIMED) {
			_G(eventClaimed) = eventClaimedOldValue;
			return toret;
		}
	}

	_G(eventClaimed) = eventClaimedOldValue;
	*eventWasClaimed = false;
	return 0;
}

// runs the global script on_event function
void run_on_event(int evtype, RuntimeScriptValue &wparam) {
	RuntimeScriptValue params[]{ evtype , wparam };
	QueueScriptFunction(kScInstGame, "on_event", 2, params);
}

void run_room_event(int id) {
	_G(evblockbasename) = "room";

	if (_GP(thisroom).EventHandlers != nullptr) {
		run_interaction_script(_GP(thisroom).EventHandlers.get(), id);
	} else {
		run_interaction_event(&_G(croom)->intrRoom, id);
	}
}

void run_event_block_inv(int invNum, int event) {
	_G(evblockbasename) = "inventory%d";
	if (_G(loaded_game_file_version) > kGameVersion_272) {
		run_interaction_script(_GP(game).invScripts[invNum].get(), event);
	} else {
		run_interaction_event(_GP(game).intrInv[invNum].get(), event);
	}

}

// event list functions
void setevent(int evtyp, int ev1, int ev2, int ev3) {
	EventHappened evt;
	evt.type = evtyp;
	evt.data1 = ev1;
	evt.data2 = ev2;
	evt.data3 = ev3;
	evt.player = _GP(game).playercharacter;
	_GP(events).push_back(evt);
}

// TODO: this is kind of a hack, which forces event to be processed even if
// it was fired from insides of other event processing.
// The proper solution would be to do the event processing overhaul in AGS.
void force_event(int evtyp, int ev1, int ev2, int ev3) {
	if (_G(inside_processevent))
		runevent_now(evtyp, ev1, ev2, ev3);
	else
		setevent(evtyp, ev1, ev2, ev3);
}

void process_event(const EventHappened *evp) {
	RuntimeScriptValue rval_null;
	if (evp->type == EV_TEXTSCRIPT) {
		cc_clear_error();
		RuntimeScriptValue params[2]{ evp->data2, evp->data3 };
		if (evp->data3 > -1000)
			QueueScriptFunction(kScInstGame, _G(tsnames)[evp->data1], 2, params);
		else if (evp->data2 > -1000)
			QueueScriptFunction(kScInstGame, _G(tsnames)[evp->data1], 1, params);
		else
			QueueScriptFunction(kScInstGame, _G(tsnames)[evp->data1]);
	}  else if (evp->type == EV_NEWROOM) {
		NewRoom(evp->data1);
	} else if (evp->type == EV_RUNEVBLOCK) {
		Interaction *evpt = nullptr;
		PInteractionScripts scriptPtr = nullptr;
		const char *oldbasename = _G(evblockbasename);
		int   oldblocknum = _G(evblocknum);

		if (evp->data1 == EVB_HOTSPOT) {

			if (_GP(thisroom).Hotspots[evp->data2].EventHandlers != nullptr)
				scriptPtr = _GP(thisroom).Hotspots[evp->data2].EventHandlers;
			else
				evpt = &_G(croom)->intrHotspot[evp->data2];

			_G(evblockbasename) = "hotspot%d";
			_G(evblocknum) = evp->data2;
			//Debug::Printf("Running hotspot interaction for hotspot %d, event %d", evp->data2, evp->data3);
		} else if (evp->data1 == EVB_ROOM) {

			if (_GP(thisroom).EventHandlers != nullptr)
				scriptPtr = _GP(thisroom).EventHandlers;
			else
				evpt = &_G(croom)->intrRoom;

			_G(evblockbasename) = "room";
			if (evp->data3 == EVROM_BEFOREFADEIN) {
				_G(in_enters_screen)++;
				run_on_event(GE_ENTER_ROOM, RuntimeScriptValue().SetInt32(_G(displayed_room)));
			} else if (evp->data3 == EVROM_AFTERFADEIN) {
				run_on_event(GE_ENTER_ROOM_AFTERFADE, RuntimeScriptValue().SetInt32(_G(displayed_room)));
			}
			//Debug::Printf("Running room interaction, event %d", evp->data3);
		}

		if (scriptPtr != nullptr) {
			run_interaction_script(scriptPtr.get(), evp->data3);
		} else if (evpt != nullptr) {
			run_interaction_event(evpt, evp->data3);
		} else
			quit("process_event: RunEvBlock: unknown evb type");

		if (_G(abort_engine))
			return;

		_G(evblockbasename) = oldbasename;
		_G(evblocknum) = oldblocknum;

		if ((evp->data1 == EVB_ROOM) && (evp->data3 == EVROM_BEFOREFADEIN))
			_G(in_enters_screen)--;
	} else if (evp->type == EV_FADEIN) {
		// if they change the transition type before the fadein, make
		// sure the screen doesn't freeze up
		_GP(play).screen_is_faded_out = 0;

		// determine the transition style
		int theTransition = _GP(play).fade_effect;

		if (_GP(play).next_screen_transition >= 0) {
			// a one-off transition was selected, so use it
			theTransition = _GP(play).next_screen_transition;
			_GP(play).next_screen_transition = -1;
		}

		if (pl_run_plugin_hooks(AGSE_TRANSITIONIN, 0))
			return;

		if (_GP(play).fast_forward)
			return;

		const bool ignore_transition = (_GP(play).screen_tint > 0);
		if (((theTransition == FADE_CROSSFADE) || (theTransition == FADE_DISSOLVE)) &&
		        (_G(saved_viewport_bitmap) == nullptr) && !ignore_transition) {
			// transition type was not crossfade/dissolve when the screen faded out,
			// but it is now when the screen fades in (Eg. a save game was restored
			// with a different setting). Therefore just fade normally.
			my_fade_out(5);
			theTransition = FADE_NORMAL;
		}

		// TODO: use normal coordinates instead of "native_size" and multiply_up_*?
		const Rect &viewport = _GP(play).GetMainViewport();

		if ((theTransition == FADE_INSTANT) || ignore_transition)
			set_palette_range(_G(palette), 0, 255, 0);
		else if (theTransition == FADE_NORMAL) {
			my_fade_in(_G(palette), 5);
		} else if (theTransition == FADE_BOXOUT) {
			if (!_G(gfxDriver)->UsesMemoryBackBuffer()) {
				_G(gfxDriver)->BoxOutEffect(false, get_fixed_pixel_size(16), 1000 / GetGameSpeed());
			} else {
				// First of all we render the game once again and save backbuffer from further editing.
				// We put temporary bitmap as a new backbuffer for the transition period, and
				// will be drawing saved image of the game over to that backbuffer, simulating "box-out".
				set_palette_range(_G(palette), 0, 255, 0);
				construct_game_scene(true);
				construct_game_screen_overlay(false);
				_G(gfxDriver)->RenderToBackBuffer();
				Bitmap *saved_backbuf = _G(gfxDriver)->GetMemoryBackBuffer();
				Bitmap *temp_scr = new Bitmap(saved_backbuf->GetWidth(), saved_backbuf->GetHeight(), saved_backbuf->GetColorDepth());
				_G(gfxDriver)->SetMemoryBackBuffer(temp_scr);
				temp_scr->Clear();
				render_to_screen();

				const int speed = get_fixed_pixel_size(16);
				const int yspeed = viewport.GetHeight() / (viewport.GetWidth() / speed);
				int boxwid = speed, boxhit = yspeed;
				while (boxwid < temp_scr->GetWidth()) {
					boxwid += speed;
					boxhit += yspeed;
					boxwid = Math::Clamp(boxwid, 0, viewport.GetWidth());
					boxhit = Math::Clamp(boxhit, 0, viewport.GetHeight());
					int lxp = viewport.GetWidth() / 2 - boxwid / 2;
					int lyp = viewport.GetHeight() / 2 - boxhit / 2;
					temp_scr->Blit(saved_backbuf, lxp, lyp, lxp, lyp,
					               boxwid, boxhit);
					render_to_screen();
					WaitForNextFrame();
				}
				_G(gfxDriver)->SetMemoryBackBuffer(saved_backbuf);
			}
			_GP(play).screen_is_faded_out = 0;
		} else if (theTransition == FADE_CROSSFADE) {
			if (_GP(game).color_depth == 1)
				quit("!Cannot use crossfade screen transition in 256-colour games");

			IDriverDependantBitmap *ddb = prepare_screen_for_transition_in();

			for (int alpha = 254; alpha > 0; alpha -= 16) {
				// do the crossfade
				ddb->SetAlpha(alpha);
				invalidate_screen();
				construct_game_scene(true);
				construct_game_screen_overlay(false);
				// draw old screen on top while alpha > 16
				if (alpha > 16) {
					_G(gfxDriver)->BeginSpriteBatch(_GP(play).GetMainViewport(), SpriteTransform());
					_G(gfxDriver)->DrawSprite(0, 0, ddb);
					_G(gfxDriver)->EndSpriteBatch();
				}
				render_to_screen();
				update_polled_stuff();
				WaitForNextFrame();
			}

			delete _G(saved_viewport_bitmap);
			_G(saved_viewport_bitmap) = nullptr;
			set_palette_range(_G(palette), 0, 255, 0);
			_G(gfxDriver)->DestroyDDB(ddb);
		} else if (theTransition == FADE_DISSOLVE) {
			int pattern[16] = { 0, 4, 14, 9, 5, 11, 2, 8, 10, 3, 12, 7, 15, 6, 13, 1 };
			int aa, bb, cc;
			RGB interpal[256];

			IDriverDependantBitmap *ddb = prepare_screen_for_transition_in();
			for (aa = 0; aa < 16; aa++) {
				// merge the palette while dithering
				if (_GP(game).color_depth == 1) {
					fade_interpolate(_G(old_palette), _G(palette), interpal, aa * 4, 0, 255);
					set_palette_range(interpal, 0, 255, 0);
				}
				// do the dissolving
				int maskCol = _G(saved_viewport_bitmap)->GetMaskColor();
				for (bb = 0; bb < viewport.GetWidth(); bb += 4) {
					for (cc = 0; cc < viewport.GetHeight(); cc += 4) {
						_G(saved_viewport_bitmap)->PutPixel(bb + pattern[aa] / 4, cc + pattern[aa] % 4, maskCol);
					}
				}
				_G(gfxDriver)->UpdateDDBFromBitmap(ddb, _G(saved_viewport_bitmap), false);
				construct_game_scene(true);
				construct_game_screen_overlay(false);
				_G(gfxDriver)->BeginSpriteBatch(_GP(play).GetMainViewport(), SpriteTransform());
				_G(gfxDriver)->DrawSprite(0, 0, ddb);
				_G(gfxDriver)->EndSpriteBatch();
				render_to_screen();
				update_polled_stuff();
				WaitForNextFrame();
			}

			delete _G(saved_viewport_bitmap);
			_G(saved_viewport_bitmap) = nullptr;
			set_palette_range(_G(palette), 0, 255, 0);
			_G(gfxDriver)->DestroyDDB(ddb);
		}

	} else if (evp->type == EV_IFACECLICK) {
		process_interface_click(evp->data1, evp->data2, evp->data3);
	} else {
		quit("process_event: unknown event to process");
	}
}


void runevent_now(int evtyp, int ev1, int ev2, int ev3) {
	EventHappened evh;
	evh.type = evtyp;
	evh.data1 = ev1;
	evh.data2 = ev2;
	evh.data3 = ev3;
	evh.player = _GP(game).playercharacter;
	process_event(&evh);
}

void processallevents() {
	if (_G(inside_processevent)) {
		_GP(events).clear(); // flush queued events
		return;
	}

	// Take ownership of the pending events
	// Note: upstream AGS used std::move, which I haven't been able
	// to properly implement in ScummVM. Luckily, our events are
	// a pointer, so I could get the same result swapping them
	std::vector<EventHappened> *evtCopy = new std::vector<EventHappened>();
	SWAP(evtCopy, _G(events));

	int room_was = _GP(play).room_changes;

	_G(inside_processevent)++;

	for (size_t i = 0; i < evtCopy->size() && !_G(abort_engine); ++i) {
		process_event(&(*evtCopy)[i]);

		if (room_was != _GP(play).room_changes)
			break;  // changed room, so discard other events
	}

	delete evtCopy;
	_G(inside_processevent)--;
}

// end event list functions

void ClaimEvent() {
	if (_G(eventClaimed) == EVENT_NONE)
		quit("!ClaimEvent: no event to claim");

	_G(eventClaimed) = EVENT_CLAIMED;
}

} // namespace AGS3
