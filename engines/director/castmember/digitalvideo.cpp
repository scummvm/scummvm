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

#include "graphics/surface.h"
#include "graphics/macgui/macwidget.h"

#include "video/avi_decoder.h"
#include "video/qt_decoder.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/channel.h"
#include "director/movie.h"
#include "director/window.h"
#include "director/castmember/digitalvideo.h"
#include "director/lingo/lingo-the.h"

namespace Director {

DigitalVideoCastMember::DigitalVideoCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastDigitalVideo;
	_video = nullptr;
	_lastFrame = nullptr;
	_channel = nullptr;

	_getFirstFrame = false;
	_duration = 0;

	_initialRect = Movie::readRect(stream);
	_vflags = stream.readUint32();
	_frameRate = (_vflags >> 24) & 0xff;

	_frameRateType = kFrameRateDefault;
	if (_vflags & 0x0800) {
		_frameRateType = (FrameRateType)((_vflags & 0x3000) >> 12);
	}
	_qtmovie = _vflags & 0x8000;
	_avimovie = _vflags & 0x4000;
	_preload = _vflags & 0x0400;
	_enableVideo = !(_vflags & 0x0200);
	_pausedAtStart = _vflags & 0x0100;
	_showControls = _vflags & 0x40;
	_directToStage = _vflags & 0x20;
	_looping = _vflags & 0x10;
	_enableSound = _vflags & 0x08;
	_crop = !(_vflags & 0x02);
	_center = _vflags & 0x01;

	if (debugChannelSet(2, kDebugLoading))
		_initialRect.debugPrint(2, "DigitalVideoCastMember(): rect:");

	debugC(2, kDebugLoading, "DigitalVideoCastMember(): flags: (%d 0x%04x)", _vflags, _vflags);

	debugC(2, kDebugLoading, "_frameRate: %d", _frameRate);
	debugC(2, kDebugLoading, "_frameRateType: %d, _preload: %d, _enableVideo %d, _pausedAtStart %d",
			_frameRateType, _preload, _enableVideo, _pausedAtStart);
	debugC(2, kDebugLoading, "_showControls: %d, _looping: %d, _enableSound: %d, _crop %d, _center: %d, _directToStage: %d",
			_showControls, _looping, _enableSound, _crop, _center, _directToStage);
	debugC(2, kDebugLoading, "_avimovie: %d, _qtmovie: %d", _avimovie, _qtmovie);
}

DigitalVideoCastMember::~DigitalVideoCastMember() {
	if (_lastFrame) {
		_lastFrame->free();
		delete _lastFrame;
	}

	if (_video)
		delete _video;
}

bool DigitalVideoCastMember::loadVideo(Common::String path) {
	// TODO: detect file type (AVI, QuickTime, FLIC) based on magic number,
	// insert the right video decoder

	if (_video)
		delete _video;

	_filename = path;
	_video = new Video::QuickTimeDecoder();

	Common::Path location = findPath(path);
	if (location.empty()) {
		warning("DigitalVideoCastMember::loadVideo(): unable to resolve path %s", path.c_str());
		return false;
	}

	debugC(2, kDebugLoading | kDebugImages, "Loading video %s -> %s", path.c_str(), location.toString().c_str());
	bool result = _video->loadFile(location);
	if (!result) {
		delete _video;
		_video = new Video::AVIDecoder();
		result = _video->loadFile(location);
		if (!result) {
		    warning("DigitalVideoCastMember::loadVideo(): format not supported, skipping");
		    delete _video;
		    _video = nullptr;
		}
	}

	if (result && g_director->_pixelformat.bytesPerPixel == 1) {
		// Director supports playing back RGB and paletted video in 256 colour mode.
		// In both cases they are dithered to match the Director palette.
		byte palette[256 * 3];
		g_system->getPaletteManager()->grabPalette(palette, 0, 256);
		_video->setDitheringPalette(palette);
	}

	return result;
}

bool DigitalVideoCastMember::isModified() {
	if (!_video || !_video->isVideoLoaded())
		return true;

	if (_getFirstFrame)
		return true;

	if (_channel->_movieRate == 0.0)
		return false;

	return _video->needsUpdate();
}

