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
 *
 * $URL$
 * $Id$
 *
 */

#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/queue.h"
#include "common/util.h"

#include "sound/audiostream.h"
#include "sound/flac.h"
#include "sound/mixer.h"
#include "sound/mp3.h"
#include "sound/raw.h"
#include "sound/vorbis.h"


namespace Audio {

struct StreamFileFormat {
	/** Decodername */
	const char* decoderName;
	const char* fileExtension;
	/**
	 * Pointer to a function which tries to open a file of type StreamFormat.
	 * Return NULL in case of an error (invalid/nonexisting file).
	 */
	SeekableAudioStream *(*openStreamFile)(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse);
};

static const StreamFileFormat STREAM_FILEFORMATS[] = {
	/* decoderName,		fileExt, openStreamFuntion */
#ifdef USE_FLAC
	{ "Flac",			".flac", makeFlacStream },
	{ "Flac",			".fla",  makeFlacStream },
#endif
#ifdef USE_VORBIS
	{ "Ogg Vorbis",		".ogg",  makeVorbisStream },
#endif
#ifdef USE_MAD
	{ "MPEG Layer 3",	".mp3",  makeMP3Stream },
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

	if (stream == NULL) {
		debug(1, "SeekableAudioStream::openStreamFile: Could not open compressed AudioFile %s", basename.c_str());
	}

	return stream;
}

#pragma mark -
#pragma mark --- LoopingAudioStream ---
#pragma mark -

LoopingAudioStream::LoopingAudioStream(RewindableAudioStream *stream, uint loops, DisposeAfterUse::Flag disposeAfterUse)
    : _parent(stream), _disposeAfterUse(disposeAfterUse), _loops(loops), _completeIterations(0) {
}

LoopingAudioStream::~LoopingAudioStream() {
	if (_disposeAfterUse == DisposeAfterUse::YES)
		delete _parent;
}

int LoopingAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	int samplesRead = _parent->readBuffer(buffer, numSamples);

	if (_parent->endOfStream()) {
		++_completeIterations;
		if (_completeIterations == _loops)
			return samplesRead;

		int remainingSamples = numSamples - samplesRead;

		if (!_parent->rewind()) {
			// TODO: Properly indicate error
			_loops = _completeIterations = 1;
			return samplesRead;
		}

		samplesRead += _parent->readBuffer(buffer + samplesRead, remainingSamples);
	}

	return samplesRead;
}

bool LoopingAudioStream::endOfData() const {
	return (_loops != 0 && (_completeIterations == _loops));
}

AudioStream *makeLoopingAudioStream(RewindableAudioStream *stream, uint loops) {
	if (loops != 1)
		return new LoopingAudioStream(stream, loops);
	else
		return stream;
}

AudioStream *makeLoopingAudioStream(SeekableAudioStream *stream, Timestamp start, Timestamp end, uint loops) {
	if (!start.totalNumberOfFrames() && (!end.totalNumberOfFrames() || end == stream->getLength())) {
		return makeLoopingAudioStream(stream, loops);
	} else {
		if (!end.totalNumberOfFrames())
			end = stream->getLength();

		if (start >= end) {
			warning("makeLoopingAudioStream: start (%d) >= end (%d)", start.msecs(), end.msecs());
			delete stream;
			return 0;
		}

		return makeLoopingAudioStream(new SubSeekableAudioStream(stream, start, end), loops);
	}
}

#pragma mark -
#pragma mark --- SubLoopingAudioStream ---
#pragma mark -

SubLoopingAudioStream::SubLoopingAudioStream(SeekableAudioStream *stream,
	                                         uint loops,
	                                         const Timestamp loopStart,
	                                         const Timestamp loopEnd,
	                                         DisposeAfterUse::Flag disposeAfterUse)
    : _parent(stream), _disposeAfterUse(disposeAfterUse), _loops(loops),
      _pos(0, getRate() * (isStereo() ? 2 : 1)),
      _loopStart(loopStart.convertToFramerate(getRate() * (isStereo() ? 2 : 1))),
      _loopEnd(loopEnd.convertToFramerate(getRate() * (isStereo() ? 2 : 1))),
      _done(false) {
	if (!_parent->rewind())
		_done = true;
}

SubLoopingAudioStream::~SubLoopingAudioStream() {
	if (_disposeAfterUse == DisposeAfterUse::YES)
		delete _parent;
}

int SubLoopingAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	int framesLeft = MIN(_loopEnd.frameDiff(_pos), numSamples);
	int framesRead = _parent->readBuffer(buffer, framesLeft);
	_pos = _pos.addFrames(framesRead);

	if (framesLeft < numSamples || framesRead < framesLeft) {
		if (_loops != 0) {
			--_loops;
			if (!_loops) {
				_done = true;
				return framesRead;
			}
		}

		if (!_parent->seek(_loopStart)) {
			_done = true;
			return framesRead;
		}

		_pos = _loopStart;
		framesLeft = numSamples - framesLeft;
		framesRead += _parent->readBuffer(buffer + framesRead, framesLeft);

		if (_parent->endOfStream())
			_done = true;
	}

	return framesRead;
}

#pragma mark -
#pragma mark --- SubSeekableAudioStream ---
#pragma mark -

SubSeekableAudioStream::SubSeekableAudioStream(SeekableAudioStream *parent, const Timestamp start, const Timestamp end, DisposeAfterUse::Flag disposeAfterUse)
    : _parent(parent), _disposeAfterUse(disposeAfterUse),
      _start(start.convertToFramerate(getRate())),
       _pos(0, getRate() * (isStereo() ? 2 : 1)),
      _length((end - start).convertToFramerate(getRate() * (isStereo() ? 2 : 1))) {

	assert(_length.totalNumberOfFrames() % (isStereo() ? 2 : 1) == 0);
	_parent->seek(_start);
}

SubSeekableAudioStream::~SubSeekableAudioStream() {
	if (_disposeAfterUse)
		delete _parent;
}

int SubSeekableAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	int framesLeft = MIN(_length.frameDiff(_pos), numSamples);
	int framesRead = _parent->readBuffer(buffer, framesLeft);
	_pos = _pos.addFrames(framesRead);
	return framesRead;
}

bool SubSeekableAudioStream::seek(const Timestamp &where) {
	_pos = where.convertToFramerate(getRate());
	if (_pos > _length) {
		_pos = _length;
		return false;
	}

	if (_parent->seek(_pos + _start)) {
		return true;
	} else {
		_pos = _length;
		return false;
	}
}

#pragma mark -
#pragma mark --- Queueing audio stream ---
#pragma mark -


void QueuingAudioStream::queueBuffer(byte *data, uint32 size, DisposeAfterUse::Flag disposeAfterUse, byte flags) {
	AudioStream *stream = makeRawMemoryStream(data, size, getRate(), flags, disposeAfterUse);
	queueAudioStream(stream, DisposeAfterUse::YES);
}


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
	Common::Queue<StreamHolder> _queue;

public:
	QueuingAudioStreamImpl(int rate, bool stereo)
		: _rate(rate), _stereo(stereo), _finished(false) {}
	~QueuingAudioStreamImpl();

	// Implement the AudioStream API
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo() const { return _stereo; }
	virtual int getRate() const { return _rate; }
	virtual bool endOfData() const {
		//Common::StackLock lock(_mutex);
		return _queue.empty();
	}
	virtual bool endOfStream() const { return _finished && _queue.empty(); }

	// Implement the QueuingAudioStream API
	virtual void queueAudioStream(AudioStream *stream, DisposeAfterUse::Flag disposeAfterUse);
	virtual void finish() { _finished = true; }

	uint32 numQueuedStreams() const {
		//Common::StackLock lock(_mutex);
		return _queue.size();
	}
};

QueuingAudioStreamImpl::~QueuingAudioStreamImpl() {
	while (!_queue.empty()) {
		StreamHolder tmp = _queue.pop();
		if (tmp._disposeAfterUse == DisposeAfterUse::YES)
			delete tmp._stream;
	}
}

void QueuingAudioStreamImpl::queueAudioStream(AudioStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	if ((stream->getRate() != getRate()) || (stream->isStereo() != isStereo()))
		error("QueuingAudioStreamImpl::queueAudioStream: stream has mismatched parameters");

	Common::StackLock lock(_mutex);
	_queue.push(StreamHolder(stream, disposeAfterUse));
}

int QueuingAudioStreamImpl::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);
	int samplesDecoded = 0;

	while (samplesDecoded < numSamples && !_queue.empty()) {
		AudioStream *stream = _queue.front()._stream;
		samplesDecoded += stream->readBuffer(buffer + samplesDecoded, numSamples - samplesDecoded);

		if (stream->endOfData()	) {
			StreamHolder tmp = _queue.pop();
			if (tmp._disposeAfterUse == DisposeAfterUse::YES)
				delete stream;
		}
	}

	return samplesDecoded;
}



QueuingAudioStream *makeQueuingAudioStream(int rate, bool stereo) {
	return new QueuingAudioStreamImpl(rate, stereo);
}



} // End of namespace Audio
