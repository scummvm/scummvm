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

#include "common/memstream.h"
#include "graphics/surface.h"
#include "graphics/macgui/macwidget.h"

#include "video/avi_decoder.h"
#include "video/qt_decoder.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/channel.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sprite.h"
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
	_index = -1;
	_score = nullptr;

	if (cast->_version >= kFileVer400) {
		_initialRect = Movie::readRect(stream);
		uint32 flags = stream.readUint32BE();
		uint16 unk1 = stream.readUint16BE();
		_looping = flags & 32 ? 0 : 1;
		_enableSound = flags & 8 ? 1 : 0;
		_crop = flags & 2 ? 0 : 1;
		_center = flags & 1 ? 1 : 0;

		debugC(5, kDebugLoading, "FilmLoopCastMember::FilmLoopCastMember(): flags: %d, unk1: %d, looping: %d, enableSound: %d, crop: %d, center: %d", flags, unk1, _looping, _enableSound, _crop, _center);
	}
}

FilmLoopCastMember::FilmLoopCastMember(Cast *cast, uint16 castId, FilmLoopCastMember &source)
		: CastMember(cast, castId) {
	_type = kCastFilmLoop;
	// force a load so we can copy the cast resource information
	source.load();
	_loaded = true;

	_initialRect = source._initialRect;
	_boundingRect = source._boundingRect;
	if (cast == source._cast)
		_children = source._children;

	_enableSound = source._enableSound;
	_crop = source._crop;
	_center = source._center;
	_score = source._score;
	_subchannels = source._subchannels;
	_looping = source._looping;
}

FilmLoopCastMember::~FilmLoopCastMember() {

}

bool FilmLoopCastMember::isModified() {
	if (_score->_scoreCache.size())
		return true;

	if (_initialRect.width() && _initialRect.height())
		return true;

	return false;
}

