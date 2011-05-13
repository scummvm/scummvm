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
#include "common/util.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "audio/audiostream.h"
#include "audio/decoders/quicktime.h"
#include "audio/decoders/quicktime_intern.h"

// Codecs
#include "audio/decoders/aac.h"
#include "audio/decoders/adpcm.h"
#include "audio/decoders/qdm2.h"
#include "audio/decoders/raw.h"

namespace Audio {

QuickTimeAudioDecoder::QuickTimeAudioDecoder() : Common::QuickTimeParser() {
	_audStream = 0;
}

QuickTimeAudioDecoder::~QuickTimeAudioDecoder() {
	delete _audStream;
}

bool QuickTimeAudioDecoder::loadAudioFile(const Common::String &filename) {
	if (!Common::QuickTimeParser::parseFile(filename))
		return false;

	init();
	return true;
}

bool QuickTimeAudioDecoder::loadAudioStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle) {
	if (!Common::QuickTimeParser::parseStream(stream, disposeFileHandle))
		return false;

	init();
	return true;
}

void QuickTimeAudioDecoder::init() {
	Common::QuickTimeParser::init();

	_audioStreamIndex = -1;

	// Find an audio stream
	for (uint32 i = 0; i < _numStreams; i++)
		if (_streams[i]->codec_type == CODEC_TYPE_AUDIO && _audioStreamIndex < 0)
			_audioStreamIndex = i;

	// Initialize audio, if present
	if (_audioStreamIndex >= 0) {
		AudioSampleDesc *entry = (AudioSampleDesc *)_streams[_audioStreamIndex]->sampleDescs[0];

		if (checkAudioCodecSupport(entry->codecTag, _streams[_audioStreamIndex]->objectTypeMP4)) {
			_audStream = makeQueuingAudioStream(entry->sampleRate, entry->channels == 2);
			_curAudioChunk = 0;

			// Make sure the bits per sample transfers to the sample size
			if (entry->codecTag == MKTAG('r', 'a', 'w', ' ') || entry->codecTag == MKTAG('t', 'w', 'o', 's'))
				_streams[_audioStreamIndex]->sample_size = (entry->bitsPerSample / 8) * entry->channels;
		}
	}
}

Common::QuickTimeParser::SampleDesc *QuickTimeAudioDecoder::readSampleDesc(MOVStreamContext *st, uint32 format) {
	if (st->codec_type == CODEC_TYPE_AUDIO) {
		debug(0, "Audio Codec FourCC: \'%s\'", tag2str(format));

		AudioSampleDesc *entry = new AudioSampleDesc();
		entry->codecTag = format;

		uint16 stsdVersion = _fd->readUint16BE();
		_fd->readUint16BE(); // revision level
		_fd->readUint32BE(); // vendor

		entry->channels = _fd->readUint16BE();			 // channel count
		entry->bitsPerSample = _fd->readUint16BE();	  // sample size

		_fd->readUint16BE(); // compression id = 0
		_fd->readUint16BE(); // packet size = 0

		entry->sampleRate = (_fd->readUint32BE() >> 16);

		debug(0, "stsd version =%d", stsdVersion);
		if (stsdVersion == 0) {
			// Not used, except in special cases. See below.
			entry->samplesPerFrame = entry->bytesPerFrame = 0;
		} else if (stsdVersion == 1) {
			// Read QT version 1 fields. In version 0 these dont exist.
			entry->samplesPerFrame = _fd->readUint32BE();
			debug(0, "stsd samples_per_frame =%d",entry->samplesPerFrame);
			_fd->readUint32BE(); // bytes per packet
			entry->bytesPerFrame = _fd->readUint32BE();
			debug(0, "stsd bytes_per_frame =%d", entry->bytesPerFrame);
			_fd->readUint32BE(); // bytes per sample
		} else {
			warning("Unsupported QuickTime STSD audio version %d", stsdVersion);
			delete entry;
			return 0;
		}

		// Version 0 videos (such as the Riven ones) don't have this set,
		// but we need it later on. Add it in here.
		if (format == MKTAG('i', 'm', 'a', '4')) {
			entry->samplesPerFrame = 64;
			entry->bytesPerFrame = 34 * entry->channels;
		}

		if (entry->sampleRate == 0 && st->time_scale > 1)
			entry->sampleRate = st->time_scale;

		return entry;
	}

	return 0;
}

bool QuickTimeAudioDecoder::checkAudioCodecSupport(uint32 tag, byte objectTypeMP4) {
	// Check if the codec is a supported codec
	if (tag == MKTAG('t', 'w', 'o', 's') || tag == MKTAG('r', 'a', 'w', ' ') || tag == MKTAG('i', 'm', 'a', '4'))
		return true;

#ifdef AUDIO_QDM2_H
	if (tag == MKTAG('Q', 'D', 'M', '2'))
		return true;
#endif

	if (tag == MKTAG('m', 'p', '4', 'a')) {
		Common::String audioType;
		switch (objectTypeMP4) {
		case 0x40: // AAC
#ifdef USE_FAAD
			return true;
#else
			audioType = "AAC";
			break;
#endif
		default:
			audioType = "Unknown";
			break;
		}
		warning("No MPEG-4 audio (%s) support", audioType.c_str());
	} else
		warning("Audio Codec Not Supported: \'%s\'", tag2str(tag));

	return false;
}

