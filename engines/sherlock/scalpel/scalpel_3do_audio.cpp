/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names are
 * too numerous to list here. Please refer to the COPYRIGHT
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

#include "sherlock/scalpel/scalpel_3do_audio.h"
#include "common/file.h"
#include "common/tokenizer.h"
#include <cstdlib>

namespace Sherlock {
namespace Scalpel {

const Common::HashMap<Common::String, uint32> &get3doAudioDurations() {
	static Common::HashMap<Common::String, uint32> map;
	static bool loaded = false;

	if (loaded)
		return map;

	loaded = true;
	Common::File f;
	if (!f.open(Common::Path("sherlock_scalpel_3do_audio.dat"))) {
		return map;
	}

	while (!f.eos() && !f.err()) {
		Common::String line = f.readLine();
		line.trim();
		if (line.empty() || line.hasPrefix("#"))
			continue;

		Common::StringTokenizer tok(line, " \t");
		Common::Array<Common::String> parts = tok.split();
		if (parts.size() < 2)
			continue;

		uint32 duration = (uint32)strtoul(parts[1].c_str(), nullptr, 10);
		if (duration > 0)
			map.setVal(parts[0], duration);
	}

	return map;
}

} // namespace Scalpel
} // namespace Sherlock
