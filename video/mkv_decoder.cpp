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

	_codec = nullptr;
	_reader = nullptr;
}

MKVDecoder::~MKVDecoder() {
	close();
}

long long audioChannels;

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

vorbis_dsp_state vorbisDspState;
int64 audioNsPerByte;
int64 audioNsPlayed;
int64 audioNsBuffered;
int64 audioBufferLen;
bool movieSoundPlaying = false;
int movieAudioIndex;

bool MKVDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	warning("MKVDecoder::loadStream()");

	_fileStream = stream;

	_codec = new vpx_codec_ctx_t;
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

	warning("Number of tracks: %ld", j);

	enum {VIDEO_TRACK = 1, AUDIO_TRACK = 2};
	videoTrack = -1;
	audioTrack = -1;

	while (i != j) {
		const mkvparser::Track *const pTrack = pTracks->GetTrackByIndex(i++);

		if (pTrack == NULL)
			continue;

		const long long trackType = pTrack->GetType();
		//const unsigned long long trackUid = pTrack->GetUid();
		//const char* _trackName = pTrack->GetNameAsUTF8();

		if (trackType == mkvparser::Track::kVideo && videoTrack < 0) {
			videoTrack = pTrack->GetNumber();

			_videoTrack = new VPXVideoTrack(pTrack);

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

#if 0
	video_queue_init(&videoQ);
#endif

	const unsigned long clusterCount = pSegment->GetCount();

	if (clusterCount == 0) {
		error("Movie error: Segment has no clusters.\n");
	}

	/* Initialize video codec */
	if (vpx_codec_dec_init(_codec, &vpx_codec_vp8_dx_algo, NULL, 0))
		error("Failed to initialize decoder for movie.");

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

	return true;
}

void MKVDecoder::close() {
	VideoDecoder::close();

	delete _codec;
	delete _reader;
}

void MKVDecoder::readNextPacket() {
	warning("MKVDecoder::readNextPacket()");

	const mkvparser::Block *pBlock = pBlockEntry->GetBlock();
	long long trackNum = pBlock->GetTrackNumber();
	unsigned long tn = static_cast<unsigned long>(trackNum);
	const mkvparser::Track *pTrack = pTracks->GetTrackByNumber(tn);
	long long trackType = pTrack->GetType();
	int frameCount = pBlock->GetFrameCount();
	long long time_ns = pBlock->GetTime(_cluster);

	// First, let's get our frame
	while (_cluster != nullptr && !_cluster->EOS()) {
		if (frameCounter >= frameCount) {
			int res = _cluster->GetNext(pBlockEntry, pBlockEntry);

			if  ((res != -1) || pBlockEntry->EOS()) {
				_cluster = pSegment->GetNext(_cluster);
				if ((_cluster == NULL) || _cluster->EOS()) {
					_videoTrack->setEndOfVideo();
					break;
				}
				int ret = _cluster->GetFirst(pBlockEntry);

				if (ret == -1)
					error("MKVDecoder::readNextPacket(): GetFirst() failed");
			}
			pBlock  = pBlockEntry->GetBlock();
			trackNum = pBlock->GetTrackNumber();
			tn = static_cast<unsigned long>(trackNum);
			pTrack = pTracks->GetTrackByNumber(tn);
			trackType = pTrack->GetType();
			frameCount = pBlock->GetFrameCount();
			time_ns = pBlock->GetTime(_cluster);

			frameCounter = 0;
		}

		const mkvparser::Block::Frame &theFrame = pBlock->GetFrame(frameCounter);
		const long size = theFrame.len;
		//                const long long offset = theFrame.pos;

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

				unsigned int y;
#if 0
				GLubyte *ytex = NULL;
				GLubyte *utex = NULL;
				GLubyte *vtex = NULL;

				if (! ytex) {
					ytex = new GLubyte[img->d_w * img->d_h];
					utex = new GLubyte[(img->d_w >> 1) * (img->d_h >> 1)];
					vtex = new GLubyte[(img->d_w >> 1) * (img->d_h >> 1)];
					if (!ytex || !utex || !vtex)
						error("MKVDecoder: Out of memory"

				}

				unsigned char *buf =img->planes[0];
				for (y = 0; y < img->d_h; y++) {
					memcpy(ytex + y * img->d_w, buf, img->d_w);
					buf += img->stride[0];
				}
				buf = img->planes[1];
				for (y = 0; y < img->d_h >> 1; y++) {
					memcpy(utex + y * (img->d_w >> 1), buf, img->d_w >> 1);
					buf += img->stride[1];
				}
				buf = img->planes[2];
				for (y = 0; y < img->d_h >> 1; y++) {
					memcpy(vtex + y * (img->d_w >> 1), buf, img->d_w >> 1);
					buf += img->stride[2];
				}
				video_queue_put(&videoQ, ytex, utex, vtex,
								img->d_w, img->d_h, time_ns/1000000);
#endif


			}
		} else if (trackNum == audioTrack) {
			warning("MKVDecoder::readNextPacket(): audio track");

			if (size > 0) {
				ogg_packet oggPacket;

				theFrame.Read(_reader, frame);
				oggPacket.packet = frame;
				oggPacket.bytes = size;
				oggPacket.b_o_s = false;
				oggPacket.packetno++;
				oggPacket.granulepos = -1;

				_audioTrack->decodeSamples(oggPacket);
			}
		}
		++frameCounter;
	}

	// Then make sure we have enough audio buffered
	ensureAudioBufferSize();
}

