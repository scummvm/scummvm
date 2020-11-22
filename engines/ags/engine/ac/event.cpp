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

#include "event.h"
#include "ac/common.h"
#include "ac/draw.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/global_game.h"
#include "ac/global_room.h"
#include "ac/global_screen.h"
#include "ac/gui.h"
#include "ac/roomstatus.h"
#include "ac/screen.h"
#include "script/cc_error.h"
#include "platform/base/agsplatformdriver.h"
#include "plugin/agsplugin.h"
#include "plugin/plugin_engine.h"
#include "script/script.h"
#include "gfx/bitmap.h"
#include "gfx/ddb.h"
#include "gfx/graphicsdriver.h"
#include "media/audio/audio_system.h"
#include "ac/timer.h"

using namespace AGS::Shared;
using namespace AGS::Engine;

extern GameSetupStruct game;
extern RoomStruct thisroom;
extern RoomStatus *croom;
extern int displayed_room;
extern GameState play;
extern color palette[256];
extern IGraphicsDriver *gfxDriver;
extern AGSPlatformDriver *platform;
extern color old_palette[256];

int in_enters_screen = 0, done_es_error = 0;
int in_leaves_screen = -1;

EventHappened event[MAXEVENTS + 1];
int numevents = 0;

const char *evblockbasename;
int evblocknum;

int inside_processevent = 0;
int eventClaimed = EVENT_NONE;

const char *tsnames[4] = {nullptr, REP_EXEC_NAME, "on_key_press", "on_mouse_click"};


int run_claimable_event(const char *tsname, bool includeRoom, int numParams, const RuntimeScriptValue *params, bool *eventWasClaimed) {
	*eventWasClaimed = true;
	// Run the room script function, and if it is not claimed,
	// then run the main one
	// We need to remember the eventClaimed variable's state, in case
	// this is a nested event
	int eventClaimedOldValue = eventClaimed;
	eventClaimed = EVENT_INPROGRESS;
	int toret;

	if (includeRoom && roominst) {
		toret = RunScriptFunctionIfExists(roominst, tsname, numParams, params);

		if (eventClaimed == EVENT_CLAIMED) {
			eventClaimed = eventClaimedOldValue;
			return toret;
		}
	}

	// run script modules
	for (int kk = 0; kk < numScriptModules; kk++) {
		toret = RunScriptFunctionIfExists(moduleInst[kk], tsname, numParams, params);

		if (eventClaimed == EVENT_CLAIMED) {
			eventClaimed = eventClaimedOldValue;
			return toret;
		}
	}

	eventClaimed = eventClaimedOldValue;
	*eventWasClaimed = false;
	return 0;
}

// runs the global script on_event function
void run_on_event(int evtype, RuntimeScriptValue &wparam) {
	QueueScriptFunction(kScInstGame, "on_event", 2, RuntimeScriptValue().SetInt32(evtype), wparam);
}

void run_room_event(int id) {
	evblockbasename = "room";

	if (thisroom.EventHandlers != nullptr) {
		run_interaction_script(thisroom.EventHandlers.get(), id);
	} else {
		run_interaction_event(&croom->intrRoom, id);
	}
}

void run_event_block_inv(int invNum, int event) {
	evblockbasename = "inventory%d";
	if (loaded_game_file_version > kGameVersion_272) {
		run_interaction_script(game.invScripts[invNum].get(), event);
	} else {
		run_interaction_event(game.intrInv[invNum].get(), event);
	}

}

// event list functions
void setevent(int evtyp, int ev1, int ev2, int ev3) {
	event[numevents].type = evtyp;
	event[numevents].data1 = ev1;
	event[numevents].data2 = ev2;
	event[numevents].data3 = ev3;
	event[numevents].player = game.playercharacter;
	numevents++;
	if (numevents >= MAXEVENTS) quit("too many events posted");
}

// TODO: this is kind of a hack, which forces event to be processed even if
// it was fired from insides of other event processing.
// The proper solution would be to do the event processing overhaul in AGS.
void force_event(int evtyp, int ev1, int ev2, int ev3) {
	if (inside_processevent)
		runevent_now(evtyp, ev1, ev2, ev3);
	else
		setevent(evtyp, ev1, ev2, ev3);
}

