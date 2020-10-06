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

#include "engines/icb/p4.h" //for machine version
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/player.h"
#include "engines/icb/direct_input.h"
#include "engines/icb/common/px_route_barriers.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_switches.h"

namespace ICB {

#define FACING_FORWARDS ((70 * 4096) / 360)
#define FACING_BACKWARDS ((135 * 4096) / 360)

#define STOOD_TURN_TOLERANCE ((10 * 4096) / 360)
#define WALK_TURN_TOLERANCE ((20 * 4096) / 360)
#define STAIR_TURN_TOLERANCE ((130 * 4096) / 360)
#define STOOD_HARD_TURN_TOLERANCE ((60 * 4096) / 360)
#define NO_TURN_TOLERANCE (2 * 4096)
#define RUN_TURN_TOLERANCE ((30 * 4096) / 360)

#define NEAREST_INT(X) (X) > 0.0 ? int((X) + 0.5) : int((X)-0.5)
#define FIXED_POINT_ANG(X) NEAREST_INT(((X)*4096.0f) / (2 * M_PI))

#define SECOND_ZONE 80

#define R_LIMIT 50

#define CHECKDXDY(dx, dy)                                                                                                                                                          \
	{                                                                                                                                                                          \
		int r2 = (dx) * (dx) + (dy) * (dy);                                                                                                                                \
		if (r2 < R_LIMIT * R_LIMIT) {                                                                                                                                      \
			if (r2 < 1)                                                                                                                                                \
				r2 = 1;                                                                                                                                            \
			int scale = NEAREST_INT(PXsqrt((double)(128 * 128 * R_LIMIT * R_LIMIT) / r2));                                                                             \
			(dx) = ((dx)*scale) / 128;                                                                                                                                 \
			(dy) = ((dy)*scale) / 128;                                                                                                                                 \
		}                                                                                                                                                                  \
	}

// Which pan value the joystick is pointing at
uint32 g_targetPan = 0;
int g_playerPan;
int g_joyPan;

uint8 _DRAWCOMPASS_ = 0;

// pc device switch
__pc_input device = __KEYS; // default to keys

uint32 fire_key;
uint32 interact_key;
uint32 inventory_key;
uint32 arm_key;
uint32 remora_key;
uint32 crouch_key;
uint32 sidestep_key;
uint32 run_key;
uint32 up_key;
uint32 down_key;
uint32 left_key;
uint32 right_key;
uint32 pause_key;

void _player::Update_input_state() {
	// looks at the input devices in use and returns a single user-is-doing state

	// note - analogue controllers return a direction which is different from forward-and-rotate schemes

	// this routine requires seperate pc/psx coding

	//				pc devices
	//				----------
	//				keys
	//				joysticks
	//				gamepads
	//				analogue devices

	// Are we trying to find a new pan value
	static uint32 hunting = 0;
	g_targetPan = 0;
	g_playerPan = NEAREST_INT(log->pan * 4096.0f);
	int screenPan = 0;
	int clastCameraPan = NEAREST_INT(lastCameraPan * 4096.0f);
	int cdeltaCameraPan = NEAREST_INT(deltaCameraPan * 4096.0f);

	int stairs = 0;
	int ladders = 0;

	int turn_tolerance = STOOD_TURN_TOLERANCE;
	int hard_turn_tolerance = STOOD_HARD_TURN_TOLERANCE;

	// Defaults
	cur_state.turn = __NO_TURN;
	cur_state.bitflag = 0;

	// Make sure we have a set to use !
	// Use lazy logic evaluation
	int haveCamera_ = MS->set.OK();

	Poll_direct_input();

	// Button controls are universal for all control modes

	if ((Read_DI_keys(fire_key)) || (Read_Joystick(fire_button))) // Attack
		cur_state.SetButton(__ATTACK);

	if ((Read_DI_keys(interact_key)) || (Read_Joystick(interact_button))) // Interact
		cur_state.SetButton(__INTERACT);

	if ((Read_DI_keys(inventory_key)) || (Read_Joystick(inventory_button))) // Inventory
		cur_state.SetButton(__INVENTORY);

	if ((Read_DI_keys(arm_key)) || (Read_Joystick(arm_button))) // Arm/unarm
		cur_state.SetButton(__ARMUNARM);

	if ((Read_DI_keys(remora_key)) || (Read_Joystick(remora_button))) // Remora
		cur_state.SetButton(__REMORA);

	if ((Read_DI_keys(crouch_key)) || (Read_Joystick(crouch_button))) // Crouch
		cur_state.SetButton(__CROUCH);

	if ((Read_DI_keys(sidestep_key)) || (Read_Joystick(sidestep_button))) // Sidestep
		cur_state.SetButton(__SIDESTEP);

	if ((Read_DI_keys(run_key)) || (Read_Joystick(run_button))) // Jog
		cur_state.SetButton(__JOG);

	// No control whilst sliding down a ladder !
	if (player_status == SLIP_SLIDIN_AWAY)
		return;

	// No control whilst getting onto a ladder
	if ((player_status == EASY_LINKING) && (stat_after_link == BEGIN_DOWN_LADDER)) {
		cur_state.momentum = __STILL;
		return;
	}

	// Have different turn tolerances & control method for being on or off stairs
	if ((stat_after_link == ON_STAIRS) || (player_status == ON_STAIRS) || (stat_after_link == STOOD_ON_STAIRS) || (player_status == STOOD_ON_STAIRS) ||
	    (stat_after_link == REVERSE_ON_STAIRS) || (player_status == REVERSE_ON_STAIRS) || (stat_after_link == RUNNING_ON_STAIRS) || (player_status == RUNNING_ON_STAIRS)) {
		stairs = 1;
	}

	// Have different bigger tolerances & control method for being on or off ladders
	if (((player_status == EASY_LINKING) && ((stat_after_link == ON_LADDER) || (stat_after_link == SLIP_SLIDIN_AWAY) || (stat_after_link == LEAVE_LADDER) ||
	                                         (stat_after_link == BEGIN_DOWN_LADDER) || (stat_after_link == LEAVE_LADDER_BOTTOM))) ||
	    (player_status == ON_LADDER) || (player_status == SLIP_SLIDIN_AWAY) || (player_status == LEAVE_LADDER) || (player_status == BEGIN_DOWN_LADDER) ||
	    (player_status == LEAVE_LADDER_BOTTOM)) {
		ladders = 1;
		g_playerPan = NEAREST_INT(4096.0f * MS->stairs[stair_num].pan_ref);
	}

	if (currentJoystick != NO_JOYSTICK) {
		// If we are in inventory or remora mode then we should accept input
		// from the keyboard by default (force attached joystick to digital)
		if ((player_status == REMORA) || (player_status == INVENTORY)) {
			// This mode will accept keyboard and joystick
			currentJoystick = DIGITAL_JOYSTICK;
		} else {
			// Set joystick type to whatever is attached
			currentJoystick = attachedJoystick;

			// Set control mode dependent on attached device
			if (currentJoystick != ANALOGUE_JOYSTICK) {
				// Force mode for digital control device
				Set_control_mode(ACTOR_RELATIVE);
			}
		}

		// Only allow SCREEN_RELATIVE control if we have an analogue device
		if ((Get_control_mode() == SCREEN_RELATIVE) && (haveCamera_ == 1)) {
			PXcamera &camera = MSS.GetCamera();

			int32 cameraPan = camera.pan;

			if ((cameraPan > 2048) || (cameraPan < -2048)) {
				// Make it +/- HALF_TURN
				if (cameraPan > 0)
					cameraPan -= 4096;
				else
					cameraPan += 4096;
			}

			screenPan = cameraPan;

			// Catch if the screen pan has never been set
			if (clastCameraPan > 2 * 4096) {
				screenPan = cameraPan;
				cdeltaCameraPan = 0;
				panCycle = 0;
				clastCameraPan = cameraPan;
			} else {
				// Have we changed camera angles ?
				if (clastCameraPan != cameraPan) {
					// Compute the new deltaCameraPan
					cdeltaCameraPan = cameraPan - screenPan;
					// Make -2048 -> 2048
					if (cdeltaCameraPan > 2048)
						cdeltaCameraPan -= 4096;
					else if (cdeltaCameraPan < -2048)
						cdeltaCameraPan += 4096;

					cdeltaCameraPan = cdeltaCameraPan / CAMERA_SMOOTH_CYCLES;
					panCycle = 0;
				}

				if (panCycle < CAMERA_SMOOTH_CYCLES) {
					screenPan += cdeltaCameraPan;
					panCycle++;
				} else {
					screenPan = cameraPan;
				}
			}

			// Read joystick values
			int x = Read_Joystick(Y_AXIS);
			int y = Read_Joystick(X_AXIS);
			int padlr = (x * x) + (y * y);

			// Set new direction to hunt for
			if (padlr > 0) {
				if (x == 0) {
					g_joyPan = 1024;
				} else {
					// This works jes' fine
					g_joyPan = NEAREST_INT((atan((float)y / (float)x) / TWO_PI) * 4096.0f);
				}

				if ((x < 0) && (y > 0))
					g_joyPan += 2048;
				if ((x <= 0) && (y <= 0))
					g_joyPan += 2048;

				// Directly down
				if (g_joyPan == 0)
					g_joyPan = 1;

				// Hunted for pan = g_joyPan - screenPan
				g_targetPan = g_joyPan - screenPan;

				// Make 0-4096
				g_targetPan = g_targetPan % 4096;
				// Looking for a new pan value
				hunting = 1;
			} else {
				// Within deadzone so reset all
				hunting = 0;
				g_joyPan = 0;
				g_targetPan = g_playerPan;
				cur_state.turn = __NO_TURN;
				cur_state.momentum = __STILL;
			}

			// If running then always running
			if (cur_state.IsButtonSet(__JOG)) {
				if (player_status == STOOD)
					cur_state.momentum = __FORWARD_1;
				else
					cur_state.momentum = __FORWARD_2;
			} else
			    // Sidestep can be used as a brake
			    if (cur_state.IsButtonSet(__SIDESTEP)) {
				if (player_status != STOOD)
					cur_state.momentum = __STILL;
			}

			if (hunting) {
				// Correct to get 0-4096
				if (g_playerPan < 0)
					g_playerPan += 4096;

				ASSERT1(((g_playerPan >= 0) && (g_playerPan <= 4096)), "Bad g_playerPan %d", g_playerPan);

				// So we should have a g_targetPan the character is trying to turn to
				int delta = g_targetPan - g_playerPan;

				delta = delta % 4096;

				if (delta > 2048)
					delta -= 4096;
				if (delta < -2048)
					delta += 4096;

				int absDelta = abs(delta);

				// Has the run been pressed
				if (cur_state.IsButtonSet(__JOG)) {
					// Not hunting for a pan value anymore
					hunting = 0;

					if (stairs == 1) {
						// No turning on stairs whilst running
						turn_tolerance = NO_TURN_TOLERANCE;
						// Run : go from stand -> walk -> run
						if (absDelta < FACING_FORWARDS) {
							if (player_status == STOOD_ON_STAIRS)
								cur_state.momentum = __FORWARD_1;
							else
								cur_state.momentum = __FORWARD_2;
						} else {
							// Turn LEFT or RIGHT you decide whilst running
							if (delta > turn_tolerance)
								cur_state.turn = __LEFT;
							else if (delta < -turn_tolerance)
								cur_state.turn = __RIGHT;
						}
					} else if (ladders == 1) {
						// RUN on ladder means slide down it fireman style
						cur_state.momentum = __FORWARD_2;
					} else {
						turn_tolerance = RUN_TURN_TOLERANCE;

						// Turn LEFT or RIGHT you decide whilst running
						if (delta > turn_tolerance)
							cur_state.turn = __LEFT;
						else if (delta < -turn_tolerance)
							cur_state.turn = __RIGHT;
					}
				} else if (cur_state.IsButtonSet(__SIDESTEP)) {
					hunting = 0;

					if (stairs == 1) {
						// No turning on stairs whilst running
						turn_tolerance = NO_TURN_TOLERANCE;

						// Run : go from stand -> walk -> run
						if (absDelta < FACING_FORWARDS) {
							if (player_status == STOOD_ON_STAIRS)
								cur_state.momentum = __FORWARD_1;
							else
								cur_state.momentum = __FORWARD_2;
						} else {
							// Turn LEFT or RIGHT you decide whilst running
							if (delta > turn_tolerance)
								cur_state.turn = __LEFT;
							else if (delta < -turn_tolerance)
								cur_state.turn = __RIGHT;
						}
					} else if (ladders == 1) {
						// RUN on ladder means slide down it fireman style
						cur_state.momentum = __FORWARD_2;
					} else {
						turn_tolerance = RUN_TURN_TOLERANCE;

						// The back facing 90 degrees is step backwards
						if (absDelta > FACING_BACKWARDS) {
							cur_state.momentum = __BACKWARD_1;
						}
						// So just left or right based on sign
						// This means sidestep left or right
						else {
							if (delta > 0)
								cur_state.turn = __LEFT;
							else
								cur_state.turn = __RIGHT;
						}
					}
				} else {
					// Nope, normal screen relative movement

					// Catch if not pressing run but running ! then start walking
					if (cur_state.momentum == __FORWARD_2) {
						cur_state.momentum = __FORWARD_1;
					}

					// Have different turn tolerances for standing and walking
					if (stairs == 1) {
						turn_tolerance = STAIR_TURN_TOLERANCE;
					} else if (ladders == 1) {
						// No turning on ladders
						turn_tolerance = NO_TURN_TOLERANCE;
					} else if (player_status == WALKING) {
						turn_tolerance = WALK_TURN_TOLERANCE;
					} else {
						turn_tolerance = STOOD_TURN_TOLERANCE;
						hard_turn_tolerance = STOOD_HARD_TURN_TOLERANCE;
					}

					// Get these checks dealt with first
					if (ladders == 1) {
						// Just need to test for going down the ladder i.e. BACKWARDS
						if (absDelta > FACING_BACKWARDS) {
							cur_state.momentum = __BACKWARD_1;
						} else if (absDelta < FACING_FORWARDS) {
							cur_state.momentum = __FORWARD_1;
						}

						return;
					}

					// Turn LEFT or RIGHT you decide
					if (delta > hard_turn_tolerance) {
						cur_state.turn = __HARD_LEFT;
					} else if (delta < -hard_turn_tolerance) {
						cur_state.turn = __HARD_RIGHT;
					} else if (delta > turn_tolerance) {
						cur_state.turn = __LEFT;
					} else if (delta < -turn_tolerance) {
						cur_state.turn = __RIGHT;
					}
					// So at the right direction start walking
					else {
						if (ladders == 1) {
							// Just need to test for going down the ladder i.e. BACKWARDS
							if (absDelta > FACING_BACKWARDS) {
								cur_state.momentum = __BACKWARD_1;
							} else if (absDelta < FACING_FORWARDS) {
								cur_state.momentum = __FORWARD_1;
							}
						} else if ((stairs == 0) || (absDelta < FACING_FORWARDS)) {
							hunting = 0;

							// Are past the dead zone
							if (padlr > 0) {
								// Walk
								cur_state.momentum = __FORWARD_1;
							}
						}
					}
				}
			} // if(hunting)
			else if (ladders == 1) {
				// RUN on ladder means slide down it fireman style
				if (cur_state.IsButtonSet(__JOG)) {
					cur_state.momentum = __FORWARD_2;
				}
			}

			// Can only use HARD_LEFT & HARD_RIGHT when STOOD
			if (player_status != STOOD) {
				if (cur_state.turn == __HARD_LEFT)
					cur_state.turn = __LEFT;
				else if (cur_state.turn == __HARD_RIGHT)
					cur_state.turn = __RIGHT;
			}
		} // Get_control_mode() == SCREEN_RELATIVE

		if (Get_control_mode() == ACTOR_RELATIVE) {
			// Don't need to draw this
			_DRAWCOMPASS_ = 0;

			// Two modes of actor relative, analogue and digital
			if (currentJoystick == ANALOGUE_JOYSTICK) {
				int x = Read_Joystick(X_AXIS);
				int y = Read_Joystick(Y_AXIS);
				int padlr = (x * x) + (y * y);

				// If running then always running
				if (cur_state.IsButtonSet(__JOG)) {
					if (player_status == STOOD)
						cur_state.momentum = __FORWARD_1;
					else
						cur_state.momentum = __FORWARD_2;

					// Turn left/right whilst walking/running
					if (padlr > SECOND_ZONE) {
						if (x < 0)
							cur_state.turn = __LEFT;
						else
							cur_state.turn = __RIGHT;
					}
				} else
				    // Are we out of the deadzone
				    if (padlr > 0) {
					// We want to turn right
					if (x > 0) {
						// We want to turn right fast
						if (x > SECOND_ZONE) {
							// Only allowed to hard turn when stood still and not sidestepping
							if ((cur_state.momentum == __STILL) && (!cur_state.IsButtonSet(__SIDESTEP))) {
								// Also consider button restraints
								if (cur_state.IsButtonSet(__ARMUNARM) || cur_state.IsButtonSet(__CROUCH))
									cur_state.turn = __RIGHT;
								else
									cur_state.turn = __HARD_RIGHT;
							} else
								cur_state.turn = __RIGHT;
						}
						// Turn right
						else
							cur_state.turn = __RIGHT;
					} else
					    // We want to turn left
					    if (x < 0) {
						// We want to turn left fast
						if (x < -SECOND_ZONE) {
							// Only allowed to hard turn when stood still and not sidestepping
							if ((cur_state.momentum == __STILL) && (!cur_state.IsButtonSet(__SIDESTEP))) {
								// Also consider button restraints
								if (cur_state.IsButtonSet(__ARMUNARM) || cur_state.IsButtonSet(__CROUCH))
									cur_state.turn = __LEFT;
								else
									cur_state.turn = __HARD_LEFT;
							} else
								cur_state.turn = __LEFT;
						}
						// Turn left
						else
							cur_state.turn = __LEFT;
					}
					// Don't want to turn at all
					else
						cur_state.turn = __NO_TURN;

					// We want move forward
					if (y < 0) {
						// Can't move forward with sidestep held (unless on stairs or ladders)
						if (cur_state.IsButtonSet(__SIDESTEP) && stairs == 0 && ladders == 0) {
							cur_state.momentum = __STILL;
						} else
							// We want to walk
							cur_state.momentum = __FORWARD_1;
					} else
					    // We want to move backwards
					    if (y > 0) {
						cur_state.momentum = __BACKWARD_1;
					}
					// Don't want to move
					else
						cur_state.momentum = __STILL;
				} else {
					// Within deadzone so don't move or turn
					cur_state.momentum = __STILL;
					cur_state.turn = __NO_TURN;
				}
			} // End of Analogue actor relative controls
			else {
				// Digital actor relative controls (same as keys)
				// If running then always running
				if (cur_state.IsButtonSet(__JOG)) {
					if (player_status == STOOD)
						cur_state.momentum = __FORWARD_1;
					else
						cur_state.momentum = __FORWARD_2;
				} else
				    // Forward/backward - one or the other
				    if ((Read_DI_keys(up_key)) || (Read_Joystick(Y_AXIS) == JOY_UP)) {
					// Can't move forward with sidestep held (unless on stairs or ladders)
					if (cur_state.IsButtonSet(__SIDESTEP) && stairs == 0 && ladders == 0) {
						cur_state.momentum = __STILL;
					} else
						// We want to walk
						cur_state.momentum = __FORWARD_1;
				} else if ((Read_DI_keys(down_key)) || (Read_Joystick(Y_AXIS) == JOY_DOWN))
					cur_state.momentum = __BACKWARD_1;
				else
					cur_state.momentum = __STILL;

				// Left/right
				if ((Read_DI_keys(left_key)) || (Read_Joystick(X_AXIS) == JOY_LEFT)) {
					cur_state.turn = __LEFT;
				} else if ((Read_DI_keys(right_key)) || (Read_Joystick(X_AXIS) == JOY_RIGHT)) {
					cur_state.turn = __RIGHT;
				} else
					cur_state.turn = __NO_TURN;

			} // End of Digital actor relative control
		}

	}
	// No Joystick
	else {
		// Keyboard support (generic controls)

		// If running then always running
		if (cur_state.IsButtonSet(__JOG)) {
			if (player_status == STOOD)
				cur_state.momentum = __FORWARD_1;
			else
				cur_state.momentum = __FORWARD_2;
		} else
		    // Forward/backward - one or the other
		    if (Read_DI_keys(up_key)) {
			// Can't move forward with sidestep held (unless on stairs or ladders)
			if (cur_state.IsButtonSet(__SIDESTEP) && stairs == 0 && ladders == 0) {
				cur_state.momentum = __STILL;
			} else
				// We want to walk
				cur_state.momentum = __FORWARD_1;
		} else if (Read_DI_keys(down_key))
			cur_state.momentum = __BACKWARD_1;
		else
			cur_state.momentum = __STILL;

		// Left/right
		if (Read_DI_keys(left_key)) {
			cur_state.turn = __LEFT;
		} else if (Read_DI_keys(right_key)) {
			cur_state.turn = __RIGHT;
		} else
			cur_state.turn = __NO_TURN;
	}

	// Set the floating point version of the fixed point variables we've used
	lastCameraPan = ((float)clastCameraPan / 4096.0f);
	deltaCameraPan = ((float)cdeltaCameraPan / 4096.0f);
}

void _player::DrawCompass() {
	// User can disable joystick in which case mode is forced to actor
	// relative and only the keys will work until the joystick is enabled
	if (currentJoystick != NO_JOYSTICK) {
		// Only draw compass in THREED mode and not while player is in inventory or remora mode
		if ((px.display_mode == THREED) && (player_status != REMORA) && (player_status != INVENTORY) && _DRAWCOMPASS_) {
			uint32 pitch;
			uint8 *ad;

#define COMPASS_LENGTH 50.0f
#define COMPASS_OX 60
#define COMPASS_OY 420

			_rgb purple = {250, 60, 200, 0};
			_rgb turq = {0, 250, 250, 0};
			_rgb yellow = {250, 250, 0, 0};

			ad = surface_manager->Lock_surface(working_buffer_id);
			pitch = surface_manager->Get_pitch(working_buffer_id);

			if (g_joyPan == 0) {
				// Centered/Within deadzone (display a cross)
				General_draw_line_24_32(COMPASS_OX, COMPASS_OY - 5, COMPASS_OX, COMPASS_OY + 5, &yellow, ad, pitch);
				General_draw_line_24_32(COMPASS_OX - 5, COMPASS_OY, COMPASS_OX + 5, COMPASS_OY, &yellow, ad, pitch);
			} else {
				float theta = ((float)g_joyPan / 4096.0f) * TWO_PI;

				int x = NEAREST_INT(COMPASS_LENGTH * PXsin(theta));
				int y = NEAREST_INT(COMPASS_LENGTH * PXcos(theta));

				General_draw_line_24_32(COMPASS_OX, COMPASS_OY, (int16)(COMPASS_OX + x), (int16)(COMPASS_OY + y), &yellow, ad, pitch);
			}

			// Get the actor's screen position
			PXvector origin_world, origin_scrn;
			origin_world = log->mega->actor_xyz;
			bool8 result;
			PXcamera &camera = MSS.GetCamera();
			PXWorldToFilm(origin_world, camera, result, origin_scrn);

			PXvector point_world, point_scrn;

			int dx, dy;

			double s_ang, c_ang;

			s_ang = PXsin(((float)g_targetPan / 4096.0f) * TWO_PI);
			c_ang = PXcos(((float)g_targetPan / 4096.0f) * TWO_PI);

			int sang = FIXED_POINT_ANG(s_ang);
			int cang = FIXED_POINT_ANG(c_ang);

			point_world.x = (float)(origin_world.x + (COMPASS_LENGTH * sang / 4096));
			point_world.y = origin_world.y;
			point_world.z = (float)(origin_world.z + (COMPASS_LENGTH * cang / 4096));

			PXWorldToFilm(point_world, camera, result, point_scrn);

			dx = NEAREST_INT(point_scrn.x - origin_scrn.x);
			dy = NEAREST_INT(point_scrn.y - origin_scrn.y);

			CHECKDXDY(dx, dy)

			General_draw_line_24_32(COMPASS_OX, COMPASS_OY, (int16)(COMPASS_OX + dx), (int16)(COMPASS_OY - dy), &turq, ad, pitch);

			s_ang = PXsin(((float)g_playerPan / 4096.0f) * TWO_PI);
			c_ang = PXcos(((float)g_playerPan / 4096.0f) * TWO_PI);

			sang = FIXED_POINT_ANG(s_ang);
			cang = FIXED_POINT_ANG(c_ang);

			point_world.x = (float)(origin_world.x + (COMPASS_LENGTH * sang / 4096));
			point_world.y = origin_world.y;
			point_world.z = (float)(origin_world.z + (COMPASS_LENGTH * cang / 4096));
			PXWorldToFilm(point_world, camera, result, point_scrn);

			dx = NEAREST_INT(point_scrn.x - origin_scrn.x);
			dy = NEAREST_INT(point_scrn.y - origin_scrn.y);

			CHECKDXDY(dx, dy)

			General_draw_line_24_32(COMPASS_OX, COMPASS_OY, (int16)(COMPASS_OX + dx), (int16)(COMPASS_OY - dy), &purple, ad, pitch);

			surface_manager->Unlock_surface(working_buffer_id);
		}
	}
}

} // End of namespace ICB
