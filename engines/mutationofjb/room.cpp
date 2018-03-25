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

#include "mutationofjb/room.h"
#include "mutationofjb/animationdecoder.h"
#include "mutationofjb/encryptedfile.h"
#include "mutationofjb/util.h"
#include "common/str.h"
#include "common/translation.h"
#include "graphics/screen.h"

namespace MutationOfJB {

class RoomAnimationDecoderCallback : public AnimationDecoderCallback {
public:
	RoomAnimationDecoderCallback(Room &room) : _room(room) {}
	virtual void onFrame(int frameNo, Graphics::Surface &surface) override;
	virtual void onPaletteUpdated(byte palette[PALETTE_SIZE]) override;
private:
	Room &_room;
};

void RoomAnimationDecoderCallback::onPaletteUpdated(byte palette[PALETTE_SIZE]) {
	_room._screen->setPalette(palette, 0x00, 0xC0); // Load only 0xC0 colors.
}

void RoomAnimationDecoderCallback::onFrame(int frameNo, Graphics::Surface &surface) {
	if (frameNo != 0) {
		return;
	}

	_room._screen->blitFrom(surface);
}

Room::Room(Graphics::Screen *screen) : _screen(screen) {}

bool Room::load(uint8 roomNumber, bool roomB) {
	const Common::String fileName = Common::String::format("room%d%s.dat", roomNumber, roomB ? "b" : "");
	AnimationDecoder decoder(fileName);
	RoomAnimationDecoderCallback callback(*this);
	return decoder.decode(&callback);
}

}