AudioStream *QuickTimeAudioDecoder::createAudioStream(Common::SeekableReadStream *stream) {
	if (!stream || _audioStreamIndex < 0)
		return NULL;

	AudioSampleDesc *entry = (AudioSampleDesc *)_streams[_audioStreamIndex]->sampleDescs[0];

	if (entry->codecTag == MKTAG('t', 'w', 'o', 's') || entry->codecTag == MKTAG('r', 'a', 'w', ' ')) {
		// Fortunately, most of the audio used in Myst videos is raw...
		uint16 flags = 0;
		if (entry->codecTag == MKTAG('r', 'a', 'w', ' '))
			flags |= FLAG_UNSIGNED;
		if (entry->channels == 2)
			flags |= FLAG_STEREO;
		if (entry->bitsPerSample == 16)
			flags |= FLAG_16BITS;
		uint32 dataSize = stream->size();
		byte *data = (byte *)malloc(dataSize);
		stream->read(data, dataSize);
		delete stream;
		return makeRawStream(data, dataSize, entry->sampleRate, flags);
	} else if (entry->codecTag == MKTAG('i', 'm', 'a', '4')) {
		// Riven uses this codec (as do some Myst ME videos)
		return makeADPCMStream(stream, DisposeAfterUse::YES, stream->size(), kADPCMApple, entry->sampleRate, entry->channels, 34);
	} else if (entry->codecTag == MKTAG('m', 'p', '4', 'a')) {
		// The 7th Guest iOS uses an MPEG-4 codec
#ifdef USE_FAAD
		if (_streams[_audioStreamIndex]->objectTypeMP4 == 0x40)
			return makeAACStream(stream, DisposeAfterUse::YES, _streams[_audioStreamIndex]->extradata);
#endif
#ifdef AUDIO_QDM2_H
	} else if (entry->codecTag == MKTAG('Q', 'D', 'M', '2')) {
		// Myst ME uses this codec for many videos
		return makeQDM2Stream(stream, _streams[_audioStreamIndex]->extradata);
#endif
	}

	error("Unsupported audio codec");

	return NULL;
}

uint32 QuickTimeAudioDecoder::getAudioChunkSampleCount(uint chunk) {
	if (_audioStreamIndex < 0)
		return 0;

	uint32 sampleCount = 0;

	for (uint32 j = 0; j < _streams[_audioStreamIndex]->sample_to_chunk_sz; j++)
		if (chunk >= _streams[_audioStreamIndex]->sample_to_chunk[j].first)
			sampleCount = _streams[_audioStreamIndex]->sample_to_chunk[j].count;

	return sampleCount;
}

bool QuickTimeAudioDecoder::isOldDemuxing() const {
	assert(_audioStreamIndex >= 0);
	return _streams[_audioStreamIndex]->stts_count == 1 && _streams[_audioStreamIndex]->stts_data[0].duration == 1;
}

void QuickTimeAudioDecoder::queueNextAudioChunk() {
	AudioSampleDesc *entry = (AudioSampleDesc *)_streams[_audioStreamIndex]->sampleDescs[0];
	Common::MemoryWriteStreamDynamic *wStream = new Common::MemoryWriteStreamDynamic();

	_fd->seek(_streams[_audioStreamIndex]->chunk_offsets[_curAudioChunk]);

	// First, we have to get the sample count
	uint32 sampleCount = getAudioChunkSampleCount(_curAudioChunk);
	assert(sampleCount);

	if (isOldDemuxing()) {
		// Old-style audio demuxing

		// Then calculate the right sizes
		while (sampleCount > 0) {
			uint32 samples = 0, size = 0;

			if (entry->samplesPerFrame >= 160) {
				samples = entry->samplesPerFrame;
				size = entry->bytesPerFrame;
			} else if (entry->samplesPerFrame > 1) {
				samples = MIN<uint32>((1024 / entry->samplesPerFrame) * entry->samplesPerFrame, sampleCount);
				size = (samples / entry->samplesPerFrame) * entry->bytesPerFrame;
			} else {
				samples = MIN<uint32>(1024, sampleCount);
				size = samples * _streams[_audioStreamIndex]->sample_size;
			}

			// Now, we read in the data for this data and output it
			byte *data = (byte *)malloc(size);
			_fd->read(data, size);
			wStream->write(data, size);
			free(data);
			sampleCount -= samples;
		}
	} else {
		// New-style audio demuxing

		// Find our starting sample
		uint32 startSample = 0;
		for (uint32 i = 0; i < _curAudioChunk; i++)
			startSample += getAudioChunkSampleCount(i);

		for (uint32 i = 0; i < sampleCount; i++) {
			uint32 size = (_streams[_audioStreamIndex]->sample_size != 0) ? _streams[_audioStreamIndex]->sample_size : _streams[_audioStreamIndex]->sample_sizes[i + startSample];

			// Now, we read in the data for this data and output it
			byte *data = (byte *)malloc(size);
			_fd->read(data, size);
			wStream->write(data, size);
			free(data);
		}
	}

	// Now queue the buffer
	_audStream->queueAudioStream(createAudioStream(new Common::MemoryReadStream(wStream->getData(), wStream->size(), DisposeAfterUse::YES)));
	delete wStream;

	_curAudioChunk++;
}

