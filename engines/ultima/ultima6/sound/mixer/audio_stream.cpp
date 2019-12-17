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

//#include "common/debug.h"
//#include "common/endian.h"
//#include "common/file.h"
//#include "common/queue.h"
//#include "common/util.h"
#include <queue>
#include <assert.h>
#include "audiostream.h"
#include "mutex.h"
//#include "sound/decoders/flac.h"
#include "mixer.h"
//#include "sound/decoders/mp3.h"
//#include "sound/decoders/raw.h"
//#include "sound/decoders/vorbis.h"


namespace Audio {
/*
struct StreamFileFormat {
    // Decodername
    const char *decoderName;
    const char *fileExtension;
    //
    // Pointer to a function which tries to open a file of type StreamFormat.
    // Return NULL in case of an error (invalid/nonexisting file).
    //
    SeekableAudioStream *(*openStreamFile)(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse);
};

static const StreamFileFormat STREAM_FILEFORMATS[] = {
    // decoderName,  fileExt, openStreamFuntion
#ifdef USE_FLAC
    { "FLAC",         ".flac", makeFLACStream },
    { "FLAC",         ".fla",  makeFLACStream },
#endif
#ifdef USE_VORBIS
    { "Ogg Vorbis",   ".ogg",  makeVorbisStream },
#endif
#ifdef USE_MAD
    { "MPEG Layer 3", ".mp3",  makeMP3Stream },
#endif

    { NULL, NULL, NULL } // Terminator
};

SeekableAudioStream *SeekableAudioStream::openStreamFile(const Common::String &basename) {
    SeekableAudioStream *stream = NULL;
    Common::File *fileHandle = new Common::File();

    for (int i = 0; i < ARRAYSIZE(STREAM_FILEFORMATS)-1 && stream == NULL; ++i) {
        Common::String filename = basename + STREAM_FILEFORMATS[i].fileExtension;
        fileHandle->open(filename);
        if (fileHandle->isOpen()) {
            // Create the stream object
            stream = STREAM_FILEFORMATS[i].openStreamFile(fileHandle, DisposeAfterUse::YES);
            fileHandle = 0;
            break;
        }
    }

    delete fileHandle;

    if (stream == NULL)
        debug(1, "SeekableAudioStream::openStreamFile: Could not open compressed AudioFile %s", basename.c_str());

    return stream;
}
*/

#pragma mark -
#pragma mark --- LoopingAudioStream ---
#pragma mark -

LoopingAudioStream::LoopingAudioStream(RewindableAudioStream *stream, uint32 loops, DisposeAfterUse::Flag disposeAfterUse)
	: _parent(stream), _disposeAfterUse(disposeAfterUse), _loops(loops), _completeIterations(0) {
	assert(stream);

	if (!stream->rewind()) {
		// TODO: Properly indicate error
		_loops = _completeIterations = 1;
	}
}

LoopingAudioStream::~LoopingAudioStream() {
	if (_disposeAfterUse == DisposeAfterUse::YES)
		delete _parent;
}

int LoopingAudioStream::readBuffer(sint16 *buffer, const int numSamples) {
	if ((_loops && _completeIterations == _loops) || !numSamples)
		return 0;

	int samplesRead = _parent->readBuffer(buffer, numSamples);

	if (_parent->endOfStream()) {
		++_completeIterations;
		if (_completeIterations == _loops)
			return samplesRead;

		const int remainingSamples = numSamples - samplesRead;

		if (!_parent->rewind()) {
			// TODO: Properly indicate error
			_loops = _completeIterations = 1;
			return samplesRead;
		}

		return samplesRead + readBuffer(buffer + samplesRead, remainingSamples);
	}

	return samplesRead;
}

bool LoopingAudioStream::endOfData() const {
	return (_loops != 0 && (_completeIterations == _loops));
}

AudioStream *makeLoopingAudioStream(RewindableAudioStream *stream, uint32 loops) {
	if (loops != 1)
		return new LoopingAudioStream(stream, loops);
	else
		return stream;
}

#pragma mark -
#pragma mark --- Queueing audio stream ---
#pragma mark -

class QueuingAudioStreamImpl : public QueuingAudioStream {
private:
	/**
	 * We queue a number of (pointers to) audio stream objects.
	 * In addition, we need to remember for each stream whether
	 * to dispose it after all data has been read from it.
	 * Hence, we don't store pointers to stream objects directly,
	 * but rather StreamHolder structs.
	 */
	struct StreamHolder {
		AudioStream *_stream;
		DisposeAfterUse::Flag _disposeAfterUse;
		StreamHolder(AudioStream *stream, DisposeAfterUse::Flag disposeAfterUse)
			: _stream(stream),
			  _disposeAfterUse(disposeAfterUse) {}
	};

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
	 * A mutex to avoid access problems (causing e.g. corruption of
	 * the linked list) in thread aware environments.
	 */
	Common::Mutex _mutex;

	/**
	 * The queue of audio streams.
	 */
	std::queue<StreamHolder> _queue;

	uint32 totalDuration;

public:
	QueuingAudioStreamImpl(int rate, bool stereo)
		: _rate(rate), _stereo(stereo), _finished(false), totalDuration(0) {}
	~QueuingAudioStreamImpl();

