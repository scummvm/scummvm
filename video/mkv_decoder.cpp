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
#include "common/debug.h"
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
	MkvReader(Common::SeekableReadStream *stream);
	virtual ~MkvReader() { Close(); }

	int Open(Common::SeekableReadStream *stream);
	void Close();

	virtual int Read(long long position, long length, unsigned char *buffer);
	virtual int Length(long long *total, long long *available);

private:
	Common::SeekableReadStream *_stream;
};

MkvReader::MkvReader(Common::SeekableReadStream *stream) {
	_stream = stream;
}

int MkvReader::Open(Common::SeekableReadStream *stream) {
	_stream = stream;

	return 0;
}

void MkvReader::Close() {
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
	if (_stream->read(buffer, length) < (uint32)length)
		return -1;

	return 0;
}

int MkvReader::Length(long long *total, long long *available) {
	if (!_stream)
		return -1;

	if (total)
		*total = _stream->size();

	if (available)
		*available = _stream->size();

	return 0;
}

} // end of namespace mkvparser

namespace Video {

MKVDecoder::MKVDecoder() {
	_fileStream = 0;

	_videoTrack = 0;
	_audioTrack = 0;
	_hasVideo = _hasAudio = false;

	_reader = nullptr;
}

MKVDecoder::~MKVDecoder() {
	close();
}

static uint64 xiph_lace_value(byte **np) {
	uint64 lace;
	uint64 value;
	byte *p = *np;

	lace = *p++;
	value = lace;
	while (lace == 255) {
		lace = *p++;
		value += lace;
	}

	*np = p;
	return value;
}

bool MKVDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	warning("MKVDecoder::loadStream()");

	_fileStream = stream;
	_reader = new mkvparser::MkvReader(stream);

	long long pos = 0;
	mkvparser::EBMLHeader ebmlHeader;
	ebmlHeader.Parse(_reader, pos);

	long long ret = mkvparser::Segment::CreateInstance(_reader, pos, pSegment);
	if (ret) {
		error("MKVDecoder::loadStream(): Segment::CreateInstance() failed (%lld).", ret);
	}

	ret = pSegment->Load();
	if (ret) {
		error("MKVDecoder::loadStream(): Segment::Load() failed (%lld).", ret);
	}

	pTracks = pSegment->GetTracks();

	uint32 i = 0;
	const unsigned long j = pTracks->GetTracksCount();

	debug(1, "Number of tracks: %d", j);

	enum {VIDEO_TRACK = 1, AUDIO_TRACK = 2};
	videoTrack = -1;
	audioTrack = -1;

	while (i != j) {
		const mkvparser::Track *const pTrack = pTracks->GetTrackByIndex(i++);

		if (pTrack == NULL)
			continue;

		const long long trackType = pTrack->GetType();
		if (trackType == mkvparser::Track::kVideo && videoTrack < 0) {
			videoTrack = pTrack->GetNumber();

			_videoTrack = new VPXVideoTrack(getDefaultHighColorFormat(), pTrack);

			addTrack(_videoTrack);
		}

		if (trackType == mkvparser::Track::kAudio && audioTrack < 0) {
			audioTrack = pTrack->GetNumber();

			const mkvparser::AudioTrack *const pAudioTrack = static_cast<const mkvparser::AudioTrack *>(pTrack);

			size_t audioHeaderSize;
			byte *audioHeader = const_cast<byte *>(pAudioTrack->GetCodecPrivate(audioHeaderSize));

			if (audioHeaderSize < 1) {
				warning("Strange audio track in movie.");
				audioTrack = -1;
				continue;
			}

			byte *p = audioHeader;

			uint count = *p++ + 1;
			if (count != 3) {
				warning("Strange audio track in movie.");
				audioTrack = -1;
				continue;
			}

			_audioTrack = new VorbisAudioTrack(pTrack);
			addTrack(_audioTrack);
		}
	}

	if (videoTrack < 0)
		error("Movie error: No video in movie file.");

	if (audioTrack < 0)
		error("Movie error: No sound found.");

	const unsigned long long clusterCount = pSegment->GetCount();

	if (clusterCount == 0) {
		error("Movie error: Segment has no clusters.\n");
	}

	frame = new byte[256 * 1024];
	if (!frame)
		return false;

	_cluster = pSegment->GetFirst();

	if (_cluster->GetFirst(pBlockEntry))
		error("_cluster::GetFirst() failed");

	if ((pBlockEntry == NULL) || pBlockEntry->EOS()) {
		_cluster = pSegment->GetNext(_cluster);
		if ((_cluster == NULL) || _cluster->EOS()) {
			error("Error: No movie found in the movie file.");
		}
		if (_cluster->GetFirst(pBlockEntry))
			error("_cluster::GetFirst() failed");
	}

	pBlock = pBlockEntry->GetBlock();
	trackNum = pBlock->GetTrackNumber();
	frameCount = pBlock->GetFrameCount();

	return true;
}

void MKVDecoder::close() {
	VideoDecoder::close();

	delete _reader;
}

