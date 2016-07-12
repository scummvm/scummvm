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
#include "titanic/core/resource_key.h"
#include "titanic/support/movie_range_info.h"

namespace Titanic {

class CSoundManager;
class CVideoSurface;

enum MovieFlag {
	MOVIE_1 = 1, MOVIE_STOP_PREVIOUS = 2, MOVIE_NO_OBJECT = 4,
	MOVIE_REVERSE = 8, MOVIE_GAMESTATE = 0x10
};

class AVIDecoder : public Video::AVIDecoder {
public:
	AVIDecoder(Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType, SelectTrackFn trackFn = nullptr) :
		Video::AVIDecoder(soundType, trackFn) {}
	AVIDecoder(const Common::Rational &frameRateOverride, Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType,
		SelectTrackFn trackFn = nullptr) : Video::AVIDecoder(frameRateOverride, soundType, trackFn) {}
	
	Video::AVIDecoder::AVIVideoTrack &getVideoTrack();
};

class AVISurface {
private:
	AVIDecoder *_decoders[2];
	CVideoSurface *_videoSurface;
	int _currentPos;
	int _priorFrame;
	CMovieRangeInfoList _movieRangeInfo;
	int _streamCount;
	CVideoSurface *_movieFrameSurface[2];
private:
	/**
	 * Render a frame to the video surface
	 */
	bool renderFrame();

	/**
	 * Sets up for video decompression
	 */
	void setupDecompressor();
protected:
	/**
	 * Change the frame with ??? checking
	 */
	virtual bool changeFrame(int frameNumber);

	/**
	 * Seeks to a given frame number in the video
	 */
	virtual void seekToFrame(uint frameNumber);
public:
	CSoundManager *_soundManager;
	bool _hasAudio;
	bool _isPlaying;
	double _frameRate;
public:
	AVISurface(const CResourceKey &key);
	~AVISurface();

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
	int getFrame() const;

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
	CVideoSurface *getSecondarySurface();

	/**
	 * Get a reference to the movie range info list
	 */
	const CMovieRangeInfoList *getMovieRangeInfo() const {
		return &_movieRangeInfo;
	}

	/**
	 * Duplicates the secondary frame, if the movie has a second video track
	 */
	CVideoSurface *duplicateSecondaryFrame() const;
};

} // End of namespace Titanic

#endif /* TITANIC_AVI_SURFACE_H */
