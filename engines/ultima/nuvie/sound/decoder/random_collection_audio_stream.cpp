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

#include "ultima/nuvie/sound/decoder/random_collection_audio_stream.h"
#include "ultima/nuvie/core/game.h"
#include "audio/audiostream.h"
#include "common/mutex.h"
#include "audio/mixer.h"

namespace Ultima {
namespace Nuvie {
namespace U6Audio {

#pragma mark -
#pragma mark --- random collection audio stream ---
#pragma mark -

class RandomCollectionAudioStreamImpl : public RandomCollectionAudioStream {
private:
	/**
	 * The sampling rate of this audio stream.
	 */
	const int _rate;

	/**
	 * Whether this audio stream is mono (=false) or stereo (=true).
	 */
	const int _stereo;

	/**
	 * This flag is set by the finish() method only. See there for more details.
	 */
	bool _finished;

	/**
	 * An array of audio streams.
	 */
	Std::vector<Audio::RewindableAudioStream *> _streams;

	DisposeAfterUse::Flag _disposeAfterUse;

	Audio::RewindableAudioStream *_currentStream;
public:
	RandomCollectionAudioStreamImpl(int rate, bool stereo, Std::vector<Audio::RewindableAudioStream *> streams, DisposeAfterUse::Flag disposeAfterUse)
		: _rate(rate), _stereo(stereo), _finished(false), _streams(streams), _disposeAfterUse(disposeAfterUse) {
		if (_streams.size() > 0)
			_currentStream = _streams[NUVIE_RAND() % _streams.size()];
		else
			_currentStream = NULL;
	}

	~RandomCollectionAudioStreamImpl() override;

	// Implement the AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override {
		return _stereo;
	}
	int getRate() const override {
		return _rate;
	}
	bool endOfData() const override {
		return false;
	}
	bool endOfStream() const override {
		return _finished;
	}

	void finish() override {
		_finished = true;
	}
};

RandomCollectionAudioStreamImpl::~RandomCollectionAudioStreamImpl() {
	if (_disposeAfterUse == DisposeAfterUse::YES) {
		while (!_streams.empty()) {
			delete _streams.back();
			_streams.pop_back();
		}
	}
}

int RandomCollectionAudioStreamImpl::readBuffer(int16 *buffer, const int numSamples) {
	int samplesDecoded = 0;

	if (_currentStream) {
		while (samplesDecoded < numSamples) {
			samplesDecoded += _currentStream->readBuffer(buffer + samplesDecoded, numSamples - samplesDecoded);

			if (_currentStream->endOfData()) {
				_currentStream->rewind();

				//pseudo random we don't want to play the same stream twice in a row.
				int32 idx = NUVIE_RAND() % _streams.size();
				Audio::RewindableAudioStream *tmp = _streams[idx];
				if (_currentStream == tmp) {
					idx = (idx + (NUVIE_RAND() % 1 == 1 ? 1 : _streams.size() - 1)) % _streams.size();
					_currentStream = _streams[idx];
				} else
					_currentStream = tmp;

				//DEBUG(0, LEVEL_INFORMATIONAL, "new sample_num = %d\n", idx);
			}
		}
	}
	return samplesDecoded;
}

RandomCollectionAudioStream *makeRandomCollectionAudioStream(int rate, bool stereo,
		Std::vector<Audio::RewindableAudioStream *> streams, DisposeAfterUse::Flag disposeAfterUse) {
	return new RandomCollectionAudioStreamImpl(rate, stereo, streams, disposeAfterUse);
}

} // End of namespace U6Audio
} // End of namespace Nuvie
} // End of namespace Ultima
