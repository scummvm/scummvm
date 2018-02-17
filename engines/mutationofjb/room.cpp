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
#include "mutationofjb/encryptedfile.h"
#include "mutationofjb/util.h"
#include "common/str.h"
#include "common/translation.h"
#include "graphics/screen.h"

namespace MutationOfJB {

Room::Room(Graphics::Screen *screen) : _screen(screen) {}

bool Room::load(uint8 roomNumber, bool roomB) {
	EncryptedFile file;
	Common::String fileName = Common::String::format("room%d%s.dat", roomNumber, roomB ? "b" : "");

	file.open(fileName);

	if (!file.isOpen()) {
		reportFileMissingError(fileName.c_str());

		return false;
	}

	file.seek(0x80);

	while (!file.eos()) {
		// Record.
		const uint32 length = file.readUint32LE();
		uint8 info[4] = {0};
		file.read(info, 4);

		// TODO Find out what these are.
		uint32 unknown;
		unknown = file.readUint32LE();
		unknown = file.readUint32LE();

		// Subrecords.
		if (info[0] == 0xFA && info[1] == 0xF1) {
			for (int i = 0; i < info[2]; ++i) {
				const uint32 subLength = file.readUint32LE();
				const uint16 type = file.readUint16LE();

				if (type == 0x0B) {
					loadPalette(file);
				} else if (type == 0x0F) {
					loadBackground(file, subLength - 6);
				} else {
					debug(_("Unsupported record type %02X."), type);
					file.seek(subLength - 6, SEEK_CUR);
				}
			}
		}
	}

	file.close();

	return true;
}

void Room::loadPalette(EncryptedFile &file) {
	uint32 unknown;

	// TODO Find out what this is.
	unknown = file.readUint32LE();

	uint8 palette[PALETTE_SIZE];
	file.read(palette, PALETTE_SIZE);

	for (int j = 0; j < PALETTE_SIZE; ++j) {
		palette[j] <<= 2; // Uses 6-bit colors.
	}

	_screen->setPalette(palette, 0x00, 0xC0); // Load only 0xC0 colors.
}

void Room::loadBackground(EncryptedFile &file, uint32 size) {
	_screen->clear();

	uint8 * const pixels = static_cast<uint8 *>(_screen->getPixels());
	uint8 *ptr = pixels;
	uint32 readBytes = 0;
	uint32 lines = 0;

	while (readBytes != size) {
		if (lines == 200) {
			// Some background files have an unknown byte at the end,
			// so break when we encounter all 200 lines.
			break;
		}

		uint8 no = file.readByte();
		readBytes++;
		while (no--) {
			uint8 n = file.readByte();
			readBytes++;
			if (n < 0x80) {
				// RLE - Copy color n times.
				uint8 color = file.readByte();
				readBytes++;
				while(n--) {
					*ptr++ = color;
				}
			} else {
				// Take next 0x100 - n bytes as they are.
				const uint32 rawlen = 0x100 - n;
				file.read(ptr, rawlen);
				readBytes += rawlen;
				ptr += rawlen;
			}
		}
		lines++;
	}

	_screen->update();
}

}
