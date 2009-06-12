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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef ASYLUM_VIDEO_H_
#define ASYLUM_VIDEO_H_

#include "sound/mixer.h"
#include "graphics/surface.h"
#include "graphics/video/smk_decoder.h"

#include "common/events.h"
#include "common/system.h"
#include "common/list.h"

#include "asylum/asylum.h"

namespace Asylum {

class Video {
public:
	Video(AsylumEngine *vm);
	virtual ~Video();

	bool playVideo(const char *filename);
	bool playVideo(int number);

private:
	Common::List<Common::Event> _stopEvents;
	Graphics::SmackerDecoder *_smkDecoder;
	Graphics::VideoPlayer *_player;
	AsylumEngine *_vm;
}; // end of class Video

} // end of namespace Asylum

#endif
