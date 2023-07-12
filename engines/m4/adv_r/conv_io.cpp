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

#include "m4/adv_r/conv_io.h"

namespace M4 {

void Converstation_Globals::syncGame(Common::Serializer &s) {
//	FILE *fp = NULL;
	uint32 val;

	conv_reset_all();

	// Handle size
	val = conv_save_buff.size();
	s.syncAsUint32LE(val);
	if (s.isLoading())
		conv_save_buff.resize(val);

	// Read in the buffer
	s.syncBytes(&conv_save_buff[0], conv_save_buff.size());
}

void Converstation_Globals::conv_reset_all() {
	conv_save_buff.clear();
}

/*------------------------------------------------------------------------*/

void conv_unload(Conv *c) {
	error("TODO: conv_unload");
}

Conv *conv_get_handle() {
	error("TODO: conv_get_handle");
	return nullptr;
}


} // End of namespace M4
