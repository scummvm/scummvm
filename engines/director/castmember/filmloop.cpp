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

#include "common/stream.h"
#include "graphics/surface.h"
#include "graphics/macgui/macwidget.h"

#include "video/avi_decoder.h"
#include "video/qt_decoder.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/channel.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/window.h"
#include "director/castmember/bitmap.h"
#include "director/castmember/filmloop.h"

namespace Director {

FilmLoopCastMember::FilmLoopCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastFilmLoop;
	_looping = true;
	_enableSound = true;
	_crop = false;
	_center = false;
}

FilmLoopCastMember::~FilmLoopCastMember() {

}

bool FilmLoopCastMember::isModified() {
	if (_frames.size())
		return true;

	if (_initialRect.width() && _initialRect.height())
		return true;

	return false;
}

Common::Array<Channel> *FilmLoopCastMember::getSubChannels(Common::Rect &bbox, Channel *channel) {
	Common::Rect widgetRect(bbox.width() ? bbox.width() : _initialRect.width(), bbox.height() ? bbox.height() : _initialRect.height());

	_subchannels.clear();

	if (channel->_filmLoopFrame >= _frames.size()) {
		warning("Film loop frame %d requested, only %d available", channel->_filmLoopFrame, _frames.size());
		return &_subchannels;
	}

	// get the list of sprite IDs for this frame
	Common::Array<int> spriteIds;
	for (auto &iter : _frames[channel->_filmLoopFrame].sprites) {
		spriteIds.push_back(iter._key);
	}
	Common::sort(spriteIds.begin(), spriteIds.end());

	// copy the sprites in order to the list
	for (auto &iter : spriteIds) {
		Sprite src = _frames[channel->_filmLoopFrame].sprites[iter];
		if (!src._cast)
			continue;
		// translate sprite relative to the global bounding box
		int16 relX = (src._startPoint.x - _initialRect.left) * widgetRect.width() / _initialRect.width();
		int16 relY = (src._startPoint.y - _initialRect.top) * widgetRect.height() / _initialRect.height();
		int16 absX = relX + bbox.left;
		int16 absY = relY + bbox.top;
		int16 width = src._width * widgetRect.width() / _initialRect.width();
		int16 height = src._height * widgetRect.height() / _initialRect.height();

		// Film loop frames are constructed as a series of Channels, much like how a normal frame
		// is rendered by the Score. We don't include a pointer to the current Score here,
		// that's only for querying the constraint channel which is not used.
		Channel chan(nullptr, &src);
		chan._currentPoint = Common::Point(absX, absY);
		chan._width = width;
		chan._height = height;

		_subchannels.push_back(chan);

	}
	// Initialise the widgets on all of the subchannels.
	// This has to be done once the list has been constructed, otherwise
	// the list grow operation will erase the widgets as they aren't
	// part of the Channel assignment constructor.
	for (auto &iter : _subchannels) {
		iter.replaceWidget();
	}

	return &_subchannels;
}

