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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "asylum/views/video.h"

#include "asylum/system/config.h"
#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/savegame.h"
#include "asylum/system/screen.h"
#include "asylum/system/sound.h"
#include "asylum/system/text.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"
#include "asylum/staticres.h"

namespace Asylum {

VideoPlayer::VideoPlayer(AsylumEngine *engine, Audio::Mixer *mixer) : _vm(engine),
	_currentMovie(0), _subtitleIndex(0), _subtitleCounter(0), _previousFont(kResourceNone), _done(false) {
	_smkDecoder = new Video::SmackerDecoder(mixer);
}

VideoPlayer::~VideoPlayer() {
	delete _smkDecoder;

	// Zero-out passed pointers
	_vm = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Event Handler
//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::handleEvent(const AsylumEvent &evt) {
	switch ((int32)evt.type) {
	default:
		break;

	case EVENT_ASYLUM_INIT:
		_previousFont = getText()->loadFont(MAKE_RESOURCE(kResourcePackShared, 57));
		_subtitleCounter = 0;
		_subtitleIndex = -1;
		break;

	case EVENT_ASYLUM_DEINIT:
		getScreen()->clear();
		getText()->loadFont(_previousFont);
		break;

	case EVENT_ASYLUM_SUBTITLE: {
		int32 newIndex = (evt.param2 == 1) ? evt.param1 : -1;

		if (_subtitleIndex != newIndex) {
			_subtitleIndex = newIndex;
			_subtitleCounter = 2;
		}

		if (_subtitleCounter > 0) {
			getScreen()->fillRect(0, 400, 640, 80, 0);

			if (_subtitleIndex >= 0) {
				char *text1 = getText()->get((ResourceId)_currentMovie);

				int16 y = (int16)(10 * (44 - getText()->draw(0, 99, kTextCalculate, Common::Point(10, 400), 20, 620, text1)));
				if (y <= 400)
					y = 405;

				char *text = getText()->get(_subtitles[_subtitleIndex].resourceId);
				getText()->draw(0, 99, kTextCenter, Common::Point(10, y), 20, 620, text);
			}

			--_subtitleCounter;
		}

		return true;
		}

	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_KEYDOWN:
		_done = true;
		getScreen()->clear();

		// Original set a value that does not seems to be used anywhere
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Playing
//////////////////////////////////////////////////////////////////////////
void VideoPlayer::play(uint32 videoNumber, EventHandler *handler) {
	getSaveLoad()->setMovieViewed(videoNumber);
	_currentMovie = videoNumber;

	// Prepare
	getCursor()->hide();
	getSharedData()->setFlag(kFlag1, true);
	getScreen()->paletteFade(0, 25, 10);
	getSound()->stopAll();

	// Play movie
	_vm->switchEventHandler(this);
	play(Common::String::format("mov%03d.smk", videoNumber), Config.showMovieSubtitles);

	// Cleanup and switch to previous event handler
	getCursor()->show();
	getSharedData()->setFlag(kFlag1, false);
	_vm->switchEventHandler(handler);
}

void VideoPlayer::play(Common::String filename, bool showSubtitles) {
	if (!_smkDecoder->loadFile(filename))
		error("[Video::playVideo] Invalid video index (%d)", _currentMovie);

	int32 x = Common::Rational(g_system->getWidth()  - _smkDecoder->getWidth(),  2).toInt();
	int32 y = Common::Rational(g_system->getHeight() - _smkDecoder->getHeight(), 2).toInt();

	getScreen()->clear();

	// TODO check flags and setup volume panning

	// Load subtitles
	if (showSubtitles)
		loadSubtitles();

	// Setup playing
	_done = false;
	uint32 index = 0;
	int32 frameStart = 0;
	int32 frameEnd = 0;
	int32 currentSubtitle = 0;

	while (!_done && !Engine::shouldQuit() && !_smkDecoder->endOfVideo()) {
		_vm->handleEvents();

		if (_smkDecoder->needsUpdate()) {
			const Graphics::Surface *frame = _smkDecoder->decodeNextFrame();

			if (!frame)
				continue;

			if (_smkDecoder->hasDirtyPalette())
				setupPalette();

			getScreen()->copyToBackBuffer((byte *)frame->pixels, frame->pitch, x, y, frame->w, frame->h);

			if (showSubtitles) {
				int32 currentFrame = _smkDecoder->getCurFrame() + 1;

				// Check for next frame
				if (currentFrame > frameEnd) {
					if (index < _subtitles.size()) {
						frameStart = _subtitles[index].frameStart;
						frameEnd = _subtitles[index].frameEnd;
						currentSubtitle = index;
						++index;
					}
				}

				if (currentFrame < frameStart || currentFrame > frameEnd)
					_vm->notify(EVENT_ASYLUM_SUBTITLE, 0, 0);
				else
					_vm->notify(EVENT_ASYLUM_SUBTITLE, currentSubtitle, 1);
			}

			getScreen()->copyBackBufferToScreen();

			g_system->updateScreen();
		}
		g_system->delayMillis(10);
	}

	_smkDecoder->close();
	_subtitles.clear();
}

void VideoPlayer::setupPalette() {
	getScreen()->setMainPalette(_smkDecoder->getPalette());
	getScreen()->setupPalette(NULL, 0, 0);
}

void VideoPlayer::loadSubtitles() {
	char movieToken[10];
	sprintf(movieToken, "[MOV%03d]", _currentMovie);

	Common::File subsFile;
	subsFile.open("vids.cap");
	uint32 fileSize = (uint32)subsFile.size();
	char *buffer = new char[fileSize + 1];
	subsFile.read(buffer, fileSize);
	subsFile.close();
	buffer[fileSize] = 0;

	char *start = strstr(buffer, movieToken);
	char *line = 0;

	if (start) {
		start += 20; // skip token, newline and "CAPTION = "

		uint32 count = strcspn(start, "\r\n");
		line = new char[count + 1];

		strncpy(line, start, count);
		line[count] = 0;

		char *tok = strtok(line, " ");

		while (tok) {
			VideoSubtitle newSubtitle;
			newSubtitle.frameStart = atoi(tok);

			tok = strtok(NULL, " ");
			if (!tok)
				error("[Video::loadSubtitles] Invalid subtitle (frame end missing)!");

			newSubtitle.frameEnd = atoi(tok);

			tok = strtok(NULL, " ");
			if (!tok)
				error("[Video::loadSubtitles] Invalid subtitle (resource id missing)!");

			newSubtitle.resourceId = (ResourceId)(atoi(tok) + video_subtitle_resourceIds[_currentMovie]);

			tok = strtok(NULL, " ");

			_subtitles.push_back(newSubtitle);
		}

		delete [] line;
	}

	delete [] buffer;
}

} // end of namespace Asylum
