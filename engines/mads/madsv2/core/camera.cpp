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

#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/tile.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/inter.h"

namespace MADS {
namespace MADSV2 {


Camera camera_x;        /* Horizontal panning camera */
Camera camera_y;        /* Vertical panning camera   */

int camera_old_x_target = 0;/* For saving/restoring game */
int camera_old_y_target = 0;


void camera_init_default(void) {
	camera_x.panning = false;
	camera_y.panning = false;

	if (picture_map.one_to_one) {
		camera_x.pans = false;
		camera_y.pans = false;
		goto done;
	}

	camera_x.pans = (picture_map.total_x_size > video_x);
	camera_y.pans = (picture_map.total_y_size > display_y);

	if (camera_x.pans) {
		camera_x.pan_mode = CAMERA_PLAYER;
		camera_x.pan_rate = 4;
		camera_x.pan_velocity = CAMERA_DEFAULT_X_VELOCITY;
		camera_x.pan_target = 0;
		camera_x.pan_off_center = CAMERA_DEFAULT_X_OFF_CENTER;
		camera_x.pan_on_tolerance = CAMERA_DEFAULT_X_START;
		camera_x.pan_off_tolerance = CAMERA_DEFAULT_X_STOP;
		camera_x.pan_clock = kernel.clock;
	}

	if (camera_y.pans) {
		camera_y.pan_mode = CAMERA_MANUAL;
		camera_y.pan_rate = 4;
		camera_y.pan_velocity = CAMERA_DEFAULT_Y_VELOCITY;
		camera_y.pan_target = 0;
		camera_y.pan_off_center = CAMERA_DEFAULT_Y_OFF_CENTER;
		camera_y.pan_on_tolerance = CAMERA_DEFAULT_Y_START;
		camera_y.pan_off_tolerance = CAMERA_DEFAULT_Y_STOP;
		camera_y.pan_clock = kernel.clock;
	}

done:
	;
}

static int camera_pan(Camera *camera, int *picture_view, int *player_loc,
		int display_size, int picture_size) {
	int any_pan = false;
	int loc;
	int high_edge, low_edge;
	int dif;
	int direction;
	int magnitude;
	int pan_me = 0;
	long determining_clock;

	if (camera->pans) {
		camera->pan_this_frame = false;

		// Try to keep player & panning in synch
		if ((abs(camera->pan_clock - player.clock) < camera->pan_rate) &&
			(player.frame_delay == camera->pan_rate)) {
			determining_clock = player.clock;
		} else {
			determining_clock = camera->pan_clock;
		}

		if (camera->panning && (kernel.clock < determining_clock)) goto done;

		camera->pan_clock = kernel.clock + camera->pan_rate;

		if (camera->pan_mode == CAMERA_MANUAL) {
			if (camera->panning) {
				dif = camera->pan_target - *picture_view;
				direction = sgn(dif);
				magnitude = abs(dif);

				magnitude = MIN(magnitude, camera->pan_velocity);

				if (magnitude == 0) {
					camera->panning = false;
				} else {
					pan_me = sgn_in(magnitude, direction);
					*picture_view += pan_me;

					any_pan = true;
					camera->pan_this_frame = true;
				}
			}
		} else {

			if (!camera->panning) {
				low_edge = *picture_view + camera->pan_on_tolerance;
				high_edge = *picture_view - camera->pan_on_tolerance + display_size - 1;

				if (*player_loc < low_edge) {
					if (picture_view) {
						camera->panning = true;
						camera->pan_direction = -1;
					}
				}

				if (*player_loc > high_edge) {
					if (*picture_view < (picture_size - display_size)) {
						camera->panning = true;
						camera->pan_direction = 1;
					}
				}
			}

			loc = *player_loc - (display_size >> 1);

			loc += sgn_in(camera->pan_off_center, camera->pan_direction);

			loc = MAX(0, loc);
			loc = MIN(loc, (picture_size - display_size));

			camera->pan_target = loc;

			dif = loc - *picture_view;
			magnitude = abs(dif);
			direction = sgn(dif);


			if (camera->panning) {
				if (magnitude <= camera->pan_off_tolerance) {
					camera->panning = false;
				}
			}

			if (camera->panning) {
				magnitude = MIN(magnitude, camera->pan_velocity);
				pan_me = sgn_in(magnitude, direction);

				if (pan_me) {
					*picture_view += pan_me;
					any_pan = true;
					camera->pan_this_frame = true;
				}
			}
		}
	}

done:
	return any_pan;
}

void camera_jump_to(int x, int y) {
	picture_view_x = x;
	picture_view_y = y;

	tile_pan(&picture_map, picture_view_x, picture_view_y);
	tile_pan(&depth_map, picture_view_x, picture_view_y);
	matte_refresh_work();
}

void camera_pan_to(Camera *camera, int target) {
	if (camera->pans) {
		camera->panning = true;
		camera->pan_mode = CAMERA_MANUAL;
		camera->pan_target = target;
		camera->pan_clock = kernel.clock;
	}
}

void camera_update() {
	int any_pan = false;

	any_pan |= camera_pan(&camera_x, &picture_view_x, &player.x, video_x, picture_map.total_x_size);
	any_pan |= camera_pan(&camera_y, &picture_view_y, &player.y, display_y, picture_map.total_y_size);

	if (any_pan) {
		camera_jump_to(picture_view_x, picture_view_y);
		inter_force_rescan = true;
	}
}

} // namespace MADSV2
} // namespace MADS
