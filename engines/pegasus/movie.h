/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_MOVIE_H
#define PEGASUS_MOVIE_H

#include "common/str.h"

#include "pegasus/elements.h"
#include "pegasus/surface.h"

namespace Video {
class VideoDecoder;
}

namespace Pegasus {

class Movie : public Animation, public PixelImage {
public:
	Movie(const DisplayElementID);
	~Movie() override;

	virtual void initFromMovieFile(const Common::String &fileName, bool transparent = false);

	bool isMovieValid() { return _video != 0; }

	virtual void releaseMovie();

	void draw(const Common::Rect &) override;
	virtual void redrawMovieWorld();

	void setTime(const TimeValue, const TimeScale = 0) override;

	void setRate(const Common::Rational) override;

	void start() override;
	void stop() override;
	void resume() override;
	void pause() override;

	virtual void moveMovieBoxTo(const CoordType, const CoordType);

	void setStop(const TimeValue, const TimeScale = 0) override;

	TimeValue getDuration(const TimeScale = 0) const override;

	// *** HACK ALERT
	Video::VideoDecoder *getMovie() { return _video; }
	void setVolume(uint16);

protected:
	void updateTime() override;

	Video::VideoDecoder *_video;
	Common::Rect _movieBox;
};

class GlowingMovie : public Movie {
public:
	GlowingMovie(DisplayElementID);
	~GlowingMovie() override {}

	void draw(const Common::Rect &) override;

	void setBounds(const Common::Rect &) override;

	void setGlowing(const bool glowing) { _glowing = glowing; }

protected:
	bool _glowing;
};

class ScalingMovie : public GlowingMovie {
public:
	ScalingMovie(DisplayElementID);
	~ScalingMovie() override {}

	void draw(const Common::Rect &) override;
};

} // End of namespace Pegasus

#endif
