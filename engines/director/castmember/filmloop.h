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

#ifndef DIRECTOR_CASTMEMBER_FILMLOOP_H
#define DIRECTOR_CASTMEMBER_FILMLOOP_H

#include "director/castmember/castmember.h"

namespace Director {

class Sprite;

struct FilmLoopFrame {
	Common::HashMap<int, Sprite> sprites;
};

class FilmLoopCastMember : public CastMember {
public:
	FilmLoopCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);
	~FilmLoopCastMember();

	bool isModified() override;
	//Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;

	Common::Array<Channel> *getSubChannels(Common::Rect &bbox, Channel *channel);

	void loadFilmLoopData(Common::SeekableReadStreamEndian &stream);
	void loadFilmLoopDataV4(Common::SeekableReadStreamEndian &stream);

	Common::String formatInfo() override;

	void load() override;
	void unload() override;

	Common::Point getRegistrationOffset() override;
	Common::Point getRegistrationOffset(int16 width, int16 height) override;

	bool _enableSound;
	bool _looping;
	bool _crop;
	bool _center;

	Common::Array<FilmLoopFrame> _frames;
	Common::Array<Channel> _subchannels;
};

} // End of namespace Director

#endif