Common::Array<Channel> *FilmLoopCastMember::getSubChannels(Common::Rect &bbox, uint frame) {
	Common::Rect widgetRect(bbox.width() ? bbox.width() : _initialRect.width(), bbox.height() ? bbox.height() : _initialRect.height());

	_subchannels.clear();

	if (frame >= _score->_scoreCache.size()) {
		warning("FilmLoopCastMember::getSubChannels(): Film loop frame %d requested, only %d available", frame, _score->_scoreCache.size());
		return &_subchannels;
	}

	// get the list of sprite IDs for this frame
	Common::Array<int> spriteIds;
	for (uint i = 0; i < _score->_channels.size(); ++i) {
		if (_score->_channels[i]->_sprite && !_score->_channels[i]->_sprite->_castId.isNull())
			spriteIds.push_back(i);
	}

	debugC(5, kDebugImages, "FilmLoopCastMember::getSubChannels(): castId: %d, frame: %d, count: %d, initRect: %d,%d %dx%d, bbox: %d,%d %dx%d",
			_castId, frame, spriteIds.size(),
			_initialRect.left + _initialRect.width()/2,
			_initialRect.top + _initialRect.height()/2,
			_initialRect.width(), _initialRect.height(),
			bbox.left + bbox.width()/2,
			bbox.top + bbox.height()/2,
			bbox.width(), bbox.height());

	bool needToScale = (bbox.width() != _initialRect.width() || bbox.height() != _initialRect.height());
	float scaleX = 1.0f;
	float scaleY = 1.0f;

	if (needToScale) {
		scaleX = (float)bbox.width() / _initialRect.width();
		scaleY = (float)bbox.height() / _initialRect.height();
	}

	// copy the sprites in order to the list
	for (auto &iter : spriteIds) {
		Sprite src = *_score->_channels[iter]->_sprite;
		if (src._castId.isNull())
			continue;

		debugCN(5, kDebugImages, "FilmLoopCastMember::getSubChannels(): sprite: %d - cast: %s, orig: %d,%d %dx%d",
				iter, src._castId.asString().c_str(),
				src._startPoint.x, src._startPoint.y, src._width, src._height);

		// translate sprite relative to the global bounding box
		if (needToScale) {
			src._startPoint.x = (src._startPoint.x - _initialRect.left) * scaleX + bbox.left;
			src._startPoint.y = (src._startPoint.y - _initialRect.top) * scaleY + bbox.top;
			src._width = widgetRect.width();
			src._height = widgetRect.height();
			src._stretch = true;

			debugCN(5, kDebugImages, ", scaled: %d,%d %dx%d", src._startPoint.x, src._startPoint.y, src._width, src._height);
		} else {
			src._startPoint.x = (src._startPoint.x - _initialRect.left) + bbox.left;
			src._startPoint.y = (src._startPoint.y - _initialRect.top) + bbox.top;

			debugCN(5, kDebugImages, ", no scaling");
		}

		// Film loop frames are constructed as a series of Channels, much like how a normal frame
		// is rendered by the Score. We don't include a pointer to the current Score here,
		// that's only for querying the constraint channel which is not used.
		Channel chan(nullptr, &src);
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

CastMemberID FilmLoopCastMember::getSubChannelSound1(uint frame) {
	if (frame >= _score->_scoreCache.size()) {
		warning("FilmLoopCastMember::getSubChannelSound1(): Film loop frame %d requested, only %d available", frame, _score->_scoreCache.size());
		return CastMemberID();
	}

	return _score->_scoreCache[frame]->_mainChannels.sound1;
}

CastMemberID FilmLoopCastMember::getSubChannelSound2(uint frame) {
	if (frame >= _score->_scoreCache.size()) {
		warning("FilmLoopCastMember::getSubChannelSound2(): Film loop frame %d requested, only %d available", frame, _score->_scoreCache.size());
		return CastMemberID();
	}

	return _score->_scoreCache[frame]->_mainChannels.sound2;
}

Common::String FilmLoopCastMember::formatInfo() {
	return Common::String::format(
		"initialRect: %dx%d@%d,%d, boundingRect: %dx%d@%d,%d, frameCount: %d, subchannelCount: %d, enableSound: %d, looping: %d, crop: %d, center: %d",
		_initialRect.width(), _initialRect.height(),
		_initialRect.left, _initialRect.top,
		_boundingRect.width(), _boundingRect.height(),
		_boundingRect.left, _boundingRect.top,
		_score ? _score->_scoreCache.size() : -1, _score ? _subchannels.size() : -1, _enableSound, _looping,
		_crop, _center
	);
}

void FilmLoopCastMember::load() {
	if (_loaded)
		return;

	Common::SeekableReadStreamEndian *loop = nullptr;
	uint16 filmLoopId = 0;
		uint32 tag = 0;

	if (_cast->_version < kFileVer400) {
		// Director 3 and below should have a SCVW resource
		filmLoopId = _castId + _cast->_castIDoffset;
		tag = MKTAG('S', 'C', 'V', 'W');
		loop = _cast->getResource(tag, filmLoopId);
	} else if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer700) {
		for (auto &it : _children) {
			if (it.tag == MKTAG('S', 'C', 'V', 'W')) {
				filmLoopId = it.index;
				tag = it.tag;
				loop = _cast->getResource(tag, filmLoopId);
				break;
			} else {
				debugC(5, kDebugLoading, "FilmLoopCastMember::load(): Ignoring child with tag '%s' id: %d", tag2str(it.tag), it.index);
			}
		}
	} else {
		warning("STUB: FilmLoopCastMember::load(): Film loops not yet supported for version v%d (%d)", humanVersion(_cast->_version), _cast->_version);
	}

	if (loop) {
		debugC(2, kDebugLoading, "****** FilmLoopCastMember::load(): Loading '%s' id: %d, %d bytes", tag2str(tag), filmLoopId, (int)loop->size());
		_score = new Score(g_director->getCurrentMovie(), false);
		_score->loadFrames(*loop, _cast->_version, true);
		delete loop;
	} else {
		warning("FilmLoopCastMember::load(): Film loop not found");
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

uint32 FilmLoopCastMember::getCastDataSize() {
	// We're only reading the _initialRect and _vflags from the Cast Data
	// _initialRect : 8 bytes + flags : 4 bytes + 2 bytes unk1 + 2 bytes (castType and _flags1 (see Cast::loadCastData() for Director 4 only)
	if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer500) {
		// It has been observed that the FilmCastMember has _flags as 0x00
		return 8 + 4 + 2 + 2;
	} else if (_cast->_version >= kFileVer500 && _cast->_version < kFileVer600) {
		return 8 + 4 + 2;
	}

	warning("FilmLoopCastMember::getCastDataSize(): unhandled or invalid cast version: %d", _cast->_version);
	return 0;
}

void FilmLoopCastMember::writeCastData(Common::SeekableWriteStream *writeStream) {
	Movie::writeRect(writeStream, _initialRect);

	uint32 flags = 0;
	if (_cast->_version >= kFileVer400) {
		flags |= (_looping) ? 0 : 32;
		flags |= (_enableSound) ? 8 : 0;
		flags |= (_crop) ? 0 : 2;
		flags |= (_center) ? 1 : 0;
	}

	writeStream->writeUint32LE(flags);
	writeStream->writeUint16LE(0);		// May need to save proper value in the future, currently ignored
}

void FilmLoopCastMember::writeSCVWResource(Common::SeekableWriteStream *writeStream, uint32 offset) {
	// Load it before writing
	if (!_loaded) {
		load();
	}

	uint32 channelSize = 0;
	if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer500) {
		channelSize = kSprChannelSizeD4;
	} else if (_cast->_version >= kFileVer500 && _cast->_version < kFileVer600) {
		channelSize = kSprChannelSizeD5;
	} else {
		warning("FilmLoopCastMember::writeSCVWResource: Writing Director Version 6+ not supported yet");
		return;
	}

	// Go to the desired offset put in the memory map
	writeStream->seek(offset);

	uint32 filmloopSize = getSCVWResourceSize();
	debugC(5, kDebugSaving, "FilmLoopCastmember::writeSCVWResource: Saving FilmLoop 'SCVW' data of size: %d", filmloopSize);

	writeStream->writeUint32LE(MKTAG('S', 'C', 'V', 'W'));
	writeStream->writeUint32LE(filmloopSize);	// Size of the resource

	writeStream->writeUint32BE(filmloopSize);

	uint32 frameOffset = 20;							// Should be greater than 20
	writeStream->writeUint32BE(frameOffset);			// framesOffset
	writeStream->seek(6, SEEK_CUR);						// Ignored data
	writeStream->writeUint16BE(channelSize);
	writeStream->seek(frameOffset - 16, SEEK_CUR);				// Ignored data

	// The structure of the filmloop 'SCVW' data is as follows
	// The 'SCVW' tag -> the size of the resource ->
	// frameoffset (This offset is where the frame date actually starts) ->
	// Some headers which we ignore except the Sprite Channel Size (which we also ignore during loading) ->

	// until there are no more frames
		// size of the frame ->
		// until there are no more channels in the frame
			// width of message (One chunk of data) (This is the size of data for the sprite that needs to be read) ->
			// order of message (this order tells us the channel we're reading) ->
			// 1-20 bytes of Sprite data

	for (Frame *frame : _score->_scoreCache) {
		writeStream->writeUint16BE(frame->_sprites.size() * (channelSize + 4) + 2);					// Frame Size

		for (uint channel = 0; channel < frame->_sprites.size(); ++channel) {
			// TODO: For now writing the order considering that each sprite will have 20 bytes of data
			// In the future, for optimization, we can actually calculate the data of each sprite
			// And write the order accordingly
			// But for this we'll need a way to find how many data values (out of 20) of a sprite are valid, i.e. determine message width
			// this means while loading, the channelOffset will always be 0, order will always be multiple of 20
			// And message width will always be 20
			// Channel indexes start with 0
			writeStream->writeUint16BE(channelSize);						// message width
			writeStream->writeUint16BE(channel * channelSize);

			Sprite *sprite = frame->_sprites[channel];

			if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer500) {
				writeSpriteDataD4(writeStream, *sprite);
			} else if (_cast->_version >= kFileVer500 && _cast->_version < kFileVer600) {
				writeSpriteDataD5(writeStream, *sprite);
			}
		}

	}

	if (debugChannelSet(7, kDebugSaving)) {
		// Adding +8 because the stream doesn't include the header and the entry for the size itself
		byte *dumpData = (byte *)calloc(filmloopSize + 8, sizeof(byte));

		Common::SeekableMemoryWriteStream *dumpStream = new Common::SeekableMemoryWriteStream(dumpData, filmloopSize + 8);

		uint32 currentPos = writeStream->pos();
		writeStream->seek(offset);
		dumpStream->write(writeStream, filmloopSize + 8);
		writeStream->seek(currentPos);

		dumpFile("FilmLoopData", 0, MKTAG('V', 'W', 'C', 'F'), dumpData, filmloopSize);
		free(dumpData);
		delete dumpStream;
	}
}

uint32 FilmLoopCastMember::getSCVWResourceSize() {
	uint32 channelSize = 0;
	if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer500) {
		channelSize = kSprChannelSizeD4;
	} else if (_cast->_version >= kFileVer500) {
		channelSize = kSprChannelSizeD5;
	} else {
		warning("FilmLoopCastMember::getSCVWResourceSize: Director version unsupported");
	}

	uint32 framesSize = 0;
	for (Frame *frame : _score->_scoreCache) {
		// Frame size
		framesSize += 2;

		// message width: 2 bytes
		// order: 2 bytes
		// Sprite data: 20 bytes
		framesSize += (2 + 2 + channelSize) * frame->_sprites.size();
	}

	// Size: 4 bytes
	// frameoffset: 4 bytes
	// Header (Ignored data): 16 bytes
	return 4 + 4 + 16 + framesSize;
}

} // End of namespace Director
