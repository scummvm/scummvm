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

#include "video/mkv_decoder.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/pixelformat.h"
#include "graphics/yuv_to_rgb.h"

#include "video/mkv/mkvparser.h"

namespace mkvparser  {

class MkvReader : public mkvparser::IMkvReader {
public:
	MkvReader() { _stream = nullptr; }
	MkvReader(Common::SeekableReadStream *stream, uint size = 0);
	virtual ~MkvReader() { Close(); }

	int Open(Common::SeekableReadStream *stream);
	void Close();

	virtual int Read(long long position, long length, unsigned char *buffer);
	virtual int Length(long long *total, long long *available);

private:
	Common::SeekableReadStream *_stream;
	uint _size;
};

MkvReader::MkvReader(Common::SeekableReadStream *stream, uint size) {
	_stream = stream;

	if (size == 0)
		size = _stream->size();

	_size = size;
}

int MkvReader::Open(Common::SeekableReadStream *stream) {
	_stream = stream;

	return 0;
}

void MkvReader::Close() {
	delete _stream;

	_stream = nullptr;
}

int MkvReader::Read(long long position, long length, unsigned char *buffer) {
	if (!_stream)
		return -1;

	if (position > _stream->size() || position < 0)
		return -1;

	if (length <= 0)
		return -1;

	_stream->seek(position);
	if (_stream->read(buffer, length) < length)
		return -1;

	return 0;
}

int MkvReader::Length(long long *total, long long *available) {
	if (!_stream)
		return -1;

	if (*total)
		*total = _size;

	if (*available)
		*available = _size;

	return 0;
}

} // end of namespace mkvparser

namespace Video {

MKVDecoder::MKVDecoder() {
	_fileStream = 0;

	_videoTrack = 0;
	_audioTrack = 0;
	_hasVideo = _hasAudio = false;

	_codec = nullptr;
	_reader = nullptr;
}

MKVDecoder::~MKVDecoder() {
	close();
}

bool MKVDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	_fileStream = stream;

	_codec = new vpx_codec_ctx_t;
	_reader = new mkvparser::MkvReader(stream);

	long long pos = 0;

	mkvparser::EBMLHeader ebmlHeader;

	ebmlHeader.Parse(_reader, pos);

	mkvparser::Segment *pSegment;

	long long ret = mkvparser::Segment::CreateInstance(_reader, pos, pSegment);
	if (ret) {
		error("MKVDecoder::loadStream(): Segment::CreateInstance() failed.");
	}

	ret = pSegment->Load();
	if (ret < 0) {
		error("MKVDecoder::loadStream(): Segment::Load() failed.\n");
	}

	const mkvparser::Tracks *pTracks = pSegment->GetTracks();

	unsigned long i = 0;
	const unsigned long j = pTracks->GetTracksCount();

	warning("Number of tracks: %ld", j);

