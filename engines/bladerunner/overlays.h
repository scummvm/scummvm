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

#ifndef BLADERUNNER_OVERLAYS_H
#define BLADERUNNER_OVERLAYS_H

#include "common/array.h"
#include "common/str.h"

namespace Graphics {
struct Surface;
}

namespace BladeRunner {

class BladeRunnerEngine;
class VQAPlayer;


class Overlays {
	static const int kOverlayVideos = 5;

	struct Video {
		bool       loaded;
		VQAPlayer *vqaPlayer;
		// char       name[13];
		int32      id;
		int        field0;
		int        field1;
		int        field2;
	};

	BladeRunnerEngine *_vm;
	Common::Array<Video> _videos;

public:
	Overlays(BladeRunnerEngine *vm);
	bool init();
	~Overlays();

	int play(const Common::String &name, int a3, int a4, int a5, int a6);
	void remove(const Common::String &name);
	void removeAll();
	void tick();

private:
	int findById(int32 id) const;
	int findEmpty() const;

	void resetSingle(int i);
	void reset();
};

} // End of namespace BladeRunner

#endif