void QuickTimeAudioDecoder::setAudioStreamPos(const Timestamp &where) {
	if (!_audStream)
		return;

	// Re-create the audio stream
	delete _audStream;
	Audio::QuickTimeAudioDecoder::AudioSampleDesc *entry = (Audio::QuickTimeAudioDecoder::AudioSampleDesc *)_streams[_audioStreamIndex]->sampleDescs[0];
	_audStream = Audio::makeQueuingAudioStream(entry->sampleRate, entry->channels == 2);

	// First, we need to track down what audio sample we need
	Audio::Timestamp curAudioTime = where.convertToFramerate(_streams[_audioStreamIndex]->time_scale);
	uint32 sample = curAudioTime.totalNumberOfFrames();
	uint32 seekSample = sample; 

	if (!isOldDemuxing()) {
		// We shouldn't have audio samples that are a different duration
		// That would be quite bad!
		if (_streams[_audioStreamIndex]->stts_count != 1) {
			warning("Failed seeking");
			return;
		}

		// Note that duration is in terms of *one* channel
		// This eases calculation a bit
		seekSample /= _streams[_audioStreamIndex]->stts_data[0].duration;
	}

	// Now to track down what chunk it's in
	uint32 totalSamples = 0;
	_curAudioChunk = 0;
	for (uint32 i = 0; i < _streams[_audioStreamIndex]->chunk_count; i++, _curAudioChunk++) {
		uint32 chunkSampleCount = getAudioChunkSampleCount(i);

		if (seekSample < totalSamples + chunkSampleCount)
			break;

		totalSamples += chunkSampleCount;
	}
		
	// Reposition the audio stream
	queueNextAudioChunk();
	if (sample != totalSamples) {
		// HACK: Skip a certain amount of samples from the stream
		// (There's got to be a better way to do this!)
		int skipSamples = (sample - totalSamples) * entry->channels;

		int16 *tempBuffer = new int16[skipSamples];
		_audStream->readBuffer(tempBuffer, skipSamples);
		delete[] tempBuffer;
	}
}

QuickTimeAudioDecoder::AudioSampleDesc::AudioSampleDesc() : Common::QuickTimeParser::SampleDesc() {
	channels = 0;
	sampleRate = 0;
	samplesPerFrame = 0;
	bytesPerFrame = 0;
}

/**
 * A wrapper around QuickTimeAudioDecoder that implements the RewindableAudioStream API
 */
class QuickTimeAudioStream : public SeekableAudioStream, public QuickTimeAudioDecoder {
public:
	QuickTimeAudioStream() {}
	~QuickTimeAudioStream() {}

	bool openFromFile(const Common::String &filename) {
		return QuickTimeAudioDecoder::loadAudioFile(filename) && _audioStreamIndex >= 0 && _audStream;
	}

	bool openFromStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle) {
		return QuickTimeAudioDecoder::loadAudioStream(stream, disposeFileHandle) && _audioStreamIndex >= 0 && _audStream;
	}

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) {
		int samples = 0;

		while (samples < numSamples && !endOfData()) {
			if (_audStream->numQueuedStreams() == 0)
				queueNextAudioChunk();

			samples += _audStream->readBuffer(buffer + samples, numSamples - samples);
		}

		return samples;
	}

	bool isStereo() const { return _audStream->isStereo(); }
	int getRate() const { return _audStream->getRate(); }
	bool endOfData() const { return _curAudioChunk >= _streams[_audioStreamIndex]->chunk_count && _audStream->endOfData(); }

	// SeekableAudioStream API
	bool seek(const Timestamp &where) {
		if (where > getLength())
			return false;

		setAudioStreamPos(where);
		return true;
	}

	Timestamp getLength() const {
		return Timestamp(0, _streams[_audioStreamIndex]->duration, _streams[_audioStreamIndex]->time_scale);
	}
};

SeekableAudioStream *makeQuickTimeStream(const Common::String &filename) {
	QuickTimeAudioStream *audioStream = new QuickTimeAudioStream();

	if (!audioStream->openFromFile(filename)) {
		delete audioStream;
		return 0;
	}

	return audioStream;
}

SeekableAudioStream *makeQuickTimeStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	QuickTimeAudioStream *audioStream = new QuickTimeAudioStream();

	if (!audioStream->openFromStream(stream, disposeAfterUse)) {
		delete audioStream;
		return 0;
	}

	return audioStream;
}

} // End of namespace Audio
