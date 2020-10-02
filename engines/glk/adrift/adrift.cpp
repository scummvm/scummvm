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

#include "glk/adrift/adrift.h"
#include "glk/adrift/os_glk.h"
#include "glk/adrift/scprotos.h"
#include "glk/adrift/serialization.h"

namespace Glk {
namespace Adrift {

Adrift *g_vm = nullptr;

Adrift::Adrift(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc) {
	g_vm = this;
}

void Adrift::runGame() {
	if (adrift_startup_code(&_gameFile))
		adrift_main();
}

Common::Error Adrift::readSaveData(Common::SeekableReadStream *rs) {
	LoadSerializer ser((sc_gameref_t)gsc_game, if_read_saved_game, rs);
	return ser.load() ? Common::kNoError : Common::kReadingFailed;
}

Common::Error Adrift::writeGameData(Common::WriteStream *ws) {
	SaveSerializer ser((sc_gameref_t)gsc_game, if_write_saved_game, ws);
	ser.save();
	return Common::kNoError;
}

sc_int Adrift::if_read_saved_game(void *opaque, sc_byte *buffer, sc_int length) {
	Common::SeekableReadStream *rs = (Common::SeekableReadStream *)opaque;
	return rs->read(buffer, length);
}

void Adrift::if_write_saved_game(void *opaque, const sc_byte *buffer, sc_int length) {
	Common::WriteStream *ws = (Common::WriteStream *)opaque;
	ws->write(buffer, length);
}

} // End of namespace Adrift
} // End of namespace Glk
