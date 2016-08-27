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

#include "common/system.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/file.h"
#include "common/stream.h"

#include "adl/hires4.h"
#include "adl/detection.h"
#include "adl/display.h"
#include "adl/graphics.h"
#include "adl/disk.h"

namespace Adl {

HiRes4Engine::~HiRes4Engine() {
	delete _disk2;
	delete _disk3;
}

void HiRes4Engine::init() {
	_graphics = new Graphics_v2(*_display);

	const char *const *names = getDiskImageNames();

	_disk = new DiskImage();
	if (!_disk->open(names[0]))
		error("Failed to open disk image '%s'", names[0]);

	_disk2 = new DiskImage();
	if (!_disk2->open(names[1]))
		error("Failed to open disk image '%s'", names[1]);

	_disk3 = new DiskImage();
	if (!_disk3->open(names[2]))
		error("Failed to open disk image '%s'", names[2]);

	StreamPtr stream(createReadStream(_disk, 0x06, 0xd, 0x12, 2));
	loadItemDescriptions(*stream, IDI_HR4_NUM_ITEM_DESCS);

	stream.reset(createReadStream(_disk, 0x05, 0x4, 0x00, 3));
	loadWords(*stream, _verbs, _priVerbs);

	stream.reset(createReadStream(_disk, 0x03, 0xb, 0x00, 6));
	loadWords(*stream, _nouns, _priNouns);
}

void HiRes4Engine::initGameState() {
	StreamPtr stream(createReadStream(_disk, 0x02, 0xc, 0x00, 12));
	loadItems(*stream);
}

Common::SeekableReadStream *HiRes4Engine::createReadStream(DiskImage *disk, byte track, byte sector, byte offset, byte size) const {
	adjustDataBlockPtr(track, sector, offset, size);
	return disk->createReadStream(track, sector, offset, size);
}

void HiRes4Engine_Atari::adjustDataBlockPtr(byte &track, byte &sector, byte &offset, byte &size) const {
	// Convert the Apple II disk offsets in the game, to Atari disk offsets
	uint sectorIndex = (track * 16 + sector + 1) << 1;

	// Atari uses 128 bytes per sector vs. 256 on the Apple II
	// Note that size indicates *additional* sectors to read after reading one sector
	size *= 2;

	if (offset >= 128) {
		// Offset in the second half of an Apple II sector, skip one sector and adjust offset
		++sectorIndex;
		offset -= 128;
	} else {
		// Offset in the first half of an Apple II sector, we need to read one additional sector
		++size;
	}

	// Compute track/sector for Atari's 18 sectors per track (sectorIndex is 1-based)
	track = (sectorIndex - 1) / 18;
	sector = (sectorIndex - 1) % 18;
}

const char *const *HiRes4Engine_Atari::getDiskImageNames() const {
	static const char *const disks[] = { "ULYS1A.XFD", "ULYS1B.XFD", "ULYS2C.XFD" };
	return disks;
}

Engine *HiRes4Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	switch (gd->desc.platform) {
	case Common::kPlatformAtariST:
		return new HiRes4Engine_Atari(syst, gd);
	default:
		error("Unsupported platform");
	}
}

} // End of namespace Adl
