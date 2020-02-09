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

#ifndef CHEWY_VIDEO_CFO_DECODER_H
#define CHEWY_VIDEO_CFO_DECODER_H

#include "graphics/surface.h"
#include "video/flic_decoder.h"

namespace Chewy {

#define MAX_SOUND_EFFECTS 14

class Sound;

// A FLIC decoder, with a modified header and additional custom frames
class CfoDecoder : public Video::FlicDecoder {
public:
	CfoDecoder(Sound *sound) : Video::FlicDecoder(), _sound(sound) {}
	~CfoDecoder() override {}

	bool loadStream(Common::SeekableReadStream *stream) override;

private:
	Sound *_sound;

	class CfoVideoTrack : public Video::FlicDecoder::FlicVideoTrack {
	public:
		CfoVideoTrack(Common::SeekableReadStream *stream, uint16 frameCount, uint16 width, uint16 height, Sound *sound);
		~CfoVideoTrack() override;

		void readHeader() override;

		bool isRewindable() const override { return false; }
		bool rewind() override { return false; }

		const ::Graphics::Surface *decodeNextFrame() override;

	private:
		void handleFrame() override;
		void handleCustomFrame();
		void fadeOut();

		Sound *_sound;

		byte *_soundEffects[MAX_SOUND_EFFECTS];
		uint32 _soundEffectSize[MAX_SOUND_EFFECTS];
		byte *_musicData;
		uint32 _musicSize;
	};
};

} // End of namespace Chewy

#endif