void process_event(EventHappened *evp) {
	RuntimeScriptValue rval_null;
	if (evp->type == EV_TEXTSCRIPT) {
		ccError = 0;
		if (evp->data2 > -1000) {
			QueueScriptFunction(kScInstGame, tsnames[evp->data1], 1, RuntimeScriptValue().SetInt32(evp->data2));
		} else {
			QueueScriptFunction(kScInstGame, tsnames[evp->data1]);
		}
	} else if (evp->type == EV_NEWROOM) {
		NewRoom(evp->data1);
	} else if (evp->type == EV_RUNEVBLOCK) {
		Interaction *evpt = nullptr;
		PInteractionScripts scriptPtr = nullptr;
		const char *oldbasename = evblockbasename;
		int   oldblocknum = evblocknum;

		if (evp->data1 == EVB_HOTSPOT) {

			if (thisroom.Hotspots[evp->data2].EventHandlers != nullptr)
				scriptPtr = thisroom.Hotspots[evp->data2].EventHandlers;
			else
				evpt = &croom->intrHotspot[evp->data2];

			evblockbasename = "hotspot%d";
			evblocknum = evp->data2;
			//Debug::Printf("Running hotspot interaction for hotspot %d, event %d", evp->data2, evp->data3);
		} else if (evp->data1 == EVB_ROOM) {

			if (thisroom.EventHandlers != nullptr)
				scriptPtr = thisroom.EventHandlers;
			else
				evpt = &croom->intrRoom;

			evblockbasename = "room";
			if (evp->data3 == 5) {
				in_enters_screen ++;
				run_on_event(GE_ENTER_ROOM, RuntimeScriptValue().SetInt32(displayed_room));

			}
			//Debug::Printf("Running room interaction, event %d", evp->data3);
		}

		if (scriptPtr != nullptr) {
			run_interaction_script(scriptPtr.get(), evp->data3);
		} else if (evpt != nullptr) {
			run_interaction_event(evpt, evp->data3);
		} else
			quit("process_event: RunEvBlock: unknown evb type");

		evblockbasename = oldbasename;
		evblocknum = oldblocknum;

		if ((evp->data3 == 5) && (evp->data1 == EVB_ROOM))
			in_enters_screen --;
	} else if (evp->type == EV_FADEIN) {
		// if they change the transition type before the fadein, make
		// sure the screen doesn't freeze up
		play.screen_is_faded_out = 0;

		// determine the transition style
		int theTransition = play.fade_effect;

		if (play.next_screen_transition >= 0) {
			// a one-off transition was selected, so use it
			theTransition = play.next_screen_transition;
			play.next_screen_transition = -1;
		}

		if (pl_run_plugin_hooks(AGSE_TRANSITIONIN, 0))
			return;

		if (play.fast_forward)
			return;

		const bool ignore_transition = (play.screen_tint > 0);
		if (((theTransition == FADE_CROSSFADE) || (theTransition == FADE_DISSOLVE)) &&
		        (saved_viewport_bitmap == nullptr) && !ignore_transition) {
			// transition type was not crossfade/dissolve when the screen faded out,
			// but it is now when the screen fades in (Eg. a save game was restored
			// with a different setting). Therefore just fade normally.
			my_fade_out(5);
			theTransition = FADE_NORMAL;
		}

		// TODO: use normal coordinates instead of "native_size" and multiply_up_*?
		const Size &data_res = game.GetDataRes();
		const Rect &viewport = play.GetMainViewport();

		if ((theTransition == FADE_INSTANT) || ignore_transition)
			set_palette_range(palette, 0, 255, 0);
		else if (theTransition == FADE_NORMAL) {
			my_fade_in(palette, 5);
		} else if (theTransition == FADE_BOXOUT) {
			if (!gfxDriver->UsesMemoryBackBuffer()) {
				gfxDriver->BoxOutEffect(false, get_fixed_pixel_size(16), 1000 / GetGameSpeed());
			} else {
				// First of all we render the game once again and save backbuffer from further editing.
				// We put temporary bitmap as a new backbuffer for the transition period, and
				// will be drawing saved image of the game over to that backbuffer, simulating "box-out".
				set_palette_range(palette, 0, 255, 0);
				construct_game_scene(true);
				construct_game_screen_overlay(false);
				gfxDriver->RenderToBackBuffer();
				Bitmap *saved_backbuf = gfxDriver->GetMemoryBackBuffer();
				Bitmap *temp_scr = new Bitmap(saved_backbuf->GetWidth(), saved_backbuf->GetHeight(), saved_backbuf->GetColorDepth());
				gfxDriver->SetMemoryBackBuffer(temp_scr);
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
					gfxDriver->Vsync();
					temp_scr->Blit(saved_backbuf, lxp, lyp, lxp, lyp,
					               boxwid, boxhit);
					render_to_screen();
					update_polled_mp3();
					WaitForNextFrame();
				}
				gfxDriver->SetMemoryBackBuffer(saved_backbuf);
			}
			play.screen_is_faded_out = 0;
		} else if (theTransition == FADE_CROSSFADE) {
			if (game.color_depth == 1)
				quit("!Cannot use crossfade screen transition in 256-colour games");

			IDriverDependantBitmap *ddb = prepare_screen_for_transition_in();

			int transparency = 254;

			while (transparency > 0) {
				// do the crossfade
				ddb->SetTransparency(transparency);
				invalidate_screen();
				construct_game_scene(true);
				construct_game_screen_overlay(false);

				if (transparency > 16) {
					// on last frame of fade (where transparency < 16), don't
					// draw the old screen on top
					gfxDriver->DrawSprite(0, 0, ddb);
				}
				render_to_screen();
				update_polled_stuff_if_runtime();
				WaitForNextFrame();
				transparency -= 16;
			}
			saved_viewport_bitmap->Release();

			delete saved_viewport_bitmap;
			saved_viewport_bitmap = nullptr;
			set_palette_range(palette, 0, 255, 0);
			gfxDriver->DestroyDDB(ddb);
		} else if (theTransition == FADE_DISSOLVE) {
			int pattern[16] = {0, 4, 14, 9, 5, 11, 2, 8, 10, 3, 12, 7, 15, 6, 13, 1};
			int aa, bb, cc;
			color interpal[256];

			IDriverDependantBitmap *ddb = prepare_screen_for_transition_in();
			for (aa = 0; aa < 16; aa++) {
				// merge the palette while dithering
				if (game.color_depth == 1) {
					fade_interpolate(old_palette, palette, interpal, aa * 4, 0, 255);
					set_palette_range(interpal, 0, 255, 0);
				}
				// do the dissolving
				int maskCol = saved_viewport_bitmap->GetMaskColor();
				for (bb = 0; bb < viewport.GetWidth(); bb += 4) {
					for (cc = 0; cc < viewport.GetHeight(); cc += 4) {
						saved_viewport_bitmap->PutPixel(bb + pattern[aa] / 4, cc + pattern[aa] % 4, maskCol);
					}
				}
				gfxDriver->UpdateDDBFromBitmap(ddb, saved_viewport_bitmap, false);
				construct_game_scene(true);
				construct_game_screen_overlay(false);
				gfxDriver->DrawSprite(0, 0, ddb);
				render_to_screen();
				update_polled_stuff_if_runtime();
				WaitForNextFrame();
			}

			delete saved_viewport_bitmap;
			saved_viewport_bitmap = nullptr;
			set_palette_range(palette, 0, 255, 0);
			gfxDriver->DestroyDDB(ddb);
		}

	} else if (evp->type == EV_IFACECLICK)
		process_interface_click(evp->data1, evp->data2, evp->data3);
	else quit("process_event: unknown event to process");
}


void runevent_now(int evtyp, int ev1, int ev2, int ev3) {
	EventHappened evh;
	evh.type = evtyp;
	evh.data1 = ev1;
	evh.data2 = ev2;
	evh.data3 = ev3;
	evh.player = game.playercharacter;
	process_event(&evh);
}

void processallevents(int numev, EventHappened *evlist) {
	int dd;

	if (inside_processevent)
		return;

	// make a copy of the events - if processing an event includes
	// a blocking function it will continue to the next game loop
	// and wipe out the event pointer we were passed
	EventHappened copyOfList[MAXEVENTS];
	memcpy(&copyOfList[0], &evlist[0], sizeof(EventHappened) * numev);

	int room_was = play.room_changes;

	inside_processevent++;

	for (dd = 0; dd < numev; dd++) {

		process_event(&copyOfList[dd]);

		if (room_was != play.room_changes)
			break;  // changed room, so discard other events
	}

	inside_processevent--;
}

void update_events() {
	processallevents(numevents, &event[0]);
	numevents = 0;
}
// end event list functions


void ClaimEvent() {
	if (eventClaimed == EVENT_NONE)
		quit("!ClaimEvent: no event to claim");

	eventClaimed = EVENT_CLAIMED;
}
