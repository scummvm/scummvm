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

#include "glk/tads/tads.h"
#include "glk/tads/os_glk.h"
#include "common/config-manager.h"

namespace Glk {
namespace TADS {

TADS *g_vm;

TADS::TADS(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc) {
	g_vm = this;
}

bool TADS::hasFeature(EngineFeature f) const {
	// The TADS codebases rely on a lot of static globals,
	// so return to launcher can't be supported.
	if (f == kSupportsReturnToLauncher)
		return false;

	return GlkAPI::hasFeature(f);
}

Common::Error TADS::readSaveData(Common::SeekableReadStream *rs) {
	// TODO
	return Common::kReadingFailed;
}

Common::Error TADS::writeGameData(Common::WriteStream *ws) {
	// TODO
	return Common::kWritingFailed;
}

} // End of namespace TADS
} // End of namespace Glk