void FilmLoopCastMember::loadFilmLoopDataD2(Common::SeekableReadStreamEndian &stream) {
	_initialRect = Common::Rect();
	_frames.clear();

	uint32 size = stream.readUint32BE();
	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "loadFilmLoopDataD2: SCVW body:");
		uint32 pos = stream.pos();
		stream.seek(0);
		stream.hexdump(size);
		stream.seek(pos);
	}
	uint16 channelSize = kSprChannelSizeD2;
	FilmLoopFrame newFrame;

	while (stream.pos() < size) {
		uint16 frameSize = stream.readUint16BE() - 2;
		if (debugChannelSet(5, kDebugLoading)) {
			debugC(5, kDebugLoading, "loadFilmLoopDataD2: Frame entry:");
			stream.hexdump(frameSize);
		}

		while (frameSize > 0) {
			int msgWidth = stream.readByte() * 2;
			int order = stream.readByte() * 2 - 0x20;
			frameSize -= 2;

			int channel = (order / channelSize) - 1;
			int channelOffset = order % channelSize;

			debugC(8, kDebugLoading, "loadFilmLoopDataD2: Message: msgWidth %d, channel %d, channelOffset %d", msgWidth, channel, channelOffset);
			if (debugChannelSet(8, kDebugLoading)) {
				stream.hexdump(msgWidth);
			}

			uint16 segSize = msgWidth;
			uint16 nextStart = (channel + 1) * kSprChannelSizeD2;

			while (segSize > 0) {
				Sprite sprite(nullptr);
				sprite._movie = g_director->getCurrentMovie();
				if (newFrame.sprites.contains(channel)) {
					sprite = newFrame.sprites.getVal(channel);
				}

				sprite._spriteType = kCastMemberSprite;
				sprite._puppet = 1;
				sprite._stretch = 1;

				uint16 needSize = MIN((uint16)(nextStart - channelOffset), segSize);
				int startPosition = stream.pos() - channelOffset;
				int finishPosition = stream.pos() + needSize;
				readSpriteDataD2(stream, sprite, startPosition, finishPosition);
				newFrame.sprites.setVal(channel, sprite);
				segSize -= needSize;
				channel += 1;
				channelOffset = 0;
				nextStart += kSprChannelSizeD2;
			}

			frameSize -= msgWidth;
		}

		for (auto &s : newFrame.sprites) {
			debugC(5, kDebugLoading, "loadFilmLoopDataD2: Sprite: channel %d, castId %s, bbox %d %d %d %d", s._key,
					s._value._castId.asString().c_str(), s._value._startPoint.x, s._value._startPoint.y,
					s._value._width, s._value._height);

			s._value.setCast(s._value._castId);
			Common::Point topLeft = s._value._startPoint;
			if (s._value._cast) {
				topLeft -= s._value._cast->getRegistrationOffset(s._value._width, s._value._height);
			}
			Common::Rect spriteBbox(
				topLeft.x,
				topLeft.y,
				topLeft.x + s._value._width,
				topLeft.y + s._value._height
			);
			if (!((spriteBbox.width() == 0) && (spriteBbox.height() == 0))) {
				if ((_initialRect.width() == 0) && (_initialRect.height() == 0)) {
					_initialRect = spriteBbox;
				} else {
					_initialRect.extend(spriteBbox);
				}
			}
			debugC(8, kDebugLoading, "loadFilmLoopDataD2: New bounding box: %d %d %d %d", _initialRect.left, _initialRect.top, _initialRect.width(), _initialRect.height());

		}

		_frames.push_back(newFrame);

	}
	debugC(5, kDebugLoading, "loadFilmLoopDataD2: Full bounding box: %d %d %d %d", _initialRect.left, _initialRect.top, _initialRect.width(), _initialRect.height());
}

void FilmLoopCastMember::loadFilmLoopDataD4(Common::SeekableReadStreamEndian &stream) {
	_initialRect = Common::Rect();
	_frames.clear();

	uint32 size = stream.readUint32BE();
	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "loadFilmLoopDataD4: SCVW body:");
		uint32 pos = stream.pos();
		stream.seek(0);
		stream.hexdump(size);
		stream.seek(pos);
	}
	uint32 framesOffset = stream.readUint32BE();
	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "loadFilmLoopDataD4: SCVW header:");
		stream.hexdump(framesOffset - 8);
	}
	stream.skip(6);
	uint16 channelSize = kSprChannelSizeD4;
	stream.readUint16BE(); // should be kSprChannelSizeD4 = 20!
	stream.skip(framesOffset - 16);

	FilmLoopFrame newFrame;

	while (stream.pos() < size) {
		uint16 frameSize = stream.readUint16BE() - 2;
		if (debugChannelSet(5, kDebugLoading)) {
			debugC(5, kDebugLoading, "loadFilmLoopDataD4: Frame entry:");
			stream.hexdump(frameSize);
		}

		while (frameSize > 0) {
			uint16 msgWidth = stream.readUint16BE();
			uint16 order = stream.readUint16BE();
			frameSize -= 4;

			int channel = (order / channelSize) - 1;
			int channelOffset = order % channelSize;

			debugC(8, kDebugLoading, "loadFilmLoopDataD4: Message: msgWidth %d, channel %d, channelOffset %d", msgWidth, channel, channelOffset);
			if (debugChannelSet(8, kDebugLoading)) {
				stream.hexdump(msgWidth);
			}

			uint16 segSize = msgWidth;
			uint16 nextStart = (channel + 1) * kSprChannelSizeD4;

			while (segSize > 0) {
				Sprite sprite(nullptr);
				sprite._movie = g_director->getCurrentMovie();
				if (newFrame.sprites.contains(channel)) {
					sprite = newFrame.sprites.getVal(channel);
				}

				sprite._puppet = 1;
				sprite._stretch = 1;

				uint16 needSize = MIN((uint16)(nextStart - channelOffset), segSize);
				int startPosition = stream.pos() - channelOffset;
				int finishPosition = stream.pos() + needSize;
				readSpriteDataD4(stream, sprite, startPosition, finishPosition);
				newFrame.sprites.setVal(channel, sprite);
				segSize -= needSize;
				channel += 1;
				channelOffset = 0;
				nextStart += kSprChannelSizeD4;
			}

			frameSize -= msgWidth;
		}

		for (auto &s : newFrame.sprites) {
			debugC(5, kDebugLoading, "loadFilmLoopDataD4: Sprite: channel %d, castId %s, bbox %d %d %d %d", s._key,
					s._value._castId.asString().c_str(), s._value._startPoint.x, s._value._startPoint.y,
					s._value._width, s._value._height);

			if (s._key == -1) {
				debugC(5, kDebugLoading, "loadFilmLoopDataD4: Skipping channel -1");
				if (s._value._startPoint.x != 0 || s._value._startPoint.y != 0 || s._value._width != 0 ||
						 (s._value._height != -256 && s._value._height != 0))
					warning("BUILDBOT: loadFilmLoopDataD4: Malformed VWSC resource: Sprite: channel %d, castId %s, bbox %d %d %d %d", s._key,
						s._value._castId.asString().c_str(), s._value._startPoint.x, s._value._startPoint.y,
						s._value._width, s._value._height);
				continue;
			}

			s._value.setCast(s._value._castId);
			Common::Point topLeft = s._value._startPoint;
			if (s._value._cast) {
				topLeft -= s._value._cast->getRegistrationOffset(s._value._width, s._value._height);
			}
			Common::Rect spriteBbox(
				topLeft.x,
				topLeft.y,
				topLeft.x + s._value._width,
				topLeft.y + s._value._height
			);
			if (!((spriteBbox.width() == 0) && (spriteBbox.height() == 0))) {
				if ((_initialRect.width() == 0) && (_initialRect.height() == 0)) {
					_initialRect = spriteBbox;
				} else {
					_initialRect.extend(spriteBbox);
				}
			}
			debugC(8, kDebugLoading, "loadFilmLoopDataD4: New bounding box: %d %d %d %d", _initialRect.left, _initialRect.top, _initialRect.width(), _initialRect.height());

		}

		_frames.push_back(newFrame);

	}
	debugC(5, kDebugLoading, "loadFilmLoopDataD4: Full bounding box: %d %d %d %d", _initialRect.left, _initialRect.top, _initialRect.width(), _initialRect.height());

}

