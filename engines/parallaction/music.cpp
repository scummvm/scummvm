/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/file.h"
#include "parallaction/parallaction.h"

namespace Parallaction {

// NOTE: these two guys are never changed.
static int16 _musicFlag2 = 1;
static int16 _musicFlag1 = 1;

static bool _allocated = false;
static bool _playing = false;
// UNUSED
// static char byte_14D22[10] = { 0 };
static const char *_musicFilesNames[] = { "intro", "dino", "donna", "nuts", "soft", "boogie2" };
static uint16 _musicFilesSizes[] = { 18805, 5486, 6195, 13006, 15818, 7507 };
static byte *_musicBits = NULL;

// TODO
// move this into a proper midi driver and decode the numeric commands
void _music_command(int32, int32, int32, int32) {

}

void stopMusic() {

	if (_musicFlag1 == 0 && _musicFlag2 == 0) return;
	if (_playing == false) return;
	if (_allocated == false) return;

	_music_command(4, 0, 0, 0);     // stop
	_music_command(5, 0, 0, 0);     // reset timer

	memFree(_musicBits);

	_allocated = false;
	_playing = false;

	return;
}

void playMusic() {

	if (_musicFlag1 == 0 && _musicFlag2 == 0) return;
	if (_playing == true) return;
	if (_allocated == false) return;

	_music_command(0, 0, 0, 0);     // init driver
	_music_command(1, 0, 0, 0);     // init timer
	_music_command(17, 1, 0, 0);    // set source segment
	_music_command(7, 1, 0, 0);     // set source offset and do SOMETHING

	// FIXME: casting pointer to uint32
	_music_command(2, (uint32)_musicBits, 0, 0);    // play

	_playing = true;

	return;
}

void loadMusic(const char *filename) {

	uint16 _di = 0;

	if (!scumm_strnicmp(_location, "museo", 5)) return;
	if (!scumm_strnicmp(_location, "intgrottadopo", 13)) return;
	if (!scumm_strnicmp(_location, "caveau", 6)) return;
	if (!scumm_strnicmp(_location, "estgrotta", 9)) return;
	if (!scumm_strnicmp(_location, "plaza1", 6)) return;
	if (!scumm_strnicmp(_location, "endtgz", 6)) return;

	if (_musicFlag1 == 0 && _musicFlag2 == 0) return;
	if (_allocated == true) return;

//  UNUSED
//	strcpy(byte_14D22, filename);

	for (uint16 _si = 0; _si < 6; _si++) {
		if (!strcmp(filename, _musicFilesNames[_si])) {
			_di = _musicFilesSizes[_si];
		}
	}

	if (_di == 0 ) return;

	char path[PATH_LEN];
	sprintf(path, "%s.mid", filename);

	Common::File stream;

	if (!stream.open(path))
		return;

	_musicBits = (byte*)memAlloc(_di);
	if (!_musicBits) return;

	stream.read(_musicBits, _di);
	stream.close();

	_allocated = true;

	return;
}

} // namespace Parallaction
