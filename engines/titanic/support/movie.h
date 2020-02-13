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

#ifndef TITANIC_MOVIE_H
#define TITANIC_MOVIE_H

#include "common/list.h"
#include "video/video_decoder.h"
#include "titanic/core/list.h"
#include "titanic/core/resource_key.h"
#include "titanic/support/avi_surface.h"
#include "titanic/support/movie_range_info.h"

namespace Titanic {

class CGameObject;
class CMovie;
class CSoundManager;
class CVideoSurface;

class CMovieList : public List<CMovie> {
public:
};

class CMovie : public ListItem {
protected:
	/**
	 * Adds the movie to the list of currently playing movies
	 */
	void addToPlayingMovies();
public:
	bool _handled;
	bool _hasVideoFrame;
public:
	static CMovieList *_playingMovies;
	static CVideoSurface *_movieSurface;

	/**
	 * Initializes statics
	 */
	static void init();

	/**
	 * Deinitializes statics
	 */
	static void deinit();
public:
	CMovie();
	~CMovie() override;

	/**
	 * Starts playing the movie
	 */
	virtual void play(uint flags, CGameObject *obj) = 0;

	/**
	 * Starts playing the movie
	 */
	virtual void play(uint startFrame, uint endFrame, uint flags, CGameObject *obj) = 0;

	/**
	 * Starts playing the movie
	 */
	virtual void play(uint startFrame, uint endFrame, uint initialFrame, uint flags, CGameObject *obj) = 0;

	/**
	 * Plays a sub-section of a movie, and doesn't return until either
	 * the playback ends or a key has been pressed
	 * @returns		True if the cutscene was not interrupted
	 */
	virtual bool playCutscene(const Rect &drawRect, uint startFrame, uint endFrame) = 0;

	/**
	 * Pauses a movie
	 * @remarks	Acts a workaround for our video decoder, since some movies started
	 * as part of a scene load need to be paused until the scene is interactive,
	 * or else they get played back too quickly
	 */
	virtual void pause() = 0;

	/**
	 * Stops the movie
	 */
	virtual void stop() = 0;

	/**
	 * Add a playback event
	 */
	virtual void addEvent(int frameNumber, CGameObject *obj) = 0;

	/**
	 * Set the current frame number
	 */
	virtual void setFrame(uint frameNumber) = 0;

	/**
	 * Handle any pending movie events
	 */
	virtual bool handleEvents(CMovieEventList &events) = 0;

	/**
	 * Return any movie range info associated with the movie
	 */
	virtual const CMovieRangeInfoList *getMovieRangeInfo() const = 0;

	/**
	 * Set the sound manager reference
	 */
	virtual void setSoundManager(CSoundManager *soundManager) = 0;

	/**
	 * Get the current movie frame
	 */
	virtual int getFrame() const = 0;

	/**
	 * Set the frame rate for the movie
	 */
	virtual void setFrameRate(double rate) = 0;

	/**
	 * Sets whether the video is playing (versus paused)
	 */
	virtual void setPlaying(bool playingFlag) = 0;

	/**
	 * Creates a duplicate of the transparency surface
	 */
	virtual Graphics::ManagedSurface *duplicateTransparency() const = 0;

	/**
	 * Removes the movie from the list of currently playing movies
	 */
	void removeFromPlayingMovies();

	/**
	 * Returns true if the movie is currently active
	 */
	bool isActive() const;

	/**
	 * Returns true if there's a video frame
	 */
	bool hasVideoFrame();
};

class OSMovie : public CMovie {
private:
	AVISurface _aviSurface;
	CVideoSurface *_videoSurface;
	int _field18;
	int _field24;
	int _field28;
	int _field2C;
private:
	/**
	 * Called when a movie is started playing
	 */
	void movieStarted();
public:
	OSMovie(const CResourceKey &name, CVideoSurface *surface);
	~OSMovie() override;

	/**
	 * Starts playing the movie
	 */
	void play(uint flags, CGameObject *obj) override;

	/**
	 * Starts playing the movie
	 */
	void play(uint startFrame, uint endFrame, uint flags, CGameObject *obj) override;

	/**
	 * Starts playing the movie
	 */
	void play(uint startFrame, uint endFrame, uint initialFrame, uint flags, CGameObject *obj) override;

	/**
	 * Plays a sub-section of a movie, and doesn't return until either
	 * the playback ends or a key has been pressed
	 * @returns		True if the cutscene was not interrupted
	 */
	bool playCutscene(const Rect &drawRect, uint startFrame, uint endFrame) override;

	/**
	 * Pauses a movie
	 * @remarks		Acts a workaround for our video decoder, since some movies started
	 * as part of a scene load need to be paused until the scene is interactive,
	 * or else they get played back too quickly
	 */
	void pause() override;

	/**
	 * Stops the movie
	 */
	void stop() override;

	/**
	 * Add a playback event
	 */
	void addEvent(int eventId, CGameObject *obj) override;

	/**
	 * Set the current frame number
	 */
	void setFrame(uint frameNumber) override;

	/**
	 * Handle any pending movie events
	 */
	bool handleEvents(CMovieEventList &events) override;

	/**
	 * Get the current frame number
	 */
	int getFrame() const override;

	/**
	 * Return any movie range info associated with the movie
	 */
	const CMovieRangeInfoList *getMovieRangeInfo() const override;

	/**
	 * Set the sound manager reference
	 */
	void setSoundManager(CSoundManager *soundManager) override;

	/**
	 * Set the frame rate for the movie
	 */
	void setFrameRate(double rate) override;

	/**
	 * Sets whether the video is playing (versus paused)
	 */
	void setPlaying(bool playingFlag) override;

	/**
	 * Creates a duplicate of the transparency surface
	 */
	Graphics::ManagedSurface *duplicateTransparency() const override;
};

} // End of namespace Titanic

#endif /* TITANIC_MOVIE_H */
