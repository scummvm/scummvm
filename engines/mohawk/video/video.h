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

#ifndef MOHAWK_VIDEO_H
#define MOHAWK_VIDEO_H

#include "common/queue.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "graphics/pixelformat.h"
#include "graphics/video/codecs/codec.h"

namespace Mohawk {

class MohawkEngine;

struct MLSTRecord {
	uint16 index;
	uint16 movieID;
	uint16 code;
	uint16 left;
	uint16 top;
	uint16 u0[3];
	uint16 loop;
	uint16 volume;
	uint16 u1;

	bool enabled;
};

class QueuedAudioStream : public Audio::AudioStream {
public:
	QueuedAudioStream(int rate, int channels, bool autofree = true);
	~QueuedAudioStream();

	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return _channels == 2; }
	int getRate() const { return _rate; }
	bool endOfData() const { return _queue.empty(); }
	bool endOfStream() const { return _finished; }

	void queueAudioStream(Audio::AudioStream *audStream);
	void finish() { _finished = true; }

	uint32 streamsInQueue() { return _queue.size(); }

private:
	bool _autofree;
	bool _finished;
	int _rate;
	int _channels;

	Common::Queue<Audio::AudioStream*> _queue;
};

enum ScaleMode {
	kScaleNormal = 1,
	kScaleHalf = 2,
	kScaleQuarter = 4
};

class Video {
public:
	Video();
	virtual ~Video();

	virtual bool loadFile(Common::SeekableReadStream *stream) = 0;
	virtual void closeFile() = 0;
	void stop();
	void reset();

	Graphics::Surface *getNextFrame();
	virtual uint16 getWidth() = 0;
	virtual uint16 getHeight() = 0;
	virtual uint32 getFrameCount() = 0;

	virtual void updateAudioBuffer() {}
	void startAudio();
	void stopAudio();
	void pauseAudio();
	void resumeAudio();

	int32 getCurFrame() { return _curFrame; }
	void addPauseTime(uint32 p) { _lastFrameStart += p; _nextFrameStart += p; }
	bool needsUpdate();
	bool endOfVideo();

	virtual byte getBitsPerPixel() = 0;
	virtual byte *getPalette() { return NULL; }
	virtual uint32 getCodecTag() = 0;
	virtual ScaleMode getScaleMode() { return kScaleNormal; }

private:
	Graphics::Codec *_videoCodec;
	bool _noCodecFound;
	Graphics::Codec *createCodec(uint32 codecTag, byte bitsPerPixel);
	int32 _curFrame;
	uint32 _lastFrameStart, _nextFrameStart; // In 1/100 ms
	Audio::SoundHandle _audHandle;

	uint32 getFrameDuration() { return getFrameDuration(_curFrame); }

protected:
	virtual Common::SeekableReadStream *getNextFramePacket() = 0;
	virtual uint32 getFrameDuration(uint32 frame) = 0;
	virtual QueuedAudioStream *getAudioStream() = 0;
	virtual void resetInternal() {}
};

struct VideoEntry {
	Video *video;
	uint16 x;
	uint16 y;
	bool loop;
	Common::String filename;
	uint16 id; // Riven only

	Video *operator->() const { assert(video); return video; }
};

enum VideoType {
	kQuickTimeVideo,
	kBinkVideo
};

class VideoManager {
public:
	VideoManager(MohawkEngine *vm);
	~VideoManager();

	// Generic movie functions
	void playMovie(Common::String filename, uint16 x = 0, uint16 y = 0, bool clearScreen = false);
	void playMovieCentered(Common::String filename, bool clearScreen = true);
	void playBackgroundMovie(Common::String filename, int16 x = -1, int16 y = -1, bool loop = false);
	bool updateBackgroundMovies();
	void pauseVideos();
	void resumeVideos();
	void stopVideos();

	// Riven-related functions
	void activateMLST(uint16 mlstId, uint16 card);
	void enableMovie(uint16 id);
	void disableMovie(uint16 id);
	void disableAllMovies();
	void playMovie(uint16 id);
	void stopMovie(uint16 id);
	void playMovieBlocking(uint16 id);

	// Riven-related variables
	Common::Array<MLSTRecord> _mlstRecords;

private:
	MohawkEngine *_vm;

	void playMovie(VideoEntry videoEntry);
	Video *createVideo();

	// Keep tabs on any videos playing
	VideoType _videoType;
	Common::Array<VideoEntry> _videoStreams;
	uint32 _pauseStart;
};

} // End of namespace Mohawk

#endif
