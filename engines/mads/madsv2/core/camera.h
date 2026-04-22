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

#ifndef MADS_CORE_CAMERA_H
#define MADS_CORE_CAMERA_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {


#define CAMERA_PLAYER           0               /* Camera follows player  */
#define CAMERA_MANUAL           1               /* Camera manually panned */

#define CAMERA_DEFAULT_X_VELOCITY       4       /* Camera defaults        */
#define CAMERA_DEFAULT_X_START          80
#define CAMERA_DEFAULT_X_STOP           4
#define CAMERA_DEFAULT_X_OFF_CENTER     80

#define CAMERA_DEFAULT_Y_VELOCITY       2
#define CAMERA_DEFAULT_Y_START          60
#define CAMERA_DEFAULT_Y_STOP           4
#define CAMERA_DEFAULT_Y_OFF_CENTER     80


typedef struct {
	int pans;                     /* Flag if camera can pan in this room */
	int panning;                  /* Flag if camera now panning          */
	int pan_mode;                 /* Panning mode (CAMERA_PLAYER, etc.)  */
	int pan_velocity;             /* Pixel velocity of pan               */
	int pan_rate;                 /* Pan frame rate                      */
	int pan_target;               /* Pan X or Y target for manual pans   */
	int pan_off_center;           /* Pan distance off center             */
	int pan_on_tolerance;         /* Pan initiator tolerance             */
	int pan_off_tolerance;        /* Pan terminator tolerance            */
	int pan_direction;            /* Pan initial direction               */
	long pan_clock;               /* Pan clock for next update           */
	int pan_this_frame;           /* Flag if panned this frame           */
} Camera;


extern Camera camera_x;        /* Horizontal panning camera */
extern Camera camera_y;        /* Vertical panning camera   */

extern int camera_old_x_target;/* For saving/restoring game */
extern int camera_old_y_target;

extern void camera_init_default();
extern void camera_jump_to(int x, int y);
extern void camera_pan_to(Camera *camera, int target);
extern void camera_update();

} // namespace MADSV2
} // namespace MADS

#endif
