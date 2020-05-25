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

#include "sludge/movie.h"
#include "sludge/newfatal.h"
#include "sludge/sound.h"
#include "sludge/timing.h"

namespace Sludge {

// sound_openal.cpp
void playMovieStream(int a);
#if 0
int initMovieSound(int f, ALenum format, int audioChannels, ALuint samplerate,
		ALuint(*callback)(void *userdata, ALubyte *data, ALuint bytes));
#endif

MovieStates movieIsPlaying = nothing;

int movieIsEnding = 0;

float movieAspect = 1.6F;
#if 0
typedef struct audioBuffers {
	char *buffer;
	uint size;
	audioBuffers *next;
	Uint32 time_ms;
}audioBuffers;

typedef struct audioQueue {
	audioBuffers *first, *last;
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;
}audioQueue;

audioQueue audioQ;

Uint32 movieStartTick, movieCurrentTime;

long long audioNsPerByte;
long long audioNsPlayed;
long long audioNsBuffered;
long long audioBufferLen;
bool movieSoundPlaying = false;
int movieAudioIndex;
GLuint yTextureName = 0;
GLuint uTextureName = 0;
GLuint vTextureName = 0;

typedef struct videoBuffers {
	GLubyte *ytex;
	GLubyte *utex;
	GLubyte *vtex;
	videoBuffers *next;
	GLsizei w, h;
	Uint32 time_ms;
}videoBuffers;

typedef struct videoQueue {
	videoBuffers *first, *last;
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;
}videoQueue;

videoQueue videoQ;

void audio_queue_init(audioQueue *q) {
	memset(q, 0, sizeof(audioQueue));

	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();

}
int audio_queue_put(audioQueue *q, char *buffer, uint size, long long time_ms) {

	audioBuffers *audioBuf = new audioBuffers;
	if (!audioBuf)
	return -1;
	audioBuf->buffer = buffer;
	audioBuf->next = NULL;
	audioBuf->size = size;
	audioBuf->time_ms = time_ms;

	SDL_LockMutex(q->mutex);

	if (!q->last)
	q->first = audioBuf;
	else
	q->last->next = audioBuf;
	q->last = audioBuf;
	q->size ++;
	SDL_CondSignal(q->cond);

	SDL_UnlockMutex(q->mutex);

	return 0;
}
inline static int audio_queue_get(audioQueue *q, char **buffer) {
	int ret = 0;

	audioBuffers *audioBuf;

	SDL_LockMutex(q->mutex);

	audioBuf = q->first;
	if (audioBuf) {
		// Synch video timer to audio
		Uint32 tick = SDL_GetTicks() + 100;
		if (ABS((long int)((tick - movieStartTick) - (audioBuf->time_ms))) > 300) {
			movieStartTick = tick - audioBuf->time_ms;
		}

		q->first = audioBuf->next;
		if (!q->first)
		q->last = NULL;
		q->size--;
		*buffer = audioBuf->buffer;
		ret = audioBuf->size;
		delete audioBuf;
	}

	SDL_UnlockMutex(q->mutex);

	return ret;
}

void video_queue_init(videoQueue *q) {
	memset(q, 0, sizeof(videoQueue));
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}
int video_queue_put(videoQueue *q, GLubyte *ytex,
		GLubyte *utex,
		GLubyte *vtex,
		GLsizei w, GLsizei h,
		long long time_ms) {

	videoBuffers *videoBuf = new videoBuffers;
	if (!videoBuf)
	return -1;
	videoBuf->ytex = ytex;
	videoBuf->utex = utex;
	videoBuf->vtex = vtex;
	videoBuf->next = NULL;
	videoBuf->w = w;
	videoBuf->h = h;
	videoBuf->time_ms = time_ms;

	SDL_LockMutex(q->mutex);

	if (!q->last)
	q->first = videoBuf;
	else
	q->last->next = videoBuf;
	q->last = videoBuf;
	q->size ++;
	SDL_CondSignal(q->cond);

	SDL_UnlockMutex(q->mutex);
	return 0;
}
inline static int video_queue_get(videoQueue *q,
		GLubyte **ytex,
		GLubyte **utex,
		GLubyte **vtex,
		GLsizei *w, GLsizei *h) {
	videoBuffers *videoBuf;
	int ret = 0;

	SDL_LockMutex(q->mutex);

	videoBuf = q->first;
	if (videoBuf) {
		q->first = videoBuf->next;
		if (!q->first)
		q->last = NULL;
		q->size--;
		*ytex = videoBuf->ytex;
		*utex = videoBuf->utex;
		*vtex = videoBuf->vtex;
		*w = videoBuf->w;
		*h = videoBuf->h;
		ret = 1;
		delete videoBuf;
	}

	SDL_UnlockMutex(q->mutex);

	return ret;
}

#if 0
static void die_codec(vpx_codec_ctx_t *ctx, const char *s) {
	//const char *detail = vpx_codec_error_detail(ctx);
	fatal(s, vpx_codec_error(ctx));
}
#endif

void setMovieViewport() {
	float realAspect = (float) realWinWidth / realWinHeight;

	int vpHeight, vpWidth, vpOffsetX, vpOffsetY;
	if (realAspect > movieAspect) {
		vpHeight = realWinHeight;
		vpWidth = (int)(realWinHeight * movieAspect);
		vpOffsetY = 0;
		vpOffsetX = (realWinWidth - vpWidth) / 2;
	} else {
		vpWidth = realWinWidth;
		vpHeight = (int)((float) realWinWidth / movieAspect);
		vpOffsetY = (realWinHeight - vpHeight) / 2;
		vpOffsetX = 0;
	}
#if 0
	glViewport(vpOffsetX, vpOffsetY, vpWidth, vpHeight);
#endif
	const GLfloat bPMVMatrix[] = {
		2.0f / 640.f, .0, .0, .0,
		.0, -2.0f / 400.f, .0, .0,
		.0, .0, 1.0f, .0,
		-1.0, 1.0f, .0, 1.0f

	};
	for (int i = 0; i < 16; i++) {
		aPMVMatrix[i] = bPMVMatrix[i];
	}
}

static uint64_t xiph_lace_value(byte **np) {
	uint64_t lace;
	uint64_t value;
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
long long audioChannels;

bool fakeAudio = false;

// send audio to audio device...
ALuint feedAudio(void *userdata, ALubyte *data, ALuint length) {
	static char *buffer = NULL;
	static uint bufOffset = 0;
	static uint bufSize = 0;

	ALuint got = 0;
	int bufLen;

	if (! buffer) {
		bufSize = audio_queue_get(&audioQ, &buffer);
		bufOffset = 0;
		if (bufSize <= 0) {
			bufSize = 0;
			buffer = NULL;
			if (! got) {
				got = audioChannels * 2;
				memset(data, 0, got);
				fprintf(stderr, "Faking audio...\n");
				fakeAudio = true;
			}
//				SDL_CondSignal(audioQ.cond);
			return got;
		}
	}

	fakeAudio = false;

	if (length > bufSize - bufOffset)
	bufLen = bufSize - bufOffset;
	else
	bufLen = length;

	memcpy(data, buffer + bufOffset, bufLen);

	bufOffset += bufLen;
	length -= bufLen;
	got += bufLen;

	if (bufSize <= bufOffset) {
		buffer = NULL;
		delete [] buffer;
	}
//	fprintf (stderr, "Sending %d bytes of audio.\n", got);

	return got;
}
#endif

int playMovie(int fileNumber) {
#if 0
	if (specialSettings & SPECIAL_SILENT)
	return 0;

	if (movieIsPlaying) return 0;

	movieSoundPlaying = false;

	vpx_codec_ctx_t codec;

	float pausefade = 1.0;

	using namespace mkvparser;

	MkvReader reader;

	if (reader.Open(fileNumber)) {
		warning(ERROR_MOVIE_ODDNESS);
		return 0;
	}

	long long pos = 0;

	EBMLHeader ebmlHeader;

	ebmlHeader.Parse(&reader, pos);

	mkvparser::Segment *pSegment;

	long long ret = mkvparser::Segment::CreateInstance(&reader, pos, pSegment);
	if (ret) {
		fatal("Movie error: Segment::CreateInstance() failed.\n");
	}

	ret = pSegment->Load();
	if (ret < 0) {
		fatal("Movie error: Segment::Load() failed.\n");
	}

	//const SegmentInfo* const pSegmentInfo = pSegment->GetInfo();
	//const long long timeCodeScale = pSegmentInfo->GetTimeCodeScale();
	//const long long duration_ns = pSegmentInfo->GetDuration();
	//const char* const pTitle = pSegmentInfo->GetTitleAsUTF8();
	//const char* const pMuxingApp = pSegmentInfo->GetMuxingAppAsUTF8();
	//const char* const pWritingApp = pSegmentInfo->GetWritingAppAsUTF8();

	const mkvparser::Tracks *pTracks = pSegment->GetTracks();

	unsigned long i = 0;
	const unsigned long j = pTracks->GetTracksCount();

	enum {VIDEO_TRACK = 1, AUDIO_TRACK = 2};
	int videoTrack = -1;
	int audioTrack = -1;
	long long audioBitDepth;
	double audioSampleRate;
	ogg_packet oggPacket;
	vorbis_info vorbisInfo;
	vorbis_comment vorbisComment;
	vorbis_block vorbisBlock;

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

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

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

	const GLfloat texCoords[] = {
		0.0, 0.0,
		1.0, 0.0,
		0.0, 1.0,
		1.0, 1.0
	};

	const GLfloat vertices[] = {
		0.0, 0.0, 0.1,
		640.0, 0.0, 0.1,
		0.0, 400.0, 0.1,
		640.0, 400.0, 0.1
	};

	const GLfloat vertices1[] = {
		7.0, 7.0, 0.1,
		17.0, 7.0, 0.1,
		7.0, 29.0, 0.1,
		17.0, 29.0, 0.1
	};

	const GLfloat vertices2[] = {
		27.0, 7.0, 0.1,
		37.0, 7.0, 0.1,
		27.0, 29.0, 0.1,
		37.0, 29.0, 0.1
	};

	const GLfloat vertices3[] = {
		5.0, 5.0, 0.1,
		15.0, 5.0, 0.1,
		5.0, 27.0, 0.1,
		15.0, 27.0, 0.1
	};

	const GLfloat vertices4[] = {
		25.0, 5.0, 0.1,
		35.0, 5.0, 0.1,
		25.0, 27.0, 0.1,
		35.0, 27.0, 0.1
	};

	int frameCounter = 0;

	movieStartTick = SDL_GetTicks();

	while (movieIsPlaying) {

		checkInput();
		if (weAreDoneSoQuit)
		break;
		handleInput();

		if (movieIsPlaying && (! movieIsEnding) && (videoQ.size < 100 || audioQ.size < 100)) {
			// Decode a frame!

			if ((pCluster != NULL) && !pCluster->EOS()) {

				if (frameCounter >= frameCount) {

					pBlockEntry = pCluster->GetNext(pBlockEntry);
					if ((pBlockEntry == NULL) || pBlockEntry->EOS()) {
						pCluster = pSegment->GetNext(pCluster);
						if ((pCluster == NULL) || pCluster->EOS()) {
							goto movieHasEnded;
						}
						pBlockEntry = pCluster->GetFirst();
					}
					pBlock = pBlockEntry->GetBlock();
					trackNum = pBlock->GetTrackNumber();
					tn = static_cast<unsigned long>(trackNum);
					pTrack = pTracks->GetTrackByNumber(tn);
					trackType = pTrack->GetType();
					frameCount = pBlock->GetFrameCount();
					time_ns = pBlock->GetTime(pCluster);

					frameCounter = 0;
				}

				const Block::Frame &theFrame = pBlock->GetFrame(frameCounter);
				const long size = theFrame.len;
				//                const long long offset = theFrame.pos;

				if (size > sizeof(frame)) {
					if (frame) delete [] frame;
					frame = new byte[size];
					if (! checkNew(frame)) return 0;
				}
				/*
				 fprintf (stderr, "Block :%s,%s,%15lld\n",
				 (trackType == VIDEO_TRACK) ? "V" : "A",
				 pBlock->IsKey() ? "I" : "P",
				 time_ns);
				 */

				if (trackNum == videoTrack) {

					theFrame.Read(&reader, frame);

					/* Decode the frame */
					if (vpx_codec_decode(&codec, frame, size, NULL, 0))
					die_codec(&codec, "Failed to decode frame");

					// Let's decode an image frame!
					vpx_codec_iter_t iter = NULL;
					vpx_image_t *img;
					/* Get frame data */
					while ((img = vpx_codec_get_frame(&codec, &iter))) {
						if (img->fmt != VPX_IMG_FMT_I420)
						fatal("Movie error. The movie is not in I420 colour format, which is the only one I can hanlde at the moment.");

						uint y;

						GLubyte *ytex = NULL;
						GLubyte *utex = NULL;
						GLubyte *vtex = NULL;

						if (! ytex) {
							ytex = new GLubyte [img->d_w * img->d_h];
							utex = new GLubyte [(img->d_w >> 1) * (img->d_h >> 1)];
							vtex = new GLubyte [(img->d_w >> 1) * (img->d_h >> 1)];
							if (!ytex || !utex || !vtex)
							fatal(ERROR_OUT_OF_MEMORY);

						}

						byte *buf = img->planes[0];
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
								img->d_w, img->d_h, time_ns / 1000000);

					}

				} else if (trackNum == audioTrack) {
					// Use this Audio Track
					if (size > 0) {
						theFrame.Read(&reader, frame);
						oggPacket.packet = frame;
						oggPacket.bytes = size;
						oggPacket.b_o_s = false;
						oggPacket.packetno++;
						oggPacket.granulepos = -1;
						if (! vorbis_synthesis(&vorbisBlock, &oggPacket)) {
							if (vorbis_synthesis_blockin(&vorbisDspState, &vorbisBlock))
							fprintf(stderr, "Vorbis Synthesis block in error.\n");

						} else {
							fprintf(stderr, "Vorbis Synthesis error.\n");
						}

						float **pcm;

						int numSamples = vorbis_synthesis_pcmout(&vorbisDspState, &pcm);

						if (numSamples > 0) {
							int word = 2;
							int sgned = 1;
							int i, j;
							long bytespersample = audioChannels * word;
							vorbis_fpu_control fpu;

							char *buffer = new char[bytespersample * numSamples];
							if (! checkNew(buffer)) return false;

							/* a tight loop to pack each size */
							{
								int val;
								if (word == 1) {
									int off = (sgned ? 0 : 128);
									vorbis_fpu_setround(&fpu);
									for (j = 0; j < numSamples; j++)
									for (i = 0; i < audioChannels; i++) {
										val = vorbis_ftoi(pcm[i][j] * 128.f);
										if (val > 127)val = 127;
										else if (val < -128)val = -128;
										*buffer++ = val + off;
									}
									vorbis_fpu_restore(fpu);
								} else {
									int off = (sgned ? 0 : 32768);

									if (sgned) {

										vorbis_fpu_setround(&fpu);
										for (i = 0; i < audioChannels; i++) { /* It's faster in this order */
											float *src = pcm[i];
											int16 *dest = ((int16 *)buffer) + i;
											for (j = 0; j < numSamples; j++) {
												val = vorbis_ftoi(src[j] * 32768.f);
												if (val > 32767)val = 32767;
												else if (val < -32768)val = -32768;
												*dest = val;
												dest += audioChannels;
											}
										}
										vorbis_fpu_restore(fpu);

									} else {

										vorbis_fpu_setround(&fpu);
										for (i = 0; i < audioChannels; i++) {
											float *src = pcm[i];
											int16 *dest = ((int16 *)buffer) + i;
											for (j = 0; j < numSamples; j++) {
												val = vorbis_ftoi(src[j] * 32768.f);
												if (val > 32767)val = 32767;
												else if (val < -32768)val = -32768;
												*dest = val + off;
												dest += audioChannels;
											}
										}
										vorbis_fpu_restore(fpu);

									}

								}
							}

							vorbis_synthesis_read(&vorbisDspState, numSamples);
							audioBufferLen = bytespersample * numSamples;
							audio_queue_put(&audioQ, buffer, audioBufferLen, time_ns / 1000000);

							//fprintf (stderr, "Audio buffered: %lld byte %lld ns\n",audioBufferLen, audioNsPerByte*audioBufferLen);

							if (! movieSoundPlaying && size > 1) {
								fprintf(stderr, "** starting sound ** \n");
								playMovieStream(movieAudioIndex);
								movieSoundPlaying = true;
							}
						}
					}
				}
				frameCounter++;

			} else {
				movieHasEnded:
				movieIsEnding = 1;
			}
		}

		bool videoUpdated = false;
		// Get a video frame.
		if (movieIsPlaying == playing) {

			videoBuffers *vB;
			// Do we have decoded video waiting?
			if (vB = videoQ.first) {
				Uint32 tick = SDL_GetTicks() - movieStartTick;

				// Is it time to display the frame yet?
				if ((tick + 1) < vB->time_ms) {
					SDL_Delay(1);
				} else {
					GLubyte *ytex = NULL;
					GLubyte *utex = NULL;
					GLubyte *vtex = NULL;
					GLsizei w, h;

					if (video_queue_get(&videoQ, &ytex, &utex, &vtex, &w, &h)) {

						if (! yTextureName) glGenTextures(1, &yTextureName);
						if (! uTextureName) glGenTextures(1, &uTextureName);
						if (! vTextureName) glGenTextures(1, &vTextureName);
						glBindTexture(GL_TEXTURE_2D, yTextureName);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0,
								GL_ALPHA, GL_UNSIGNED_BYTE, ytex);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glBindTexture(GL_TEXTURE_2D, uTextureName);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w >> 1, h >> 1, 0,
								GL_ALPHA, GL_UNSIGNED_BYTE, utex);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glBindTexture(GL_TEXTURE_2D, vTextureName);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w >> 1, h >> 1, 0,
								GL_ALPHA, GL_UNSIGNED_BYTE, vtex);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

						glBindTexture(GL_TEXTURE_2D, yTextureName);
						glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h,
								GL_ALPHA, GL_UNSIGNED_BYTE, ytex);
						glBindTexture(GL_TEXTURE_2D, uTextureName);
						glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w >> 1, h >> 1,
								GL_ALPHA, GL_UNSIGNED_BYTE, utex);
						glBindTexture(GL_TEXTURE_2D, vTextureName);
						glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w >> 1, h >> 1,
								GL_ALPHA, GL_UNSIGNED_BYTE, vtex);