MKVDecoder::VPXVideoTrack::VPXVideoTrack(const mkvparser::Track *const pTrack) {
	const mkvparser::VideoTrack *const pVideoTrack = static_cast<const mkvparser::VideoTrack *>(pTrack);

	const long long width = pVideoTrack->GetWidth();
	const long long height = pVideoTrack->GetHeight();

	const double rate = pVideoTrack->GetFrameRate();

	warning("VideoTrack: %lld x %lld @ %g fps", width, height, rate);

	_frameRate = 10; // FIXME

	_endOfVideo = false;
	_nextFrameStartTime = 0.0;
	_curFrame = -1;
}

MKVDecoder::VPXVideoTrack::~VPXVideoTrack() {
	_surface.free();
	_displaySurface.setPixels(0);
}

bool MKVDecoder::VPXVideoTrack::decodePacket(ogg_packet &_oggPacket) {
	if (th_decode_packetin(_theoraDecode, &_oggPacket, 0) == 0) {
		_curFrame++;

		// Convert YUV data to RGB data
		th_ycbcr_buffer yuv;
		th_decode_ycbcr_out(_theoraDecode, yuv);
		translateYUVtoRGBA(yuv);

		double time = th_granule_time(_theoraDecode, _oggPacket.granulepos);

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

MKVDecoder::VorbisAudioTrack::VorbisAudioTrack(const mkvparser::Track *const pTrack) :
		AudioTrack(Audio::Mixer::kPlainSoundType) {

	long long audioBitDepth;
	double audioSampleRate;
	vorbis_info vorbisInfo;
	vorbis_comment vorbisComment;

	const mkvparser::AudioTrack *const pAudioTrack = static_cast<const mkvparser::AudioTrack *>(pTrack);

	audioChannels = pAudioTrack->GetChannels();
	audioBitDepth = pAudioTrack->GetBitDepth();
	audioSampleRate = pAudioTrack->GetSamplingRate();

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
	vorbis_info_init(&vorbisInfo);
	vorbis_comment_init(&vorbisComment);
	memset(&vorbisDspState, 0, sizeof(vorbisDspState));
	memset(&vorbisBlock, 0, sizeof(vorbisBlock));

	ogg_packet oggPacket;

	oggPacket.e_o_s = false;
	oggPacket.granulepos = 0;
	oggPacket.packetno = 0;
	int r;
	for (int s = 0; s < 3; s++) {
		oggPacket.packet = p;
		oggPacket.bytes = sizes[s];
		oggPacket.b_o_s = oggPacket.packetno == 0;
		r = vorbis_synthesis_headerin(&vorbisInfo, &vorbisComment, &oggPacket);
		if (r)
			warning("vorbis_synthesis_headerin failed, error: %d", r);
		oggPacket.packetno++;
		p += sizes[s];
	}

	r = vorbis_synthesis_init(&vorbisDspState, &vorbisInfo);
	if (r)
		warning("vorbis_synthesis_init failed, error: %d", r);
	r = vorbis_block_init(&vorbisDspState, &vorbisBlock);
	if (r)
		warning("vorbis_block_init failed, error: %d", r);

#if 0
	ALenum audioFormat = alureGetSampleFormat(audioChannels, 16, 0);
	movieAudioIndex = initMovieSound(fileNumber, audioFormat, audioChannels, (ALuint) audioSampleRate, feedAudio);
#endif

	debug(1, "Movie sound inited.");
#if 0
	audio_queue_init(&audioQ);
#endif
	audioNsPerByte = (1000000000 / audioSampleRate) / (audioChannels * 2);
	audioNsBuffered = 0;
	audioBufferLen = audioChannels * audioSampleRate;


	_audStream = Audio::makeQueuingAudioStream(vorbisInfo.rate, vorbisInfo.channels != 1);

	_audioBufferFill = 0;
	_audioBuffer = 0;
	_endOfAudio = false;
}

MKVDecoder::VorbisAudioTrack::~VorbisAudioTrack() {
	vorbis_dsp_clear(&_vorbisDSP);
	vorbis_block_clear(&vorbisBlock);
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

bool MKVDecoder::VorbisAudioTrack::decodeSamples(ogg_packet &oggPacket) {
	if(!vorbis_synthesis(&vorbisBlock, &oggPacket) ) {
		if (vorbis_synthesis_blockin(&vorbisDspState, &vorbisBlock))
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
		long bytespersample=audioChannels * word;
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
		//audio_queue_put(&audioQ, buffer, audioBufferLen, time_ns / 1000000);

		//warning("Audio buffered: %lld byte %lld ns",audioBufferLen, audioNsPerByte*audioBufferLen);

		if (!movieSoundPlaying) {
			warning("** starting sound **");
			//playMovieStream(movieAudioIndex);
			movieSoundPlaying = true;
		}
	}

}

bool MKVDecoder::VorbisAudioTrack::hasAudio() const {
	return _audStream->numQueuedStreams() > 0;
}

bool MKVDecoder::VorbisAudioTrack::needsAudio() const {
	// TODO: 5 is very arbitrary. We probably should do something like QuickTime does.
	return !_endOfAudio && _audStream->numQueuedStreams() < 5;
}

void MKVDecoder::VorbisAudioTrack::synthesizePacket(ogg_packet &_oggPacket) {
	if (vorbis_synthesis(&vorbisBlock, &_oggPacket) == 0) // test for success
		vorbis_synthesis_blockin(&_vorbisDSP, &vorbisBlock);
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

#if 0
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
#endif

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
