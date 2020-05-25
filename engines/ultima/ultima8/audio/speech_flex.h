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

#ifndef ULTIMA8_AUDIO_SPEECHFLEX_H
#define ULTIMA8_AUDIO_SPEECHFLEX_H

#include "ultima/ultima8/audio/sound_flex.h"
#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class SpeechFlex : public SoundFlex {
	Std::vector<istring> _phrases;

public:
	SpeechFlex(Common::SeekableReadStream *rs);
	~SpeechFlex(void) override;

	int getIndexForPhrase(const Std::string &phrase,
	                      uint32 start, uint32 &end) const;

	//! get total length (in milliseconds) of speech for a phrase
	uint32 getSpeechLength(const Std::string &phrase);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
