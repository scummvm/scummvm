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

#include "ags/lib/audio/aldumb.h"

namespace AGS3 {

void dumb_register_packfiles() {
}

DUMBFILE *dumbfile_open_packfile(PACKFILE *p) {
	warning("TODO: dumbfile_open_packfile");
	return nullptr;
}

DUMBFILE *dumbfile_from_packfile(PACKFILE *p) {
	warning("TODO: dumbfile_from_packfile");
	return nullptr;
}

void dumb_register_dat_duh(long type) {
	warning("TODO: dumb_register_dat_duh");
}

void dumb_register_dat_it(long type) {
	warning("TODO: dumb_register_dat_it");
}

void dumb_register_dat_xm(long type) {
	warning("TODO: dumb_register_dat_xm");
}

void dumb_register_dat_s3m(long type) {
	warning("TODO: dumb_register_dat_s3m");
}

void dumb_register_dat_mod(long type) {
	warning("TODO: dumb_register_dat_mod");
}

void dumb_register_dat_it_quick(long type) {
	warning("TODO: dumb_register_dat_it_quick");
}

void dumb_register_dat_xm_quick(long type) {
	warning("TODO: dumb_register_dat_xm_quick");
}

void dumb_register_dat_s3m_quick(long type) {
	warning("TODO: dumb_register_dat_s3m_quick");
}

void dumb_register_dat_mod_quick(long type) {
	warning("TODO: dumb_register_dat_mod_quick");
}

/* DUH Playing Functions */

AL_DUH_PLAYER *al_start_duh(DUH *duh, int n_channels, long pos, float volume, long bufsize, int freq) {
	warning("TODO: al_start_duh");
	return nullptr;
}

void al_stop_duh(AL_DUH_PLAYER *dp) {
	warning("TODO: al_stop_duh");
}

void al_pause_duh(AL_DUH_PLAYER *dp) {
	warning("TODO: al_pause_duh");
}

void al_resume_duh(AL_DUH_PLAYER *dp) {
	warning("TODO: al_resume_duh");
}

void al_duh_set_priority(AL_DUH_PLAYER *dp, int priority) {
	warning("TODO: al_duh_set_priority");
}

void al_duh_set_volume(AL_DUH_PLAYER *dp, float volume) {
	warning("TODO: al_duh_set_volume");
}

float al_duh_get_volume(AL_DUH_PLAYER *dp) {
	warning("TODO: al_duh_get_volume");
	return 0;
}

int al_poll_duh(AL_DUH_PLAYER *dp) {
	warning("TODO: al_poll_duh");
	return 0;
}

long al_duh_get_position(AL_DUH_PLAYER *dp) {
	warning("TODO: al_duh_get_position");
	return 0;
}

AL_DUH_PLAYER *al_duh_encapsulate_sigrenderer(DUH_SIGRENDERER *sigrenderer, float volume, long bufsize, int freq) {
	warning("TODO: al_duh_encapsulate_sigrenderer");
	return nullptr;
}

DUH_SIGRENDERER *al_duh_get_sigrenderer(AL_DUH_PLAYER *dp) {
	warning("TODO: al_duh_get_sigrenderer");
	return nullptr;
}

DUH_SIGRENDERER *al_duh_decompose_to_sigrenderer(AL_DUH_PLAYER *dp) {
	warning("TODO: al_duh_decompose_to_sigrenderer");
	return nullptr;
}

} // namespace AGS3
