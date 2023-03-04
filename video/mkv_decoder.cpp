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
	if (_stream->read(buffer, length) < length)
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
	if (ret < 0) {
		error("MKVDecoder::loadStream(): Segment::Load() failed (%lld).", ret);
	}

	pTracks = pSegment->GetTracks();

	unsigned long i = 0;
	const unsigned long j = pTracks->GetTracksCount();

	debug(1, "Number of tracks: %ld", j);

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
			//setRate(_videoTrack->getFrameRate());

#if 0
			if (rate > 0)
				Init_Special_Timer(rate); // TODO
#endif
		}

		if (trackType == mkvparser::Track::kAudio && audioTrack < 0) {
			audioTrack = pTrack->GetNumber();

			const mkvparser::AudioTrack *const pAudioTrack = static_cast<const mkvparser::AudioTrack *>(pTrack);

			size_t audioHeaderSize;
			byte *audioHeader = (byte *)pAudioTrack->GetCodecPrivate(audioHeaderSize);

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

#if 0
	movieIsPlaying = playing;
	movieIsEnding = 0;
#endif

	//const long long timeCode = _cluster->GetTimeCode();
	long long time_ns = _cluster->GetTime();

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
	tn = static_cast<unsigned long>(trackNum);
	frameCount = pBlock->GetFrameCount();
	time_ns = pBlock->GetTime(_cluster);

	return true;
}

void MKVDecoder::close() {
	VideoDecoder::close();

	delete _reader;
}

void MKVDecoder::readNextPacket() {
	//warning("MKVDecoder::readNextPacket()");

	// First, let's get our frame
	if (_cluster == nullptr || _cluster->EOS()) {
		_videoTrack->setEndOfVideo();
		return;
	}

	//warning("trackNum: %d frameCounter: %d frameCount: %d, time_ns: %d", tn, frameCounter, frameCount, time_ns);

	if (frameCounter >= frameCount) {
		_cluster->GetNext(pBlockEntry, pBlockEntry);

		if ((pBlockEntry == NULL) || pBlockEntry->EOS()) {
			_cluster = pSegment->GetNext(_cluster);
			if ((_cluster == NULL) || _cluster->EOS()) {
				_videoTrack->setEndOfVideo();
				return;
			}
			int ret = _cluster->GetFirst(pBlockEntry);

			if (ret < 0)
				error("MKVDecoder::readNextPacket(): GetFirst() failed");
		}
		pBlock  = pBlockEntry->GetBlock();
		trackNum = pBlock->GetTrackNumber();
		frameCount = pBlock->GetFrameCount();
		time_ns = pBlock->GetTime(_cluster);
		frameCounter = 0;
	}

	const mkvparser::Block::Frame &theFrame = pBlock->GetFrame(frameCounter);
	const long size = theFrame.len;

	if (size > sizeof(frame)) {
		if (frame)
			delete[] frame;
		frame = new unsigned char[size];
		if (!frame)
			return;
	}

	if (trackNum == videoTrack) {
		warning("MKVDecoder::readNextPacket(): video track");

		theFrame.Read(_reader, frame);
		_videoTrack->decodeFrame(frame, size);
	} else if (trackNum == audioTrack) {
		warning("MKVDecoder::readNextPacket(): audio track");

		if (size > 0) {
			theFrame.Read(_reader, frame);
			_audioTrack->decodeSamples(frame, size);
		}
	} else {
		warning("Unprocessed track %d", trackNum);
	}
	++frameCounter;
}

MKVDecoder::VPXVideoTrack::VPXVideoTrack(const Graphics::PixelFormat &format, const mkvparser::Track *const pTrack) {
	const mkvparser::VideoTrack *const pVideoTrack = static_cast<const mkvparser::VideoTrack *>(pTrack);

	const long long width = pVideoTrack->GetWidth();
	const long long height = pVideoTrack->GetHeight();
	const double rate = pVideoTrack->GetFrameRate();

	warning("VideoTrack: %lld x %lld @ %f fps", width, height, rate);

	_displaySurface.create(width, height, format);

	_frameRate = 10; // FIXME

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
	delete _codec;
}

