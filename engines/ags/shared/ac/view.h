/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AGS_SHARED_AC_VIEW_H
#define AGS_SHARED_AC_VIEW_H

#include "common/std/vector.h"
#include "ags/shared/core/types.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

#define VFLG_FLIPSPRITE 1

struct ViewFrame {
	int   pic;
	short xoffs, yoffs;
	short speed;
	int   flags;  // VFLG_* flags
	int   sound;  // play sound when this frame comes round
	int   reserved_for_future[2]; // kept only for plugin api
	// not saved, set at runtime only
	int audioclip; // actual audio clip reference (in case sound is a legacy number)

	ViewFrame();

	void ReadFromFile(Shared::Stream *in);
	void WriteToFile(Shared::Stream *out);
};

#define LOOPFLAG_RUNNEXTLOOP 1

struct ViewLoopNew {
	int numFrames;
	int   flags;
	std::vector<ViewFrame> frames;
	// NOTE: we still need numFrames:
	// as we always allocate at least 1 frame for safety, to avoid crashes,
	// but have to report "logical" number of frames for the engine API.

	ViewLoopNew();
	void Initialize(int frameCount);
	void Dispose();
	bool RunNextLoop();
	void WriteToFile_v321(Shared::Stream *out);
	void ReadFromFile_v321(Shared::Stream *in);
	void WriteFrames(Shared::Stream *out);
	void ReadFrames(Shared::Stream *in);
};

struct ViewStruct {
	int numLoops;
	std::vector<ViewLoopNew> loops;

	ViewStruct();
	void Initialize(int loopCount);
	void Dispose();
	void WriteToFile(Shared::Stream *out);
	void ReadFromFile(Shared::Stream *in);
};

struct ViewStruct272 {
	short     numloops;
	short     numframes[16];
	int32_t   loopflags[16];
	ViewFrame frames[16][20];

	ViewStruct272();
	void ReadFromFile(Shared::Stream *in);
};

extern void Convert272ViewsToNew(const std::vector<ViewStruct272> &oldv, std::vector<ViewStruct> &newv);

} // namespace AGS3

#endif