	// Implement the AudioStream API
	virtual int readBuffer(sint16 *buffer, const int numSamples);
	virtual bool isStereo() const {
		return _stereo;
	}
	virtual int getRate() const {
		return _rate;
	}
	virtual bool endOfData() const {
		//Common::StackLock lock(_mutex);
		return _queue.empty();
	}
	virtual bool endOfStream() const {
		return _finished && _queue.empty();
	}

	// Implement the QueuingAudioStream API
	virtual void queueAudioStream(AudioStream *stream, DisposeAfterUse::Flag disposeAfterUse);
	virtual void finish() {
		_finished = true;
	}

	uint32 numQueuedStreams() const {
		//Common::StackLock lock(_mutex);
		return _queue.size();
	}

	uint32 getLengthInMsec();
};

QueuingAudioStreamImpl::~QueuingAudioStreamImpl() {
	while (!_queue.empty()) {
		StreamHolder tmp = _queue.front();
		if (tmp._disposeAfterUse == DisposeAfterUse::YES)
			delete tmp._stream;
		_queue.pop();
	}
}

void QueuingAudioStreamImpl::queueAudioStream(AudioStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	assert(!_finished);
	if ((stream->getRate() != getRate()) || (stream->isStereo() != isStereo()))
		DEBUG(0, LEVEL_WARNING, "QueuingAudioStreamImpl::queueAudioStream: stream has mismatched parameters");

	Common::StackLock lock(_mutex);
	_queue.push(StreamHolder(stream, disposeAfterUse));

	totalDuration += stream->getLengthInMsec();
}

int QueuingAudioStreamImpl::readBuffer(sint16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);
	int samplesDecoded = 0;

	while (samplesDecoded < numSamples && !_queue.empty()) {
		AudioStream *stream = _queue.front()._stream;
		samplesDecoded += stream->readBuffer(buffer + samplesDecoded, numSamples - samplesDecoded);

		if (stream->endOfData()) {
			StreamHolder tmp = _queue.front();
			if (tmp._disposeAfterUse == DisposeAfterUse::YES)
				delete stream;
			_queue.pop();
			if (_queue.empty())
				finish();
		}
	}

	return samplesDecoded;
}

uint32 QueuingAudioStreamImpl::getLengthInMsec() {
	return totalDuration;
}

QueuingAudioStream *makeQueuingAudioStream(int rate, bool stereo) {
	return new QueuingAudioStreamImpl(rate, stereo);
}


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
	std::vector<RewindableAudioStream *> _streams;

	DisposeAfterUse::Flag _disposeAfterUse;

	RewindableAudioStream *_currentStream;
public:
	RandomCollectionAudioStreamImpl(int rate, bool stereo, std::vector<RewindableAudioStream *> streams, DisposeAfterUse::Flag disposeAfterUse)
		: _rate(rate), _stereo(stereo), _finished(false), _streams(streams), _disposeAfterUse(disposeAfterUse) {
		if (_streams.size() > 0)
			_currentStream = _streams[NUVIE_RAND() % _streams.size()];
		else
			_currentStream = NULL;
	}

	~RandomCollectionAudioStreamImpl();

	// Implement the AudioStream API
	virtual int readBuffer(sint16 *buffer, const int numSamples);
	virtual bool isStereo() const {
		return _stereo;
	}
	virtual int getRate() const {
		return _rate;
	}
	virtual bool endOfData() const {
		return false;
	}
	virtual bool endOfStream() const {
		return _finished;
	}

	virtual void finish() {
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

int RandomCollectionAudioStreamImpl::readBuffer(sint16 *buffer, const int numSamples) {
	int samplesDecoded = 0;

	if (_currentStream) {
		while (samplesDecoded < numSamples) {
			samplesDecoded += _currentStream->readBuffer(buffer + samplesDecoded, numSamples - samplesDecoded);

			if (_currentStream->endOfData()) {
				_currentStream->rewind();

				//pseudo random we don't want to play the same stream twice in a row.
				sint32 idx = NUVIE_RAND() % _streams.size();
				RewindableAudioStream *tmp = _streams[idx];
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

RandomCollectionAudioStream *makeRandomCollectionAudioStream(int rate, bool stereo, std::vector<RewindableAudioStream *> streams, DisposeAfterUse::Flag disposeAfterUse) {
	return new RandomCollectionAudioStreamImpl(rate, stereo, streams, disposeAfterUse);
}

Timestamp convertTimeToStreamPos(const Timestamp &where, int rate, bool isStereo) {
	Timestamp result(where.convertToFramerate(rate * (isStereo ? 2 : 1)));

	// When the Stream is a stereo stream, we have to assure
	// that the sample position is an even number.
	if (isStereo && (result.totalNumberOfFrames() & 1))
		result = result.addFrames(-1); // We cut off one sample here.

	// Since Timestamp allows sub-frame-precision it might lead to odd behaviors
	// when we would just return result.
	//
	// An example is when converting the timestamp 500ms to a 11025 Hz based
	// stream. It would have an internal frame counter of 5512.5. Now when
	// doing calculations at frame precision, this might lead to unexpected
	// results: The frame difference between a timestamp 1000ms and the above
	// mentioned timestamp (both with 11025 as framerate) would be 5512,
	// instead of 5513, which is what a frame-precision based code would expect.
	//
	// By creating a new Timestamp with the given parameters, we create a
	// Timestamp with frame-precision, which just drops a sub-frame-precision
	// information (i.e. rounds down).
	return Timestamp(result.secs(), result.numberOfFrames(), result.framerate());
}
} // End of namespace Audio
