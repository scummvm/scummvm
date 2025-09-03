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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */


#include "engines/wintermute/video/video_player.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/gfx/base_surface.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/dcgf.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
VideoPlayer::VideoPlayer(BaseGame *inGame) : BaseClass(inGame) {
	setDefaults();
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::setDefaults() {
	_playing = false;

	_aviDecoder = nullptr;

	_playPosX = _playPosY = 0;
	_playZoom = 0.0f;

	_texture = nullptr;
	_videoFrameReady = false;
	_playbackStarted = false;
	_freezeGame = false;

	_filename.clear();

	_subtitler = nullptr;
	_foundSubtitles = false;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
VideoPlayer::~VideoPlayer() {
	cleanup();
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::cleanup() {
	_playing = false;

	SAFE_DELETE(_subtitler);
	if (_aviDecoder) {
		_aviDecoder->close();
	}
	SAFE_DELETE(_aviDecoder);
	SAFE_DELETE(_texture);

	return setDefaults();
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::initialize(const Common::String &filename, const Common::String &subtitleFile) {
	cleanup();

	if (BaseEngine::instance().getGameId() == "sof1" ||
	    BaseEngine::instance().getGameId() == "sof2") {
		warning("PlayVideo: %s - Xvid support not implemented yet", filename.c_str());
		return STATUS_FAILED;
	}

	_filename = filename;

	// Load a file, but avoid having the File-manager handle the disposal of it.
	Common::SeekableReadStream *file = BaseFileManager::getEngineInstance()->openFile(filename, true, false);
	if (!file) {
		return STATUS_FAILED;
	}

	_aviDecoder = new Video::AVIDecoder();
	_aviDecoder->loadStream(file);

	_subtitler = new VideoSubtitler(_game);
	_foundSubtitles = _subtitler->loadSubtitles(_filename, subtitleFile);

	if (!_aviDecoder->isVideoLoaded()) {
		return STATUS_FAILED;
	}

	// Additional setup.
	_texture = _game->_renderer->createSurface();
	_texture->create(_aviDecoder->getWidth(), _aviDecoder->getHeight());
	_playZoom = 100;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::update() {
	if (!isPlaying()) {
		return STATUS_OK;
	}

	if (_playbackStarted) {
		return STATUS_OK;
	}

	if (_playbackStarted && !_freezeGame && _game->_state == GAME_FROZEN) {
		return STATUS_OK;
	}

	if (_subtitler && _foundSubtitles && _game->_subtitles) {
		_subtitler->update(_aviDecoder->getCurFrame());
	}

	if (_aviDecoder->endOfVideo()) {
		_playbackStarted = false;
		if (_freezeGame) {
			_game->unfreeze();
		}
	}
	if (!_aviDecoder->endOfVideo() && _aviDecoder->getTimeToNextFrame() == 0) {
		const Graphics::Surface *decodedFrame = _aviDecoder->decodeNextFrame();
		if (decodedFrame && _texture) {
			_texture->putSurface(*decodedFrame, false);
			_videoFrameReady = true;
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::display() {
	Common::Rect32 rc;
	bool res;

	if (_texture && _videoFrameReady) {
		BasePlatform::setRect(&rc, 0, 0, _texture->getWidth(), _texture->getHeight());
		if (_playZoom == 100.0f) {
			res = _texture->display(_playPosX, _playPosY, rc);
		} else {
			res = _texture->displayTransZoom(_playPosX, _playPosY, rc, _playZoom, _playZoom);
		}
	} else {
		res = STATUS_FAILED;
	}

	if (_subtitler && _foundSubtitles && _game->_subtitles) {
		_subtitler->display();
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::play(TVideoPlayback type, int x, int y, bool freezeMusic) {
	if (!_aviDecoder)
		return STATUS_FAILED;

	if (!_playbackStarted && freezeMusic) {
		_game->freeze(freezeMusic);
		_freezeGame = freezeMusic;
	}

	_playbackStarted = false;
	float width, height;
	if (_aviDecoder) {
		if (_subtitler && _foundSubtitles && _game->_subtitles) {
			_subtitler->update(_aviDecoder->getFrameCount());
			_subtitler->display();
		}
		_playPosX = x;
		_playPosY = y;

		width = (float)_aviDecoder->getWidth();
		height = (float)_aviDecoder->getHeight();
	} else {
		width = (float)_game->_renderer->getWidth();
		height = (float)_game->_renderer->getHeight();
	}

	switch (type) {
		case VID_PLAY_POS:
			_playZoom = 100.0f;
			_playPosX = x;
			_playPosY = y;
			break;

		case VID_PLAY_STRETCH: {
			float zoomX = (float)((float)_game->_renderer->getWidth() / width * 100);
			float zoomY = (float)((float)_game->_renderer->getHeight() / height * 100);
			_playZoom = MIN(zoomX, zoomY);
			_playPosX = (int)((_game->_renderer->getWidth() - width * (_playZoom / 100)) / 2);
			_playPosX = (int)((_game->_renderer->getHeight() - height * (_playZoom / 100)) / 2);
		}
			break;

		case VID_PLAY_CENTER:
			_playZoom = 100.0f;
			_playPosX = (int)((_game->_renderer->getWidth() - width) / 2);
			_playPosY = (int)((_game->_renderer->getHeight() - height) / 2);
			break;

		default:
			break;
	}

	if (_aviDecoder)
		_aviDecoder->start();

	_playing = true;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::stop() {
	_aviDecoder->close();

	cleanup();

	if (_freezeGame)
		_game->unfreeze();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::isPlaying() {
	return _playing;
}

} // End of namespace Wintermute