bool MKVDecoder::VPXVideoTrack::decodeFrame(byte *frame, long size) {

	//warning("In within decodeFrame");

	/* Decode the frame */
	if (vpx_codec_decode(_codec, frame, size, NULL, 0))
		error("Failed to decode frame");

	// Let's decode an image frame!
	vpx_codec_iter_t  iter = NULL;
	vpx_image_t      *img;
	/* Get frame data */
	while ((img = vpx_codec_get_frame(_codec, &iter))) {
		if (img->fmt != VPX_IMG_FMT_I420)
			error("Movie error. The movie is not in I420 colour format, which is the only one I can hanlde at the moment.");

		YUVToRGBMan.convert420(&_displaySurface, Graphics::YUVToRGBManager::kScaleITU, img->planes[0], img->planes[1], img->planes[2], img->d_w, img->d_h, img->stride[0], img->stride[1]);
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

	warning("audioChannels %d audioBitDepth %d audioSamplerate %f", audioChannels, audioBitDepth, audioSampleRate);

	size_t audioHeaderSize;
	byte *audioHeader = (byte *)pAudioTrack->GetCodecPrivate(audioHeaderSize);
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

	_endOfAudio = false;
}

MKVDecoder::VorbisAudioTrack::~VorbisAudioTrack() {
	//vorbis_dsp_clear(&_vorbisDSP);
	//vorbis_block_clear(&vorbisBlock);
	//delete _audStream;
	//free(_audioBuffer);
}

Audio::AudioStream *MKVDecoder::VorbisAudioTrack::getAudioStream() const {
	return _audStream;
}

bool MKVDecoder::VorbisAudioTrack::decodeSamples(byte *frame, long size) {
	//return true;

	ogg_packet packet;
	packet.packet = frame;
	packet.bytes = size;
	packet.b_o_s = false;
	packet.e_o_s = false;
	packet.packetno = ++oggPacket.packetno;
	packet.granulepos = -1;

	warning("oggPacket bytes : %d packno %d\n", oggPacket.bytes, oggPacket.packetno);

	if(!vorbis_synthesis(&_vorbisBlock, &packet) ) {
		if (vorbis_synthesis_blockin(&vorbisDspState, &_vorbisBlock))
			warning("Vorbis Synthesis block in error");

	} else {
		warning("Vorbis Synthesis error");
	}

	float **pcm;

	int numSamples = vorbis_synthesis_pcmout(&vorbisDspState, &pcm);

	if (numSamples > 0) {
		int word = 2;
		int sgned = 1;
		int i, j;
		long bytespersample= audioChannels * word;
		//vorbis_fpu_control fpu;

		char *buffer = new char[bytespersample * numSamples];
		if (!buffer)
			error("MKVDecoder::readNextPacket(): buffer allocation failed");

		/* a tight loop to pack each size */
		{
			int val;
			if (word == 1) {
				int off = (sgned ? 0 : 128);
				//vorbis_fpu_setround(&fpu);
				for (j = 0; j < numSamples; j++)
					for (i = 0;i < audioChannels; i++) {
						val = (int)(pcm[i][j] * 128.f);
						val = CLIP(val, -128, 127);

						*buffer++ = val + off;
					}
				//vorbis_fpu_restore(fpu);
			} else {
				int off = (sgned ? 0 : 32768);

				if (sgned) {
					//vorbis_fpu_setround(&fpu);
					for (i = 0; i < audioChannels; i++) { /* It's faster in this order */
						float *src = pcm[i];
						short *dest = ((short *)buffer) + i;
						for (j = 0; j < numSamples; j++) {
							val = (int)(src[j] * 32768.f);
							val = CLIP(val, -32768, 32767);

							*dest = val;
							dest += audioChannels;
						}
					}
					//vorbis_fpu_restore(fpu);
				} else { // unsigned
					//vorbis_fpu_setround(&fpu);

					for (i = 0; i < audioChannels; i++) {
						float *src = pcm[i];
						short *dest = ((short *)buffer) + i;
						for (j = 0; j < numSamples; j++) {
							val = (int)(src[j] * 32768.f);
							val = CLIP(val, -32768, 32767);

							*dest = val + off;
							dest += audioChannels;
						}
					}
					//vorbis_fpu_restore(fpu);
				}
			}
		}

		vorbis_synthesis_read(&vorbisDspState, numSamples);
		audioBufferLen = bytespersample * numSamples;
		if (!movieSoundPlaying) {
			warning("** starting sound **");
			movieSoundPlaying = true;
		}
	}
	return true;
}

bool MKVDecoder::VorbisAudioTrack::hasAudio() const {
	return _audStream->numQueuedStreams() > 0;
}

bool MKVDecoder::VorbisAudioTrack::needsAudio() const {
	// TODO: 5 is very arbitrary. We probably should do something like QuickTime does.
	return !_endOfAudio && _audStream->numQueuedStreams() < 5;
}

void MKVDecoder::VorbisAudioTrack::synthesizePacket(ogg_packet &_oggPacket) {
	warning("in synthesizePacket");
	if (vorbis_synthesis(&_vorbisBlock, &_oggPacket) == 0) // test for success
		vorbis_synthesis_blockin(&_vorbisDSP, &_vorbisBlock);
}

bool MKVDecoder::queueAudio() {
	if (!_hasAudio)
		return false;

	bool queuedAudio = false;
	return queuedAudio;
}
} // End of namespace Video
