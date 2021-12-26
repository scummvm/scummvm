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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/util.h"

#include "video/avi_decoder.h"
#include "video/smk_decoder.h"
#include "video/theora_decoder.h"

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

	memset(_subtitlePalette, 0, sizeof(_subtitlePalette));
	if (_vm->checkGameVersion("Steam")) {
#ifdef USE_THEORADEC
		_decoder = new Video::TheoraDecoder();

		Common::File paletteFile;
		paletteFile.open("palette");
		paletteFile.read(_subtitlePalette, PALETTE_SIZE);
		paletteFile.close();
#else
		error("The Steam version of the game uses Theora videos but ScummVM has been compiled without Theora support");
#endif
	} else if (_vm->isAltDemo()) {
		_decoder = new Video::AVIDecoder();
	} else {
		_decoder = new Video::SmackerDecoder();
	}
}

VideoPlayer::~VideoPlayer() {
	delete _decoder;
}

//////////////////////////////////////////////////////////////////////////
// Event Handler
//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::handleEvent(const AsylumEvent &evt) {
	switch ((int32)evt.type) {
	default:
		break;

	case EVENT_ASYLUM_INIT:
		if (!_vm->checkGameVersion("Demo"))
			_previousFont = getText()->loadFont(MAKE_RESOURCE(kResourcePackShared, 57));
		_subtitleCounter = 0;
		_subtitleIndex = -1;
		break;

	case EVENT_ASYLUM_DEINIT:
		getScreen()->clear();
		if (!_vm->checkGameVersion("Demo"))
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
				char *text = getText()->get(_subtitles[_subtitleIndex].resourceId);

				int16 y = (int16)(10 * (44 - getText()->draw(0, 99, kTextCalculate, Common::Point(10, 400), 20, 620, text)));
				if (y <= 400)
					y = 405;

				getText()->draw(0, 99, kTextCenter, Common::Point(10, y), 20, 620, text);

				if (_vm->checkGameVersion("Steam")) {
					Graphics::Surface *st = getScreen()->getSurface().convertTo(g_system->getScreenFormat(), _subtitlePalette);
					g_system->copyRectToScreen((const byte *)st->getBasePtr(0, 400), st->pitch, 0, 400, 640, 80);
					st->free();
					delete st;
				}
			}

			--_subtitleCounter;
		}

		return true;
		}

	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_KEYDOWN:
		_done = true;
		if (!_vm->checkGameVersion("Steam") && !_vm->isAltDemo())
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

	Common::String filename;
	if (_vm->checkGameVersion("Steam"))
		filename = videoNumber == 0 ? "mov000_2_smk.ogv" : Common::String::format("mov%03d_smk.ogv", videoNumber);
	else if (_vm->isAltDemo())
		filename = Common::String::format("mov%03d.avi", videoNumber);
	else
		filename = Common::String::format("mov%03d.smk", videoNumber);
	play(filename, Config.showMovieSubtitles);

	// Cleanup and switch to previous event handler
	getCursor()->show();
	getSharedData()->setFlag(kFlag1, false);
	_vm->switchEventHandler(handler);
}

void VideoPlayer::play(const Common::String &filename, bool showSubtitles) {
	if (!_decoder->loadFile(filename))
		error("[Video::playVideo] Invalid video index (%d)", _currentMovie);

	int16 x = (int16)Common::Rational(g_system->getWidth()  - _decoder->getWidth(),  2).toInt();
	int16 y = (int16)Common::Rational(g_system->getHeight() - _decoder->getHeight(), 2).toInt();

	getScreen()->clear();

	// TODO check flags and setup volume panning

	// Load subtitles
	if (showSubtitles && !_vm->checkGameVersion("Demo"))
		loadSubtitles();

	// Setup playing
	_done = false;
	uint32 index = 0;
	int32 frameStart = 0;
	int32 frameEnd = 0;
	int32 currentSubtitle = 0;

	_decoder->start();

	if (_vm->checkGameVersion("Steam") || _vm->isAltDemo()) {
		Graphics::PixelFormat decoderFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
		_decoder->setDefaultHighColorFormat(decoderFormat);
		initGraphics(640, 480, &decoderFormat);
	}

	while (!_done && !Engine::shouldQuit() && !_decoder->endOfVideo()) {
		_vm->handleEvents();

		if (_decoder->needsUpdate()) {
			const Graphics::Surface *frame = _decoder->decodeNextFrame();

			if (!frame)
				continue;

			if (_vm->checkGameVersion("Steam") || _vm->isAltDemo()) {
				g_system->copyRectToScreen((const byte *)frame->getPixels(), frame->pitch, x, y, frame->w, frame->h);
			} else {
				if (_decoder->hasDirtyPalette())
					setupPalette();
				getScreen()->copyToBackBuffer((const byte *)frame->getPixels(), frame->pitch, x, y, frame->w, frame->h);
			}

			if (showSubtitles) {
				int32 currentFrame = _decoder->getCurFrame() + 1;
				debugC(kDebugLevelVideo, "[Video] {%s} Playing Frame %d", filename.c_str(), currentFrame);
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

			if (!_vm->checkGameVersion("Steam") && !_vm->isAltDemo())
				getScreen()->copyBackBufferToScreen();

			g_system->updateScreen();
		}

		if (!_vm->checkGameVersion("Steam") && !_vm->isAltDemo())
			g_system->delayMillis(10);
	}

	if (_vm->checkGameVersion("Steam") || _vm->isAltDemo())
		initGraphics(640, 480);

	_decoder->close();
	_subtitles.clear();
}

void VideoPlayer::setupPalette() {
	getScreen()->setMainPalette(_decoder->getPalette());
	getScreen()->setupPalette(nullptr, 0, 0);
}

void VideoPlayer::loadSubtitles() {
	char movieToken[10];
	snprintf(movieToken, 10, "[MOV%03d]", _currentMovie);

	Common::File subsFile;
	subsFile.open("vids.cap");
	uint32 fileSize = (uint32)subsFile.size();
	char *buffer = new char[fileSize + 1];
	subsFile.read(buffer, fileSize);
	subsFile.close();
	buffer[fileSize] = 0;

	char *start = strstr(buffer, movieToken);
	char *line = nullptr;

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

			tok = strtok(nullptr, " ");
			if (!tok)
				error("[Video::loadSubtitles] Invalid subtitle (frame end missing)!");

			newSubtitle.frameEnd = atoi(tok);

			tok = strtok(nullptr, " ");
			if (!tok)
				error("[Video::loadSubtitles] Invalid subtitle (resource id missing)!");

			int index = atoi(tok);

			// Original bug: index starts from 1 instead of 0
			if (_currentMovie == 36)
				index--;

			newSubtitle.resourceId = (ResourceId)(index + video_subtitle_resourceIds[_currentMovie]);

			tok = strtok(nullptr, " ");

			_subtitles.push_back(newSubtitle);
		}

		delete [] line;
	}

	delete [] buffer;
}

} // end of namespace Asylum