void DigitalVideoCastMember::startVideo(Channel *channel) {
	_channel = channel;

	if (!_video || !_video->isVideoLoaded()) {
		warning("DigitalVideoCastMember::startVideo: No video %s", !_video ? "decoder" : "loaded");
		return;
	}

	if (_pausedAtStart) {
		_getFirstFrame = true;
	} else {
		if (_channel->_movieRate == 0.0)
			_channel->_movieRate = 1.0;
	}

	if (_video->isPlaying())
		_video->rewind();
	else
		_video->start();

	debugC(2, kDebugImages, "STARTING VIDEO %s", _filename.c_str());

	if (_channel->_stopTime == 0)
		_channel->_stopTime = getMovieTotalTime();

	_duration = getMovieTotalTime();
}

void DigitalVideoCastMember::stopVideo() {
	if (!_video || !_video->isVideoLoaded()) {
		warning("DigitalVideoCastMember::stopVideo: No video decoder");
		return;
	}

	_video->stop();

	debugC(2, kDebugImages, "STOPPING VIDEO %s", _filename.c_str());
}

void DigitalVideoCastMember::rewindVideo() {
	if (!_video || !_video->isVideoLoaded()) {
		warning("DigitalVideoCastMember::rewindVideo: No video decoder");
		return;
	}

	_video->rewind();

	debugC(2, kDebugImages, "REWINDING VIDEO %s", _filename.c_str());
}

Graphics::MacWidget *DigitalVideoCastMember::createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) {
	Graphics::MacWidget *widget = new Graphics::MacWidget(g_director->getCurrentWindow(), bbox.left, bbox.top, bbox.width(), bbox.height(), g_director->_wm, false);

	_channel = channel;

	if (!_video || !_video->isVideoLoaded()) {
		warning("DigitalVideoCastMember::createWidget: No video decoder");
		delete widget;

		return nullptr;
	}

	// Do not render stopped videos
	if (_channel->_movieRate == 0.0 && !_getFirstFrame && _lastFrame) {
		widget->getSurface()->blitFrom(*_lastFrame);

		return widget;
	}

	const Graphics::Surface *frame = _video->decodeNextFrame();

	debugC(1, kDebugImages, "Video time: %d  rate: %f", _channel->_movieTime, _channel->_movieRate);

	if (frame) {
		if (_lastFrame) {
			_lastFrame->free();
			delete _lastFrame;
		}

		_lastFrame = frame->convertTo(g_director->_pixelformat, g_director->getPalette());
	}
	if (_lastFrame)
		widget->getSurface()->blitFrom(*_lastFrame);

	if (_getFirstFrame) {
		_video->stop();
		_getFirstFrame = false;
	}

	if (_video->endOfVideo()) {
		if (_looping) {
			_video->rewind();
		} else {
			_channel->_movieRate = 0.0;
		}
	}

	return widget;
}

uint DigitalVideoCastMember::getDuration() {
	if (!_video || !_video->isVideoLoaded()) {
		Common::String path = getCast()->getVideoPath(_castId);
		if (!path.empty())
			loadVideo(path);

		_duration = getMovieTotalTime();
	}
	return _duration;
}

uint DigitalVideoCastMember::getMovieCurrentTime() {
	if (!_video)
		return 0;

	int stamp = MIN<int>(_video->getTime() * 60 / 1000, getMovieTotalTime());

	return stamp;
}

uint DigitalVideoCastMember::getMovieTotalTime() {
	if (!_video)
		return 0;

	int stamp = _video->getDuration().msecs() * 60 / 1000;

	return stamp;
}

void DigitalVideoCastMember::seekMovie(int stamp) {
	if (!_video)
		return;

	_channel->_startTime = stamp;

	Audio::Timestamp dur = _video->getDuration();

	_video->seek(Audio::Timestamp(_channel->_startTime * 1000 / 60, dur.framerate()));
}

void DigitalVideoCastMember::setStopTime(int stamp) {
	if (!_video)
		return;

	_channel->_stopTime = stamp;

	Audio::Timestamp dur = _video->getDuration();

	_video->setEndTime(Audio::Timestamp(_channel->_stopTime * 1000 / 60, dur.framerate()));
}

