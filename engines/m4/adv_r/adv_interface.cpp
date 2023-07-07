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
 * but WITHOUT ANY WARRANTY {} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/adv_r/adv_interface.h"
#include "m4/vars.h"

namespace M4 {

void interface_hide() {
	warning("TODO: interface_hide");
}

void interface_show() {
	warning("TODO: interface_show");
}

void track_hotspots_refresh() {
}

bool intr_EventHandler(void *bufferPtr, int32 eventType, int32 event, int32 x, int32 y, bool *z) {
	warning("TODO: intr_EventHandler");
	return true;
}

void intr_set_verb(char *name) {
	warning("TODO: intr_set_verb");
}

void intr_set_object(char *name) {
	warning("TODO: intr_set_object");
}

void intr_set_prep(char *name) {
	warning("TODO: intr_set_prep");
}

void intr_set_ind_object(char *name) {
	warning("TODO: intr_set_ind_object");
}

void intr_cancel_sentence() {
	_GI().cancel_sentence();
}

void intr_show_sentence() {
	warning("TODO: inter_inv_button_callback");
}

void inter_callback(void *a, void *b) {
	warning("TODO: inter_inv_button_callback");
}

void inter_inv_button_callback(void *myItem, void *myDlog) {
	warning("TODO: inter_inv_button_callback");
}

} // End of namespace M4