	enum {VIDEO_TRACK = 1, AUDIO_TRACK = 2};
	int videoTrack = -1;
	int audioTrack = -1;
	long long audioBitDepth;
	double audioSampleRate;
	ogg_packet oggPacket;
	vorbis_info vorbisInfo;
	vorbis_comment vorbisComment;
	vorbis_block vorbisBlock;

#if 0
	while (i != j) {
		const Track *const pTrack = pTracks->GetTrackByIndex(i++);

		if (pTrack == NULL)
		continue;

		const long long trackType = pTrack->GetType();
		//const unsigned long long trackUid = pTrack->GetUid();
		//const char* pTrackName = pTrack->GetNameAsUTF8();

		if (trackType == VIDEO_TRACK && videoTrack < 0) {
			videoTrack = pTrack->GetNumber();
			const VideoTrack *const pVideoTrack =
			static_cast<const VideoTrack *>(pTrack);

			const long long width = pVideoTrack->GetWidth();
			const long long height = pVideoTrack->GetHeight();

			const double rate = pVideoTrack->GetFrameRate();

			if (rate > 0)
				Init_Special_Timer(rate);

			movieAspect = (float)width / height;
		}

		if (trackType == AUDIO_TRACK && audioTrack < 0) {
			audioTrack = pTrack->GetNumber();
			const AudioTrack *const pAudioTrack =
			static_cast<const AudioTrack *>(pTrack);

			audioChannels = pAudioTrack->GetChannels();
			audioBitDepth = pAudioTrack->GetBitDepth();
			audioSampleRate = pAudioTrack->GetSamplingRate();

			uint audioHeaderSize;
			const byte *audioHeader = pAudioTrack->GetCodecPrivate(audioHeaderSize);

			if (audioHeaderSize < 1) {
				warning("Strange audio track in movie.");
				audioTrack = -1;
				continue;
			}

			byte *p = (byte *)audioHeader;

			uint count = *p++ + 1;
			if (count != 3) {
				warning("Strange audio track in movie.");
				audioTrack = -1;
				continue;
			}

			uint64_t sizes[3], total;

			int i = 0;
			total = 0;
			while (--count) {
				sizes[i] = xiph_lace_value(&p);
				total += sizes[i];
				i += 1;
			}
			sizes[i] = audioHeaderSize - total - (p - audioHeader);

			// initialize vorbis
			vorbis_info_init(&vorbisInfo);
			vorbis_comment_init(&vorbisComment);
			memset(&vorbisDspState, 0, sizeof(vorbisDspState));
			memset(&vorbisBlock, 0, sizeof(vorbisBlock));

			oggPacket.e_o_s = false;
			oggPacket.granulepos = 0;
			oggPacket.packetno = 0;
			int r;
			for (int i = 0; i < 3; i++) {
				oggPacket.packet = p;
				oggPacket.bytes = sizes[i];
				oggPacket.b_o_s = oggPacket.packetno == 0;
				r = vorbis_synthesis_headerin(&vorbisInfo, &vorbisComment, &oggPacket);
				if (r)
				fprintf(stderr, "vorbis_synthesis_headerin failed, error: %d", r);
				oggPacket.packetno++;
				p += sizes[i];
			}

			r = vorbis_synthesis_init(&vorbisDspState, &vorbisInfo);
			if (r)
			fprintf(stderr, "vorbis_synthesis_init failed, error: %d", r);
			r = vorbis_block_init(&vorbisDspState, &vorbisBlock);
			if (r)
			fprintf(stderr, "vorbis_block_init failed, error: %d", r);

			ALenum audioFormat = alureGetSampleFormat(audioChannels, 16, 0);
			movieAudioIndex = initMovieSound(fileNumber, audioFormat, audioChannels, (ALuint) audioSampleRate, feedAudio);

			fprintf(stderr, "Movie sound inited.\n");
			audio_queue_init(&audioQ);
			audioNsPerByte = (1000000000 / audioSampleRate) / (audioChannels * 2);
			audioNsBuffered = 0;
			audioBufferLen = audioChannels * audioSampleRate;
		}
	}

	if (videoTrack < 0)
	fatal("Movie error: No video in movie file.");

	if (audioTrack < 0)
	fatal("Movie error: No sound found.");

	video_queue_init(&videoQ);

	const unsigned long clusterCount = pSegment->GetCount();

	if (clusterCount == 0) {
		fatal("Movie error: Segment has no clusters.\n");
	}

	/* Initialize video codec */
	if (vpx_codec_dec_init(&codec, interface, NULL, 0))
	die_codec(&codec, "Failed to initialize decoder for movie.");

	byte *frame = new byte[256 * 1024];
	if (! checkNew(frame)) return false;

	const mkvparser::Cluster *pCluster = pSegment->GetFirst();

	setMovieViewport();

	movieIsPlaying = playing;
	movieIsEnding = 0;

	//const long long timeCode = pCluster->GetTimeCode();
	long long time_ns = pCluster->GetTime();

	const BlockEntry *pBlockEntry = pCluster->GetFirst();

	if ((pBlockEntry == NULL) || pBlockEntry->EOS()) {
		pCluster = pSegment->GetNext(pCluster);
		if ((pCluster == NULL) || pCluster->EOS()) {
			fatal("Error: No movie found in the movie file.");
		}
		pBlockEntry = pCluster->GetFirst();
	}
	const Block *pBlock = pBlockEntry->GetBlock();
	long long trackNum = pBlock->GetTrackNumber();
	unsigned long tn = static_cast<unsigned long>(trackNum);
	const Track *pTrack = pTracks->GetTrackByNumber(tn);
	long long trackType = pTrack->GetType();
	int frameCount = pBlock->GetFrameCount();
	time_ns = pBlock->GetTime(pCluster);
#endif

	return true;
}

void MKVDecoder::close() {
	VideoDecoder::close();

	delete _codec;
	delete _reader;
}

void MKVDecoder::readNextPacket() {
	// First, let's get our frame
	if (_hasVideo) {
		while (!_videoTrack->endOfTrack()) {
			// theora is one in, one out...
			if (ogg_stream_packetout(&_theoraOut, &_oggPacket) > 0) {
				if (_videoTrack->decodePacket(_oggPacket))
					break;
			} else if (_theoraOut.e_o_s || _fileStream->eos()) {
				// If we can't get any more frames, we're done.
				_videoTrack->setEndOfVideo();
			} else {
				// Queue more data
				bufferData();
				while (ogg_sync_pageout(&_oggSync, &_oggPage) > 0)
					queuePage(&_oggPage);
			}

			// Update audio if we can
			queueAudio();
		}
	}

	// Then make sure we have enough audio buffered
	ensureAudioBufferSize();
}