void MKVDecoder::readNextPacket() {

	// First, let's get our frame
	if (_cluster == nullptr || _cluster->EOS()) {
		_videoTrack->setEndOfVideo();
		if (!_audioTrack->hasAudio())
			_audioTrack->setEndOfAudio();
		return;
	}

	// ensure we have enough buffers in the stream
	while (_audioTrack->needsAudio()) {
		if (frameCounter >= frameCount) {
		_cluster->GetNext(pBlockEntry, pBlockEntry);

			if ((pBlockEntry == NULL) || pBlockEntry->EOS()) {
				_cluster = pSegment->GetNext(_cluster);
				if ((_cluster == NULL) || _cluster->EOS()) {
					_videoTrack->setEndOfVideo();
					_audioTrack->setEndOfAudio();
					return;
				}
				int ret = _cluster->GetFirst(pBlockEntry);
				if (ret < 0)
					error("MKVDecoder::readNextPacket(): GetFirst() failed");
			}

			pBlock  = pBlockEntry->GetBlock();
			trackNum = pBlock->GetTrackNumber();
			frameCount = pBlock->GetFrameCount();
			frameCounter = 0;
		}

		const mkvparser::Block::Frame &theFrame = pBlock->GetFrame(frameCounter);
		const uint32 size = theFrame.len;

		if (size > sizeof(frame)) {
			if (frame)
				delete[] frame;
			frame = new unsigned char[size];
			if (!frame)
				return;
		}

		if (trackNum == videoTrack) {
			theFrame.Read(_reader, frame);
			_videoTrack->decodeFrame(frame, size);
		} else if (trackNum == audioTrack) {
			if (size > 0) {
				theFrame.Read(_reader, frame);
				queueAudio(size);
			}
		} else {
			warning("Unprocessed track %lld", trackNum);
		}
		++frameCounter;
	}
}

MKVDecoder::VPXVideoTrack::VPXVideoTrack(const Graphics::PixelFormat &format, const mkvparser::Track *const pTrack) {
	const mkvparser::VideoTrack *const pVideoTrack = static_cast<const mkvparser::VideoTrack *>(pTrack);

	const long long width = pVideoTrack->GetWidth();
	const long long height = pVideoTrack->GetHeight();

	debug(1, "VideoTrack: %lld x %lld", width, height);

	_displaySurface.create(width, height, format);

	_endOfVideo = false;
	_nextFrameStartTime = 0.0;
	_curFrame = -1;

	_codec = new vpx_codec_ctx_t;

	/* Initialize video codec */
	if (vpx_codec_dec_init(_codec, &vpx_codec_vp8_dx_algo, NULL, 0))
		error("Failed to initialize decoder for movie.");
}

MKVDecoder::VPXVideoTrack::~VPXVideoTrack() {
	_displaySurface.free();
	// The last frame is not freed in decodeNextFrame(), clear it hear instead.
	_surface.free();
	delete _codec;
}

bool MKVDecoder::VPXVideoTrack::endOfTrack() const {
	if (_endOfVideo && _displayQueue.size())
		return false;
	return _endOfVideo;
}

const Graphics::Surface *MKVDecoder::VPXVideoTrack::decodeNextFrame() {
	if (_displayQueue.size()) {
		if (_surface.getPixels())
			_surface.free();
		_surface = _displayQueue.pop();
	}
	return &_surface;
}

bool MKVDecoder::VPXVideoTrack::decodeFrame(byte *frame, long size) {

	//warning("In within decodeFrame");

	/* Decode the frame */
	if (vpx_codec_decode(_codec, frame, size, NULL, 0))
		error("Failed to decode frame");

	// Let's decode an image frame!
	vpx_codec_iter_t iter = NULL;
	vpx_image_t *img;
	Graphics::Surface tmp;
	tmp.create(getWidth(), getHeight(), getPixelFormat());

	/* Get frame data */
	while ((img = vpx_codec_get_frame(_codec, &iter))) {
		if (img->fmt != VPX_IMG_FMT_I420)
			error("Movie error. The movie is not in I420 colour format, which is the only one I can hanlde at the moment.");

		YUVToRGBMan.convert420(&tmp, Graphics::YUVToRGBManager::kScaleITU, img->planes[0], img->planes[1], img->planes[2], img->d_w, img->d_h, img->stride[0], img->stride[1]);
		_displayQueue.push(tmp);
	}
	return false;
}

