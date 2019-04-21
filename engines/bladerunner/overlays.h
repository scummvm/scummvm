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
class SaveFileReadStream;
class SaveFileWriteStream;
class VQAPlayer;

class Overlays {
	friend class Debugger;

	static const int kOverlayVideos = 5;

	struct Video {
		bool            loaded;
		VQAPlayer      *vqaPlayer;
		Common::String  name;
		int32           hash;
		int             loopId;
		int             enqueuedLoopId;
		bool            loopForever;
		int             frame;
	};

	BladeRunnerEngine *_vm;
	Common::Array<Video> _videos;

public:
	Overlays(BladeRunnerEngine *vm);
	bool init();
	~Overlays();

	int play(const Common::String &name, int loopId, bool loopForever, bool startNow, int a6);
	void resume(bool isLoadingGame);
	void remove(const Common::String &name);
	void removeAll();
	void tick();

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);

private:
	int findByHash(int32 hash) const;
	int findEmpty() const;

	void resetSingle(int i);
	void reset();
};

} // End of namespace BladeRunner

#endif