void DigitalVideoCastMember::setMovieRate(double rate) {
	if (!_video)
		return;

	_channel->_movieRate = rate;

	if (rate < 0.0)
		warning("STUB: DigitalVideoCastMember::setMovieRate(%g)", rate);
	else
		_video->setRate(Common::Rational((int)(rate * 100.0), 100));

	if (_video->endOfVideo())
		_video->rewind();
}

void DigitalVideoCastMember::setFrameRate(int rate) {
	if (!_video)
		return;

	warning("STUB: DigitalVideoCastMember::setFrameRate(%d)", rate);
}

Common::String DigitalVideoCastMember::formatInfo() {
	return Common::String::format(
		"initialRect: %dx%d@%d,%d, boundingRect: %dx%d@%d,%d, filename: \"%s\", duration: %d, enableVideo: %d, enableSound: %d, looping: %d, crop: %d, center: %d, showControls: %d",
		_initialRect.width(), _initialRect.height(),
		_initialRect.left, _initialRect.top,
		_boundingRect.width(), _boundingRect.height(),
		_boundingRect.left, _boundingRect.top,
		_filename.c_str(), _duration,
		_enableVideo, _enableSound,
		_looping, _crop, _center, _showControls
	);
}

Common::Point DigitalVideoCastMember::getRegistrationOffset() {
	return Common::Point(_initialRect.width() / 2, _initialRect.height() / 2);
}

Common::Point DigitalVideoCastMember::getRegistrationOffset(int16 width, int16 height) {
	return Common::Point(width / 2, height / 2);
}

bool DigitalVideoCastMember::hasField(int field) {
	switch (field) {
	case kTheCenter:
	case kTheController:
	case kTheCrop:
	case kTheDirectToStage:
	case kTheDuration:
	case kTheFrameRate:
	case kTheLoop:
	case kTheMovieRate:
	case kTheMovieTime:
	case kThePausedAtStart:
	case kThePreLoad:
	case kTheSound:
	case kTheVideo:
	case kTheVolume:
		return true;
	default:
		break;
	}
	return CastMember::hasField(field);
}

Datum DigitalVideoCastMember::getField(int field) {
	Datum d;

	switch (field) {
	case kTheCenter:
		d = _center;
		break;
	case kTheController:
		d = _showControls;
		break;
	case kTheCrop:
		d = _crop;
		break;
	case kTheDirectToStage:
		d = _directToStage;
		break;
	case kTheDuration:
		// sometimes, we will get duration before we start video.
		// _duration is initialized in startVideo, thus we will not get the correct number.
		d = (int)getDuration();
		break;
	case kTheFrameRate:
		d = _frameRate;
		break;
	case kTheLoop:
		d = _looping;
		break;
	case kThePausedAtStart:
		d = _pausedAtStart;
		break;
	case kThePreLoad:
		d = _preload;
		break;
	case kTheSound:
		d = _enableSound;
		break;
	case kTheVideo:
		d = _enableVideo;
		break;
	default:
		d = CastMember::getField(field);
	}

	return d;
}

bool DigitalVideoCastMember::setField(int field, const Datum &d) {
	switch (field) {
	case kTheCenter:
		_center = (bool)d.asInt();
		return true;
	case kTheController:
		_showControls = (bool)d.asInt();
		return true;
	case kTheCrop:
		_crop = (bool)d.asInt();
		return true;
	case kTheDirectToStage:
		_directToStage = (bool)d.asInt();
		return true;
	case kTheDuration:
		warning("DigitalVideoCastMember::setField(): Attempt to set read-only field %s of cast %d", g_lingo->entity2str(field), _castId);
		return false;
	case kTheFrameRate:
		_frameRate = d.asInt();
		setFrameRate(d.asInt());
		return true;
	case kTheLoop:
		_looping = (bool)d.asInt();
		if (_looping && _channel && _channel->_movieRate == 0.0) {
			setMovieRate(1.0);
		}
		return true;
	case kThePausedAtStart:
		_pausedAtStart = (bool)d.asInt();
		return true;
	case kThePreLoad:
		_preload = (bool)d.asInt();
		return true;
	case kTheSound:
		_enableSound = (bool)d.asInt();
		return true;
	case kTheVideo:
		_enableVideo = (bool)d.asInt();
		return true;
	default:
		break;
	}

	return CastMember::setField(field, d);
}

} // End of namespace Director