MKVDecoder::VorbisAudioTrack::VorbisAudioTrack(const mkvparser::Track *const pTrack) :
		AudioTrack(Audio::Mixer::kPlainSoundType) {

	vorbis_comment vorbisComment;

	const mkvparser::AudioTrack *const pAudioTrack = static_cast<const mkvparser::AudioTrack *>(pTrack);

	const long long audioChannels = pAudioTrack->GetChannels();
	const long long audioBitDepth = pAudioTrack->GetBitDepth();
	const double audioSampleRate = pAudioTrack->GetSamplingRate();

	debug(1, "audioChannels %lld audioBitDepth %lld audioSamplerate %f", audioChannels, audioBitDepth, audioSampleRate);

	size_t audioHeaderSize;
	byte *audioHeader = const_cast<byte *>(pAudioTrack->GetCodecPrivate(audioHeaderSize));
	byte *p = audioHeader;

	uint count = *p++ + 1;

	uint64 sizes[3], total;

	int l = 0;
	total = 0;
	while (--count) {
		sizes[l] = xiph_lace_value(&p);
		total += sizes[l];
		l += 1;
	}
	sizes[l] = audioHeaderSize - total - (p - audioHeader);

	// initialize vorbis
	vorbis_info_init(&_vorbisInfo);
	vorbis_comment_init(&vorbisComment);
	memset(&_vorbisDSP, 0, sizeof(_vorbisDSP));
	memset(&_vorbisBlock, 0, sizeof(_vorbisBlock));

	oggPacket.e_o_s = false;
	oggPacket.granulepos = 0;
	oggPacket.packetno = 0;
	int r;
	for (int s = 0; s < 3; s++) {
		oggPacket.packet = p;
		oggPacket.bytes = sizes[s];
		oggPacket.b_o_s = oggPacket.packetno == 0;
		r = vorbis_synthesis_headerin(&_vorbisInfo, &vorbisComment, &oggPacket);
		if (r)
			warning("vorbis_synthesis_headerin failed, error: %d", r);
		oggPacket.packetno++;
		p += sizes[s];
	}

	r = vorbis_synthesis_init(&_vorbisDSP, &_vorbisInfo);
	if(r)
		error("vorbis_synthesis_init, error: %d", r);
	r = vorbis_block_init(&_vorbisDSP, &_vorbisBlock);
	if(r)
		error("vorbis_block_init, error: %d", r);

	_audStream = Audio::makeQueuingAudioStream(_vorbisInfo.rate, _vorbisInfo.channels != 1);
	_endOfAudio = false;
}

MKVDecoder::VorbisAudioTrack::~VorbisAudioTrack() {
	vorbis_dsp_clear(&_vorbisDSP);
	vorbis_block_clear(&_vorbisBlock);
	delete _audStream;
}

Audio::AudioStream *MKVDecoder::VorbisAudioTrack::getAudioStream() const {
	return _audStream;
}

bool MKVDecoder::VorbisAudioTrack::decodeSamples(byte *frame, long size) {
	float **pcm;

	int32 numSamples = vorbis_synthesis_pcmout(&_vorbisDSP, &pcm);

	if (numSamples > 0) {
		int32 channels = _vorbisInfo.channels;
		long bytespersample = _vorbisInfo.channels * 2;

		char *buffer = (char*)malloc(bytespersample * numSamples * sizeof(char));
		if (!buffer)
			error("MKVDecoder::readNextPacket(): buffer allocation failed");

		for (int32 i = 0; i < channels; i++) { /* It's faster in this order */
			float *src = pcm[i];
			short *dest = ((short *)buffer) + i;
			for (int32 j = 0; j < numSamples; j++) {
				int val = rint(src[j] * 32768.f);
				val = CLIP(val, -32768, 32767);
				*dest = (short)val;
				dest += channels;
			}
		}

		byte flags = Audio::FLAG_16BITS;

		if (_audStream->isStereo())
			flags |= Audio::FLAG_STEREO;

#ifdef SCUMM_LITTLE_ENDIAN
		flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif
		int64 audioBufferLen = bytespersample * numSamples;
		_audStream->queueBuffer((byte *)buffer, audioBufferLen, DisposeAfterUse::YES, flags);
		vorbis_synthesis_read(&_vorbisDSP, numSamples);
		return true;
	}
	return false;
}

bool MKVDecoder::VorbisAudioTrack::hasAudio() const {
	return _audStream->numQueuedStreams() > 0;
}

bool MKVDecoder::VorbisAudioTrack::needsAudio() const {
	// TODO: 10 is very arbitrary. We probably should do something like QuickTime does.
	return _audStream->numQueuedStreams() < 10;
}

bool MKVDecoder::VorbisAudioTrack::synthesizePacket(byte *frame, long size) {
	bool res = true;
	oggPacket.packet = frame;
	oggPacket.bytes = size;
	oggPacket.b_o_s = false;
	oggPacket.packetno++;
	oggPacket.granulepos = -1;
	if (vorbis_synthesis(&_vorbisBlock, &oggPacket) == 0) { // test for success
		if(vorbis_synthesis_blockin(&_vorbisDSP, &_vorbisBlock)) {
			res = false;
			warning("vorbis_synthesis_blockin failed");
		}
	}
	else {
		res = false;
		warning("Vorbis synthesis failed");
	}
	return res;
}

bool MKVDecoder::queueAudio(long size) {
	bool queuedAudio = false;

	if (_audioTrack->synthesizePacket(frame, size) && _audioTrack->decodeSamples(frame, size))
		queuedAudio = true;

	return queuedAudio;
}
} // End of namespace Video
