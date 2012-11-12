/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/str.h"

#include "engines/grim/emi/poolsound.h"
#include "engines/grim/resource.h"

namespace Grim {

	PoolSound::PoolSound() {
		track = NULL;
	}

	PoolSound::PoolSound(const Common::String &filename) {
		openFile(filename);
	}

	// Called when the engine restarts or Lua code calls FreeSound
	PoolSound::~PoolSound() {
		delete track;
	}

	void PoolSound::openFile(const Common::String &filename) {
		track = new AIFFTrack(Audio::Mixer::kSFXSoundType, DisposeAfterUse::NO);
		Common::SeekableReadStream *stream = g_resourceloader->openNewStreamFile(filename);
		if (!stream)
			return;
		track->openSound(filename, stream);
	}

	void PoolSound::saveState(SaveGame *state) {
		if (track && track->isStreamOpen()) {
			state->writeBool(true);
			state->writeString(track->getSoundName());
		} else {
			state->writeBool(false);
		}
	}

	void PoolSound::restoreState(SaveGame *state) {
		bool hasStream = state->readBool();
		if (hasStream)
			openFile(state->readString());
	}
}
