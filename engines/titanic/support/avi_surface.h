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

#ifndef TITANIC_AVI_SURFACE_H
#define TITANIC_AVI_SURFACE_H

#include "video/avi_decoder.h"
#include "graphics/managed_surface.h"
#include "titanic/core/resource_key.h"
#include "titanic/support/movie_range_info.h"

namespace Titanic {

class CSoundManager;
class CVideoSurface;

enum MovieFlag {
	MOVIE_REPEAT = 1, MOVIE_STOP_PREVIOUS = 2, MOVIE_NOTIFY_OBJECT = 4,
	MOVIE_REVERSE = 8, MOVIE_GAMESTATE = 0x10
};

class AVIDecoder : public Video::AVIDecoder {
public:
	AVIDecoder(Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType) :
		Video::AVIDecoder(soundType) {}
	AVIDecoder(const Common::Rational &frameRateOverride, Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType) :
		Video::AVIDecoder(frameRateOverride, soundType) {}

	/**
	 * Returns the number of video tracks the decoder has
	 */
	uint videoTrackCount() const { return _videoTracks.size(); }

	/**
	 * Returns the specified video track
	 */
	Video::AVIDecoder::AVIVideoTrack &getVideoTrack(uint idx);
};

class AVISurface {
private:
	AVIDecoder *_decoder;
	CVideoSurface *_videoSurface;
	CMovieRangeInfoList _movieRangeInfo;
	int _streamCount;
	Graphics::ManagedSurface *_movieFrameSurface[2];
	Graphics::ManagedSurface *_framePixels;
	bool _isReversed;
	int _currentFrame;
	uint32 _priorFrameTime;
	Common::String _movieName;
private:
	/**
	 * Render a frame to the video surface
	 */
	bool renderFrame();

	/**
	 * Sets up for video decompression
	 */
	void setupDecompressor();

	/**
	 * Copys a movie frame into a local 16-bit frame surface
	 * @param src	Source raw movie frame
	 * @param dest	Destination 16-bit copy of the frame
	 * @remarks		The important thing this methods different from a straight
	 * copy is that any pixels marked as fully transparent are replaced with
	 * the special transparent color value.
	 */
	void copyMovieFrame(const Graphics::Surface &src, Graphics::ManagedSurface &dest);
protected:
	/**
	 * Start playback at the specified frame
	 */
	bool startAtFrame(int frameNumber);

	/**
	 * Sets whether the movie is playing in reverse
	 */
	void setReversed(bool isReversed);

	/**
	 * Seeks to a given frame number in the video
	 */
	virtual void seekToFrame(uint frameNumber);
public:
	CSoundManager *_soundManager;
	bool _hasAudio;
	double _frameRate;
public:
	AVISurface(const CResourceKey &key);
	virtual ~AVISurface();

	/**
	 * Start playing the loaded AVI video
	 */
	virtual bool play(uint flags, CGameObject *obj);

	/**
	 * Start playing the loaded AVI video
	 */
	virtual bool play(int startFrame, int endFrame, uint flags, CGameObject *obj);

	/**
	 * Start playing the loaded AVI video
	 */
	virtual bool play(int startFrame, int endFrame, int initialFrame, uint flags, CGameObject *obj);

	/**
	 * Stop the currently playing video
	 */
	virtual void stop();

	/**
	 * Pauses video playback
	 */
	virtual void pause();

	/**
	 * Resumes the video if it's paused
	 */
	virtual void resume();

	/**
	 * Return true if a video is currently playing
	 */
	virtual bool isPlaying() const {
		return _decoder->isPlaying();
	}

	/**
	 * Handle any movie events relevent for the frame
	 */
	virtual bool handleEvents(CMovieEventList &events);

	/**
	 * Set the video surface the AVI Surface will render on
	 */
	void setVideoSurface(CVideoSurface *surface);

	/**
	 * Get the width of the video
	 */
	uint getWidth() const;

	/**
	 * Get the height of the video
	 */
	uint getHeight() const;

	/**
	 * Set the current frame
	 */
	void setFrame(int frameNumber);

	/**
	 * Gets the current frame
	 */
	int getFrame() const { return _currentFrame; }

	/**
	 * Add a movie event
	 */
	bool addEvent(int frameNumber, CGameObject *obj);

	/**
	 * Set the frame rate
	 */
	void setFrameRate(double rate);

	/**
	 * Returns the surface for the secondary video track frame, if present
	 */
	Graphics::ManagedSurface *getSecondarySurface();

	/**
	 * Get a reference to the movie range info list
	 */
	const CMovieRangeInfoList *getMovieRangeInfo() const {
		return &_movieRangeInfo;
	}

	/**
	 * Duplicates the transparency mask for the frame, if the movie includes it
	 */
	Graphics::ManagedSurface *duplicateTransparency() const;

	/**
	 * Returns true if it's time for the next
	 */
	bool isNextFrame();

	/**
	 * Plays an interruptable cutscene
	 */
	void playCutscene(const Rect &r, uint startFrame, uint endFrame);

	/**
	 * Returns the pixel depth of the movie in bits
	 */
	uint getBitDepth() const;
};

} // End of namespace Titanic

#endif /* TITANIC_AVI_SURFACE_H */
