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

//include <string.h>
#include "ags/shared/ac/view.h"
#include "ags/shared/util/alignedstream.h"

namespace AGS3 {

using AGS::Shared::AlignedStream;
using AGS::Shared::Stream;

ViewFrame::ViewFrame()
	: pic(0)
	, xoffs(0)
	, yoffs(0)
	, speed(0)
	, flags(0)
	, sound(0) {
	reserved_for_future[0] = 0;
	reserved_for_future[1] = 0;
}

void ViewFrame::ReadFromFile(Stream *in) {
	pic = in->ReadInt32();
	xoffs = in->ReadInt16();
	yoffs = in->ReadInt16();
	speed = in->ReadInt16();
	flags = in->ReadInt32();
	sound = in->ReadInt32();
	reserved_for_future[0] = in->ReadInt32();
	reserved_for_future[1] = in->ReadInt32();
}

void ViewFrame::WriteToFile(Stream *out) {
	out->WriteInt32(pic);
	out->WriteInt16(xoffs);
	out->WriteInt16(yoffs);
	out->WriteInt16(speed);
	out->WriteInt32(flags);
	out->WriteInt32(sound);
	out->WriteInt32(reserved_for_future[0]);
	out->WriteInt32(reserved_for_future[1]);
}

ViewLoopNew::ViewLoopNew()
	: numFrames(0)
	, flags(0)
	, frames(nullptr) {
}

bool ViewLoopNew::RunNextLoop() {
	return (flags & LOOPFLAG_RUNNEXTLOOP);
}

void ViewLoopNew::Initialize(int frameCount) {
	numFrames = frameCount;
	flags = 0;
	frames = (ViewFrame *)calloc(numFrames + 1, sizeof(ViewFrame));
}

void ViewLoopNew::Dispose() {
	if (frames != nullptr) {
		free(frames);
		frames = nullptr;
		numFrames = 0;
	}
}

void ViewLoopNew::WriteToFile_v321(Stream *out) {
	out->WriteInt16(numFrames);
	out->WriteInt32(flags);
	WriteFrames_Aligned(out);
}

void ViewLoopNew::WriteFrames_Aligned(Stream *out) {
	AlignedStream align_s(out, Shared::kAligned_Write);
	for (int i = 0; i < numFrames; ++i) {
		frames[i].WriteToFile(&align_s);
		align_s.Reset();
	}
}

void ViewLoopNew::ReadFromFile_v321(Stream *in) {
	Initialize(in->ReadInt16());
	flags = in->ReadInt32();
	ReadFrames_Aligned(in);

	// an extra frame is allocated in memory to prevent
	// crashes with empty loops -- set its picture to teh BLUE CUP!!
	frames[numFrames].pic = 0;
}

void ViewLoopNew::ReadFrames_Aligned(Stream *in) {
	AlignedStream align_s(in, Shared::kAligned_Read);
	for (int i = 0; i < numFrames; ++i) {
		frames[i].ReadFromFile(&align_s);
		align_s.Reset();
	}
}

ViewStruct::ViewStruct()
	: numLoops(0)
	, loops(nullptr) {
}

void ViewStruct::Initialize(int loopCount) {
	numLoops = loopCount;
	if (numLoops > 0) {
		loops = (ViewLoopNew *)calloc(numLoops, sizeof(ViewLoopNew));
	}
}

void ViewStruct::Dispose() {
	if (numLoops > 0) {
		free(loops);
		numLoops = 0;
	}
}

void ViewStruct::WriteToFile(Stream *out) {
	out->WriteInt16(numLoops);
	for (int i = 0; i < numLoops; i++) {
		loops[i].WriteToFile_v321(out);
	}
}

void ViewStruct::ReadFromFile(Stream *in) {
	Initialize(in->ReadInt16());

	for (int i = 0; i < numLoops; i++) {
		loops[i].ReadFromFile_v321(in);
	}
}

ViewStruct272::ViewStruct272()
	: numloops(0) {
	memset(numframes, 0, sizeof(numframes));
	memset(loopflags, 0, sizeof(loopflags));
}

void ViewStruct272::ReadFromFile(Stream *in) {
	numloops = in->ReadInt16();
	for (int i = 0; i < 16; ++i) {
		numframes[i] = in->ReadInt16();
	}
	in->ReadArrayOfInt32(loopflags, 16);
	for (int j = 0; j < 16; ++j) {
		for (int i = 0; i < 20; ++i) {
			frames[j][i].ReadFromFile(in);
		}
	}
}

void Convert272ViewsToNew(const std::vector<ViewStruct272> &oldv, ViewStruct *newv) {
	for (size_t a = 0; a < oldv.size(); a++) {
		newv[a].Initialize(oldv[a].numloops);

		for (int b = 0; b < oldv[a].numloops; b++) {
			newv[a].loops[b].Initialize(oldv[a].numframes[b]);

			if ((oldv[a].numframes[b] > 0) &&
				(oldv[a].frames[b][oldv[a].numframes[b] - 1].pic == -1)) {
				newv[a].loops[b].flags = LOOPFLAG_RUNNEXTLOOP;
				newv[a].loops[b].numFrames--;
			} else
				newv[a].loops[b].flags = 0;

			for (int c = 0; c < newv[a].loops[b].numFrames; c++)
				newv[a].loops[b].frames[c] = oldv[a].frames[b][c];
		}
	}
}

} // namespace AGS3
