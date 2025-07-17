/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef DIRECTOR_CASTMEMBER_MOVIE_H
#define DIRECTOR_CASTMEMBER_MOVIE_H

#include "director/castmember/filmloop.h"
#include "director/window.h"

namespace Director {

// For MovieCastMember
// virtual-window.cpp
class SubWindow : public Window {
public:
	SubWindow(Window *parent, Common::Rect rect);
	~SubWindow() = default;

	void setMainWindow() override;

private:
	Window *_parent;
	int offsetX;	// With respect to parent window origin
	int offsetY;	// With respect to parent window origin
	int width;
	int height;
};

class MovieCastMember : public CastMember {
public:
	MovieCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);
	MovieCastMember(Cast *cast, uint16 castId, MovieCastMember &source);

	CastMember *duplicate(Cast *cast, uint16 castId) override { return (CastMember *)(new MovieCastMember(cast, castId, *this)); }

	Common::Array<Channel> *getSubChannels(Common::Rect &bbox, uint frame) override;
	void load() override;

	bool hasField(int field) override;
	Datum getField(int field) override;
	bool setField(int field, const Datum &value) override;

	Common::String formatInfo() override;

	uint32 _flags;
	bool _enableScripts;

	bool _enableSound;
	bool _looping;
	bool _crop;
	bool _center;

	Common::Array<FilmLoopFrame> _frames;
	Common::Array<Channel> _subchannels;

private:
	Common::Path _filename;
	Archive *_archive;
	Movie *_movie;

	SubWindow *_window;
};

} // End of namespace Director

#endif