MKVDecoder::VPXVideoTrack::VPXVideoTrack(const Graphics::PixelFormat &format, th_info &theoraInfo, th_setup_info *theoraSetup) {
	_theoraDecode = th_decode_alloc(&theoraInfo, theoraSetup);

	if (theoraInfo.pixel_fmt != TH_PF_420)
		error("Only theora YUV420 is supported");

	int postProcessingMax;
	th_decode_ctl(_theoraDecode, TH_DECCTL_GET_PPLEVEL_MAX, &postProcessingMax, sizeof(postProcessingMax));
	th_decode_ctl(_theoraDecode, TH_DECCTL_SET_PPLEVEL, &postProcessingMax, sizeof(postProcessingMax));

	_surface.create(theoraInfo.frame_width, theoraInfo.frame_height, format);

	// Set up a display surface
	_displaySurface.init(theoraInfo.pic_width, theoraInfo.pic_height, _surface.pitch,
	                    _surface.getBasePtr(theoraInfo.pic_x, theoraInfo.pic_y), format);

	// Set the frame rate
	_frameRate = Common::Rational(theoraInfo.fps_numerator, theoraInfo.fps_denominator);

	_endOfVideo = false;
	_nextFrameStartTime = 0.0;
	_curFrame = -1;
}

MKVDecoder::VPXVideoTrack::~VPXVideoTrack() {
	th_decode_free(_theoraDecode);

	_surface.free();
	_displaySurface.setPixels(0);
}

bool MKVDecoder::VPXVideoTrack::decodePacket(ogg_packet &oggPacket) {
	if (th_decode_packetin(_theoraDecode, &oggPacket, 0) == 0) {
		_curFrame++;

		// Convert YUV data to RGB data
		th_ycbcr_buffer yuv;
		th_decode_ycbcr_out(_theoraDecode, yuv);
		translateYUVtoRGBA(yuv);

		double time = th_granule_time(_theoraDecode, oggPacket.granulepos);

		// We need to calculate when the next frame should be shown
		// This is all in floating point because that's what the Ogg code gives us
		// Ogg is a lossy container format, so it doesn't always list the time to the
		// next frame. In such cases, we need to calculate it ourselves.
		if (time == -1.0)
			_nextFrameStartTime += _frameRate.getInverse().toDouble();
		else
			_nextFrameStartTime = time;

		return true;
	}

	return false;
}

enum TheoraYUVBuffers {
	kBufferY = 0,
	kBufferU = 1,
	kBufferV = 2
};

void MKVDecoder::VPXVideoTrack::translateYUVtoRGBA(th_ycbcr_buffer &YUVBuffer) {
	// Width and height of all buffers have to be divisible by 2.
	assert((YUVBuffer[kBufferY].width & 1) == 0);
	assert((YUVBuffer[kBufferY].height & 1) == 0);
	assert((YUVBuffer[kBufferU].width & 1) == 0);
	assert((YUVBuffer[kBufferV].width & 1) == 0);

	// UV images have to have a quarter of the Y image resolution
	assert(YUVBuffer[kBufferU].width == YUVBuffer[kBufferY].width >> 1);
	assert(YUVBuffer[kBufferV].width == YUVBuffer[kBufferY].width >> 1);
	assert(YUVBuffer[kBufferU].height == YUVBuffer[kBufferY].height >> 1);
	assert(YUVBuffer[kBufferV].height == YUVBuffer[kBufferY].height >> 1);

	YUVToRGBMan.convert420(&_surface, Graphics::YUVToRGBManager::kScaleITU, YUVBuffer[kBufferY].data, YUVBuffer[kBufferU].data, YUVBuffer[kBufferV].data, YUVBuffer[kBufferY].width, YUVBuffer[kBufferY].height, YUVBuffer[kBufferY].stride, YUVBuffer[kBufferU].stride);
}

static vorbis_info *info = 0;

MKVDecoder::VorbisAudioTrack::VorbisAudioTrack(Audio::Mixer::SoundType soundType, vorbis_info &vorbisInfo) :
		AudioTrack(soundType) {
	vorbis_synthesis_init(&_vorbisDSP, &vorbisInfo);
	vorbis_block_init(&_vorbisDSP, &_vorbisBlock);
	info = &vorbisInfo;

	_audStream = Audio::makeQueuingAudioStream(vorbisInfo.rate, vorbisInfo.channels != 1);

	_audioBufferFill = 0;
	_audioBuffer = 0;
	_endOfAudio = false;
}