Common::String FilmLoopCastMember::formatInfo() {
	return Common::String::format(
		"initialRect: %dx%d@%d,%d, boundingRect: %dx%d@%d,%d, frameCount: %d, subchannelCount: %d, enableSound: %d, looping: %d, crop: %d, center: %d",
		_initialRect.width(), _initialRect.height(),
		_initialRect.left, _initialRect.top,
		_boundingRect.width(), _boundingRect.height(),
		_boundingRect.left, _boundingRect.top,
		_frames.size(), _subchannels.size(), _enableSound, _looping,
		_crop, _center
	);
}

void FilmLoopCastMember::load() {
	if (_loaded)
		return;

	if (_cast->_version < kFileVer400) {
		// Director 3 and below should have a SCVW resource
		uint16 filmLoopId = _castId + _cast->_castIDoffset;
		uint32 tag = MKTAG('S', 'C', 'V', 'W');
		Common::SeekableReadStreamEndian *loop = _cast->getResource(tag, filmLoopId);
		if (loop) {
			debugC(2, kDebugLoading, "****** Loading '%s' id: %d, %d bytes", tag2str(tag), filmLoopId, (int)loop->size());
			loadFilmLoopDataD2(*loop);
			delete loop;
		} else {
			warning("FilmLoopCastMember::load(): Film loop not found");
		}
	} else if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer500) {
		if (_children.size() == 1) {
			uint16 filmLoopId = _children[0].index;
			uint32 tag = _children[0].tag;
			Common::SeekableReadStreamEndian *loop = _cast->getResource(tag, filmLoopId);
			if (loop) {
				debugC(2, kDebugLoading, "****** Loading '%s' id: %d, %d bytes", tag2str(tag), filmLoopId, (int)loop->size());
				loadFilmLoopDataD4(*loop);
				delete loop;
			} else {
				warning("FilmLoopCastMember::load(): Film loop not found");
			}
		} else {
			warning("FilmLoopCastMember::load(): Expected 1 child for film loop cast, got %d", _children.size());
		}
	} else {
		warning("STUB: FilmLoopCastMember::load(): Film loops not yet supported for version %d", _cast->_version);
	}

	_loaded = true;
}

void FilmLoopCastMember::unload() {
	// No unload necessary.
}

Common::Point FilmLoopCastMember::getRegistrationOffset() {
	return Common::Point(_initialRect.width() / 2, _initialRect.height() / 2);
}

Common::Point FilmLoopCastMember::getRegistrationOffset(int16 currentWidth, int16 currentHeight) {
	return Common::Point(currentWidth / 2, currentHeight / 2);
}

} // End of namespace Director
