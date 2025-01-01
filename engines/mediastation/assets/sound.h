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

#ifndef MEDIASTATION_SOUND_H
#define MEDIASTATION_SOUND_H

#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "mediastation/asset.h"
#include "mediastation/chunk.h"
#include "mediastation/subfile.h"
#include "mediastation/assetheader.h"
#include "mediastation/mediascript/operand.h"

namespace MediaStation {

class Sound : public Asset {
public:
	// For standalone Sound assets.
	Sound(AssetHeader *header);

	// For sounds that are part of a movie.
	// TODO: Since these aren't Assets they should be handled elsewhere.
	//Sound(AssetHeader::SoundEncoding encoding);
	~Sound();

	virtual Operand callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) override;
	virtual void process() override;

	virtual void readChunk(Chunk& chunk) override;
	virtual void readSubfile(Subfile &subFile, Chunk &chunk) override;

	// All Media Station audio is signed 16-bit little-endian mono at 22050 Hz.
	// Some defaults must be overridden in the flags.
	static const uint RATE = 22050;
	static const byte FLAGS = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;

private:
	AssetHeader::SoundEncoding _encoding;
	byte *_samples = nullptr;
};

} // End of namespace MediaStation

#endif