MKVDecoder::VorbisAudioTrack::~VorbisAudioTrack() {
	vorbis_dsp_clear(&_vorbisDSP);
	vorbis_block_clear(&_vorbisBlock);
	delete _audStream;
	free(_audioBuffer);
}

Audio::AudioStream *MKVDecoder::VorbisAudioTrack::getAudioStream() const {
	return _audStream;
}

#define AUDIOFD_FRAGSIZE 10240

#ifndef USE_TREMOR
static double rint(double v) {
	return floor(v + 0.5);
}
#endif

bool MKVDecoder::VorbisAudioTrack::decodeSamples() {
#ifdef USE_TREMOR
	ogg_int32_t **pcm;
#else
	float **pcm;
#endif

	// if there's pending, decoded audio, grab it
	int ret = vorbis_synthesis_pcmout(&_vorbisDSP, &pcm);

	if (ret > 0) {
		if (!_audioBuffer) {
			_audioBuffer = (ogg_int16_t *)malloc(AUDIOFD_FRAGSIZE * sizeof(ogg_int16_t));
			assert(_audioBuffer);
		}

		int channels = _audStream->isStereo() ? 2 : 1;
		int count = _audioBufferFill / 2;
		int maxsamples = ((AUDIOFD_FRAGSIZE - _audioBufferFill) / channels) >> 1;
		int i;

		for (i = 0; i < ret && i < maxsamples; i++) {
			for (int j = 0; j < channels; j++) {
#ifdef USE_TREMOR
				int val = CLIP((int)pcm[j][i] >> 9, -32768, 32767);
#else
				int val = CLIP((int)rint(pcm[j][i] * 32767.f), -32768, 32767);
#endif
				_audioBuffer[count++] = val;
			}
		}

		vorbis_synthesis_read(&_vorbisDSP, i);
		_audioBufferFill += (i * channels) << 1;

		if (_audioBufferFill == AUDIOFD_FRAGSIZE) {
			byte flags = Audio::FLAG_16BITS;

			if (_audStream->isStereo())
				flags |= Audio::FLAG_STEREO;

#ifdef SCUMM_LITTLE_ENDIAN
			flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif

			_audStream->queueBuffer((byte *)_audioBuffer, AUDIOFD_FRAGSIZE, DisposeAfterUse::YES, flags);

			// The audio mixer is now responsible for the old audio buffer.
			// We need to create a new one.
			_audioBuffer = 0;
			_audioBufferFill = 0;
		}

		return true;
	}

	return false;
}

bool MKVDecoder::VorbisAudioTrack::hasAudio() const {
	return _audStream->numQueuedStreams() > 0;
}

bool MKVDecoder::VorbisAudioTrack::needsAudio() const {
	// TODO: 5 is very arbitrary. We probably should do something like QuickTime does.
	return !_endOfAudio && _audStream->numQueuedStreams() < 5;
}

void MKVDecoder::VorbisAudioTrack::synthesizePacket(ogg_packet &oggPacket) {
	if (vorbis_synthesis(&_vorbisBlock, &oggPacket) == 0) // test for success
		vorbis_synthesis_blockin(&_vorbisDSP, &_vorbisBlock);
}

void MKVDecoder::queuePage(ogg_page *page) {
	if (_hasVideo)
		ogg_stream_pagein(&_theoraOut, page);

	if (_hasAudio)
		ogg_stream_pagein(&_vorbisOut, page);
}

int MKVDecoder::bufferData() {
	char *buffer = ogg_sync_buffer(&_oggSync, 4096);
	int bytes = _fileStream->read(buffer, 4096);

	ogg_sync_wrote(&_oggSync, bytes);

	return bytes;
}

bool MKVDecoder::queueAudio() {
	if (!_hasAudio)
		return false;

	bool queuedAudio = false;

	for (;;) {
		if (_audioTrack->decodeSamples()) {
			// we queued some pending audio
			queuedAudio = true;
		} else if (ogg_stream_packetout(&_vorbisOut, &_oggPacket) > 0) {
			// no pending audio; is there a pending packet to decode?
			_audioTrack->synthesizePacket(_oggPacket);
		} else {
			// we've buffered all we have, break out for now
			break;
		}
	}

	return queuedAudio;
}

void MKVDecoder::ensureAudioBufferSize() {
	if (!_hasAudio)
		return;

	// Force at least some audio to be buffered
	while (_audioTrack->needsAudio()) {
		bufferData();
		while (ogg_sync_pageout(&_oggSync, &_oggPage) > 0)
			queuePage(&_oggPage);

		bool queuedAudio = queueAudio();
		if ((_vorbisOut.e_o_s  || _fileStream->eos()) && !queuedAudio) {
			_audioTrack->setEndOfAudio();
			break;
		}
	}
}

} // End of namespace Video