						delete [] ytex;
						delete [] utex;
						delete [] vtex;
						ytex = utex = vtex = NULL;
						videoUpdated = true;

					}
				}
			} else if (movieIsEnding) {
				// We have reached the end of the movie.
				movieIsPlaying = nothing;
			}
		}

		// Update the screen if there's new video, or if we're paused
		if (videoUpdated || movieIsPlaying == paused) {
			// Clear The Screen
			glClear(GL_COLOR_BUFFER_BIT);

			// Display the current frame here
			if (shader.yuv) {
				glUseProgram(shader.yuv);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, uTextureName);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, vTextureName);
				glActiveTexture(GL_TEXTURE0);
			}
			glBindTexture(GL_TEXTURE_2D, yTextureName);
			glEnable(GL_BLEND);
			//glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			setPMVMatrix(shader.yuv);
			drawQuad(shader.yuv, vertices, 1, texCoords);

			glUseProgram(0);

			if (movieIsPlaying == paused) {
				pausefade -= 1.0 / 24;
				if (pausefade < -1.0) pausefade = 1.0;

				// Paused.
				glEnable(GL_BLEND);

				glUseProgram(shader.color);

				setPMVMatrix(shader.color);
				setPrimaryColor(0.0f, 0.0f, 0.0f, fabs(pausefade));
				drawQuad(shader.color, vertices1, 0);
				drawQuad(shader.color, vertices2, 0);
				setPrimaryColor(1.0f, 1.0f, 1.0f, fabs(pausefade));
				drawQuad(shader.color, vertices3, 0);
				drawQuad(shader.color, vertices4, 0);

				glUseProgram(0);

				glDisable(GL_BLEND);

				Wait_Frame();
			}

			glFlush();

			EGL_SwapBuffers();

		}
		videoUpdated = false;
	}

	// Cleanup
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);

	movieIsPlaying = nothing;
	for (int i = 0; i < 10; i++)
		Wait_Frame();
	huntKillFreeSound(fileNumber);

	if (vpx_codec_destroy(&codec))
	die_codec(&codec, "Failed to destroy codec");

	vorbis_dsp_clear(&vorbisDspState);
	vorbis_block_clear(&vorbisBlock);
	vorbis_comment_clear(&vorbisComment);
	vorbis_info_clear(&vorbisInfo);
	delete pSegment;
	deleteTextures(1, &yTextureName);
	deleteTextures(1, &uTextureName);
	deleteTextures(1, &vTextureName);
	yTextureName = uTextureName = vTextureName = 0;

	// Delete any remaining buffers
	videoBuffers *vB = videoQ.first;
	while (vB = videoQ.first) {
		videoQ.first = vB->next;
		delete [] vB->ytex;
		delete [] vB->utex;
		delete [] vB->vtex;
		delete vB;
	}
	videoQ.size = 0;

	audioBuffers *aB = audioQ.first;
	while (aB = audioQ.first) {
		audioQ.first = aB->next;
		delete [] aB->buffer;
		delete aB;
	}
	audioQ.size = 0;

	Init_Timer();

	glViewport(viewportOffsetX, viewportOffsetY, viewportWidth, viewportHeight);

	setPixelCoords(false);
#endif
	return 0;
}

int stopMovie() {
	int r = movieIsPlaying;
	movieIsPlaying = nothing;
	return r;
}

int pauseMovie() {
#if 0
	if (movieIsPlaying == playing) {
		ALuint source = getSoundSource(movieAudioIndex);
		if (source) {

			alurePauseSource(source);

		}
		movieIsPlaying = paused;
		fprintf(stderr, "** Pausing **\n");
	} else if (movieIsPlaying == paused) {
		ALuint source = getSoundSource(movieAudioIndex);
		if (source) {

			alureResumeSource(source);

		}
		fprintf(stderr, "** Restarted movie ** sound: %d source: %d\n", movieSoundPlaying, source);
		movieIsPlaying = playing;
	}
#endif
	return movieIsPlaying;
}

} // End of namespace Sludge
