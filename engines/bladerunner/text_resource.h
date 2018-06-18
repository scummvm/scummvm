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

#ifndef BLADERUNNER_TEXT_RESOURCE_H
#define BLADERUNNER_TEXT_RESOURCE_H

#include "bladerunner/bladerunner.h" // needed for definition of Common::ScopedPtr (subtitles font external font file support) -- and for the subtitles relevant macro defines
#if SUBTITLES_SUPPORT
#include "common/util.h"
#endif
#include "common/str.h"

namespace BladeRunner {

class BladeRunnerEngine;
class TextResource;

class TextResource {
	BladeRunnerEngine *_vm;

	uint32  _count;
	uint32 *_ids;
	uint32 *_offsets;
	char   *_strings;

public:
	TextResource(BladeRunnerEngine *vm);
	~TextResource();

	bool open(const Common::String &name);
	#if SUBTITLES_SUPPORT
	bool openFromStream(Common::ScopedPtr<Common::SeekableReadStream> &s);
    #endif
	
	const char *getText(uint32 id) const;
	#if SUBTITLES_SUPPORT
	const char *getOuttakeTextByFrame(uint32 frame) const;
    #endif
	int getCount() const;
};

} // End of namespace BladeRunner

#endif
