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

#include "engines/icb/speech.h"
#include "engines/icb/fn_routines.h"
#include "engines/icb/session.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_switches.h"

namespace ICB {

void _game_session::Render_speech(text_sprite *bloc) {
	// if there is a speech text bloc in existence then render it
	uint8 *src;
	uint8 *write;
	uint32 x, y;
	uint32 pitch;
	uint32 width, height;
	uint8 *safe_ad;

	// is there anything to render?
	if (!bloc->please_render)
		return;

	src = bloc->GetSprite();

	// Get a pointer to the dx surface
	safe_ad = (uint8 *)surface_manager->Lock_surface(bloc->GetSurface());
	pitch = surface_manager->Get_pitch(bloc->GetSurface());

	height = bloc->GetHeight();
	width = bloc->GetWidth();

	//	get screen address
	safe_ad += (bloc->renderY * pitch);
	safe_ad += (bloc->renderX * 4);

	for (y = 0; y < height; y++) {
		write = safe_ad;

		for (x = 0; x < width; x++) {
			if ((*src) || (*(src + 1)) || (*(src + 2))) {
				*write++ = *src++; // b
				*write++ = *src++; // g
				*write++ = *src++; // r
				write++; // a
			} else {
				src += 3;
				write += 4;
			}
		}
		safe_ad += pitch;
	}

	// unlock buffer
	surface_manager->Unlock_surface(bloc->GetSurface());
}

// These functions set pallete entry 1, which controls the main colour for speech text.  These are
// used by the Remora to display its text in different colours.
void SetTextColour(uint8 r, uint8 g, uint8 b) {
	static _rgb sTempSpeechColour;

	sTempSpeechColour.red = r;
	sTempSpeechColour.green = g;
	sTempSpeechColour.blue = b;

	psTempSpeechColour = &sTempSpeechColour;
}

} // End of namespace ICB
