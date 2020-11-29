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

#include "ags/lib/dumb-0.9.2/dumb.h"
#include "common/textconsole.h"

namespace AGS3 {

void dumb_exit() {
	// No implementation
}

DUH *dumb_load_it(const char *filename) {
	warning("TODO: dumb_load_it");
	return nullptr;
}

DUH *dumb_load_xm(const char *filename) {
	warning("TODO: dumb_load_xm");
	return nullptr;
}

DUH *dumb_load_s3m(const char *filename) {
	warning("TODO: dumb_load_s3m");
	return nullptr;
}

DUH *dumb_load_mod(const char *filename) {
	warning("TODO: dumb_load_mod");
	return nullptr;
}

DUH *dumb_read_it(DUMBFILE *f) {
	warning("TODO: dumb_read_it");
	return nullptr;
}
DUH *dumb_read_xm(DUMBFILE *f) {
	warning("TODO: dumb_read_xm");
	return nullptr;
}

DUH *dumb_read_s3m(DUMBFILE *f) {
	warning("TODO: dumb_read_s3m");
	return nullptr;
}

DUH *dumb_read_mod(DUMBFILE *f) {
	warning("TODO: dumb_read_mod");
	return nullptr;
}

int dumb_it_sd_get_n_orders(DUMB_IT_SIGDATA *sd) {
	warning("TODO: dumb_it_sd_get_n_orders");
	return 0;
}

int dumb_it_sd_get_initial_global_volume(DUMB_IT_SIGDATA *sd) {
	warning("TODO: dumb_it_sd_get_initial_global_volume");
	return 0;
}

void dumb_it_sd_set_initial_global_volume(DUMB_IT_SIGDATA *sd, int gv) {
	warning("TODO: dumb_it_sd_set_initial_global_volume");
}

int dumb_it_sd_get_mixing_volume(DUMB_IT_SIGDATA *sd) {
	warning("TODO: dumb_it_sd_get_mixing_volume");
	return 0;
}

void dumb_it_sd_set_mixing_volume(DUMB_IT_SIGDATA *sd, int mv) {
	warning("TODO: dumb_it_sd_set_mixing_volume");
}

int dumb_it_sd_get_initial_speed(DUMB_IT_SIGDATA *sd) {
	warning("TODO: dumb_it_sd_get_initial_speed");
	return 0;
}

void dumb_it_sd_set_initial_speed(DUMB_IT_SIGDATA *sd, int speed) {
	warning("TODO: dumb_it_sd_set_initial_speed");
}

int dumb_it_sd_get_initial_tempo(DUMB_IT_SIGDATA *sd) {
	warning("TODO: dumb_it_sd_get_initial_tempo");
	return 0;
}

void dumb_it_sd_set_initial_tempo(DUMB_IT_SIGDATA *sd, int tempo) {
	warning("TODO: dumb_it_sd_set_initial_tempo");
}

int dumb_it_sd_get_initial_channel_volume(DUMB_IT_SIGDATA *sd, int channel) {
	warning("TODO: dumb_it_sd_get_initial_channel_volume");
	return 0;
}

void dumb_it_sd_set_initial_channel_volume(DUMB_IT_SIGDATA *sd, int channel, int volume) {
	warning("TODO: dumb_it_sd_set_initial_channel_volume");
}

int dumb_it_sr_get_current_order(DUMB_IT_SIGRENDERER *sr) {
	warning("TODO: dumb_it_sr_get_current_order");
	return 0;
}

int dumb_it_sr_get_current_row(DUMB_IT_SIGRENDERER *sr) {
	warning("TODO: dumb_it_sr_get_current_row");
	return 0;
}

int dumb_it_sr_get_global_volume(DUMB_IT_SIGRENDERER *sr) {
	warning("TODO: dumb_it_sr_get_global_volume");
	return 0;
}

void dumb_it_sr_set_global_volume(DUMB_IT_SIGRENDERER *sr, int gv) {
	warning("TODO: dumb_it_sr_set_global_volume");
}

int dumb_it_sr_get_tempo(DUMB_IT_SIGRENDERER *sr) {
	warning("TODO: dumb_it_sr_get_tempo");
	return 0;
}

void dumb_it_sr_set_tempo(DUMB_IT_SIGRENDERER *sr, int tempo) {
	warning("TODO: dumb_it_sr_set_tempo");
}

int dumb_it_sr_get_speed(DUMB_IT_SIGRENDERER *sr) {
	warning("TODO: dumb_it_sr_get_speed");
	return 0;
}

void dumb_it_sr_set_speed(DUMB_IT_SIGRENDERER *sr, int speed) {
	warning("TODO: dumb_it_sr_set_speed");
}

void unload_duh(DUH *duh) {
	warning("TODO: unload_duh");
}

DUH *load_duh(const char *filename) {
	warning("TODO: load_duh");
	return nullptr;
}

DUH *read_duh(DUMBFILE *f) {
	warning("TODO: load_duh");
	return nullptr;
}

long duh_get_length(DUH *duh) {
	warning("TODO: duh_get_length");
	return 0;
}

void duh_end_sigrenderer(DUH_SIGRENDERER *sigrenderer) {
	warning("TODO: duh_end_sigrenderer");
}


} // namespace AGS3
